package renderbridge.renderer.bridge;

import renderbridge.protocol.BridgeServer;
import renderbridge.protocol.Message;
import renderbridge.protocol.messages.CameraMessage;
import renderbridge.protocol.messages.ChunkMessage;
import renderbridge.protocol.messages.EntityMessage;
import renderbridge.protocol.messages.FrameEndMessage;
import renderbridge.protocol.messages.FrameStartMessage;
import renderbridge.protocol.messages.GuiMessage;
import renderbridge.protocol.messages.ParticleMessage;
import renderbridge.protocol.messages.WeatherMessage;
import renderbridge.protocol.transport.TransportException;

import java.io.IOException;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.List;

/**
 * Bridge client for the external renderer.
 * Manages the connection to the Minecraft mod and processes incoming messages.
 */
public class RendererBridgeClient {

    // Message handler interface
    public interface MessageHandler {
        void onMessageReceived(Message message);
        void onConnectionStateChanged(BridgeServer.ServerState state);
        void onClientConnected(String clientName);
        void onClientDisconnected(String clientName);
        void onError(TransportException exception);
    }

    // Default message handler
    private static final MessageHandler DEFAULT_HANDLER = new MessageHandler() {
        @Override
        public void onMessageReceived(Message message) {
            // Default: do nothing
        }

        @Override
        public void onConnectionStateChanged(BridgeServer.ServerState state) {
            System.out.println("Connection state changed to: " + state);
        }

        @Override
        public void onClientConnected(String clientName) {
            System.out.println("Client connected: " + clientName);
        }

        @Override
        public void onClientDisconnected(String clientName) {
            System.out.println("Client disconnected: " + clientName);
        }

        @Override
        public void onError(TransportException exception) {
            System.err.println("Bridge error: " + exception.getMessage());
        }
    };

    // Bridge server
    private BridgeServer bridgeServer;
    
    // Message queue
    private final List<Message> messageQueue = new CopyOnWriteArrayList<>();
    
    // State
    private volatile boolean running = false;
    private volatile boolean connected = false;
    
    // Statistics
    private int framesReceived = 0;
    private int messagesReceived = 0;
    private int bytesReceived = 0;
    private int errors = 0;
    
    // Message handler
    private MessageHandler messageHandler = DEFAULT_HANDLER;
    
    // Scene data
    private CameraMessage currentCamera;
    private final List<ChunkMessage> currentChunks = new CopyOnWriteArrayList<>();
    private final List<EntityMessage> currentEntities = new CopyOnWriteArrayList<>();
    private final List<ParticleMessage> currentParticles = new CopyOnWriteArrayList<>();
    private WeatherMessage currentWeather;
    private GuiMessage currentGui;
    
    // Frame data
    private long lastFrameNumber = -1;
    private long lastFrameTime = 0;
    private float frameProcessingTime = 0;

    /**
     * Create a new renderer bridge client.
     */
    public RendererBridgeClient() {
        bridgeServer = new BridgeServer("RendererBridge", true, true);
        bridgeServer.setMessageHandler(new ServerMessageHandler());
    }

    /**
     * Connect to the bridge server.
     *
     * @throws TransportException if connection fails
     * @throws IOException if an I/O error occurs
     */
    public void connect() throws TransportException, IOException {
        if (running) {
            return;
        }
        
        running = true;
        bridgeServer.start();
        connected = true;
        
        System.out.println("Bridge server started");
    }

    /**
     * Disconnect from the bridge server.
     */
    public void disconnect() {
        if (!running) {
            return;
        }
        
        running = false;
        bridgeServer.stop();
        connected = false;
        
        System.out.println("Bridge server stopped");
    }

    /**
     * Update the bridge client.
     */
    public void update() {
        if (!running) {
            return;
        }
        
        // Process queued messages
        processMessageQueue();
    }

