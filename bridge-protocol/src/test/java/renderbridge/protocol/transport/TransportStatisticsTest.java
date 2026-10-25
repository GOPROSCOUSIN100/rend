package renderbridge.protocol.transport;

import org.junit.Test;

import static org.junit.Assert.*;

/**
 * Unit tests for TransportStatistics.
 */
public class TransportStatisticsTest {

    @Test
    public void testInitialState() {
        TransportStatistics stats = new TransportStatistics();
        
        assertEquals(0, stats.getMessagesSent());
        assertEquals(0, stats.getMessagesReceived());
        assertEquals(0, stats.getBytesSent());
        assertEquals(0, stats.getBytesReceived());
        assertEquals(0, stats.getErrors());
        assertEquals(0, stats.getTimeouts());
    }

    @Test
    public void testRecordSend() {
        TransportStatistics stats = new TransportStatistics();
        
        stats.recordSend(100, 1000); // 100 bytes, 1 microsecond
        
        assertEquals(1, stats.getMessagesSent());
        assertEquals(100, stats.getBytesSent());
        assertEquals(1000, stats.getTotalSendTimeNanos());
    }

    @Test
    public void testRecordReceive() {
        TransportStatistics stats = new TransportStatistics();
        
        stats.recordReceive(200, 2000); // 200 bytes, 2 microseconds
        
        assertEquals(1, stats.getMessagesReceived());
        assertEquals(200, stats.getBytesReceived());
        assertEquals(2000, stats.getTotalReceiveTimeNanos());
    }

    @Test
    public void testRecordError() {
        TransportStatistics stats = new TransportStatistics();
        
        stats.recordError();
        stats.recordError();
        
        assertEquals(2, stats.getErrors());
    }

    @Test
    public void testRecordTimeout() {
        TransportStatistics stats = new TransportStatistics();
        
        stats.recordTimeout();
        stats.recordTimeout();
        stats.recordTimeout();
        
        assertEquals(3, stats.getTimeouts());
    }

    @Test
    public void testReset() {
        TransportStatistics stats = new TransportStatistics();
        
        stats.recordSend(100, 1000);
        stats.recordReceive(200, 2000);
        stats.recordError();
        stats.recordTimeout();
        
        stats.reset();
        
        assertEquals(0, stats.getMessagesSent());
        assertEquals(0, stats.getMessagesReceived());
        assertEquals(0, stats.getBytesSent());
        assertEquals(0, stats.getBytesReceived());
        assertEquals(0, stats.getErrors());
        assertEquals(0, stats.getTimeouts());
    }

    @Test
    public void testAverageSendTime() {
        TransportStatistics stats = new TransportStatistics();
        
        stats.recordSend(100, 1_000_000); // 1ms
        stats.recordSend(100, 2_000_000); // 2ms
        
        double avg = stats.getAverageSendTimeNanos();
        assertEquals(1_500_000, avg, 0.001);
    }

    @Test
    public void testAverageReceiveTime() {
        TransportStatistics stats = new TransportStatistics();
        
        stats.recordReceive(100, 500_000); // 0.5ms
        stats.recordReceive(100, 1_500_000); // 1.5ms
        
        double avg = stats.getAverageReceiveTimeNanos();
        assertEquals(1_000_000, avg, 0.001);
    }

    @Test
    public void testToString() {
        TransportStatistics stats = new TransportStatistics();
        stats.recordSend(100, 1000);
        stats.recordReceive(200, 2000);
        
        String str = stats.toString();
        assertTrue(str.contains("sent=1"));
        assertTrue(str.contains("received=1"));
        assertTrue(str.contains("bytesSent=100"));
        assertTrue(str.contains("bytesReceived=200"));
    }
}
