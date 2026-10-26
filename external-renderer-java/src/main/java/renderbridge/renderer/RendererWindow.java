package renderbridge.renderer;

import org.lwjgl.LWJGLException;
import org.lwjgl.opengl.Display;
import org.lwjgl.opengl.DisplayMode;
import org.lwjgl.opengl.GL11;
import org.lwjgl.opengl.PixelFormat;

import renderbridge.renderer.bridge.RendererBridgeClient;
import renderbridge.renderer.gl.GLRenderer;

import java.nio.FloatBuffer;

/**
 * Renderer window using LWJGL 2.
 * Creates and manages the OpenGL context and rendering loop.
 */
public class RendererWindow {

    // Window settings
    private int width;
    private int height;
    private String title;
    private boolean fullscreen = false;
    private boolean vsync = true;
    
    // State
    private volatile boolean running = false;
    private volatile boolean initialized = false;
    
    // Components
    private RendererBridgeClient bridgeClient;
    private GLRenderer renderer;
    
    // Timing
    private long lastFrameTime;
    private int fps = 0;
    private int frameCount = 0;
    private long lastFpsTime;

    /**
     * Create a new renderer window.
     *
     * @param width the window width
     * @param height the window height
     * @param title the window title
     */
    public RendererWindow(int width, int height, String title) {
        this.width = width;
        this.height = height;
        this.title = title;
    }

    /**
     * Start the window and rendering loop.
     */
    public void start() {
        if (running) {
            return;
        }
        
        running = true;
        initialized = false;
        
        try {
            // Initialize display
            initializeDisplay();
            
            // Initialize OpenGL
            initializeOpenGL();
            
            // Initialize renderer
            renderer = new GLRenderer(width, height);
            
            // Initialize bridge client
            if (bridgeClient != null) {
                bridgeClient.connect();
            }
            
            initialized = true;
            
            // Main rendering loop
            run();
        } catch (Exception e) {
            System.err.println("Failed to start window: " + e.getMessage());
            e.printStackTrace();
            stop();
        }
    }

    /**
     * Stop the window and cleanup resources.
     */
    public void stop() {
        running = false;
        
        try {
            // Disconnect bridge client
            if (bridgeClient != null) {
                bridgeClient.disconnect();
            }
            
            // Cleanup renderer
            if (renderer != null) {
                renderer.cleanup();
                renderer = null;
            }
            
            // Destroy display
            if (Display.isCreated()) {
                Display.destroy();
            }
        } catch (Exception e) {
            System.err.println("Error during cleanup: " + e.getMessage());
        }
        
        initialized = false;
    }

    /**
     * Initialize the display.
     *
     * @throws LWJGLException if display initialization fails
     */
    private void initializeDisplay() throws LWJGLException {
        // Set display mode
        DisplayMode displayMode = new DisplayMode(width, height);
        
        // Create pixel format
        PixelFormat pixelFormat = new PixelFormat();
        pixelFormat.withDepthBits(24);
        pixelFormat.withStencilBits(8);
        
        // Create display
        Display.setDisplayMode(displayMode);
        Display.setTitle(title);
        Display.setFullscreen(fullscreen);
        Display.setResizable(true);
        Display.setVSyncEnabled(vsync);
        Display.create(pixelFormat);
        
        System.out.println("Display created: " + Display.getWidth() + "x" + Display.getHeight());
    }

    /**
     * Initialize OpenGL.
     */
    private void initializeOpenGL() {
        // Enable depth testing
        GL11.glEnable(GL11.GL_DEPTH_TEST);
        
        // Enable alpha blending
        GL11.glEnable(GL11.GL_BLEND);
        GL11.glBlendFunc(GL11.GL_SRC_ALPHA, GL11.GL_ONE_MINUS_SRC_ALPHA);
        
        // Enable culling
        GL11.glEnable(GL11.GL_CULL_FACE);
        GL11.glCullFace(GL11.GL_BACK);
        
        // Set clear color
        GL11.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        // Set viewport
        GL11.glViewport(0, 0, width, height);
        
        // Enable point size
        GL11.glEnable(GL11.GL_PROGRAM_POINT_SIZE);
        
        // Set point size range
        FloatBuffer pointSizeRange = FloatBuffer.allocate(2);
        GL11.glGetFloat(GL11.GL_POINT_SIZE_RANGE, pointSizeRange);
        GL11.glPointSize(pointSizeRange.get(1) / 2.0f);
        
        System.out.println("OpenGL initialized");
        System.out.println("OpenGL version: " + GL11.glGetString(GL11.GL_VERSION));
        System.out.println("GLSL version: " + GL11.glGetString(GL11.GL_SHADING_LANGUAGE_VERSION));
    }

