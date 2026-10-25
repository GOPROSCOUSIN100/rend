package renderbridge.protocol;

import renderbridge.protocol.messages.CameraMessage;
import renderbridge.protocol.messages.ChunkMessage;
import renderbridge.protocol.messages.EchoMessage;
import renderbridge.protocol.messages.ErrorMessage;
import renderbridge.protocol.messages.FrameEndMessage;
import renderbridge.protocol.messages.FrameStartMessage;
import renderbridge.protocol.messages.HandshakeMessage;
import renderbridge.protocol.messages.HeartbeatMessage;
import renderbridge.protocol.messages.AckMessage;

import org.junit.Test;

import java.nio.ByteBuffer;

import static org.junit.Assert.*;

/**
 * Unit tests for message serialization and deserialization.
 */
public class MessageSerializationTest {

    @Test
    public void testHandshakeMessageSerialization() {
        HandshakeMessage original = new HandshakeMessage();
        original.setProtocolVersion((byte) 1);
        original.setCapabilities(HandshakeMessage.CAPABILITY_SHARED_MEMORY | HandshakeMessage.CAPABILITY_SOCKET);
        original.setClientType(HandshakeMessage.CLIENT_TYPE_MOD);
        original.setClientVersion(0x00010000);
        original.setClientName("TestClient");

        ByteBuffer serialized = original.serialize();
        
        MessageHeader header = MessageHeader.read(serialized);
        assertEquals(MessageType.HANDSHAKE, header.getMessageId());
        
        Message deserialized = MessageFactory.create(header.getMessageId(), header.getVersion());
        assertNotNull(deserialized);
        assertTrue(deserialized instanceof HandshakeMessage);
        
        HandshakeMessage read = (HandshakeMessage) deserialized;
        ByteBuffer payload = serialized.slice();
        payload.limit(header.getPayloadSize());
        read.readPayload(payload);
        
        assertEquals(original.getProtocolVersion(), read.getProtocolVersion());
        assertEquals(original.getCapabilities(), read.getCapabilities());
        assertEquals(original.getClientType(), read.getClientType());
        assertEquals(original.getClientVersion(), read.getClientVersion());
        assertEquals(original.getClientName(), read.getClientName());
    }

    @Test
    public void testHeartbeatMessageSerialization() {
        HeartbeatMessage original = new HeartbeatMessage();
        original.setSequenceNumber(42);
        original.setLastFrameProcessed(1000);
        original.setAvgProcessingTimeMs(16.67f);

        ByteBuffer serialized = original.serialize();
        
        MessageHeader header = MessageHeader.read(serialized);
        assertEquals(MessageType.HEARTBEAT, header.getMessageId());
        
        HeartbeatMessage read = new HeartbeatMessage(header.getVersion(), serialized.slice().limit(header.getPayloadSize()));
        
        assertEquals(original.getSequenceNumber(), read.getSequenceNumber());
        assertEquals(original.getLastFrameProcessed(), read.getLastFrameProcessed());
        assertEquals(original.getAvgProcessingTimeMs(), read.getAvgProcessingTimeMs(), 0.001f);
    }

    @Test
    public void testAckMessageSerialization() {
        AckMessage original = new AckMessage(100, 0x0200);

        ByteBuffer serialized = original.serialize();
        
        MessageHeader header = MessageHeader.read(serialized);
        assertEquals(MessageType.ACK, header.getMessageId());
        
        AckMessage read = new AckMessage(header.getVersion(), serialized.slice().limit(header.getPayloadSize()));
        
        assertEquals(original.getAcknowledgedSequence(), read.getAcknowledgedSequence());
        assertEquals(original.getAcknowledgedMessageId(), read.getAcknowledgedMessageId());
    }

    @Test
    public void testErrorMessageSerialization() {
        ErrorMessage original = new ErrorMessage(ErrorMessage.ERR_PROTOCOL_VERSION, "Test error");

        ByteBuffer serialized = original.serialize();
        
        MessageHeader header = MessageHeader.read(serialized);
        assertEquals(MessageType.ERROR, header.getMessageId());
        
        ErrorMessage read = new ErrorMessage(header.getVersion(), serialized.slice().limit(header.getPayloadSize()));
        
        assertEquals(original.getErrorCode(), read.getErrorCode());
        assertEquals(original.getErrorMessage(), read.getErrorMessage());
    }

    @Test
    public void testFrameStartMessageSerialization() {
        FrameStartMessage original = new FrameStartMessage(1000, System.nanoTime(), 0x01);

        ByteBuffer serialized = original.serialize();
        
        MessageHeader header = MessageHeader.read(serialized);
        assertEquals(MessageType.FRAME_START, header.getMessageId());
        
        FrameStartMessage read = new FrameStartMessage(header.getVersion(), serialized.slice().limit(header.getPayloadSize()));
        
        assertEquals(original.getFrameNumber(), read.getFrameNumber());
        assertEquals(original.getFlags(), read.getFlags());
    }

