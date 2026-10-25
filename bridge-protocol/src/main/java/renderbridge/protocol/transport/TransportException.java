package renderbridge.protocol.transport;

/**
 * Exception thrown when a transport error occurs.
 */
public class TransportException extends Exception {

    public TransportException() {
        super();
    }

    public TransportException(String message) {
        super(message);
    }

    public TransportException(String message, Throwable cause) {
        super(message, cause);
    }

    public TransportException(Throwable cause) {
        super(cause);
    }

    /**
     * Create a transport exception with a specific error code.
     *
     * @param message the error message
     * @param errorCode the error code
     */
    public TransportException(String message, int errorCode) {
        super(message);
        this.errorCode = errorCode;
    }

    private int errorCode;

    /**
     * Get the error code associated with this exception.
     *
     * @return the error code
     */
    public int getErrorCode() {
        return errorCode;
    }

    /**
     * Set the error code for this exception.
     *
     * @param errorCode the error code
     */
    public void setErrorCode(int errorCode) {
        this.errorCode = errorCode;
    }
}
