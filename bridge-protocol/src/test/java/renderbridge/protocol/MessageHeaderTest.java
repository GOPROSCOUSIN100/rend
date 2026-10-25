package renderbridge.protocol;

import org.junit.Test;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import static org.junit.Assert.*;

/**
 * Unit tests for MessageHeader.
 */
public class MessageHeaderTest {

    @Test
    public void testDefaultConstructor() {
        MessageHeader header = new MessageHeader();
        assertEquals(0, header.getMessageId());
        assertEquals(0, header.getPayloadSize());
        assertEquals(MessageHeader.PROTOCOL_VERSION, header.getVersion());
        assertEquals(MessageHeader.FLAG_NONE, header.getFlags());
        assertEquals(0, header.getChecksum());
        assertEquals(0, header.getSequence());
    }

    @Test
    public void testParameterizedConstructor() {
        MessageHeader header = new MessageHeader(0x0200, 100);
        assertEquals(0x0200, header.getMessageId());
        assertEquals(100, header.getPayloadSize());
        assertEquals(MessageHeader.PROTOCOL_VERSION, header.getVersion());
    }

    @Test
    public void testGetTotalSize() {
        MessageHeader header = new MessageHeader(0x0200, 100);
        assertEquals(MessageHeader.SIZE + 100, header.getTotalSize());
    }

    @Test
    public void testIsValid() {
        MessageHeader validHeader = new MessageHeader(0x0200, 100);
        assertTrue(validHeader.isValid());

        MessageHeader invalidMessageId = new MessageHeader();
        assertFalse(invalidMessageId.isValid());

        MessageHeader invalidPayloadSize = new MessageHeader(0x0200, -1);
        assertFalse(invalidPayloadSize.isValid());

        MessageHeader invalidVersion = new MessageHeader(0x0200, 100);
        invalidVersion.setVersion((byte) 2);
        assertFalse(invalidVersion.isValid());
    }

    @Test
    public void testWriteAndRead() {
        MessageHeader original = new MessageHeader();
        original.setMessageId(0x0200);
        original.setPayloadSize(100);
        original.setVersion((byte) 1);
        original.setFlags((byte) 0x01);
        original.setChecksum((short) 0xABCD);
        original.setSequence(42);

        // Write to buffer
        ByteBuffer buffer = ByteBuffer.allocate(MessageHeader.SIZE);
        original.write(buffer);
        buffer.flip();

        // Read from buffer
        MessageHeader read = MessageHeader.read(buffer);

        assertEquals(original.getMessageId(), read.getMessageId());
        assertEquals(original.getPayloadSize(), read.getPayloadSize());
        assertEquals(original.getVersion(), read.getVersion());
        assertEquals(original.getFlags(), read.getFlags());
        assertEquals(original.getChecksum(), read.getChecksum());
        assertEquals(original.getSequence(), read.getSequence());
    }

    @Test
    public void testBigEndian() {
        MessageHeader header = new MessageHeader();
        header.setMessageId(0x12345678);
        header.setPayloadSize(0x9ABCDEF0);
        header.setChecksum((short) 0xABCD);
        header.setSequence(0x12345678);

        ByteBuffer buffer = ByteBuffer.allocate(MessageHeader.SIZE);
        header.write(buffer);
        buffer.flip();

        // Verify byte order is big-endian
        assertEquals(0x12, buffer.get(0));
        assertEquals(0x34, buffer.get(1));
        assertEquals(0x56, buffer.get(2));
        assertEquals(0x78, buffer.get(3));
    }

    @Test
    public void testFlags() {
        MessageHeader header = new MessageHeader();
        
        assertFalse(header.requiresAck());
        assertFalse(header.isCompressed());
        assertFalse(header.isEncrypted());

        header.setFlags(MessageHeader.FLAG_REQUIRES_ACK);
        assertTrue(header.requiresAck());
        assertFalse(header.isCompressed());
        assertFalse(header.isEncrypted());

        header.setFlags((byte) (MessageHeader.FLAG_COMPRESSED | MessageHeader.FLAG_ENCRYPTED));
        assertFalse(header.requiresAck());
        assertTrue(header.isCompressed());
        assertTrue(header.isEncrypted());
    }

    @Test
    public void testToString() {
        MessageHeader header = new MessageHeader(0x0200, 100);
        header.setSequence(42);
        
        String str = header.toString();
        assertTrue(str.contains("messageId=0x0200"));
        assertTrue(str.contains("payloadSize=100"));
        assertTrue(str.contains("sequence=42"));
    }
}
