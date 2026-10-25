package renderbridge.protocol.transport;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageHeader;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;

/**
 * Transport channel implementation using shared memory (memory-mapped files).
 * Provides low-latency inter-process communication.
 */
public class SharedMemoryChannel implements TransportChannel {

    // Default settings
    private static final String DEFAULT_FILE_PREFIX = "renderbridge_shm";
    private static final int DEFAULT_BUFFER_SIZE = 16 * 1024 * 1024; // 16MB
    private static final long DEFAULT_TIMEOUT_MS = 1000;

    // Shared memory file
    private Path memoryFilePath;
    private FileChannel fileChannel;
    private MappedByteBuffer mappedBuffer;
    private FileLock fileLock;
    
    // Ring buffer for data
    private RingBuffer ringBuffer;
    
    // Statistics
    private final TransportStatistics statistics = new TransportStatistics();
    
    // State
    private volatile boolean connected = false;
    private long receiveTimeoutMs = DEFAULT_TIMEOUT_MS;
    private final String name;

    /**
     * Create a new shared memory channel with default settings.
     *
     * @param isClient true if this is the client side, false for server
     */
    public SharedMemoryChannel(boolean isClient) {
        this(isClient, DEFAULT_FILE_PREFIX, DEFAULT_BUFFER_SIZE);
    }

    /**
     * Create a new shared memory channel.
     *
     * @param isClient true if this is the client side, false for server
     * @param filePrefix the file prefix for the shared memory file
     * @param bufferSize the size of the shared memory buffer
     */
    public SharedMemoryChannel(boolean isClient, String filePrefix, int bufferSize) {
        this.name = filePrefix + (isClient ? "_client" : "_server");
        this.memoryFilePath = getTempFilePath(filePrefix);
        this.ringBuffer = new RingBuffer(bufferSize);
    }

    /**
     * Get the path for the shared memory file in the temp directory.
     *
     * @param filePrefix the file prefix
     * @return the path to the shared memory file
     */
    private Path getTempFilePath(String filePrefix) {
        try {
            Path tempDir = Files.createTempDirectory("renderbridge");
            return tempDir.resolve(filePrefix + ".mem");
        } catch (IOException e) {
            // Fallback to system temp directory
            return new File(System.getProperty("java.io.tmpdir"), filePrefix + ".mem").toPath();
        }
    }

    @Override
    public void connect() throws TransportException, IOException {
        if (connected) {
            return;
        }

        try {
            // Create or open the file
            Files.createDirectories(memoryFilePath.getParent());
            
            // Open the file channel
            fileChannel = FileChannel.open(
                    memoryFilePath,
                    StandardOpenOption.CREATE,
                    StandardOpenOption.READ,
                    StandardOpenOption.WRITE
            );
            
            // Map the file into memory
            mappedBuffer = fileChannel.map(
                    FileChannel.MapMode.READ_WRITE,
                    0,
                    ringBuffer.getBuffer().capacity()
            );
            
            // Create ring buffer with the mapped buffer
            ringBuffer = new RingBuffer(mappedBuffer);
            
            // Lock the file to prevent other processes from accessing it
            fileLock = fileChannel.lock();
            
            connected = true;
        } catch (IOException e) {
            disconnect();
            throw new TransportException("Failed to connect shared memory channel", e);
        }
    }

    @Override
    public void disconnect() {
        connected = false;
        
        try {
            if (fileLock != null) {
                fileLock.release();
                fileLock = null;
            }
        } catch (IOException e) {
            // Ignore
        }
        
        try {
            if (fileChannel != null) {
                fileChannel.close();
                fileChannel = null;
            }
        } catch (IOException e) {
            // Ignore
        }
        
        // Clean up the mapped buffer
        if (mappedBuffer != null) {
            // Clear the buffer to release resources
            mappedBuffer.clear();
            mappedBuffer = null;
        }
        
        // Delete the temporary file
        try {
            if (memoryFilePath != null) {
                Files.deleteIfExists(memoryFilePath);
                Files.deleteIfExists(memoryFilePath.getParent());
            }
        } catch (IOException e) {
            // Ignore
        }
    }

