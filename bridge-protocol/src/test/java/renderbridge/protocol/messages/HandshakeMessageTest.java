package renderbridge.protocol.messages;

import org.junit.Test;

import java.nio.ByteBuffer;

import static org.junit.Assert.*;

/**
 * Unit tests for HandshakeMessage.
 */
public class HandshakeMessageTest {

    @Test
    public void testDefaultConstructor() {
        HandshakeMessage message = new HandshakeMessage();
        assertEquals(1, message.getMessageId());
    }

    @Test
    public void testCapabilities() {
        HandshakeMessage message = new HandshakeMessage();
        
        assertFalse(message.hasCapability(HandshakeMessage.CAPABILITY_SHARED_MEMORY));
        assertFalse(message.hasCapability(HandshakeMessage.CAPABILITY_SOCKET));
        
        message.addCapability(HandshakeMessage.CAPABILITY_SHARED_MEMORY);
        assertTrue(message.hasCapability(HandshakeMessage.CAPABILITY_SHARED_MEMORY));
        assertFalse(message.hasCapability(HandshakeMessage.CAPABILITY_SOCKET));
        
        message.addCapability(HandshakeMessage.CAPABILITY_SOCKET);
        assertTrue(message.hasCapability(HandshakeMessage.CAPABILITY_SHARED_MEMORY));
        assertTrue(message.hasCapability(HandshakeMessage.CAPABILITY_SOCKET));
    }

    @Test
    public void testClientType() {
        HandshakeMessage message = new HandshakeMessage();
        
        message.setClientType(HandshakeMessage.CLIENT_TYPE_MOD);
        assertTrue(message.isModClient());
        assertFalse(message.isRendererClient());
        
        message.setClientType(HandshakeMessage.CLIENT_TYPE_RENDERER);
        assertFalse(message.isModClient());
        assertTrue(message.isRendererClient());
    }

    @Test
    public void testSerialization() {
        HandshakeMessage original = new HandshakeMessage();
        original.setProtocolVersion((byte) 1);
        original.setCapabilities(HandshakeMessage.CAPABILITY_SHARED_MEMORY | HandshakeMessage.CAPABILITY_SOCKET);
        original.setClientType(HandshakeMessage.CLIENT_TYPE_MOD);
        original.setClientVersion(0x00010000);
        original.setClientName("TestClient");

        ByteBuffer serialized = original.serialize();
        
        // Check payload size
        assertEquals(original.getPayloadSize(), serialized.remaining() - MessageHeader.SIZE);
        
        // Deserialize
        HandshakeMessage read = new HandshakeMessage(1, serialized.slice().limit(original.getPayloadSize()));
        
        assertEquals(original.getProtocolVersion(), read.getProtocolVersion());
        assertEquals(original.getCapabilities(), read.getCapabilities());
        assertEquals(original.getClientType(), read.getClientType());
        assertEquals(original.getClientVersion(), read.getClientVersion());
        assertEquals(original.getClientName(), read.getClientName());
    }

    @Test
    public void testToString() {
        HandshakeMessage message = new HandshakeMessage();
        message.setClientName("Test");
        
        String str = message.toString();
        assertTrue(str.contains("HandshakeMessage"));
        assertTrue(str.contains("clientName='Test'"));
    }
}
