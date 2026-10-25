package renderbridge.protocol;

import org.junit.Test;

import static org.junit.Assert.*;

/**
 * Unit tests for MessageType.
 */
public class MessageTypeTest {

    @Test
    public void testIsValid() {
        assertTrue(MessageType.isValid(MessageType.HANDSHAKE));
        assertTrue(MessageType.isValid(MessageType.HEARTBEAT));
        assertTrue(MessageType.isValid(MessageType.CAMERA));
        assertTrue(MessageType.isValid(MessageType.CHUNK));
        
        assertFalse(MessageType.isValid(0x0000));
        assertFalse(MessageType.isValid(0xFFFF));
        assertFalse(MessageType.isValid(0x1234));
    }

    @Test
    public void testIsControlMessage() {
        assertTrue(MessageType.isControlMessage(MessageType.HANDSHAKE));
        assertTrue(MessageType.isControlMessage(MessageType.HEARTBEAT));
        assertTrue(MessageType.isControlMessage(MessageType.ACK));
        assertTrue(MessageType.isControlMessage(MessageType.ERROR));
        assertTrue(MessageType.isControlMessage(MessageType.CONTROL));
        
        assertFalse(MessageType.isControlMessage(MessageType.CAMERA));
        assertFalse(MessageType.isControlMessage(MessageType.CHUNK));
    }

    @Test
    public void testIsFrameMessage() {
        assertTrue(MessageType.isFrameMessage(MessageType.FRAME_START));
        assertTrue(MessageType.isFrameMessage(MessageType.FRAME_END));
        
        assertFalse(MessageType.isFrameMessage(MessageType.HANDSHAKE));
        assertFalse(MessageType.isFrameMessage(MessageType.CAMERA));
    }

    @Test
    public void testIsDataMessage() {
        assertTrue(MessageType.isDataMessage(MessageType.CAMERA));
        assertTrue(MessageType.isDataMessage(MessageType.CHUNK));
        assertTrue(MessageType.isDataMessage(MessageType.ENTITY));
        assertTrue(MessageType.isDataMessage(MessageType.PARTICLE));
        assertTrue(MessageType.isDataMessage(MessageType.WEATHER));
        assertTrue(MessageType.isDataMessage(MessageType.GUI));
        
        assertFalse(MessageType.isDataMessage(MessageType.HANDSHAKE));
        assertFalse(MessageType.isDataMessage(MessageType.FRAME_START));
    }

    @Test
    public void testGetName() {
        assertEquals("HANDSHAKE", MessageType.getName(MessageType.HANDSHAKE));
        assertEquals("HEARTBEAT", MessageType.getName(MessageType.HEARTBEAT));
        assertEquals("CAMERA", MessageType.getName(MessageType.CAMERA));
        assertEquals("CHUNK", MessageType.getName(MessageType.CHUNK));
        
        assertEquals("UNKNOWN_0x1234", MessageType.getName(0x1234));
    }
}
