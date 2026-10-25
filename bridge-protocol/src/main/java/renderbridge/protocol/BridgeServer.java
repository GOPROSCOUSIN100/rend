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
 * Server-side bridge connection manager.
 * Handles incoming connections, handshake, heartbeat, and message receiving.
 */
public class BridgeServer {

    // Server state
    public enum ServerState {
        STOPPED,
        STARTING,
        RUNNING,
        STOPPING,
        ERROR
    }

    // Message handler interface
    public interface MessageHandler {
        void onMessageReceived(Message message);
        void onClientConnected(String clientName);
        void onClientDisconnected(String clientName);
        void onServerStateChanged(ServerState state);
        void onError(TransportException exception);
    }

    // Default message handler
    private static final MessageHandler DEFAULT_HANDLER = new MessageHandler() {
        @Override
        public void onMessageReceived(Message message) {
            // Default: do nothing
        }

        @Override
        public void onClientConnected(String clientName) {
            // Default: do nothing
        }

        @Override
        public void onClientDisconnected(String clientName) {
            // Default: do nothing
        }

        @Override
        public void onServerStateChanged(ServerState state) {
            // Default: do nothing
        }

        @Override
        public void onError(TransportException exception) {
            // Default: print error
            System.err.println("Bridge server error: " + exception.getMessage());
        }
    };

    // Server settings
    private final String name;
    private final boolean useSharedMemory;
    private final boolean useSocket;
    
    // Transport
    private TransportChannel transportChannel;
    
    // State
    private volatile ServerState state = ServerState.STOPPED;
    private final AtomicBoolean running = new AtomicBoolean(false);
    
    // Thread management
    private ExecutorService executorService;
    private Thread acceptThread;
    private Thread receiveThread;
    private Thread heartbeatThread;
    
    // Counters
    private final AtomicLong lastReceivedTime = new AtomicLong(0);
    private final AtomicLong lastSentTime = new AtomicLong(0);
    private final AtomicLong sequenceCounter = new AtomicLong(0);
    private final AtomicLong lastFrameNumber = new AtomicLong(ProtocolConstants.INVALID_FRAME_NUMBER);
    
    // Client info
    private volatile String clientName = "";
    private volatile int clientVersion = 0;
    private volatile int clientCapabilities = 0;
    
    // Configuration
    private long heartbeatIntervalMs = ProtocolConstants.HEARTBEAT_INTERVAL_MS;
    private long heartbeatTimeoutMs = ProtocolConstants.HEARTBEAT_TIMEOUT_MS;
    
    // Handler
    private MessageHandler messageHandler = DEFAULT_HANDLER;

    /**
     * Create a new bridge server with default settings.
     */
    public BridgeServer() {
        this("BridgeServer", true, true);
    }

    /**
     * Create a new bridge server.
     *
     * @param name the server name
     * @param useSharedMemory true to accept shared memory connections
     * @param useSocket true to accept socket connections
     */
    public BridgeServer(String name, boolean useSharedMemory, boolean useSocket) {
        this.name = name;
        this.useSharedMemory = useSharedMemory;
        this.useSocket = useSocket;
    }

    /**
     * Start the bridge server.
     *
     * @throws TransportException if server fails to start
     * @throws IOException if an I/O error occurs
     */
    public void start() throws TransportException, IOException {
        if (state == ServerState.RUNNING || state == ServerState.STARTING) {
            return;
        }

        setState(ServerState.STARTING);

        try {
            // Create transport channel
            if (useSharedMemory && useSocket) {
                transportChannel = TransportFactory.createChannel(false);
            } else if (useSharedMemory) {
                transportChannel = TransportFactory.createSharedMemoryChannel(false);
            } else if (useSocket) {
                transportChannel = TransportFactory.createLocalSocketChannel(false);
            } else {
                throw new TransportException("No transport method available");
            }

            transportChannel.connect();

            // Start threads
            startThreads();

            setState(ServerState.RUNNING);
            running.set(true);
        } catch (TransportException | IOException e) {
            setState(ServerState.ERROR);
            throw e;
        }
    }

    /**
     * Stop the bridge server.
     */
    public void stop() {
        if (state == ServerState.STOPPED || state == ServerState.STOPPING) {
            return;
        }

        running.set(false);
        setState(ServerState.STOPPING);

        // Stop threads
        stopThreads();

        // Disconnect transport
        if (transportChannel != null) {
            transportChannel.disconnect();
            transportChannel = null;
        }

        // Reset client info
        clientName = "";
        clientVersion = 0;
        clientCapabilities = 0;
        
        setState(ServerState.STOPPED);
    }

    /**
     * Start the receive and heartbeat threads.
     */
    private void startThreads() {
        running.set(true);
        
        // Create thread pool
        executorService = Executors.newCachedThreadPool(r -> {
            Thread thread = new Thread(r, "BridgeServer-Thread");
            thread.setDaemon(true);
            return thread;
        });

        // Start receive thread
        receiveThread = new Thread(this::receiveLoop, "BridgeServer-Receive");
        receiveThread.setDaemon(true);
        receiveThread.start();

        // Start heartbeat thread
        heartbeatThread = new Thread(this::heartbeatLoop, "BridgeServer-Heartbeat");
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
        while (running.get() && state == ServerState.RUNNING) {
            try {
                Message message = transportChannel.receive();
                
                if (message != null) {
                    lastReceivedTime.set(System.currentTimeMillis());
                    
                    // Handle special messages
                    if (message.getMessageId() == MessageType.HANDSHAKE) {
                        handleHandshake((HandshakeMessage) message);
                    } else if (message.getMessageId() == MessageType.HEARTBEAT) {
                        handleHeartbeat((HeartbeatMessage) message);
                    } else if (message.getMessageId() == MessageType.ACK) {
                        handleAck((AckMessage) message);
                    } else if (message.getMessageId() == MessageType.ECHO) {
                        handleEcho((EchoMessage) message);
                    } else {
                        // Pass to message handler
                        messageHandler.onMessageReceived(message);
                    }
                }
            } catch (Exception e) {
                messageHandler.onError(new TransportException("Receive error", e));
                stop();
            }
        }
    }

