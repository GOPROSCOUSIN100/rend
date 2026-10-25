package renderbridge.protocol;

import renderbridge.protocol.messages.AckMessage;
import renderbridge.protocol.messages.EchoMessage;
import renderbridge.protocol.messages.ErrorMessage;
import renderbridge.protocol.messages.HandshakeMessage;
import renderbridge.protocol.messages.HeartbeatMessage;
import renderbridge.protocol.transport.TransportChannel;
import renderbridge.protocol.transport.TransportException;
import renderbridge.protocol.transport.TransportFactory;

import java.io.IOException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Client-side bridge connection manager.
 * Handles connection, handshake, heartbeat, and message sending/receiving.
 */
public class BridgeClient {

    // Connection state
    public enum ConnectionState {
        DISCONNECTED,
        CONNECTING,
        HANDSHAKING,
        CONNECTED,
        ERROR
    }

    // Message handler interface
    public interface MessageHandler {
        void onMessageReceived(Message message);
        void onConnectionStateChanged(ConnectionState state);
        void onError(TransportException exception);
    }

    // Default message handler
    private static final MessageHandler DEFAULT_HANDLER = new MessageHandler() {
        @Override
        public void onMessageReceived(Message message) {
            // Default: do nothing
        }

        @Override
        public void onConnectionStateChanged(ConnectionState state) {
            // Default: do nothing
        }

        @Override
        public void onError(TransportException exception) {
            // Default: print error
            System.err.println("Bridge error: " + exception.getMessage());
        }
    };

    // Connection settings
    private final String name;
    private final boolean useSharedMemory;
    private final boolean useSocket;
    
    // Transport
    private TransportChannel transportChannel;
    
    // State
    private volatile ConnectionState state = ConnectionState.DISCONNECTED;
    private final AtomicBoolean running = new AtomicBoolean(false);
    private final AtomicBoolean connected = new AtomicBoolean(false);
    
    // Thread management
    private ExecutorService executorService;
    private Thread receiveThread;
    private Thread heartbeatThread;
    
    // Counters
    private final AtomicLong lastReceivedTime = new AtomicLong(0);
    private final AtomicLong lastSentTime = new AtomicLong(0);
    private final AtomicLong sequenceCounter = new AtomicLong(0);
    private final AtomicLong lastFrameNumber = new AtomicLong(ProtocolConstants.INVALID_FRAME_NUMBER);
    
    // Configuration
    private long heartbeatIntervalMs = ProtocolConstants.HEARTBEAT_INTERVAL_MS;
    private long heartbeatTimeoutMs = ProtocolConstants.HEARTBEAT_TIMEOUT_MS;
    private int maxRetries = ProtocolConstants.MAX_RETRIES;
    private long retryDelayMs = ProtocolConstants.RETRY_DELAY_MS;
    
    // Handler
    private MessageHandler messageHandler = DEFAULT_HANDLER;

    /**
     * Create a new bridge client with default settings.
     */
    public BridgeClient() {
        this("BridgeClient", true, true);
    }

    /**
     * Create a new bridge client.
     *
     * @param name the client name
     * @param useSharedMemory true to try shared memory transport
     * @param useSocket true to try socket transport
     */
    public BridgeClient(String name, boolean useSharedMemory, boolean useSocket) {
        this.name = name;
        this.useSharedMemory = useSharedMemory;
        this.useSocket = useSocket;
    }

    /**
     * Connect to the bridge server.
     *
     * @throws TransportException if connection fails
     * @throws IOException if an I/O error occurs
     */
    public void connect() throws TransportException, IOException {
        if (state == ConnectionState.CONNECTED || state == ConnectionState.CONNECTING) {
            return;
        }

        setState(ConnectionState.CONNECTING);

        try {
            // Create transport channel
            if (useSharedMemory && useSocket) {
                transportChannel = TransportFactory.createChannel(true);
            } else if (useSharedMemory) {
                transportChannel = TransportFactory.createSharedMemoryChannel(true);
            } else if (useSocket) {
                transportChannel = TransportFactory.createLocalSocketChannel(true);
            } else {
                throw new TransportException("No transport method available");
            }

            transportChannel.connect();

            // Perform handshake
            performHandshake();

            // Start threads
            startThreads();

            setState(ConnectionState.CONNECTED);
            connected.set(true);
        } catch (TransportException | IOException e) {
            setState(ConnectionState.ERROR);
            throw e;
        }
    }

