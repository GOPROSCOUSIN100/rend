package renderbridge.protocol;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Header for all bridge protocol messages.
 * Size: 16 bytes
 * Byte order: Big-Endian (Network Byte Order)
 */
public final class MessageHeader {

    // Header size in bytes
    public static final int SIZE = 16;

    // Protocol version
    public static final byte PROTOCOL_VERSION = 1;

    // Header offsets
    private static final int OFFSET_MESSAGE_ID = 0;
    private static final int OFFSET_PAYLOAD_SIZE = 4;
    private static final int OFFSET_VERSION = 8;
    private static final int OFFSET_FLAGS = 9;
    private static final int OFFSET_CHECKSUM = 10;
    private static final int OFFSET_SEQUENCE = 12;

    // Message flags
    public static final byte FLAG_NONE = 0x00;
    public static final byte FLAG_COMPRESSED = 0x01;
    public static final byte FLAG_ENCRYPTED = 0x02;
    public static final byte FLAG_REQUIRES_ACK = 0x04;

    private int messageId;
    private int payloadSize;
    private byte version;
    private byte flags;
    private short checksum;
    private int sequence;

    /**
     * Create a new message header.
     */
    public MessageHeader() {
        this.version = PROTOCOL_VERSION;
        this.flags = FLAG_NONE;
    }

    /**
     * Create a message header with the specified message ID and payload size.
     *
     * @param messageId the message type ID
     * @param payloadSize the size of the message payload
     */
    public MessageHeader(int messageId, int payloadSize) {
        this();
        this.messageId = messageId;
        this.payloadSize = payloadSize;
    }

    /**
     * Read a message header from a byte buffer.
     *
     * @param buffer the byte buffer to read from
     * @return the message header
     */
    public static MessageHeader read(ByteBuffer buffer) {
        buffer.order(ByteOrder.BIG_ENDIAN);
        
        MessageHeader header = new MessageHeader();
        header.messageId = buffer.getInt(OFFSET_MESSAGE_ID);
        header.payloadSize = buffer.getInt(OFFSET_PAYLOAD_SIZE);
        header.version = buffer.get(OFFSET_VERSION);
        header.flags = buffer.get(OFFSET_FLAGS);
        header.checksum = buffer.getShort(OFFSET_CHECKSUM);
        header.sequence = buffer.getInt(OFFSET_SEQUENCE);
        
        return header;
    }

    /**
     * Write this message header to a byte buffer.
     *
     * @param buffer the byte buffer to write to
     */
    public void write(ByteBuffer buffer) {
        buffer.order(ByteOrder.BIG_ENDIAN);
        buffer.putInt(OFFSET_MESSAGE_ID, messageId);
        buffer.putInt(OFFSET_PAYLOAD_SIZE, payloadSize);
        buffer.put(OFFSET_VERSION, version);
        buffer.put(OFFSET_FLAGS, flags);
        buffer.putShort(OFFSET_CHECKSUM, checksum);
        buffer.putInt(OFFSET_SEQUENCE, sequence);
    }

    /**
     * Calculate the total message size (header + payload).
     *
     * @return the total message size
     */
    public int getTotalSize() {
        return SIZE + payloadSize;
    }

    /**
     * Check if this header is valid.
     *
     * @return true if the header is valid
     */
    public boolean isValid() {
        return messageId > 0 && payloadSize >= 0 && version == PROTOCOL_VERSION;
    }

    // Getters and setters

    public int getMessageId() {
        return messageId;
    }

    public void setMessageId(int messageId) {
        this.messageId = messageId;
    }

    public int getPayloadSize() {
        return payloadSize;
    }

    public void setPayloadSize(int payloadSize) {
        this.payloadSize = payloadSize;
    }

    public byte getVersion() {
        return version;
    }

    public void setVersion(byte version) {
        this.version = version;
    }

    public byte getFlags() {
        return flags;
    }

    public void setFlags(byte flags) {
        this.flags = flags;
    }

    public short getChecksum() {
        return checksum;
    }

    public void setChecksum(short checksum) {
        this.checksum = checksum;
    }

    public int getSequence() {
        return sequence;
    }

    public void setSequence(int sequence) {
        this.sequence = sequence;
    }

    /**
     * Check if the message requires acknowledgment.
     *
     * @return true if acknowledgment is required
     */
    public boolean requiresAck() {
        return (flags & FLAG_REQUIRES_ACK) != 0;
    }

    /**
     * Check if the message payload is compressed.
     *
     * @return true if the payload is compressed
     */
    public boolean isCompressed() {
        return (flags & FLAG_COMPRESSED) != 0;
    }

    /**
     * Check if the message payload is encrypted.
     *
     * @return true if the payload is encrypted
     */
    public boolean isEncrypted() {
        return (flags & FLAG_ENCRYPTED) != 0;
    }

    @Override
    public String toString() {
        return String.format(
                "MessageHeader{messageId=0x%04X, payloadSize=%d, version=%d, flags=0x%02X, checksum=0x%04X, sequence=%d}",
                messageId, payloadSize, version, flags, checksum, sequence
        );
    }
}
