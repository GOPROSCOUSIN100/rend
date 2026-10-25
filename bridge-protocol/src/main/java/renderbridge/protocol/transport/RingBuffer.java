package renderbridge.protocol.transport;

import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Ring buffer implementation for efficient data transfer between processes.
 * Used with memory-mapped files for shared memory transport.
 */
public class RingBuffer {

    private final ByteBuffer buffer;
    private final int capacity;
    private final AtomicInteger writePosition;
    private final AtomicInteger readPosition;
    
    // Header offsets within the buffer
    private static final int HEADER_SIZE = 16;
    private static final int OFFSET_WRITE_POS = 0;
    private static final int OFFSET_READ_POS = 4;
    private static final int OFFSET_VERSION = 8;
    private static final int OFFSET_FLAGS = 9;
    private static final int OFFSET_PADDING = 10;

    // Buffer starts after header
    private static final int DATA_OFFSET = HEADER_SIZE;

    // Flags
    public static final byte FLAG_NONE = 0x00;
    public static final byte FLAG_OVERFLOW = 0x01;

    /**
     * Create a new ring buffer with the specified capacity.
     *
     * @param capacity the total capacity of the buffer (including header)
     */
    public RingBuffer(int capacity) {
        if (capacity < HEADER_SIZE * 2) {
            throw new IllegalArgumentException("Capacity must be at least " + (HEADER_SIZE * 2));
        }
        
        this.capacity = capacity;
        this.buffer = ByteBuffer.allocateDirect(capacity);
        this.writePosition = new AtomicInteger(DATA_OFFSET);
        this.readPosition = new AtomicInteger(DATA_OFFSET);
        
        // Initialize header
        initializeHeader();
    }

    /**
     * Create a ring buffer that wraps an existing byte buffer.
     *
     * @param buffer the byte buffer to wrap
     */
    public RingBuffer(ByteBuffer buffer) {
        if (buffer.capacity() < HEADER_SIZE * 2) {
            throw new IllegalArgumentException("Buffer capacity must be at least " + (HEADER_SIZE * 2));
        }
        
        this.capacity = buffer.capacity();
        this.buffer = buffer;
        this.writePosition = new AtomicInteger(DATA_OFFSET);
        this.readPosition = new AtomicInteger(DATA_OFFSET);
        
        // Read header if it exists
        readHeader();
    }

    /**
     * Initialize the ring buffer header.
     */
    private void initializeHeader() {
        buffer.putInt(OFFSET_WRITE_POS, DATA_OFFSET);
        buffer.putInt(OFFSET_READ_POS, DATA_OFFSET);
        buffer.put(OFFSET_VERSION, (byte) 1);
        buffer.put(OFFSET_FLAGS, FLAG_NONE);
        // Padding bytes are already zero
    }

    /**
     * Read the header from the buffer.
     */
    private void readHeader() {
        writePosition.set(buffer.getInt(OFFSET_WRITE_POS));
        readPosition.set(buffer.getInt(OFFSET_READ_POS));
    }

    /**
     * Write the header to the buffer.
     */
    private void writeHeader() {
        buffer.putInt(OFFSET_WRITE_POS, writePosition.get());
        buffer.putInt(OFFSET_READ_POS, readPosition.get());
    }

    /**
     * Get the data capacity of the buffer (excluding header).
     *
     * @return the data capacity
     */
    public int getDataCapacity() {
        return capacity - DATA_OFFSET;
    }

    /**
     * Get the available space for writing.
     *
     * @return the available space in bytes
     */
    public int getAvailableSpace() {
        int writePos = writePosition.get();
        int readPos = readPosition.get();
        
        if (writePos >= readPos) {
            return Math.max(0, getDataCapacity() - (writePos - readPos));
        } else {
            return Math.max(0, readPos - writePos);
        }
    }

    /**
     * Get the available space for reading.
     *
     * @return the available data in bytes
     */
    public int getAvailableData() {
        int writePos = writePosition.get();
        int readPos = readPosition.get();
        
        if (writePos >= readPos) {
            return writePos - readPos;
        } else {
            return getDataCapacity() - (readPos - writePos);
        }
    }

    /**
     * Check if the buffer is empty.
     *
     * @return true if the buffer is empty
     */
    public boolean isEmpty() {
        return writePosition.get() == readPosition.get();
    }

    /**
     * Check if the buffer is full.
     *
     * @return true if the buffer is full
     */
    public boolean isFull() {
        return getAvailableSpace() == 0;
    }

