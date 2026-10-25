package renderbridge.protocol.messages;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageType;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

/**
 * Echo message for roundtrip testing.
 * Used to measure latency between client and renderer.
 */
public class EchoMessage extends Message {

    // Maximum echo data size
    private static final int MAX_ECHO_DATA_SIZE = 255;

    private long timestamp;
    private int sequence;
    private byte[] data;

    /**
     * Create a new echo message.
     */
    public EchoMessage() {
        super(MessageType.ECHO);
    }

    /**
     * Create an echo message with the specified version.
     *
     * @param version the protocol version
     */
    public EchoMessage(int version) {
        super(MessageType.ECHO);
    }

    /**
     * Create an echo message from a header and payload buffer.
     *
     * @param version the protocol version
     * @param payload the payload buffer
     */
    public EchoMessage(int version, ByteBuffer payload) {
        super(MessageType.ECHO);
        readPayload(payload);
    }

    /**
     * Create an echo message with a timestamp and sequence number.
     *
     * @param timestamp the timestamp in nanoseconds
     * @param sequence the sequence number
     */
    public EchoMessage(long timestamp, int sequence) {
        super(MessageType.ECHO);
        this.timestamp = timestamp;
        this.sequence = sequence;
    }

    @Override
    protected void readPayload(ByteBuffer buffer) {
        timestamp = buffer.getLong();
        sequence = buffer.getInt();
        
        // Read data size (uint8)
        byte dataSize = buffer.get();
        data = new byte[dataSize];
        buffer.get(data);
    }

    @Override
    protected void writePayload(ByteBuffer buffer) {
        buffer.putLong(timestamp);
        buffer.putInt(sequence);
        
        // Write data
        if (data != null) {
            byte dataSize = (byte) Math.min(data.length, MAX_ECHO_DATA_SIZE);
            buffer.put(dataSize);
            buffer.put(data, 0, dataSize);
        } else {
            buffer.put((byte) 0);
        }
    }

    @Override
    public int getPayloadSize() {
        // timestamp(8) + sequence(4) + dataSize(1) + data.length
        int dataSize = data != null ? Math.min(data.length, MAX_ECHO_DATA_SIZE) : 0;
        return 8 + 4 + 1 + dataSize;
    }

    // Getters and setters

    public long getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(long timestamp) {
        this.timestamp = timestamp;
    }

    public int getSequence() {
        return sequence;
    }

    public void setSequence(int sequence) {
        this.sequence = sequence;
    }

    public byte[] getData() {
        return data;
    }

    public void setData(byte[] data) {
        this.data = data;
    }

    public void setData(String text) {
        this.data = text != null ? text.getBytes(StandardCharsets.UTF_8) : null;
    }

    /**
     * Calculate the roundtrip time in nanoseconds.
     *
     * @param currentTimestamp the current timestamp in nanoseconds
     * @return the roundtrip time in nanoseconds
     */
    public long calculateRoundtripTime(long currentTimestamp) {
        return currentTimestamp - timestamp;
    }

    /**
     * Calculate the roundtrip time in milliseconds.
     *
     * @param currentTimestamp the current timestamp in nanoseconds
     * @return the roundtrip time in milliseconds
     */
    public float calculateRoundtripTimeMs(long currentTimestamp) {
        return (currentTimestamp - timestamp) / 1_000_000.0f;
    }

    @Override
    public String toString() {
        return String.format(
                "EchoMessage{timestamp=%d, sequence=%d, dataSize=%d}",
                timestamp, sequence, data != null ? data.length : 0
        );
    }
}
