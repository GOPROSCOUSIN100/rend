package renderbridge.mod.bridge;

import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.GuiIngame;
import net.minecraft.client.renderer.EntityRenderer;
import net.minecraft.client.renderer.RenderGlobal;
import net.minecraft.client.renderer.texture.TextureManager;
import net.minecraft.world.World;

import renderbridge.mod.RenderBridgeMod;
import renderbridge.mod.config.ModConfig;
import renderbridge.mod.extractors.CameraExtractor;
import renderbridge.mod.extractors.ChunkExtractor;
import renderbridge.mod.extractors.EntityExtractor;
import renderbridge.mod.extractors.ParticleExtractor;
import renderbridge.mod.extractors.WeatherExtractor;
import renderbridge.protocol.BridgeClient;
import renderbridge.protocol.Message;
import renderbridge.protocol.MessageType;
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
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Manages the bridge connection and data extraction.
 */
public class BridgeManager {

    // Connection state
    public enum ConnectionState {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        ERROR
    }

    // Message handler for receiving messages from the renderer
    private class RendererMessageHandler implements BridgeClient.MessageHandler {
        @Override
        public void onMessageReceived(Message message) {
            handleRendererMessage(message);
        }

        @Override
        public void onConnectionStateChanged(BridgeClient.ConnectionState state) {
            updateConnectionState(mapState(state));
        }

        @Override
        public void onError(TransportException exception) {
            RenderBridgeMod.getLogger().error("Bridge error: " + exception.getMessage());
            updateConnectionState(ConnectionState.ERROR);
        }

        private ConnectionState mapState(BridgeClient.ConnectionState state) {
            switch (state) {
                case DISCONNECTED:
                    return ConnectionState.DISCONNECTED;
                case CONNECTING:
                    return ConnectionState.CONNECTING;
                case HANDSHAKING:
                    return ConnectionState.CONNECTING;
                case CONNECTED:
                    return ConnectionState.CONNECTED;
                case ERROR:
                    return ConnectionState.ERROR;
                default:
                    return ConnectionState.DISCONNECTED;
            }
        }
    }

    // Bridge client
    private BridgeClient bridgeClient;
    
    // Extractors
    private CameraExtractor cameraExtractor;
    private ChunkExtractor chunkExtractor;
    private EntityExtractor entityExtractor;
    private ParticleExtractor particleExtractor;
    private WeatherExtractor weatherExtractor;
    
    // Configuration
    private ModConfig config;
    
    // State
    private volatile ConnectionState connectionState = ConnectionState.DISCONNECTED;
    private volatile long lastFrameNumber = -1;
    private volatile long lastSendTime = 0;
    private volatile long lastReceiveTime = 0;
    
    // Statistics
    private int framesSent = 0;
    private int messagesSent = 0;
    private int bytesSent = 0;
    private int errors = 0;
    
    // Message queue
    private final List<Message> messageQueue = new CopyOnWriteArrayList<>();

    /**
     * Create a new bridge manager.
     *
     * @param config the mod configuration
     */
    public BridgeManager(ModConfig config) {
        this.config = config;
        
        // Initialize bridge client
        bridgeClient = new BridgeClient("RenderBridgeMod", config.isUseSharedMemory(), config.isUseSocket());
        bridgeClient.setMessageHandler(new RendererMessageHandler());
    }

