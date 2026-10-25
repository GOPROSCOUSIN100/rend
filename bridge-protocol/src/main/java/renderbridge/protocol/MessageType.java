package renderbridge.protocol;

/**
 * Message type identifiers for the bridge protocol.
 * All message types are 16-bit unsigned integers.
 */
public final class MessageType {

    // Control messages (0x0000-0x0FFF)
    public static final int HANDSHAKE = 0x0001;
    public static final int HEARTBEAT = 0x0002;
    public static final int ACK = 0x0003;
    public static final int ERROR = 0x0004;
    public static final int CONTROL = 0x0005;
    public static final int ECHO = 0x0006;

    // Frame messages (0x0100-0x01FF)
    public static final int FRAME_START = 0x0100;
    public static final int FRAME_END = 0x0101;

    // Data messages (0x0200-0x02FF)
    public static final int CAMERA = 0x0200;
    public static final int CHUNK = 0x0201;
    public static final int ENTITY = 0x0202;
    public static final int PARTICLE = 0x0203;
    public static final int WEATHER = 0x0204;
    public static final int GUI = 0x0205;

    // Reserved range (0x0300-0xFFFF)
    public static final int RESERVED_MIN = 0x0300;
    public static final int RESERVED_MAX = 0xFFFF;

    /**
     * Check if a message type is valid.
     *
     * @param type the message type to check
     * @return true if the type is a known message type
     */
    public static boolean isValid(int type) {
        return type >= HANDSHAKE && type <= GUI;
    }

    /**
     * Check if a message type is a control message.
     *
     * @param type the message type to check
     * @return true if the type is a control message
     */
    public static boolean isControlMessage(int type) {
        return type >= 0x0001 && type <= 0x0FFF;
    }

    /**
     * Check if a message type is a frame message.
     *
     * @param type the message type to check
     * @return true if the type is a frame message
     */
    public static boolean isFrameMessage(int type) {
        return type >= 0x0100 && type <= 0x01FF;
    }

    /**
     * Check if a message type is a data message.
     *
     * @param type the message type to check
     * @return true if the type is a data message
     */
    public static boolean isDataMessage(int type) {
        return type >= 0x0200 && type <= 0x02FF;
    }

    /**
     * Get the name of a message type.
     *
     * @param type the message type
     * @return the name of the message type, or "UNKNOWN" if not recognized
     */
    public static String getName(int type) {
        switch (type) {
            case HANDSHAKE:
                return "HANDSHAKE";
            case HEARTBEAT:
                return "HEARTBEAT";
            case ACK:
                return "ACK";
            case ERROR:
                return "ERROR";
            case CONTROL:
                return "CONTROL";
            case ECHO:
                return "ECHO";
            case FRAME_START:
                return "FRAME_START";
            case FRAME_END:
                return "FRAME_END";
            case CAMERA:
                return "CAMERA";
            case CHUNK:
                return "CHUNK";
            case ENTITY:
                return "ENTITY";
            case PARTICLE:
                return "PARTICLE";
            case WEATHER:
                return "WEATHER";
            case GUI:
                return "GUI";
            default:
                return "UNKNOWN_0x" + Integer.toHexString(type);
        }
    }

    private MessageType() {
        // Utility class
    }
}