    @Test
    public void testFrameEndMessageSerialization() {
        FrameEndMessage original = new FrameEndMessage(1000, System.nanoTime(), 10, 16_000_000);

        ByteBuffer serialized = original.serialize();
        
        MessageHeader header = MessageHeader.read(serialized);
        assertEquals(MessageType.FRAME_END, header.getMessageId());
        
        FrameEndMessage read = new FrameEndMessage(header.getVersion(), serialized.slice().limit(header.getPayloadSize()));
        
        assertEquals(original.getFrameNumber(), read.getFrameNumber());
        assertEquals(original.getMessageCount(), read.getMessageCount());
        assertEquals(original.getProcessingTimeNanos(), read.getProcessingTimeNanos());
    }

    @Test
    public void testCameraMessageSerialization() {
        CameraMessage original = new CameraMessage();
        original.setFrameNumber(1000);
        original.setTimestamp(System.nanoTime());
        original.setPosition(10.0f, 5.0f, 20.0f);
        original.setRotation(0.0f, 90.0f, 0.0f);
        original.setFov(70.0f);
        original.setNearPlane(0.1f);
        original.setFarPlane(1000.0f);
        
        // Set simple matrices
        float[] viewMatrix = new float[16];
        float[] projectionMatrix = new float[16];
        for (int i = 0; i < 16; i++) {
            viewMatrix[i] = i * 0.1f;
            projectionMatrix[i] = i * 0.2f;
        }
        original.setViewMatrix(viewMatrix);
        original.setProjectionMatrix(projectionMatrix);
        
        original.setViewport(0, 0, 800, 600);

        ByteBuffer serialized = original.serialize();
        
        MessageHeader header = MessageHeader.read(serialized);
        assertEquals(MessageType.CAMERA, header.getMessageId());
        
        CameraMessage read = new CameraMessage(header.getVersion(), serialized.slice().limit(header.getPayloadSize()));
        
        assertEquals(original.getFrameNumber(), read.getFrameNumber());
        assertEquals(original.getPositionX(), read.getPositionX(), 0.001f);
        assertEquals(original.getPositionY(), read.getPositionY(), 0.001f);
        assertEquals(original.getPositionZ(), read.getPositionZ(), 0.001f);
        assertEquals(original.getRotationPitch(), read.getRotationPitch(), 0.001f);
        assertEquals(original.getRotationYaw(), read.getRotationYaw(), 0.001f);
        assertEquals(original.getFov(), read.getFov(), 0.001f);
        assertEquals(original.getViewportWidth(), read.getViewportWidth());
        assertEquals(original.getViewportHeight(), read.getViewportHeight());
    }

    @Test
    public void testChunkMessageSerialization() {
        ChunkMessage original = new ChunkMessage();
        original.setChunkPosition(10, (byte) 5, 20);
        original.setSectionMask((short) 0xFFFF); // All sections present
        
        // Create simple block data (one block)
        byte[] blockData = new byte[3];
        blockData[0] = (byte) (1 & 0xFF); // Block ID high byte
        blockData[1] = (byte) (0 & 0xFF); // Block ID low byte
        blockData[2] = (byte) 0; // Metadata
        original.setBlockData(blockData);
        
        // Create simple light data (one block)
        byte[] lightData = new byte[2];
        lightData[0] = (byte) 15; // Sky light
        lightData[1] = (byte) 10; // Block light
        original.setLightData(lightData);

        ByteBuffer serialized = original.serialize();
        
        MessageHeader header = MessageHeader.read(serialized);
        assertEquals(MessageType.CHUNK, header.getMessageId());
        
        ChunkMessage read = new ChunkMessage(header.getVersion(), serialized.slice().limit(header.getPayloadSize()));
        
        assertEquals(original.getChunkX(), read.getChunkX());
        assertEquals(original.getChunkY(), read.getChunkY());
        assertEquals(original.getChunkZ(), read.getChunkZ());
        assertEquals(original.getSectionMask(), read.getSectionMask());
    }

    @Test
    public void testEchoMessageSerialization() {
        EchoMessage original = new EchoMessage(System.nanoTime(), 42);
        original.setData("Test echo data");

        ByteBuffer serialized = original.serialize();
        
        MessageHeader header = MessageHeader.read(serialized);
        assertEquals(MessageType.ECHO, header.getMessageId());
        
        EchoMessage read = new EchoMessage(header.getVersion(), serialized.slice().limit(header.getPayloadSize()));
        
        assertEquals(original.getSequence(), read.getSequence());
        assertEquals(new String(original.getData()), new String(read.getData()));
    }

    @Test
    public void testMessageFactory() {
        assertTrue(MessageFactory.isSupported(MessageType.HANDSHAKE));
        assertTrue(MessageFactory.isSupported(MessageType.CAMERA));
        assertFalse(MessageFactory.isSupported(0x9999));
        
        Message message = MessageFactory.create(MessageType.HANDSHAKE, 1);
        assertNotNull(message);
        assertTrue(message instanceof HandshakeMessage);
    }

    @Test
    public void testDeserializeMessage() {
        // Create and serialize a message
        HandshakeMessage original = new HandshakeMessage();
        original.setProtocolVersion((byte) 1);
        original.setClientName("Test");
        
        ByteBuffer serialized = original.serialize();
        serialized.rewind();
        
        // Deserialize using Message.deserialize
        Message deserialized = Message.deserialize(serialized);
        assertNotNull(deserialized);
        assertTrue(deserialized instanceof HandshakeMessage);
        
        HandshakeMessage read = (HandshakeMessage) deserialized;
        assertEquals(original.getClientName(), read.getClientName());
    }
}
