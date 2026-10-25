package renderbridge.protocol.messages;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageType;

import java.nio.ByteBuffer;

/**
 * Heartbeat message for keep-alive and connection monitoring.
 * Sent periodically to ensure the connection is still active.
 */
public class HeartbeatMessage extends Message {

    private int sequenceNumber;
    private long lastFrameProcessed;
    private float avgProcessingTimeMs;

    /**
     * Create a new heartbeat message.
     */
    public HeartbeatMessage() {
        super(MessageType.HEARTBEAT);
    }

    /**
     * Create a heartbeat message with the specified version.
     *
     * @param version the protocol version
     */
    public HeartbeatMessage(int version) {
        super(MessageType.HEARTBEAT);
    }

    /**
     * Create a heartbeat message from a header and payload buffer.
     *
     * @param version the protocol version
     * @param payload the payload buffer
     */
    public HeartbeatMessage(int version, ByteBuffer payload) {
        super(MessageType.HEARTBEAT);
        readPayload(payload);
    }

    @Override
    protected void readPayload(ByteBuffer buffer) {
        sequenceNumber = buffer.getInt();
        lastFrameProcessed = buffer.getLong();
        avgProcessingTimeMs = buffer.getFloat();
    }

    @Override
    protected void writePayload(ByteBuffer buffer) {
        buffer.putInt(sequenceNumber);
        buffer.putLong(lastFrameProcessed);
        buffer.putFloat(avgProcessingTimeMs);
    }

    @Override
    public int getPayloadSize() {
        // sequenceNumber(4) + lastFrameProcessed(8) + avgProcessingTimeMs(4)
        return 4 + 8 + 4;
    }

    // Getters and setters

    public int getSequenceNumber() {
        return sequenceNumber;
    }

    public void setSequenceNumber(int sequenceNumber) {
        this.sequenceNumber = sequenceNumber;
    }

    public long getLastFrameProcessed() {
        return lastFrameProcessed;
    }

    public void setLastFrameProcessed(long lastFrameProcessed) {
        this.lastFrameProcessed = lastFrameProcessed;
    }

    public float getAvgProcessingTimeMs() {
        return avgProcessingTimeMs;
    }

    public void setAvgProcessingTimeMs(float avgProcessingTimeMs) {
        this.avgProcessingTimeMs = avgProcessingTimeMs;
    }

    @Override
    public String toString() {
        return String.format(
                "HeartbeatMessage{sequenceNumber=%d, lastFrameProcessed=%d, avgProcessingTimeMs=%.2f}",
                sequenceNumber, lastFrameProcessed, avgProcessingTimeMs
        );
    }
}
