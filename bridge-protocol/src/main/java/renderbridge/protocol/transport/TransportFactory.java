package renderbridge.protocol.transport;

import java.io.IOException;

/**
 * Factory for creating transport channels.
 * Provides a simple way to create transport channels with fallback support.
 */
public final class TransportFactory {

    // Default settings
    private static final String DEFAULT_SHARED_MEMORY_PREFIX = "renderbridge";
    private static final int DEFAULT_SHARED_MEMORY_SIZE = 16 * 1024 * 1024; // 16MB
    private static final String DEFAULT_SOCKET_HOST = "127.0.0.1";
    private static final int DEFAULT_SOCKET_PORT = 25565;

    /**
     * Create a transport channel with automatic fallback.
     * Tries shared memory first, then falls back to local socket.
     *
     * @param isClient true if this is the client side
     * @return a connected transport channel
     * @throws TransportException if all transport methods fail
     * @throws IOException if an I/O error occurs
     */
    public static TransportChannel createChannel(boolean isClient) throws TransportException, IOException {
        return createChannel(isClient, DEFAULT_SHARED_MEMORY_PREFIX, DEFAULT_SHARED_MEMORY_SIZE,
                DEFAULT_SOCKET_HOST, DEFAULT_SOCKET_PORT);
    }

    /**
     * Create a transport channel with the specified configuration.
     * Tries shared memory first, then falls back to local socket.
     *
     * @param isClient true if this is the client side
     * @param sharedMemoryPrefix the prefix for shared memory files
     * @param sharedMemorySize the size of the shared memory buffer
     * @param socketHost the host for socket fallback
     * @param socketPort the port for socket fallback
     * @return a connected transport channel
     * @throws TransportException if all transport methods fail
     * @throws IOException if an I/O error occurs
     */
    public static TransportChannel createChannel(boolean isClient, String sharedMemoryPrefix, 
                                                   int sharedMemorySize, String socketHost, 
                                                   int socketPort) throws TransportException, IOException {
        TransportChannel channel = null;
        TransportException lastException = null;
        
        try {
            // Try shared memory first
            channel = new SharedMemoryChannel(isClient, sharedMemoryPrefix, sharedMemorySize);
            channel.connect();
            return channel;
        } catch (Exception e) {
            lastException = new TransportException("Shared memory transport failed", e);
        }
        
        try {
            // Fall back to local socket
            channel = new LocalSocketChannel(socketHost, socketPort, isClient);
            channel.connect();
            return channel;
        } catch (Exception e) {
            lastException = new TransportException("Local socket transport failed", e);
        }
        
        // All transports failed
        throw new TransportException("All transport methods failed", lastException);
    }

    /**
     * Create a shared memory transport channel.
     *
     * @param isClient true if this is the client side
     * @return a shared memory transport channel
     */
    public static TransportChannel createSharedMemoryChannel(boolean isClient) {
        return new SharedMemoryChannel(isClient, DEFAULT_SHARED_MEMORY_PREFIX, DEFAULT_SHARED_MEMORY_SIZE);
    }

    /**
     * Create a shared memory transport channel with custom settings.
     *
     * @param isClient true if this is the client side
     * @param prefix the file prefix
     * @param size the buffer size
     * @return a shared memory transport channel
     */
    public static TransportChannel createSharedMemoryChannel(boolean isClient, String prefix, int size) {
        return new SharedMemoryChannel(isClient, prefix, size);
    }

    /**
     * Create a local socket transport channel.
     *
     * @param isClient true if this is the client side
     * @return a local socket transport channel
     */
    public static TransportChannel createLocalSocketChannel(boolean isClient) {
        return new LocalSocketChannel(DEFAULT_SOCKET_HOST, DEFAULT_SOCKET_PORT, isClient);
    }

    /**
     * Create a local socket transport channel with custom settings.
     *
     * @param isClient true if this is the client side
     * @param host the host to connect to
     * @param port the port to connect to
     * @return a local socket transport channel
     */
    public static TransportChannel createLocalSocketChannel(boolean isClient, String host, int port) {
        return new LocalSocketChannel(host, port, isClient);
    }

    /**
     * Create a server-side transport channel.
     *
     * @return a server-side transport channel
     * @throws TransportException if creation fails
     * @throws IOException if an I/O error occurs
     */
    public static TransportChannel createServerChannel() throws TransportException, IOException {
        return createChannel(false);
    }

    /**
     * Create a client-side transport channel.
     *
     * @return a client-side transport channel
     * @throws TransportException if creation fails
     * @throws IOException if an I/O error occurs
     */
    public static TransportChannel createClientChannel() throws TransportException, IOException {
        return createChannel(true);
    }

    private TransportFactory() {
        // Utility class
    }
}
