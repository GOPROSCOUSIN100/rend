package renderbridge.protocol.messages;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageType;

import java.nio.ByteBuffer;

/**
 * Acknowledgment message for confirming receipt of a message.
 */
public class AckMessage extends Message {

    private int acknowledgedSequence;
    private int acknowledgedMessageId;

    /**
     * Create a new ACK message.
     */
    public AckMessage() {
        super(MessageType.ACK);
    }

    /**
     * Create an ACK message with the specified version.
     *
     * @param version the protocol version
     */
    public AckMessage(int version) {
        super(MessageType.ACK);
    }

    /**
     * Create an ACK message from a header and payload buffer.
     *
     * @param version the protocol version
     * @param payload the payload buffer
     */
    public AckMessage(int version, ByteBuffer payload) {
        super(MessageType.ACK);
        readPayload(payload);
    }

    /**
     * Create an ACK message for a specific message.
     *
     * @param sequence the sequence number to acknowledge
     * @param messageId the message ID to acknowledge
     */
    public AckMessage(int sequence, int messageId) {
        super(MessageType.ACK);
        this.acknowledgedSequence = sequence;
        this.acknowledgedMessageId = messageId;
    }

    @Override
    protected void readPayload(ByteBuffer buffer) {
        acknowledgedSequence = buffer.getInt();
        acknowledgedMessageId = buffer.getInt();
    }

    @Override
    protected void writePayload(ByteBuffer buffer) {
        buffer.putInt(acknowledgedSequence);
        buffer.putInt(acknowledgedMessageId);
    }

    @Override
    public int getPayloadSize() {
        // acknowledgedSequence(4) + acknowledgedMessageId(4)
        return 4 + 4;
    }

    // Getters and setters

    public int getAcknowledgedSequence() {
        return acknowledgedSequence;
    }

    public void setAcknowledgedSequence(int acknowledgedSequence) {
        this.acknowledgedSequence = acknowledgedSequence;
    }

    public int getAcknowledgedMessageId() {
        return acknowledgedMessageId;
    }

    public void setAcknowledgedMessageId(int acknowledgedMessageId) {
        this.acknowledgedMessageId = acknowledgedMessageId;
    }

    @Override
    public String toString() {
        return String.format(
                "AckMessage{acknowledgedSequence=%d, acknowledgedMessageId=0x%04X}",
                acknowledgedSequence, acknowledgedMessageId
        );
    }
}