    /**
     * Process messages from the queue.
     */
    private void processMessageQueue() {
        for (Message message : messageQueue) {
            processMessage(message);
        }
        messageQueue.clear();
    }

    /**
     * Process a message from the mod.
     *
     * @param message the message to process
     */
    private void processMessage(Message message) {
        try {
            switch (message.getMessageId()) {
                case MessageType.FRAME_START:
                    handleFrameStart((FrameStartMessage) message);
                    break;
                case MessageType.FRAME_END:
                    handleFrameEnd((FrameEndMessage) message);
                    break;
                case MessageType.CAMERA:
                    handleCamera((CameraMessage) message);
                    break;
                case MessageType.CHUNK:
                    handleChunk((ChunkMessage) message);
                    break;
                case MessageType.ENTITY:
                    handleEntity((EntityMessage) message);
                    break;
                case MessageType.PARTICLE:
                    handleParticle((ParticleMessage) message);
                    break;
                case MessageType.WEATHER:
                    handleWeather((WeatherMessage) message);
                    break;
                case MessageType.GUI:
                    handleGui((GuiMessage) message);
                    break;
                default:
                    System.out.println("Received unknown message: " + message);
                    break;
            }
        } catch (Exception e) {
            errors++;
            System.err.println("Failed to process message: " + e.getMessage());
        }
    }

    /**
     * Handle a frame start message.
     *
     * @param message the frame start message
     */
    private void handleFrameStart(FrameStartMessage message) {
        // Clear previous frame data
        currentChunks.clear();
        currentEntities.clear();
        currentParticles.clear();
        
        lastFrameNumber = message.getFrameNumber();
        lastFrameTime = message.getTimestamp();
    }

    /**
     * Handle a frame end message.
     *
     * @param message the frame end message
     */
    private void handleFrameEnd(FrameEndMessage message) {
        framesReceived++;
        frameProcessingTime = message.getProcessingTimeMs();
        
        // Notify message handler
        messageHandler.onMessageReceived(message);
    }

    /**
     * Handle a camera message.
     *
     * @param message the camera message
     */
    private void handleCamera(CameraMessage message) {
        currentCamera = message;
        messagesReceived++;
        bytesReceived += message.getTotalSize();
        
        // Notify message handler
        messageHandler.onMessageReceived(message);
    }

    /**
     * Handle a chunk message.
     *
     * @param message the chunk message
     */
    private void handleChunk(ChunkMessage message) {
        currentChunks.add(message);
        messagesReceived++;
        bytesReceived += message.getTotalSize();
    }

    /**
     * Handle an entity message.
     *
     * @param message the entity message
     */
    private void handleEntity(EntityMessage message) {
        currentEntities.add(message);
        messagesReceived++;
        bytesReceived += message.getTotalSize();
    }

    /**
     * Handle a particle message.
     *
     * @param message the particle message
     */
    private void handleParticle(ParticleMessage message) {
        currentParticles.add(message);
        messagesReceived++;
        bytesReceived += message.getTotalSize();
    }

    /**
     * Handle a weather message.
     *
     * @param message the weather message
     */
    private void handleWeather(WeatherMessage message) {
        currentWeather = message;
        messagesReceived++;
        bytesReceived += message.getTotalSize();
    }

    /**
     * Handle a GUI message.
     *
     * @param message the GUI message
     */
    private void handleGui(GuiMessage message) {
        currentGui = message;
        messagesReceived++;
        bytesReceived += message.getTotalSize();
    }

    /**
     * Send a message to the mod.
     *
     * @param message the message to send
     */
    public void send(Message message) {
        try {
            if (bridgeServer != null && bridgeServer.isRunning()) {
                bridgeServer.send(message);
            }
        } catch (Exception e) {
            errors++;
            System.err.println("Failed to send message: " + e.getMessage());
        }
    }

    /**
     * Queue a message for processing.
     *
     * @param message the message to queue
     */
    public void queueMessage(Message message) {
        messageQueue.add(message);
    }

