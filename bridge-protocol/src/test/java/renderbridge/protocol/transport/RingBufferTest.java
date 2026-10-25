package renderbridge.protocol.transport;

import org.junit.Test;

import java.nio.ByteBuffer;

import static org.junit.Assert.*;

/**
 * Unit tests for RingBuffer.
 */
public class RingBufferTest {

    @Test
    public void testCreateRingBuffer() {
        RingBuffer buffer = new RingBuffer(1024);
        assertEquals(1024 - RingBuffer.DATA_OFFSET, buffer.getDataCapacity());
    }

    @Test(expected = IllegalArgumentException.class)
    public void testCreateRingBufferTooSmall() {
        new RingBuffer(16); // Too small (needs at least 32 for header)
    }

    @Test
    public void testIsEmpty() {
        RingBuffer buffer = new RingBuffer(1024);
        assertTrue(buffer.isEmpty());
        assertFalse(buffer.isFull());
    }

    @Test
    public void testWriteAndRead() {
        RingBuffer buffer = new RingBuffer(1024);
        
        // Write some data
        ByteBuffer data = ByteBuffer.allocate(100);
        for (int i = 0; i < 100; i++) {
            data.put((byte) i);
        }
        data.flip();
        
        int written = buffer.write(data);
        assertEquals(100, written);
        assertFalse(buffer.isEmpty());
        assertFalse(buffer.isFull());
        
        // Read the data back
        ByteBuffer readBuffer = ByteBuffer.allocate(100);
        int read = buffer.read(readBuffer);
        assertEquals(100, read);
        
        readBuffer.flip();
        for (int i = 0; i < 100; i++) {
            assertEquals((byte) i, readBuffer.get());
        }
        
        assertTrue(buffer.isEmpty());
    }

    @Test
    public void testWriteMessageAndReadMessage() {
        RingBuffer buffer = new RingBuffer(1024);
        
        // Write a message
        ByteBuffer message = ByteBuffer.allocate(50);
        for (int i = 0; i < 50; i++) {
            message.put((byte) (i * 2));
        }
        message.flip();
        
        int written = buffer.writeMessage(message);
        assertEquals(54, written); // 4 bytes for size + 50 bytes for data
        
        // Read the message back
        ByteBuffer readMessage = buffer.readMessage();
        assertNotNull(readMessage);
        assertEquals(50, readMessage.remaining());
        
        for (int i = 0; i < 50; i++) {
            assertEquals((byte) (i * 2), readMessage.get());
        }
    }

    @Test
    public void testWrapAround() {
        RingBuffer buffer = new RingBuffer(1024);
        
        // Fill the buffer completely
        ByteBuffer data = ByteBuffer.allocate(buffer.getDataCapacity());
        for (int i = 0; i < buffer.getDataCapacity(); i++) {
            data.put((byte) (i % 256));
        }
        data.flip();
        
        int written = buffer.write(data);
        assertEquals(buffer.getDataCapacity(), written);
        assertTrue(buffer.isFull());
        
        // Clear and write again
        buffer.clear();
        assertTrue(buffer.isEmpty());
    }

    @Test
    public void testGetAvailableSpace() {
        RingBuffer buffer = new RingBuffer(1024);
        
        assertEquals(buffer.getDataCapacity(), buffer.getAvailableSpace());
        
        // Write some data
        ByteBuffer data = ByteBuffer.allocate(100);
        for (int i = 0; i < 100; i++) {
            data.put((byte) i);
        }
        data.flip();
        
        buffer.write(data);
        assertEquals(buffer.getDataCapacity() - 100, buffer.getAvailableSpace());
    }

    @Test
    public void testGetAvailableData() {
        RingBuffer buffer = new RingBuffer(1024);
        
        assertEquals(0, buffer.getAvailableData());
        
        // Write some data
        ByteBuffer data = ByteBuffer.allocate(100);
        for (int i = 0; i < 100; i++) {
            data.put((byte) i);
        }
        data.flip();
        
        buffer.write(data);
        assertEquals(100, buffer.getAvailableData());
    }

    @Test
    public void testClear() {
        RingBuffer buffer = new RingBuffer(1024);
        
        // Write some data
        ByteBuffer data = ByteBuffer.allocate(100);
        for (int i = 0; i < 100; i++) {
            data.put((byte) i);
        }
        data.flip();
        
        buffer.write(data);
        assertFalse(buffer.isEmpty());
        
        // Clear the buffer
        buffer.clear();
        assertTrue(buffer.isEmpty());
        assertEquals(buffer.getDataCapacity(), buffer.getAvailableSpace());
    }

    @Test
    public void testToString() {
        RingBuffer buffer = new RingBuffer(1024);
        String str = buffer.toString();
        
        assertTrue(str.contains("RingBuffer"));
        assertTrue(str.contains("capacity="));
        assertTrue(str.contains("availableSpace="));
    }
}
