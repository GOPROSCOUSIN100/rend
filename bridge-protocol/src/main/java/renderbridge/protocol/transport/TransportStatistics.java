package renderbridge.protocol.transport;

/**
 * Statistics for a transport channel.
 * Tracks various metrics about the transport performance.
 */
public class TransportStatistics {

    private long messagesSent;
    private long messagesReceived;
    private long bytesSent;
    private long bytesReceived;
    private long errors;
    private long timeouts;
    private long lastSendTime;
    private long lastReceiveTime;
    private long totalSendTimeNanos;
    private long totalReceiveTimeNanos;

    /**
     * Create new transport statistics.
     */
    public TransportStatistics() {
        reset();
    }

    /**
     * Reset all statistics to zero.
     */
    public void reset() {
        messagesSent = 0;
        messagesReceived = 0;
        bytesSent = 0;
        bytesReceived = 0;
        errors = 0;
        timeouts = 0;
        lastSendTime = 0;
        lastReceiveTime = 0;
        totalSendTimeNanos = 0;
        totalReceiveTimeNanos = 0;
    }

    /**
     * Record a message send.
     *
     * @param bytes the number of bytes sent
     * @param timeNanos the time taken to send in nanoseconds
     */
    public void recordSend(long bytes, long timeNanos) {
        messagesSent++;
        bytesSent += bytes;
        lastSendTime = System.nanoTime();
        totalSendTimeNanos += timeNanos;
    }

    /**
     * Record a message receive.
     *
     * @param bytes the number of bytes received
     * @param timeNanos the time taken to receive in nanoseconds
     */
    public void recordReceive(long bytes, long timeNanos) {
        messagesReceived++;
        bytesReceived += bytes;
        lastReceiveTime = System.nanoTime();
        totalReceiveTimeNanos += timeNanos;
    }

    /**
     * Record an error.
     */
    public void recordError() {
        errors++;
    }

    /**
     * Record a timeout.
     */
    public void recordTimeout() {
        timeouts++;
    }

    // Getters

    public long getMessagesSent() {
        return messagesSent;
    }

    public long getMessagesReceived() {
        return messagesReceived;
    }

    public long getBytesSent() {
        return bytesSent;
    }

    public long getBytesReceived() {
        return bytesReceived;
    }

    public long getErrors() {
        return errors;
    }

    public long getTimeouts() {
        return timeouts;
    }

    public long getLastSendTime() {
        return lastSendTime;
    }

    public long getLastReceiveTime() {
        return lastReceiveTime;
    }

    public long getTotalSendTimeNanos() {
        return totalSendTimeNanos;
    }

    public long getTotalReceiveTimeNanos() {
        return totalReceiveTimeNanos;
    }

    /**
     * Get the average send time in nanoseconds.
     *
     * @return the average send time
     */
    public double getAverageSendTimeNanos() {
        return messagesSent > 0 ? (double) totalSendTimeNanos / messagesSent : 0;
    }

    /**
     * Get the average receive time in nanoseconds.
     *
     * @return the average receive time
     */
    public double getAverageReceiveTimeNanos() {
        return messagesReceived > 0 ? (double) totalReceiveTimeNanos / messagesReceived : 0;
    }

    /**
     * Get the average send time in milliseconds.
     *
     * @return the average send time in milliseconds
     */
    public double getAverageSendTimeMs() {
        return getAverageSendTimeNanos() / 1_000_000.0;
    }

    /**
     * Get the average receive time in milliseconds.
     *
     * @return the average receive time in milliseconds
     */
    public double getAverageReceiveTimeMs() {
        return getAverageReceiveTimeNanos() / 1_000_000.0;
    }

    /**
     * Get the send rate in messages per second.
     *
     * @return the send rate
     */
    public double getSendRate() {
        if (messagesSent == 0) {
            return 0;
        }
        long duration = System.nanoTime() - lastSendTime;
        return duration > 0 ? (double) messagesSent / (duration / 1_000_000_000.0) : 0;
    }

    /**
     * Get the receive rate in messages per second.
     *
     * @return the receive rate
     */
    public double getReceiveRate() {
        if (messagesReceived == 0) {
            return 0;
        }
        long duration = System.nanoTime() - lastReceiveTime;
        return duration > 0 ? (double) messagesReceived / (duration / 1_000_000_000.0) : 0;
    }

    @Override
    public String toString() {
        return String.format(
                "TransportStatistics{sent=%d, received=%d, bytesSent=%d, bytesReceived=%d, errors=%d, timeouts=%d}",
                messagesSent, messagesReceived, bytesSent, bytesReceived, errors, timeouts
        );
    }
}