    /**
     * Connect to the bridge server.
     */
    public void connect() {
        if (connectionState == ConnectionState.CONNECTED || connectionState == ConnectionState.CONNECTING) {
            return;
        }
        
        updateConnectionState(ConnectionState.CONNECTING);
        
        try {
            bridgeClient.connect();
            
            // Initialize extractors
            Minecraft mc = Minecraft.getMinecraft();
            if (mc != null) {
                EntityRenderer entityRenderer = mc.entityRenderer;
                RenderGlobal renderGlobal = mc.renderGlobal;
                TextureManager textureManager = mc.renderEngine;
                World world = mc.theWorld;
                GuiIngame gui = mc.ingameGUI;
                
                cameraExtractor = new CameraExtractor(entityRenderer);
                chunkExtractor = new ChunkExtractor(renderGlobal, config.getMaxChunkDistance());
                entityExtractor = new EntityExtractor(renderGlobal);
                particleExtractor = new ParticleExtractor(mc.effectRenderer);
                weatherExtractor = new WeatherExtractor(world);
            }
            
            updateConnectionState(ConnectionState.CONNECTED);
            RenderBridgeMod.getLogger().info("Connected to bridge server");
        } catch (Exception e) {
            RenderBridgeMod.getLogger().error("Failed to connect to bridge server: " + e.getMessage());
            updateConnectionState(ConnectionState.ERROR);
        }
    }

    /**
     * Disconnect from the bridge server.
     */
    public void disconnect() {
        if (connectionState == ConnectionState.DISCONNECTED) {
            return;
        }
        
        bridgeClient.disconnect();
        updateConnectionState(ConnectionState.DISCONNECTED);
        RenderBridgeMod.getLogger().info("Disconnected from bridge server");
    }

    /**
     * Update the bridge manager (called every tick).
     */
    public void update() {
        if (connectionState != ConnectionState.CONNECTED) {
            return;
        }
        
        // Check if we should send a frame
        if (shouldSendFrame()) {
            sendFrame();
        }
        
        // Process queued messages
        processMessageQueue();
    }

    /**
     * Check if we should send a frame based on the update interval.
     *
     * @return true if we should send a frame
     */
    private boolean shouldSendFrame() {
        // Always send frames for now
        return true;
    }

    /**
     * Send a frame of data to the renderer.
     */
    private void sendFrame() {
        try {
            long frameStartTime = System.nanoTime();
            lastFrameNumber++;
            
            // Send frame start
            FrameStartMessage frameStart = new FrameStartMessage(lastFrameNumber, frameStartTime, 0);
            sendMessage(frameStart);
            
            // Extract and send camera data
            if (config.isEnableCameraExtraction() && cameraExtractor != null) {
                CameraMessage cameraMessage = cameraExtractor.extract();
                if (cameraMessage != null) {
                    sendMessage(cameraMessage);
                }
            }
            
            // Extract and send chunk data
            if (config.isEnableChunkExtraction() && chunkExtractor != null) {
                List<ChunkMessage> chunkMessages = chunkExtractor.extract();
                for (ChunkMessage chunkMessage : chunkMessages) {
                    sendMessage(chunkMessage);
                }
            }
            
            // Extract and send entity data
            if (config.isEnableEntityExtraction() && entityExtractor != null) {
                List<EntityMessage> entityMessages = entityExtractor.extract();
                for (EntityMessage entityMessage : entityMessages) {
                    sendMessage(entityMessage);
                }
            }
            
            // Extract and send particle data
            if (config.isEnableParticleExtraction() && particleExtractor != null) {
                List<ParticleMessage> particleMessages = particleExtractor.extract();
                for (ParticleMessage particleMessage : particleMessages) {
                    sendMessage(particleMessage);
                }
            }
            
            // Extract and send weather data
            if (config.isEnableWeatherExtraction() && weatherExtractor != null) {
                WeatherMessage weatherMessage = weatherExtractor.extract();
                if (weatherMessage != null) {
                    sendMessage(weatherMessage);
                }
            }
            
            // Extract and send GUI data
            if (config.isShowStats()) {
                GuiMessage guiMessage = createStatsGuiMessage();
                if (guiMessage != null) {
                    sendMessage(guiMessage);
                }
            }
            
            // Send frame end
            long frameEndTime = System.nanoTime();
            long processingTime = frameEndTime - frameStartTime;
            FrameEndMessage frameEnd = new FrameEndMessage(lastFrameNumber, frameEndTime, messagesSent, processingTime);
            sendMessage(frameEnd);
            
            framesSent++;
            lastSendTime = System.currentTimeMillis();
        } catch (Exception e) {
            errors++;
            RenderBridgeMod.getLogger().error("Failed to send frame: " + e.getMessage());
        }
    }