    /**
     * Disconnect from the bridge server.
     */
    public void disconnect() {
        if (state == ConnectionState.DISCONNECTED) {
            return;
        }

        running.set(false);
        connected.set(false);

        // Stop threads
        stopThreads();

        // Disconnect transport
        if (transportChannel != null) {
            transportChannel.disconnect();
            transportChannel = null;
        }

        setState(ConnectionState.DISCONNECTED);
    }

    /**
     * Perform the handshake with the server.
     *
     * @throws TransportException if handshake fails
     * @throws IOException if an I/O error occurs
     */
    private void performHandshake() throws TransportException, IOException {
        setState(ConnectionState.HANDSHAKING);

        // Create and send handshake message
        HandshakeMessage handshake = new HandshakeMessage();
        handshake.setProtocolVersion(ProtocolConstants.PROTOCOL_VERSION);
        
        // Set capabilities based on configuration
        int capabilities = 0;
        if (useSharedMemory) {
            capabilities |= HandshakeMessage.CAPABILITY_SHARED_MEMORY;
        }
        if (useSocket) {
            capabilities |= HandshakeMessage.CAPABILITY_SOCKET;
        }
        handshake.setCapabilities(capabilities);
        
        handshake.setClientType(HandshakeMessage.CLIENT_TYPE_MOD);
        handshake.setClientVersion(0x00010000); // Version 1.0.0
        handshake.setClientName(name);

        transportChannel.send(handshake);

        // Wait for handshake response
        long timeout = System.currentTimeMillis() + ProtocolConstants.DEFAULT_CONNECT_TIMEOUT_MS;
        while (System.currentTimeMillis() < timeout && running.get()) {
            Message response = transportChannel.receive();
            
            if (response != null) {
                if (response.getMessageId() == MessageType.HANDSHAKE) {
                    HandshakeMessage handshakeResponse = (HandshakeMessage) response;
                    
                    // Check protocol version
                    if (handshakeResponse.getProtocolVersion() != ProtocolConstants.PROTOCOL_VERSION) {
                        throw new TransportException("Protocol version mismatch: " + 
                                "client=" + ProtocolConstants.PROTOCOL_VERSION + 
                                ", server=" + handshakeResponse.getProtocolVersion());
                    }
                    
                    // Check if server is compatible
                    if (!handshakeResponse.isRendererClient()) {
                        throw new TransportException("Server is not a renderer client");
                    }
                    
                    return; // Handshake successful
                } else if (response.getMessageId() == MessageType.ERROR) {
                    ErrorMessage error = (ErrorMessage) response;
                    throw new TransportException("Handshake failed: " + error.getErrorMessage(), error.getErrorCode());
                }
            }
            
            // Small sleep to prevent busy waiting
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                throw new TransportException("Handshake interrupted", e);
            }
        }