    /**
     * Main rendering loop.
     */
    private void run() {
        lastFrameTime = System.nanoTime();
        lastFpsTime = lastFrameTime;
        
        while (running) {
            // Process input
            processInput();
            
            // Update bridge client
            if (bridgeClient != null) {
                bridgeClient.update();
            }
            
            // Update renderer
            if (renderer != null) {
                renderer.update();
            }
            
            // Render frame
            renderFrame();
            
            // Update display
            Display.update();
            
            // Check for close request
            if (Display.isCloseRequested()) {
                running = false;
            }
            
            // Calculate FPS
            updateFps();
            
            // Limit frame rate if vsync is disabled
            if (!vsync) {
                limitFrameRate();
            }
        }
        
        stop();
    }

    /**
     * Process input events.
     */
    private void processInput() {
        // For now, just check for close request
        // In a real implementation, we would handle keyboard/mouse input
    }

    /**
     * Render a frame.
     */
    private void renderFrame() {
        // Clear buffers
        GL11.glClear(GL11.GL_COLOR_BUFFER_BIT | GL11.GL_DEPTH_BUFFER_BIT);
        
        // Render scene
        if (renderer != null) {
            renderer.render();
        }
        
        // Swap buffers
        Display.sync(60); // Sync to 60 FPS
    }

    /**
     * Update FPS counter.
     */
    private void updateFps() {
        frameCount++;
        
        long currentTime = System.nanoTime();
        long elapsed = currentTime - lastFpsTime;
        
        if (elapsed >= 1_000_000_000) { // 1 second
            fps = (int) ((frameCount * 1_000_000_000.0) / elapsed);
            frameCount = 0;
            lastFpsTime = currentTime;
            
            // Update window title with FPS
            Display.setTitle(title + " - FPS: " + fps);
        }
    }

    /**
     * Limit frame rate.
     */
    private void limitFrameRate() {
        long currentTime = System.nanoTime();
        long elapsed = currentTime - lastFrameTime;
        long targetFrameTime = 16_666_667; // 60 FPS
        
        if (elapsed < targetFrameTime) {
            try {
                long sleepTime = (targetFrameTime - elapsed) / 1_000_000;
                if (sleepTime > 0) {
                    Thread.sleep(sleepTime);
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
        
        lastFrameTime = System.nanoTime();
    }

    /**
     * Set the bridge client.
     *
     * @param bridgeClient the bridge client
     */
    public void setBridgeClient(RendererBridgeClient bridgeClient) {
        this.bridgeClient = bridgeClient;
        
        if (renderer != null) {
            renderer.setBridgeClient(bridgeClient);
        }
    }

    /**
     * Set the window size.
     *
     * @param width the new width
     * @param height the new height
     */
    public void setSize(int width, int height) {
        this.width = width;
        this.height = height;
        
        if (Display.isCreated()) {
            Display.setDisplayMode(new DisplayMode(width, height));
            GL11.glViewport(0, 0, width, height);
            
            if (renderer != null) {
                renderer.resize(width, height);
            }
        }
    }

    /**
     * Set the window title.
     *
     * @param title the new title
     */
    public void setTitle(String title) {
        this.title = title;
        if (Display.isCreated()) {
            Display.setTitle(title);
        }
    }

    /**
     * Set fullscreen mode.
     *
     * @param fullscreen true for fullscreen, false for windowed
     */
    public void setFullscreen(boolean fullscreen) {
        this.fullscreen = fullscreen;
        if (Display.isCreated()) {
            Display.setFullscreen(fullscreen);
        }
    }

    /**
     * Set vsync.
     *
     * @param vsync true to enable vsync, false to disable
     */
    public void setVsync(boolean vsync) {
        this.vsync = vsync;
        if (Display.isCreated()) {
            Display.setVSyncEnabled(vsync);
        }
    }

    /**
     * Get the current width.
     *
     * @return the width
     */
    public int getWidth() {
        return width;
    }

    /**
     * Get the current height.
     *
     * @return the height
     */
    public int getHeight() {
        return height;
    }

    /**
     * Get the current FPS.
     *
     * @return the FPS
     */
    public int getFps() {
        return fps;
    }

    /**
     * Check if the window is running.
     *
     * @return true if running
     */
    public boolean isRunning() {
        return running;
    }

    /**
     * Check if the window is initialized.
     *
     * @return true if initialized
     */
    public boolean isInitialized() {
        return initialized;
    }

    @Override
    public String toString() {
        return String.format(
                "RendererWindow{title='%s', size=%dx%d, fps=%d, running=%s}",
                title, width, height, fps, running
        );
    }
}
