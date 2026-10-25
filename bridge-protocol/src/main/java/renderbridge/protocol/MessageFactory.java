package renderbridge.protocol;

import renderbridge.protocol.messages.CameraMessage;
import renderbridge.protocol.messages.ChunkMessage;
import renderbridge.protocol.messages.ControlMessage;
import renderbridge.protocol.messages.EchoMessage;
import renderbridge.protocol.messages.EntityMessage;
import renderbridge.protocol.messages.ErrorMessage;
import renderbridge.protocol.messages.FrameEndMessage;
import renderbridge.protocol.messages.FrameStartMessage;
import renderbridge.protocol.messages.HandshakeMessage;
import renderbridge.protocol.messages.HeartbeatMessage;
import renderbridge.protocol.messages.AckMessage;
import renderbridge.protocol.messages.GuiMessage;
import renderbridge.protocol.messages.ParticleMessage;
import renderbridge.protocol.messages.WeatherMessage;

import java.util.HashMap;
import java.util.Map;
import java.util.function.BiFunction;

/**
 * Factory for creating message instances based on message type.
 */
public final class MessageFactory {

    private static final Map<Integer, BiFunction<Integer, ByteBuffer, Message>> messageCreators = new HashMap<>();

    static {
        // Control messages
        registerCreator(MessageType.HANDSHAKE, HandshakeMessage::new);
        registerCreator(MessageType.HEARTBEAT, HeartbeatMessage::new);
        registerCreator(MessageType.ACK, AckMessage::new);
        registerCreator(MessageType.ERROR, ErrorMessage::new);
        registerCreator(MessageType.CONTROL, ControlMessage::new);
        registerCreator(MessageType.ECHO, EchoMessage::new);

        // Frame messages
        registerCreator(MessageType.FRAME_START, FrameStartMessage::new);
        registerCreator(MessageType.FRAME_END, FrameEndMessage::new);

        // Data messages
        registerCreator(MessageType.CAMERA, CameraMessage::new);
        registerCreator(MessageType.CHUNK, ChunkMessage::new);
        registerCreator(MessageType.ENTITY, EntityMessage::new);
        registerCreator(MessageType.PARTICLE, ParticleMessage::new);
        registerCreator(MessageType.WEATHER, WeatherMessage::new);
        registerCreator(MessageType.GUI, GuiMessage::new);
    }

    /**
     * Register a message creator for a specific message type.
     *
     * @param messageType the message type ID
     * @param creator the creator function
     */
    public static void registerCreator(int messageType, BiFunction<Integer, ByteBuffer, Message> creator) {
        messageCreators.put(messageType, creator);
    }

    /**
     * Create a message instance based on message type and version.
     *
     * @param messageType the message type ID
     * @param version the protocol version
     * @return a new message instance, or null if the type is unknown
     */
    public static Message create(int messageType, int version) {
        BiFunction<Integer, ByteBuffer, Message> creator = messageCreators.get(messageType);
        if (creator == null) {
            return null;
        }
        return creator.apply(version, null);
    }

    /**
     * Create a message instance from a header and payload buffer.
     *
     * @param header the message header
     * @param payload the message payload
     * @return a new message instance, or null if the type is unknown
     */
    public static Message create(MessageHeader header, ByteBuffer payload) {
        BiFunction<Integer, ByteBuffer, Message> creator = messageCreators.get(header.getMessageId());
        if (creator == null) {
            return null;
        }
        return creator.apply(header.getVersion(), payload);
    }

    /**
     * Check if a message type is supported.
     *
     * @param messageType the message type ID
     * @return true if the message type is supported
     */
    public static boolean isSupported(int messageType) {
        return messageCreators.containsKey(messageType);
    }

    private MessageFactory() {
        // Utility class
    }
}
