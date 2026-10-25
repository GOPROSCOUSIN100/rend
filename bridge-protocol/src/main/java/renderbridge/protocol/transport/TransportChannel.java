package renderbridge.protocol.transport;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageHeader;

import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Interface defining the transport layer for the bridge protocol.
 * Implementations provide the actual transport mechanism (shared memory, socket, etc.).
 */
public interface TransportChannel {

    /**
     * Connect to the transport channel.
     *
     * @throws TransportException if connection fails
     * @throws IOException if an I/O error occurs
     */
    void connect() throws TransportException, IOException;

    /**
     * Disconnect from the transport channel.
     */
    void disconnect();

    /**
     * Check if the transport channel is connected.
     *
     * @return true if connected
     */
    boolean isConnected();

    /**
     * Send a message through the transport channel.
     *
     * @param message the message to send
     * @throws TransportException if sending fails
     * @throws IOException if an I/O error occurs
     */
    void send(Message message) throws TransportException, IOException;

    /**
     * Send raw data through the transport channel.
     *
     * @param data the data to send
     * @throws TransportException if sending fails
     * @throws IOException if an I/O error occurs
     */
    void send(ByteBuffer data) throws TransportException, IOException;

    /**
     * Receive a message from the transport channel.
     * This method blocks until a message is available or timeout occurs.
     *
     * @return the received message, or null if timeout occurs
     * @throws TransportException if receiving fails
     * @throws IOException if an I/O error occurs
     */
    Message receive() throws TransportException, IOException;

    /**
     * Receive raw data from the transport channel.
     * This method blocks until data is available or timeout occurs.
     *
     * @return the received data, or null if timeout occurs
     * @throws TransportException if receiving fails
     * @throws IOException if an I/O error occurs
     */
    ByteBuffer receiveRaw() throws TransportException, IOException;

    /**
     * Get the transport type.
     *
     * @return the transport type
     */
    TransportType getType();

    /**
     * Get the name of this transport channel.
     *
     * @return the channel name
     */
    String getName();

    /**
     * Set the timeout for receive operations.
     *
     * @param timeoutMillis the timeout in milliseconds
     */
    void setReceiveTimeout(long timeoutMillis);

    /**
     * Get the current receive timeout.
     *
     * @return the timeout in milliseconds
     */
    long getReceiveTimeout();

    /**
     * Check if data is available to read.
     *
     * @return true if data is available
     * @throws IOException if an I/O error occurs
     */
    boolean isDataAvailable() throws IOException;

    /**
     * Flush any buffered data.
     *
     * @throws IOException if an I/O error occurs
     */
    void flush() throws IOException;

    /**
     * Get statistics for this transport channel.
     *
     * @return the transport statistics
     */
    TransportStatistics getStatistics();

    /**
     * Reset the transport statistics.
     */
    void resetStatistics();
}
