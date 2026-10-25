package renderbridge.protocol.transport;

import java.io.IOException;

/**
 * Exception thrown when a socket error occurs.
 */
public class SocketException extends IOException {

    public SocketException() {
        super();
    }

    public SocketException(String message) {
        super(message);
    }

    public SocketException(String message, Throwable cause) {
        super(message, cause);
    }

    public SocketException(Throwable cause) {
        super(cause);
    }
}
