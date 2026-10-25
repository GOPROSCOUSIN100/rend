package renderbridge.protocol;

import org.junit.Test;

import static org.junit.Assert.*;

/**
 * Unit tests for BridgeServer.
 */
public class BridgeServerTest {

    @Test
    public void testInitialState() {
        BridgeServer server = new BridgeServer();
        
        assertEquals(BridgeServer.ServerState.STOPPED, server.getState());
        assertFalse(server.isRunning());
    }

    @Test
    public void testName() {
        BridgeServer server = new BridgeServer("TestServer", true, true);
        assertEquals("TestServer", server.getName());
    }

    @Test
    public void testToString() {
        BridgeServer server = new BridgeServer("TestServer", true, true);
        String str = server.toString();
        
        assertTrue(str.contains("BridgeServer"));
        assertTrue(str.contains("name='TestServer'"));
    }

    @Test
    public void testConfiguration() {
        BridgeServer server = new BridgeServer();
        
        server.setHeartbeatIntervalMs(500);
        assertEquals(500, server.getHeartbeatIntervalMs());
        
        server.setHeartbeatTimeoutMs(2000);
        assertEquals(2000, server.getHeartbeatTimeoutMs());
    }

    @Test
    public void testMessageHandler() {
        BridgeServer server = new BridgeServer();
        
        BridgeServer.MessageHandler handler = new BridgeServer.MessageHandler() {
            @Override
            public void onMessageReceived(Message message) {
                // Test handler
            }

            @Override
            public void onClientConnected(String clientName) {
                // Test handler
            }

            @Override
            public void onClientDisconnected(String clientName) {
                // Test handler
            }

            @Override
            public void onServerStateChanged(BridgeServer.ServerState state) {
                // Test handler
            }

            @Override
            public void onError(TransportException exception) {
                // Test handler
            }
        };
        
        server.setMessageHandler(handler);
        // No exception should be thrown
    }

    @Test
    public void testNullMessageHandler() {
        BridgeServer server = new BridgeServer();
        server.setMessageHandler(null);
        // Should default to DEFAULT_HANDLER
    }
}
