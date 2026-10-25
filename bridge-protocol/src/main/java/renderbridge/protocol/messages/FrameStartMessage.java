package renderbridge.protocol.messages;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageType;

import java.nio.ByteBuffer;

/**
 * Message indicating the start of a new frame.
 * Sent at the beginning of each frame to synchronize rendering.
 */
public class FrameStartMessage extends Message {

    private long frameNumber;
    private long timestamp;
    private int flags;

    /**
     * Create a new frame start message.
     */
    public FrameStartMessage() {
        super(MessageType.FRAME_START);
    }

    /**
     * Create a frame start message with the specified version.
     *
     * @param version the protocol version
     */
    public FrameStartMessage(int version) {
        super(MessageType.FRAME_START);
    }

    /**
     * Create a frame start message from a header and payload buffer.
     *
     * @param version the protocol version
     * @param payload the payload buffer
     */
    public FrameStartMessage(int version, ByteBuffer payload) {
        super(MessageType.FRAME_START);
        readPayload(payload);
    }

    /**
     * Create a frame start message with frame data.
     *
     * @param frameNumber the frame number
     * @param timestamp the timestamp in nanoseconds
     * @param flags frame flags
     */
    public FrameStartMessage(long frameNumber, long timestamp, int flags) {
        super(MessageType.FRAME_START);
        this.frameNumber = frameNumber;
        this.timestamp = timestamp;
        this.flags = flags;
    }

    @Override
    protected void readPayload(ByteBuffer buffer) {
        frameNumber = buffer.getLong();
        timestamp = buffer.getLong();
        flags = buffer.getInt();
    }

    @Override
    protected void writePayload(ByteBuffer buffer) {
        buffer.putLong(frameNumber);
        buffer.putLong(timestamp);
        buffer.putInt(flags);
    }

    @Override
    public int getPayloadSize() {
        // frameNumber(8) + timestamp(8) + flags(4)
        return 8 + 8 + 4;
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

    public int getFlags() {
        return flags;
    }

    public void setFlags(int flags) {
        this.flags = flags;
    }

    @Override
    public String toString() {
        return String.format(
                "FrameStartMessage{frameNumber=%d, timestamp=%d, flags=0x%08X}",
                frameNumber, timestamp, flags
        );
    }
}
