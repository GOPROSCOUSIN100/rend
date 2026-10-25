package renderbridge.protocol;

import java.nio.ByteBuffer;

/**
 * Base class for all bridge protocol messages.
 * Provides common functionality for message serialization and deserialization.
 */
public abstract class Message {

    protected MessageHeader header;

    /**
     * Create a new message with the specified message ID.
     *
     * @param messageId the message type ID
     */
    protected Message(int messageId) {
        this.header = new MessageHeader(messageId, 0);
    }

    /**
     * Create a message from a header and payload buffer.
     *
     * @param header the message header
     * @param payload the message payload
     */
    protected Message(MessageHeader header, ByteBuffer payload) {
        this.header = header;
        readPayload(payload);
    }

    /**
     * Get the message ID.
     *
     * @return the message type ID
     */
    public int getMessageId() {
        return header.getMessageId();
    }

    /**
     * Get the message header.
     *
     * @return the message header
     */
    public MessageHeader getHeader() {
        return header;
    }

    /**
     * Get the payload size.
     *
     * @return the payload size in bytes
     */
    public int getPayloadSize() {
        return header.getPayloadSize();
    }

    /**
     * Get the total message size (header + payload).
     *
     * @return the total message size in bytes
     */
    public int getTotalSize() {
        return MessageHeader.SIZE + getPayloadSize();
    }

    /**
     * Read the message payload from a byte buffer.
     *
     * @param buffer the byte buffer containing the payload
     */
    protected abstract void readPayload(ByteBuffer buffer);

    /**
     * Write the message payload to a byte buffer.
     *
     * @param buffer the byte buffer to write to
     */
    protected abstract void writePayload(ByteBuffer buffer);

    /**
     * Serialize the entire message (header + payload) to a byte buffer.
     *
     * @return a byte buffer containing the serialized message
     */
    public ByteBuffer serialize() {
        // First, calculate the payload size
        ByteBuffer sizeBuffer = ByteBuffer.allocate(getPayloadSize());
        writePayload(sizeBuffer);
        sizeBuffer.flip();
        
        int actualPayloadSize = sizeBuffer.remaining();
        header.setPayloadSize(actualPayloadSize);
        
        // Allocate buffer for header + payload
        ByteBuffer buffer = ByteBuffer.allocate(MessageHeader.SIZE + actualPayloadSize);
        
        // Write header
        header.write(buffer);
        
        // Write payload
        sizeBuffer.rewind();
        buffer.put(sizeBuffer);
        
        buffer.flip();
        return buffer;
    }

    /**
     * Deserialize a message from a byte buffer.
     *
     * @param buffer the byte buffer containing the serialized message
     * @return the deserialized message
     */
    public static Message deserialize(ByteBuffer buffer) {
        MessageHeader header = MessageHeader.read(buffer);
        
        // Create appropriate message based on type
        Message message = MessageFactory.create(header.getMessageId(), header.getVersion());
        if (message == null) {
            throw new IllegalArgumentException("Unknown message type: 0x" + Integer.toHexString(header.getMessageId()));
        }
        
        // Read payload
        ByteBuffer payload = buffer.slice();
        payload.limit(header.getPayloadSize());
        message.readPayload(payload);
        
        return message;
    }

    /**
     * Check if this message requires acknowledgment.
     *
     * @return true if acknowledgment is required
     */
    public boolean requiresAck() {
        return header.requiresAck();
    }

    @Override
    public String toString() {
        return String.format("%s{header=%s}", getClass().getSimpleName(), header);
    }
}