    /**
     * Write data to the ring buffer.
     *
     * @param data the data to write
     * @return the number of bytes written
     */
    public synchronized int write(ByteBuffer data) {
        int bytesToWrite = data.remaining();
        int availableSpace = getAvailableSpace();
        
        if (bytesToWrite > availableSpace) {
            bytesToWrite = availableSpace;
        }
        
        if (bytesToWrite == 0) {
            return 0;
        }
        
        int writePos = writePosition.get();
        int endPos = writePos + bytesToWrite;
        
        // Check if we need to wrap around
        if (endPos > capacity) {
            int firstPart = capacity - writePos;
            buffer.position(writePos);
            buffer.put(data.array(), data.arrayOffset() + data.position(), firstPart);
            
            int secondPart = bytesToWrite - firstPart;
            buffer.position(DATA_OFFSET);
            buffer.put(data.array(), data.arrayOffset() + data.position() + firstPart, secondPart);
            
            writePosition.set(DATA_OFFSET + secondPart);
        } else {
            buffer.position(writePos);
            buffer.put(data.array(), data.arrayOffset() + data.position(), bytesToWrite);
            writePosition.set(endPos);
        }
        
        // Update header
        writeHeader();
        
        data.position(data.position() + bytesToWrite);
        return bytesToWrite;
    }

    /**
     * Read data from the ring buffer.
     *
     * @param destination the buffer to read into
     * @return the number of bytes read
     */
    public synchronized int read(ByteBuffer destination) {
        int availableData = getAvailableData();
        int bytesToRead = Math.min(destination.remaining(), availableData);
        
        if (bytesToRead == 0) {
            return 0;
        }
        
        int readPos = readPosition.get();
        int endPos = readPos + bytesToRead;
        
        // Check if we need to wrap around
        if (endPos > capacity) {
            int firstPart = capacity - readPos;
            buffer.position(readPos);
            buffer.get(destination.array(), destination.arrayOffset() + destination.position(), firstPart);
            
            int secondPart = bytesToRead - firstPart;
            buffer.position(DATA_OFFSET);
            buffer.get(destination.array(), destination.arrayOffset() + destination.position() + firstPart, secondPart);
            
            readPosition.set(DATA_OFFSET + secondPart);
        } else {
            buffer.position(readPos);
            buffer.get(destination.array(), destination.arrayOffset() + destination.position(), bytesToRead);
            readPosition.set(endPos);
        }
        
        // Update header
        writeHeader();
        
        destination.position(destination.position() + bytesToRead);
        return bytesToRead;
    }

    /**
     * Write a message to the ring buffer.
     * The message is prefixed with its total size.
     *
     * @param message the message to write
     * @return the number of bytes written
     */
    public int writeMessage(ByteBuffer message) {
        // Prefix with message size (4 bytes)
        int messageSize = message.remaining();
        ByteBuffer sizeBuffer = ByteBuffer.allocate(4);
        sizeBuffer.putInt(messageSize);
        sizeBuffer.flip();
        
        // Write size first
        int sizeBytes = write(sizeBuffer);
        if (sizeBytes < 4) {
            return sizeBytes; // Not enough space for size
        }
        
        // Then write the message
        return sizeBytes + write(message);
    }

    /**
     * Read a message from the ring buffer.
     * The message is prefixed with its total size.
     *
     * @return the message data, or null if no complete message is available
     */
    public ByteBuffer readMessage() {
        // First read the size
        ByteBuffer sizeBuffer = ByteBuffer.allocate(4);
        int sizeBytes = read(sizeBuffer);
        
        if (sizeBytes < 4) {
            return null; // Not enough data for size
        }
        
        sizeBuffer.flip();
        int messageSize = sizeBuffer.getInt();
        
        // Then read the message
        ByteBuffer messageBuffer = ByteBuffer.allocate(messageSize);
        int messageBytes = read(messageBuffer);
        
        if (messageBytes < messageSize) {
            // Not enough data for the full message, push back what we read
            // This is a simplified approach - in a real implementation, we'd need to handle this better
            return null;
        }
        
        messageBuffer.flip();
        return messageBuffer;
    }

    /**
     * Clear the ring buffer.
     */
    public synchronized void clear() {
        writePosition.set(DATA_OFFSET);
        readPosition.set(DATA_OFFSET);
        writeHeader();
    }

    /**
     * Get the underlying byte buffer.
     *
     * @return the byte buffer
     */
    public ByteBuffer getBuffer() {
        return buffer;
    }

    /**
     * Get the current write position.
     *
     * @return the write position
     */
    public int getWritePosition() {
        return writePosition.get();
    }

    /**
     * Get the current read position.
     *
     * @return the read position
     */
    public int getReadPosition() {
        return readPosition.get();
    }

    @Override
    public String toString() {
        return String.format(
                "RingBuffer{capacity=%d, availableSpace=%d, availableData=%d, writePos=%d, readPos=%d}",
                capacity, getAvailableSpace(), getAvailableData(), writePosition.get(), readPosition.get()
        );
    }
}
