package renderbridge.renderer;

import renderbridge.renderer.bridge.RendererBridgeClient;

import java.io.IOException;

/**
 * Main entry point for the external renderer.
 */
public class Main {

    private static RendererBridgeClient bridgeClient;
    private static RendererWindow window;

    /**
     * Main method - entry point for the renderer.
     *
     * @param args command line arguments
     */
    public static void main(String[] args) {
        System.out.println("Starting RenderBridge External Renderer...");
        
        try {
            // Parse command line arguments
            parseArguments(args);
            
            // Initialize bridge client
            bridgeClient = new RendererBridgeClient();
            
            // Initialize window
            window = new RendererWindow(800, 600, "RenderBridge Renderer");
            
            // Set bridge client for window
            window.setBridgeClient(bridgeClient);
            
            // Start the window
            window.start();
            
            // Cleanup
            cleanup();
        } catch (Exception e) {
            System.err.println("Failed to start renderer: " + e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }

    /**
     * Parse command line arguments.
     *
     * @param args the command line arguments
     */
    private static void parseArguments(String[] args) {
        // For now, just print help if --help is specified
        if (args != null) {
            for (String arg : args) {
                if ("--help".equals(arg) || "-h".equals(arg)) {
                    printHelp();
                    System.exit(0);
                }
            }
        }
    }

    /**
     * Print help message.
     */
    private static void printHelp() {
        System.out.println("Usage: java -jar external-renderer.jar [options]");
        System.out.println();
        System.out.println("Options:");
        System.out.println("  --help, -h          Show this help message");
        System.out.println("  --width W          Window width (default: 800)");
        System.out.println("  --height H         Window height (default: 600)");
        System.out.println("  --fullscreen       Start in fullscreen mode");
        System.out.println("  --shared-memory    Use shared memory transport (default: true)");
        System.out.println("  --socket           Use socket transport");
        System.out.println("  --port P           Socket port (default: 25565)");
    }

    /**
     * Cleanup resources.
     */
    private static void cleanup() {
        System.out.println("Shutting down RenderBridge External Renderer...");
        
        if (bridgeClient != null) {
            bridgeClient.disconnect();
            bridgeClient = null;
        }
        
        if (window != null) {
            window.stop();
            window = null;
        }
        
        System.out.println("Shutdown complete");
    }

    /**
     * Get the bridge client.
     *
     * @return the bridge client
     */
    public static RendererBridgeClient getBridgeClient() {
        return bridgeClient;
    }

    /**
     * Get the renderer window.
     *
     * @return the renderer window
     */
    public static RendererWindow getWindow() {
        return window;
    }
}
