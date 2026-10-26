#pragma once

#include <cstdint>
#include <string>
#include <memory>

namespace renderbridge {

// Forward declarations
class Window;
class RendererBridgeClient;
class Scene;
class Renderer;

/**
 * @class RenderBridgeApp
 * @brief Main application class for the external renderer
 * 
 * This class manages the application lifecycle, including initialization,
 * main loop, and cleanup.
 */
class RenderBridgeApp {
public:
    /**
     * @brief Constructor
     */
    RenderBridgeApp();
    
    /**
     * @brief Destructor
     */
    ~RenderBridgeApp();
    
    /**
     * @brief Initialize the application
     * @param windowWidth Window width
     * @param windowHeight Window height
     * @param windowTitle Window title
     * @return True if initialization succeeded
     */
    bool initialize(uint32_t windowWidth = 1280, uint32_t windowHeight = 720, const std::string& windowTitle = "RenderBridge - bgfx Renderer");
    
    /**
     * @brief Run the application
     */
    void run();
    
    /**
     * @brief Shutdown the application
     */
    void shutdown();
    
    /**
     * @brief Process a single frame
     */
    void processFrame();
    
    /**
     * @brief Handle window resize
     * @param width New width
     * @param height New height
     */
    void onResize(uint32_t width, uint32_t height);
    
    /**
     * @brief Handle key press
     * @param key Key code
     * @param scancode Scancode
     * @param action Action (press, release, repeat)
     * @param mods Modifiers
     */
    void onKey(int key, int scancode, int action, int mods);
    
    /**
     * @brief Handle mouse button
     * @param button Button code
     * @param action Action (press, release)
     * @param mods Modifiers
     */
    void onMouseButton(int button, int action, int mods);
    
    /**
     * @brief Handle mouse movement
     * @param x X position
     * @param y Y position
     */
    void onMouseMove(double x, double y);
    
    /**
     * @brief Handle mouse scroll
     * @param xOffset X offset
     * @param yOffset Y offset
     */
    void onMouseScroll(double xOffset, double yOffset);
    
    /**
     * @brief Check if the application should close
     * @return True if the application should close
     */
    bool shouldClose() const;
    
    /**
     * @brief Get the window
     * @return Pointer to the window
     */
    Window* getWindow();
    
    /**
     * @brief Get the bridge client
     * @return Pointer to the bridge client
     */
    RendererBridgeClient* getBridgeClient();
    
    /**
     * @brief Get the scene
     * @return Pointer to the scene
     */
    Scene* getScene();
    
    /**
     * @brief Get the renderer
     * @return Pointer to the renderer
     */
    Renderer* getRenderer();

private:
    std::unique_ptr<Window> m_window;
    std::unique_ptr<RendererBridgeClient> m_bridgeClient;
    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<Renderer> m_renderer;
    
    bool m_running;
    bool m_initialized;
    
    // Statistics
    uint32_t m_frameCount;
    float m_fps;
    float m_frameTime;
    
    /**
     * @brief Initialize bgfx
     * @return True if initialization succeeded
     */
    bool initBgfx();
    
    /**
     * @brief Initialize the bridge connection
     * @return True if initialization succeeded
     */
    bool initBridge();
    
    /**
     * @brief Connect to the mod
     * @return True if connection succeeded
     */
    bool connectToMod();
    
    /**
     * @brief Process bridge messages
     */
    void processBridgeMessages();
    
    /**
     * @brief Update FPS counter
     */
    void updateFps();
    
    /**
     * @brief Calculate average frame time
     * @return Average frame time in milliseconds
     */
    float calculateAverageFrameTime() const;
};

} // namespace renderbridge
