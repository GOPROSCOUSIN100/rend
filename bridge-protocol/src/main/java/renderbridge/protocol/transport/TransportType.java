package renderbridge.protocol.transport;

/**
 * Enum defining the available transport types for the bridge protocol.
 */
public enum TransportType {
    /**
     * Shared memory transport using memory-mapped files.
     * Best performance, lowest latency.
     */
    SHARED_MEMORY("Shared Memory", 0),
    
    /**
     * Local socket transport using TCP/IP on localhost.
     * Good performance, high portability.
     */
    LOCAL_SOCKET("Local Socket", 1),
    
    /**
     * Named pipe transport (platform-specific).
     * Medium performance, low portability.
     */
    NAMED_PIPE("Named Pipe", 2);

    private final String displayName;
    private final int id;

    TransportType(String displayName, int id) {
        this.displayName = displayName;
        this.id = id;
    }

    /**
     * Get the display name of this transport type.
     *
     * @return the display name
     */
    public String getDisplayName() {
        return displayName;
    }

    /**
     * Get the numeric ID of this transport type.
     *
     * @return the numeric ID
     */
    public int getId() {
        return id;
    }

    /**
     * Get a transport type by its numeric ID.
     *
     * @param id the numeric ID
     * @return the transport type, or null if not found
     */
    public static TransportType byId(int id) {
        for (TransportType type : values()) {
            if (type.id == id) {
                return type;
            }
        }
        return null;
    }

    @Override
    public String toString() {
        return displayName;
    }
}