        throw new TransportException("Handshake timeout");
    }

    /**
     * Start the receive and heartbeat threads.
     */
    private void startThreads() {
        running.set(true);
        
        // Create thread pool
        executorService = Executors.newCachedThreadPool(r -> {
            Thread thread = new Thread(r, "BridgeClient-Thread");
            thread.setDaemon(true);
            return thread;
        });

        // Start receive thread
        receiveThread = new Thread(this::receiveLoop, "BridgeClient-Receive");
        receiveThread.setDaemon(true);
        receiveThread.start();

        // Start heartbeat thread
        heartbeatThread = new Thread(this::heartbeatLoop, "BridgeClient-Heartbeat");
        heartbeatThread.setDaemon(true);
        heartbeatThread.start();
    }

    /**
     * Stop all threads.
     */
    private void stopThreads() {
        running.set(false);

        // Interrupt threads
        if (receiveThread != null) {
            receiveThread.interrupt();
            try {
                receiveThread.join(1000);
            } catch (InterruptedException e) {
                // Ignore
            }
            receiveThread = null;
        }

        if (heartbeatThread != null) {
            heartbeatThread.interrupt();
            try {
                heartbeatThread.join(1000);
            } catch (InterruptedException e) {
                // Ignore
            }
            heartbeatThread = null;
        }

        // Shutdown executor
        if (executorService != null) {
            executorService.shutdown();
            try {
                executorService.awaitTermination(1000, TimeUnit.MILLISECONDS);
            } catch (InterruptedException e) {
                // Ignore
            }
            executorService = null;
        }
    }

    /**
     * The receive loop that continuously reads messages from the transport.
     */
    private void receiveLoop() {
        while (running.get() && connected.get()) {
            try {
                Message message = transportChannel.receive();
                
                if (message != null) {
                    lastReceivedTime.set(System.currentTimeMillis());
                    
                    // Handle special messages
                    if (message.getMessageId() == MessageType.HEARTBEAT) {
                        HeartbeatMessage heartbeat = (HeartbeatMessage) message;
                        handleHeartbeat(heartbeat);
                    } else if (message.getMessageId() == MessageType.ACK) {
                        AckMessage ack = (AckMessage) message;
                        handleAck(ack);
                    } else if (message.getMessageId() == MessageType.ERROR) {
                        ErrorMessage error = (ErrorMessage) message;
                        handleError(error);
                    } else {
                        // Pass to message handler
                        messageHandler.onMessageReceived(message);
                    }
                }
            } catch (Exception e) {
                messageHandler.onError(new TransportException("Receive error", e));
                disconnect();
            }
        }
    }

    /**
     * The heartbeat loop that sends periodic heartbeat messages.
     */
    private void heartbeatLoop() {
        while (running.get() && connected.get()) {
            try {
                // Send heartbeat
                sendHeartbeat();
                
                // Check for timeout
                if (hasTimedOut()) {
                    messageHandler.onError(new TransportException("Connection timeout"));
                    disconnect();
                    return;
                }
                
                // Sleep for heartbeat interval
                Thread.sleep(heartbeatIntervalMs);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                return;
            } catch (Exception e) {
                messageHandler.onError(new TransportException("Heartbeat error", e));
                disconnect();
                return;
            }
        }
    }

    /**
     * Check if the connection has timed out.
     *
     * @return true if the connection has timed out
     */
    private boolean hasTimedOut() {
        long lastReceived = lastReceivedTime.get();
        if (lastReceived == 0) {
            return false;
        }
        
        long elapsed = System.currentTimeMillis() - lastReceived;
        return elapsed > heartbeatTimeoutMs;
    }

    /**
     * Send a heartbeat message.
     *
     * @throws TransportException if sending fails
     * @throws IOException if an I/O error occurs
     */
    private void sendHeartbeat() throws TransportException, IOException {
        HeartbeatMessage heartbeat = new HeartbeatMessage();
        heartbeat.setSequenceNumber((int) sequenceCounter.incrementAndGet());
        heartbeat.setLastFrameProcessed(lastFrameNumber.get());
        heartbeat.setAvgProcessingTimeMs(0); // TODO: Calculate actual processing time

        transportChannel.send(heartbeat);
        lastSentTime.set(System.currentTimeMillis());
    }

    /**
     * Handle a received heartbeat message.
     *
     * @param heartbeat the heartbeat message
     */
    private void handleHeartbeat(HeartbeatMessage heartbeat) {
        // Update last received time
        lastReceivedTime.set(System.currentTimeMillis());
        
        // TODO: Process heartbeat data
    }

    /**
     * Handle a received acknowledgment message.
     *
     * @param ack the acknowledgment message
     */
    private void handleAck(AckMessage ack) {
        // TODO: Handle acknowledgment
    }

    /**
     * Handle a received error message.
     *
     * @param error the error message
     */
    private void handleError(ErrorMessage error) {
        messageHandler.onError(new TransportException(error.getErrorMessage(), error.getErrorCode()));
    }

    /**
     * Send a message through the bridge.
     *
     * @param message the message to send
     * @throws TransportException if sending fails
     * @throws IOException if an I/O error occurs
     */
    public void send(Message message) throws TransportException, IOException {
        if (!connected.get()) {
            throw new TransportException("Not connected");
        }

        // Set sequence number
        if (message.getHeader() != null) {
            message.getHeader().setSequence((int) sequenceCounter.incrementAndGet());
        }

        // Send the message
        transportChannel.send(message);
        lastSentTime.set(System.currentTimeMillis());
    }

    /**
     * Send an echo message for roundtrip testing.
     *
     * @return the roundtrip time in nanoseconds
     * @throws TransportException if echo test fails
     * @throws IOException if an I/O error occurs
     */
    public long sendEcho() throws TransportException, IOException {
        if (!connected.get()) {
            throw new TransportException("Not connected");
        }

        long timestamp = System.nanoTime();
        int sequence = (int) sequenceCounter.incrementAndGet();
        
        EchoMessage echo = new EchoMessage(timestamp, sequence);
        echo.setData("Echo test");
        
        transportChannel.send(echo);
        
        // Wait for echo response
        long timeout = System.currentTimeMillis() + ProtocolConstants.DEFAULT_READ_TIMEOUT_MS;
        while (System.currentTimeMillis() < timeout && running.get()) {
            Message response = transportChannel.receive();
            
            if (response != null && response.getMessageId() == MessageType.ECHO) {
                EchoMessage echoResponse = (EchoMessage) response;
                return echoResponse.calculateRoundtripTime(System.nanoTime());
            }
            
            try {
                Thread.sleep(1);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                throw new TransportException("Echo test interrupted", e);
            }
        }

        throw new TransportException("Echo test timeout");
    }

    /**
     * Set the message handler.
     *
     * @param handler the message handler
     */
    public void setMessageHandler(MessageHandler handler) {
        this.messageHandler = handler != null ? handler : DEFAULT_HANDLER;
    }

    /**
     * Get the current connection state.
     *
     * @return the connection state
     */
    public ConnectionState getState() {
        return state;
    }

    /**
     * Check if the client is connected.
     *
     * @return true if connected
     */
    public boolean isConnected() {
        return connected.get();
    }

    /**
     * Get the transport channel.
     *
     * @return the transport channel, or null if not connected
     */
    public TransportChannel getTransportChannel() {
        return transportChannel;
    }

    /**
     * Set the connection state and notify the handler.
     *
     * @param state the new connection state
     */
    private void setState(ConnectionState state) {
        this.state = state;
        messageHandler.onConnectionStateChanged(state);
    }

    // Configuration getters and setters

    public long getHeartbeatIntervalMs() {
        return heartbeatIntervalMs;
    }

    public void setHeartbeatIntervalMs(long heartbeatIntervalMs) {
        this.heartbeatIntervalMs = heartbeatIntervalMs;
    }

    public long getHeartbeatTimeoutMs() {
        return heartbeatTimeoutMs;
    }

    public void setHeartbeatTimeoutMs(long heartbeatTimeoutMs) {
        this.heartbeatTimeoutMs = heartbeatTimeoutMs;
    }

    public int getMaxRetries() {
        return maxRetries;
    }

    public void setMaxRetries(int maxRetries) {
        this.maxRetries = maxRetries;
    }

    public long getRetryDelayMs() {
        return retryDelayMs;
    }

    public void setRetryDelayMs(long retryDelayMs) {
        this.retryDelayMs = retryDelayMs;
    }

    public String getName() {
        return name;
    }

    @Override
    public String toString() {
        return String.format(
                "BridgeClient{name='%s', state=%s, connected=%s, transport=%s}",
                name, state, connected.get(), transportChannel != null ? transportChannel.getType() : "null"
        );
    }
}
