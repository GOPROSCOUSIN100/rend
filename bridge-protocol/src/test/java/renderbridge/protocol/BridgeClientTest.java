package renderbridge.protocol;

import org.junit.Test;

import static org.junit.Assert.*;

/**
 * Unit tests for BridgeClient.
 */
public class BridgeClientTest {

    @Test
    public void testInitialState() {
        BridgeClient client = new BridgeClient();
        
        assertEquals(BridgeClient.ConnectionState.DISCONNECTED, client.getState());
        assertFalse(client.isConnected());
    }

    @Test
    public void testName() {
        BridgeClient client = new BridgeClient("TestClient", true, true);
        assertEquals("TestClient", client.getName());
    }

    @Test
    public void testToString() {
        BridgeClient client = new BridgeClient("TestClient", true, true);
        String str = client.toString();
        
        assertTrue(str.contains("BridgeClient"));
        assertTrue(str.contains("name='TestClient'"));
    }

    @Test
    public void testConfiguration() {
        BridgeClient client = new BridgeClient();
        
        client.setHeartbeatIntervalMs(500);
        assertEquals(500, client.getHeartbeatIntervalMs());
        
        client.setHeartbeatTimeoutMs(2000);
        assertEquals(2000, client.getHeartbeatTimeoutMs());
        
        client.setMaxRetries(5);
        assertEquals(5, client.getMaxRetries());
        
        client.setRetryDelayMs(200);
        assertEquals(200, client.getRetryDelayMs());
    }

    @Test
    public void testMessageHandler() {
        BridgeClient client = new BridgeClient();
        
        BridgeClient.MessageHandler handler = new BridgeClient.MessageHandler() {
            @Override
            public void onMessageReceived(Message message) {
                // Test handler
            }

            @Override
            public void onConnectionStateChanged(BridgeClient.ConnectionState state) {
                // Test handler
            }

            @Override
            public void onError(TransportException exception) {
                // Test handler
            }
        };
        
        client.setMessageHandler(handler);
        // No exception should be thrown
    }

    @Test
    public void testNullMessageHandler() {
        BridgeClient client = new BridgeClient();
        client.setMessageHandler(null);
        // Should default to DEFAULT_HANDLER
        assertNotNull(client.getMessageHandler());
    }
}