    @Override
    public boolean isConnected() {
        return connected && fileChannel != null && mappedBuffer != null;
    }

    @Override
    public void send(Message message) throws TransportException, IOException {
        if (!isConnected()) {
            throw new TransportException("Channel is not connected");
        }
        
        long startTime = System.nanoTime();
        
        try {
            ByteBuffer serialized = message.serialize();
            int bytesSent = ringBuffer.writeMessage(serialized);
            
            if (bytesSent < serialized.capacity()) {
                statistics.recordError();
                throw new TransportException("Failed to send complete message");
            }
            
            statistics.recordSend(bytesSent, System.nanoTime() - startTime);
        } catch (Exception e) {
            statistics.recordError();
            throw new TransportException("Failed to send message", e);
        }
    }

    @Override
    public void send(ByteBuffer data) throws TransportException, IOException {
        if (!isConnected()) {
            throw new TransportException("Channel is not connected");
        }
        
        long startTime = System.nanoTime();
        
        try {
            int bytesSent = ringBuffer.writeMessage(data);
            statistics.recordSend(bytesSent, System.nanoTime() - startTime);
        } catch (Exception e) {
            statistics.recordError();
            throw new TransportException("Failed to send data", e);
        }
    }

    @Override
    public Message receive() throws TransportException, IOException {
        if (!isConnected()) {
            throw new TransportException("Channel is not connected");
        }
        
        long startTime = System.nanoTime();
        long timeoutTime = startTime + (receiveTimeoutMs * 1_000_000);
        
        while (isConnected() && System.nanoTime() < timeoutTime) {
            ByteBuffer messageBuffer = ringBuffer.readMessage();
            
            if (messageBuffer != null) {
                try {
                    Message message = Message.deserialize(messageBuffer);
                    statistics.recordReceive(messageBuffer.capacity(), System.nanoTime() - startTime);
                    return message;
                } catch (Exception e) {
                    statistics.recordError();
                    throw new TransportException("Failed to deserialize message", e);
                }
            }
            
            // Small sleep to prevent busy waiting
            try {
                Thread.sleep(1);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                throw new TransportException("Receive interrupted", e);
            }
        }
        
        statistics.recordTimeout();
        return null;
    }

    @Override
    public ByteBuffer receiveRaw() throws TransportException, IOException {
        if (!isConnected()) {
            throw new TransportException("Channel is not connected");
        }
        
        long startTime = System.nanoTime();
        long timeoutTime = startTime + (receiveTimeoutMs * 1_000_000);
        
        while (isConnected() && System.nanoTime() < timeoutTime) {
            ByteBuffer messageBuffer = ringBuffer.readMessage();
            
            if (messageBuffer != null) {
                statistics.recordReceive(messageBuffer.capacity(), System.nanoTime() - startTime);
                return messageBuffer;
            }
            
            // Small sleep to prevent busy waiting
            try {
                Thread.sleep(1);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                throw new TransportException("Receive interrupted", e);
            }
        }
        
        statistics.recordTimeout();
        return null;
    }

    @Override
    public TransportType getType() {
        return TransportType.SHARED_MEMORY;
    }

    @Override
    public String getName() {
        return name;
    }

    @Override
    public void setReceiveTimeout(long timeoutMillis) {
        this.receiveTimeoutMs = timeoutMillis;
    }

    @Override
    public long getReceiveTimeout() {
        return receiveTimeoutMs;
    }

    @Override
    public boolean isDataAvailable() throws IOException {
        if (!isConnected()) {
            return false;
        }
        return !ringBuffer.isEmpty();
    }

    @Override
    public void flush() throws IOException {
        // Shared memory doesn't need flushing
    }

    @Override
    public TransportStatistics getStatistics() {
        return statistics;
    }

    @Override
    public void resetStatistics() {
        statistics.reset();
    }

    /**
     * Get the path to the shared memory file.
     *
     * @return the file path
     */
    public Path getMemoryFilePath() {
        return memoryFilePath;
    }

    @Override
    public String toString() {
        return String.format(
                "SharedMemoryChannel{name='%s', connected=%s, file='%s'}",
                name, connected, memoryFilePath
        );
    }
}