    /**
     * The heartbeat loop that sends periodic heartbeat messages.
     */
    private void heartbeatLoop() {
        while (running.get() && state == ServerState.RUNNING) {
            try {
                // Send heartbeat
                sendHeartbeat();
                
                // Check for timeout
                if (hasTimedOut()) {
                    messageHandler.onError(new TransportException("Connection timeout"));
                    stop();
                    return;
                }
                
                // Sleep for heartbeat interval
                Thread.sleep(heartbeatIntervalMs);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                return;
            } catch (Exception e) {
                messageHandler.onError(new TransportException("Heartbeat error", e));
                stop();
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
     * Handle a handshake message from the client.
     *
     * @param handshake the handshake message
     */
    private void handleHandshake(HandshakeMessage handshake) {
        // Store client info
        clientName = handshake.getClientName();
        clientVersion = handshake.getClientVersion();
        clientCapabilities = handshake.getCapabilities();
        
        // Check protocol version
        if (handshake.getProtocolVersion() != ProtocolConstants.PROTOCOL_VERSION) {
            // Send error response
            ErrorMessage error = new ErrorMessage(ErrorMessage.ERR_PROTOCOL_VERSION, 
                    "Protocol version mismatch: server=" + ProtocolConstants.PROTOCOL_VERSION + 
                    ", client=" + handshake.getProtocolVersion());
            try {
                transportChannel.send(error);
            } catch (Exception e) {
                messageHandler.onError(new TransportException("Failed to send error response", e));
            }
            return;
        }
        
        // Send handshake response
        HandshakeMessage response = new HandshakeMessage();
        response.setProtocolVersion(ProtocolConstants.PROTOCOL_VERSION);
        
        // Set server capabilities
        int serverCapabilities = 0;
        if (useSharedMemory) {
            serverCapabilities |= HandshakeMessage.CAPABILITY_SHARED_MEMORY;
        }
        if (useSocket) {
            serverCapabilities |= HandshakeMessage.CAPABILITY_SOCKET;
        }
        response.setCapabilities(serverCapabilities);
        
        response.setClientType(HandshakeMessage.CLIENT_TYPE_RENDERER);
        response.setClientVersion(0x00010000); // Version 1.0.0
        response.setClientName(name);
        
        try {
            transportChannel.send(response);
            messageHandler.onClientConnected(clientName);
        } catch (Exception e) {
            messageHandler.onError(new TransportException("Failed to send handshake response", e));
        }
    }

    /**
     * Handle a heartbeat message from the client.
     *
     * @param heartbeat the heartbeat message
     */
    private void handleHeartbeat(HeartbeatMessage heartbeat) {
        // Update last received time
        lastReceivedTime.set(System.currentTimeMillis());
        lastFrameNumber.set(heartbeat.getLastFrameProcessed());
        
        // TODO: Process heartbeat data
    }

    /**
     * Handle an acknowledgment message from the client.
     *
     * @param ack the acknowledgment message
     */
    private void handleAck(AckMessage ack) {
        // TODO: Handle acknowledgment
    }

    /**
     * Handle an echo message from the client.
     *
     * @param echo the echo message
     */
    private void handleEcho(EchoMessage echo) {
        // Echo back the same message
        try {
            transportChannel.send(echo);
        } catch (Exception e) {
            messageHandler.onError(new TransportException("Failed to send echo response", e));
        }
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
     * Send a message through the bridge.
     *
     * @param message the message to send
     * @throws TransportException if sending fails
     * @throws IOException if an I/O error occurs
     */
    public void send(Message message) throws TransportException, IOException {
        if (state != ServerState.RUNNING) {
            throw new TransportException("Server is not running");
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
     * Set the message handler.
     *
     * @param handler the message handler
     */
    public void setMessageHandler(MessageHandler handler) {
        this.messageHandler = handler != null ? handler : DEFAULT_HANDLER;
    }

    /**
     * Get the current server state.
     *
     * @return the server state
     */
    public ServerState getState() {
        return state;
    }

    /**
     * Check if the server is running.
     *
     * @return true if running
     */
    public boolean isRunning() {
        return state == ServerState.RUNNING;
    }

    /**
     * Get the transport channel.
     *
     * @return the transport channel, or null if not started
     */
    public TransportChannel getTransportChannel() {
        return transportChannel;
    }

    /**
     * Get the client name.
     *
     * @return the client name
     */
    public String getClientName() {
        return clientName;
    }

    /**
     * Get the client version.
     *
     * @return the client version
     */
    public int getClientVersion() {
        return clientVersion;
    }

    /**
     * Get the client capabilities.
     *
     * @return the client capabilities
     */
    public int getClientCapabilities() {
        return clientCapabilities;
    }

    /**
     * Set the server state and notify the handler.
     *
     * @param state the new server state
     */
    private void setState(ServerState state) {
        this.state = state;
        messageHandler.onServerStateChanged(state);
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

    public String getName() {
        return name;
    }

    @Override
    public String toString() {
        return String.format(
                "BridgeServer{name='%s', state=%s, client='%s', transport=%s}",
                name, state, clientName, transportChannel != null ? transportChannel.getType() : "null"
        );
    }
}
