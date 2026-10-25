package renderbridge.protocol.messages;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageType;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

/**
 * Error message for reporting protocol or connection errors.
 */
public class ErrorMessage extends Message {

    // Error codes
    public static final int ERR_UNKNOWN = 0;
    public static final int ERR_PROTOCOL_VERSION = 1;
    public static final int ERR_INVALID_MESSAGE = 2;
    public static final int ERR_CHECKSUM_MISMATCH = 3;
    public static final int ERR_CONNECTION_TIMEOUT = 4;
    public static final int ERR_TRANSPORT_ERROR = 5;
    public static final int ERR_UNSUPPORTED_FEATURE = 6;
    public static final int ERR_RESOURCE_EXHAUSTED = 7;

    // Maximum error message length
    private static final int MAX_ERROR_MESSAGE_LENGTH = 255;

    private int errorCode;
    private String errorMessage;

    /**
     * Create a new error message.
     */
    public ErrorMessage() {
        super(MessageType.ERROR);
    }

    /**
     * Create an error message with the specified version.
     *
     * @param version the protocol version
     */
    public ErrorMessage(int version) {
        super(MessageType.ERROR);
    }

    /**
     * Create an error message from a header and payload buffer.
     *
     * @param version the protocol version
     * @param payload the payload buffer
     */
    public ErrorMessage(int version, ByteBuffer payload) {
        super(MessageType.ERROR);
        readPayload(payload);
    }

    /**
     * Create an error message with an error code and message.
     *
     * @param errorCode the error code
     * @param errorMessage the error message
     */
    public ErrorMessage(int errorCode, String errorMessage) {
        super(MessageType.ERROR);
        this.errorCode = errorCode;
        this.errorMessage = errorMessage;
    }

    @Override
    protected void readPayload(ByteBuffer buffer) {
        errorCode = buffer.getInt();
        
        // Read error message (length-prefixed string)
        byte messageLength = buffer.get();
        byte[] messageBytes = new byte[messageLength];
        buffer.get(messageBytes);
        errorMessage = new String(messageBytes, StandardCharsets.UTF_8);
    }

    @Override
    protected void writePayload(ByteBuffer buffer) {
        buffer.putInt(errorCode);
        
        // Write error message (length-prefixed)
        byte[] messageBytes = errorMessage != null ? errorMessage.getBytes(StandardCharsets.UTF_8) : new byte[0];
        byte messageLength = (byte) Math.min(messageBytes.length, MAX_ERROR_MESSAGE_LENGTH);
        buffer.put(messageLength);
        buffer.put(messageBytes, 0, messageLength);
    }

    @Override
    public int getPayloadSize() {
        // errorCode(4) + messageLength(1) + message(up to 255)
        int messageLength = errorMessage != null ? Math.min(errorMessage.length(), MAX_ERROR_MESSAGE_LENGTH) : 0;
        return 4 + 1 + messageLength;
    }

    // Getters and setters

    public int getErrorCode() {
        return errorCode;
    }

    public void setErrorCode(int errorCode) {
        this.errorCode = errorCode;
    }

    public String getErrorMessage() {
        return errorMessage;
    }

    public void setErrorMessage(String errorMessage) {
        this.errorMessage = errorMessage;
    }

    /**
     * Get a human-readable error description.
     *
     * @return the error description
     */
    public String getErrorDescription() {
        switch (errorCode) {
            case ERR_UNKNOWN:
                return "Unknown error";
            case ERR_PROTOCOL_VERSION:
                return "Protocol version mismatch";
            case ERR_INVALID_MESSAGE:
                return "Invalid message type or format";
            case ERR_CHECKSUM_MISMATCH:
                return "Checksum mismatch";
            case ERR_CONNECTION_TIMEOUT:
                return "Connection timeout";
            case ERR_TRANSPORT_ERROR:
                return "Transport layer error";
            case ERR_UNSUPPORTED_FEATURE:
                return "Unsupported feature";
            case ERR_RESOURCE_EXHAUSTED:
                return "Resource exhausted";
            default:
                return "Unknown error code: " + errorCode;
        }
    }

    @Override
    public String toString() {
        return String.format(
                "ErrorMessage{errorCode=%d, errorMessage='%s', description='%s'}",
                errorCode, errorMessage, getErrorDescription()
        );
    }
}
