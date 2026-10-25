package renderbridge.protocol.messages;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageType;

import java.nio.ByteBuffer;

/**
 * Message indicating the end of a frame.
 * Sent after all frame data has been transmitted.
 */
public class FrameEndMessage extends Message {

    private long frameNumber;
    private long timestamp;
    private int messageCount;
    private long processingTimeNanos;

    /**
     * Create a new frame end message.
     */
    public FrameEndMessage() {
        super(MessageType.FRAME_END);
    }

    /**
     * Create a frame end message with the specified version.
     *
     * @param version the protocol version
     */
    public FrameEndMessage(int version) {
        super(MessageType.FRAME_END);
    }

    /**
     * Create a frame end message from a header and payload buffer.
     *
     * @param version the protocol version
     * @param payload the payload buffer
     */
    public FrameEndMessage(int version, ByteBuffer payload) {
        super(MessageType.FRAME_END);
        readPayload(payload);
    }

    /**
     * Create a frame end message with frame data.
     *
     * @param frameNumber the frame number
     * @param timestamp the timestamp in nanoseconds
     * @param messageCount the number of messages in this frame
     * @param processingTimeNanos the time taken to process the frame
     */
    public FrameEndMessage(long frameNumber, long timestamp, int messageCount, long processingTimeNanos) {
        super(MessageType.FRAME_END);
        this.frameNumber = frameNumber;
        this.timestamp = timestamp;
        this.messageCount = messageCount;
        this.processingTimeNanos = processingTimeNanos;
    }

    @Override
    protected void readPayload(ByteBuffer buffer) {
        frameNumber = buffer.getLong();
        timestamp = buffer.getLong();
        messageCount = buffer.getInt();
        processingTimeNanos = buffer.getLong();
    }

    @Override
    protected void writePayload(ByteBuffer buffer) {
        buffer.putLong(frameNumber);
        buffer.putLong(timestamp);
        buffer.putInt(messageCount);
        buffer.putLong(processingTimeNanos);
    }

    @Override
    public int getPayloadSize() {
        // frameNumber(8) + timestamp(8) + messageCount(4) + processingTimeNanos(8)
        return 8 + 8 + 4 + 8;
    }

    // Getters and setters

    public long getFrameNumber() {
        return frameNumber;
    }

    public void setFrameNumber(long frameNumber) {
        this.frameNumber = frameNumber;
    }

    public long getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(long timestamp) {
        this.timestamp = timestamp;
    }

    public int getMessageCount() {
        return messageCount;
    }

    public void setMessageCount(int messageCount) {
        this.messageCount = messageCount;
    }

    public long getProcessingTimeNanos() {
        return processingTimeNanos;
    }

    public void setProcessingTimeNanos(long processingTimeNanos) {
        this.processingTimeNanos = processingTimeNanos;
    }

    /**
     * Get the processing time in milliseconds.
     *
     * @return the processing time in milliseconds
     */
    public float getProcessingTimeMs() {
        return processingTimeNanos / 1_000_000.0f;
    }

    @Override
    public String toString() {
        return String.format(
                "FrameEndMessage{frameNumber=%d, timestamp=%d, messageCount=%d, processingTimeNanos=%d}",
                frameNumber, timestamp, messageCount, processingTimeNanos
        );
    }
}