    /**
     * Set the message handler.
     *
     * @param handler the message handler
     */
    public void setMessageHandler(MessageHandler handler) {
        this.messageHandler = handler != null ? handler : DEFAULT_HANDLER;
    }

    // Scene data getters

    /**
     * Get the current camera message.
     *
     * @return the current camera message, or null if not available
     */
    public CameraMessage getCurrentCamera() {
        return currentCamera;
    }

    /**
     * Get the current chunks.
     *
     * @return a list of current chunks
     */
    public List<ChunkMessage> getCurrentChunks() {
        return new CopyOnWriteArrayList<>(currentChunks);
    }

    /**
     * Get the current entities.
     *
     * @return a list of current entities
     */
    public List<EntityMessage> getCurrentEntities() {
        return new CopyOnWriteArrayList<>(currentEntities);
    }

    /**
     * Get the current particles.
     *
     * @return a list of current particles
     */
    public List<ParticleMessage> getCurrentParticles() {
        return new CopyOnWriteArrayList<>(currentParticles);
    }

    /**
     * Get the current weather.
     *
     * @return the current weather message, or null if not available
     */
    public WeatherMessage getCurrentWeather() {
        return currentWeather;
    }

    /**
     * Get the current GUI message.
     *
     * @return the current GUI message, or null if not available
     */
    public GuiMessage getCurrentGui() {
        return currentGui;
    }

    // Statistics getters

    /**
     * Get the frames received count.
     *
     * @return the frames received count
     */
    public int getFramesReceived() {
        return framesReceived;
    }

    /**
     * Get the messages received count.
     *
     * @return the messages received count
     */
    public int getMessagesReceived() {
        return messagesReceived;
    }

    /**
     * Get the bytes received count.
     *
     * @return the bytes received count
     */
    public int getBytesReceived() {
        return bytesReceived;
    }

    /**
     * Get the error count.
     *
     * @return the error count
     */
    public int getErrors() {
        return errors;
    }

    /**
     * Get the last frame number.
     *
     * @return the last frame number
     */
    public long getLastFrameNumber() {
        return lastFrameNumber;
    }

    /**
     * Get the frame processing time in milliseconds.
     *
     * @return the frame processing time
     */
    public float getFrameProcessingTime() {
        return frameProcessingTime;
    }

    /**
     * Reset statistics.
     */
    public void resetStatistics() {
        framesReceived = 0;
        messagesReceived = 0;
        bytesReceived = 0;
        errors = 0;
    }

    /**
     * Check if the bridge is connected.
     *
     * @return true if connected
     */
    public boolean isConnected() {
        return connected;
    }

    /**
     * Check if the bridge is running.
     *
     * @return true if running
     */
    public boolean isRunning() {
        return running;
    }

    /**
     * Get the bridge server.
     *
     * @return the bridge server
     */
    public BridgeServer getBridgeServer() {
        return bridgeServer;
    }

    /**
     * Server message handler for the bridge server.
     */
    private class ServerMessageHandler implements BridgeServer.MessageHandler {
        @Override
        public void onMessageReceived(Message message) {
            queueMessage(message);
        }

        @Override
        public void onClientConnected(String clientName) {
            connected = true;
            messageHandler.onClientConnected(clientName);
        }

        @Override
        public void onClientDisconnected(String clientName) {
            connected = false;
            messageHandler.onClientDisconnected(clientName);
        }

        @Override
        public void onServerStateChanged(BridgeServer.ServerState state) {
            messageHandler.onConnectionStateChanged(state);
        }

        @Override
        public void onError(TransportException exception) {
            errors++;
            messageHandler.onError(exception);
        }
    }

    @Override
    public String toString() {
        return String.format(
                "RendererBridgeClient{running=%s, connected=%s, framesReceived=%d, messagesReceived=%d, errors=%d}",
                running, connected, framesReceived, messagesReceived, errors
        );
    }
}