    /**
     * Send a message through the bridge.
     *
     * @param message the message to send
     */
    private void sendMessage(Message message) {
        try {
            bridgeClient.send(message);
            messagesSent++;
            bytesSent += message.getTotalSize();
        } catch (Exception e) {
            errors++;
            RenderBridgeMod.getLogger().error("Failed to send message: " + e.getMessage());
        }
    }

    /**
     * Create a GUI message with statistics.
     *
     * @return a GUI message with statistics
     */
    private GuiMessage createStatsGuiMessage() {
        GuiMessage message = new GuiMessage();
        message.setElementType(GuiMessage.ELEMENT_DEBUG);
        message.setValue(framesSent);
        message.setMaxValue(100);
        message.setText(String.format("FPS: %d | Msgs: %d | Bytes: %d | Errors: %d", 
                framesSent, messagesSent, bytesSent, errors));
        return message;
    }

    /**
     * Process messages from the renderer.
     */
    private void processMessageQueue() {
        // Process queued messages
        for (Message message : messageQueue) {
            handleRendererMessage(message);
        }
        messageQueue.clear();
    }

    /**
     * Handle a message from the renderer.
     *
     * @param message the message to handle
     */
    private void handleRendererMessage(Message message) {
        lastReceiveTime = System.currentTimeMillis();
        
        switch (message.getMessageId()) {
            case MessageType.CONTROL:
                handleControlMessage(message);
                break;
            case MessageType.ECHO:
                // Echo response - ignore for now
                break;
            case MessageType.ERROR:
                RenderBridgeMod.getLogger().error("Renderer error: " + message);
                break;
            default:
                RenderBridgeMod.getLogger().debug("Received message: " + message);
                break;
        }
    }

    /**
     * Handle a control message from the renderer.
     *
     * @param message the control message
     */
    private void handleControlMessage(Message message) {
        // TODO: Handle control messages
    }

    /**
     * Queue a message from the renderer for processing.
     *
     * @param message the message to queue
     */
    public void queueMessage(Message message) {
        messageQueue.add(message);
    }

    /**
     * Update the connection state.
     *
     * @param state the new connection state
     */
    private void updateConnectionState(ConnectionState state) {
        this.connectionState = state;
        RenderBridgeMod.getLogger().info("Connection state changed to: " + state);
    }

    /**
     * Get the current connection state.
     *
     * @return the connection state
     */
    public ConnectionState getConnectionState() {
        return connectionState;
    }

    /**
     * Check if the bridge is connected.
     *
     * @return true if connected
     */
    public boolean isConnected() {
        return connectionState == ConnectionState.CONNECTED;
    }

    /**
     * Get the bridge client.
     *
     * @return the bridge client
     */
    public BridgeClient getBridgeClient() {
        return bridgeClient;
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
     * Get the frames sent count.
     *
     * @return the frames sent count
     */
    public int getFramesSent() {
        return framesSent;
    }

    /**
     * Get the messages sent count.
     *
     * @return the messages sent count
     */
    public int getMessagesSent() {
        return messagesSent;
    }

    /**
     * Get the bytes sent count.
     *
     * @return the bytes sent count
     */
    public int getBytesSent() {
        return bytesSent;
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
     * Reset statistics.
     */
    public void resetStatistics() {
        framesSent = 0;
        messagesSent = 0;
        bytesSent = 0;
        errors = 0;
    }

    @Override
    public String toString() {
        return String.format(
                "BridgeManager{state=%s, framesSent=%d, messagesSent=%d, bytesSent=%d, errors=%d}",
                connectionState, framesSent, messagesSent, bytesSent, errors
        );
    }
}
