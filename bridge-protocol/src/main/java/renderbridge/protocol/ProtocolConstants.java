package renderbridge.protocol;

/**
 * Constants for the bridge protocol.
 */
public final class ProtocolConstants {

    // Protocol version
    public static final byte PROTOCOL_VERSION = 1;
    
    // Maximum message sizes
    public static final int MAX_MESSAGE_SIZE = 16 * 1024 * 1024; // 16MB
    public static final int MAX_PAYLOAD_SIZE = MAX_MESSAGE_SIZE - MessageHeader.SIZE;
    
    // Timeouts
    public static final long DEFAULT_CONNECT_TIMEOUT_MS = 5000; // 5 seconds
    public static final long DEFAULT_READ_TIMEOUT_MS = 1000; // 1 second
    public static final long DEFAULT_WRITE_TIMEOUT_MS = 1000; // 1 second
    public static final long HEARTBEAT_INTERVAL_MS = 1000; // 1 second
    public static final long HEARTBEAT_TIMEOUT_MS = 3000; // 3 seconds
    
    // Retry settings
    public static final int MAX_RETRIES = 3;
    public static final long RETRY_DELAY_MS = 100; // 100ms
    
    // Shared memory settings
    public static final String SHARED_MEMORY_FILE_PREFIX = "renderbridge_shm";
    public static final int SHARED_MEMORY_BUFFER_SIZE = 16 * 1024 * 1024; // 16MB
    
    // Socket settings
    public static final String SOCKET_HOST = "127.0.0.1";
    public static final int SOCKET_PORT = 25565;
    
    // Frame settings
    public static final long INVALID_FRAME_NUMBER = -1;
    
    private ProtocolConstants() {
        // Utility class
    }
}
