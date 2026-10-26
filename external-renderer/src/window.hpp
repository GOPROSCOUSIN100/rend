#pragma once

#include <cstdint>
#include <string>
#include <functional>

// GLFW forward declaration
struct GLFWwindow;

namespace renderbridge {

/**
 * @class Window
 * @brief GLFW-based window management class
 * 
 * This class provides a cross-platform window with input handling
 * for the external renderer.
 */
class Window {
public:
    /**
     * @brief Key callback type
     */
    using KeyCallback = std::function<void(int key, int scancode, int action, int mods)>;
    
    /**
     * @brief Mouse button callback type
     */
    using MouseButtonCallback = std::function<void(int button, int action, int mods)>;
    
    /**
     * @brief Mouse movement callback type
     */
    using MouseMoveCallback = std::function<void(double x, double y)>;
    
    /**
     * @brief Mouse scroll callback type
     */
    using MouseScrollCallback = std::function<void(double xOffset, double yOffset)>;
    
    /**
     * @brief Framebuffer size callback type
     */
    using FramebufferSizeCallback = std::function<void(int width, int height)>;
    
    /**
     * @brief Constructor
     * @param title Window title
     * @param width Window width
     * @param height Window height
     */
    Window(const std::string& title = "RenderBridge", uint32_t width = 1280, uint32_t height = 720);
    
    /**
     * @brief Destructor
     */
    ~Window();
    
    /**
     * @brief Create the window
     * @return True if window creation succeeded
     */
    bool create();
    
    /**
     * @brief Destroy the window
     */
    void destroy();
    
    /**
     * @brief Swap buffers
     */
    void swapBuffers();
    
    /**
     * @brief Poll events
     */
    void pollEvents();
    
    /**
     * @brief Check if the window should close
     * @return True if the window should close
     */
    bool shouldClose() const;
    
    /**
     * @brief Set the window title
     * @param title New title
     */
    void setTitle(const std::string& title);
    
    /**
     * @brief Get the window title
     * @return Window title
     */
    const std::string& getTitle() const;
    
    /**
     * @brief Get the window width
     * @return Width
     */
    uint32_t getWidth() const;
    
    /**
     * @brief Get the window height
     * @return Height
     */
    uint32_t getHeight() const;
    
    /**
     * @brief Get the framebuffer width
     * @return Framebuffer width
     */
    uint32_t getFramebufferWidth() const;
    
    /**
     * @brief Get the framebuffer height
     * @return Framebuffer height
     */
    uint32_t getFramebufferHeight() const;
    
    /**
     * @brief Get the aspect ratio
     * @return Aspect ratio (width / height)
     */
    float getAspectRatio() const;
    
    /**
     * @brief Get the GLFW window handle
     * @return GLFW window handle
     */
    GLFWwindow* getHandle() const;
    
    /**
     * @brief Set the key callback
     * @param callback Callback function
     */
    void setKeyCallback(KeyCallback callback);
    
    /**
     * @brief Set the mouse button callback
     * @param callback Callback function
     */
    void setMouseButtonCallback(MouseButtonCallback callback);
    
    /**
     * @brief Set the mouse move callback
     * @param callback Callback function
     */
    void setMouseMoveCallback(MouseMoveCallback callback);
    
    /**
     * @brief Set the mouse scroll callback
     * @param callback Callback function
     */
    void setMouseScrollCallback(MouseScrollCallback callback);
    
    /**
     * @brief Set the framebuffer size callback
     * @param callback Callback function
     */
    void setFramebufferSizeCallback(FramebufferSizeCallback callback);
    
    /**
     * @brief Set window size
     * @param width Width
     * @param height Height
     */
    void setSize(uint32_t width, uint32_t height);
    
    /**
     * @brief Get window position
     * @param x Output: X position
     * @param y Output: Y position
     */
    void getPosition(int32_t& x, int32_t& y) const;
    
    /**
     * @brief Set window position
     * @param x X position
     * @param y Y position
     */
    void setPosition(int32_t x, int32_t y);
    
    /**
     * @brief Center the window on the screen
     */
    void center();
    
    /**
     * @brief Set window visibility
     * @param visible True to show, false to hide
     */
    void setVisible(bool visible);
    
    /**
     * @brief Set window fullscreen
     * @param fullscreen True for fullscreen, false for windowed
     */
    void setFullscreen(bool fullscreen);
    
    /**
     * @brief Toggle fullscreen
     */
    void toggleFullscreen();
    
    /**
     * @brief Set VSync
     * @param enabled True to enable VSync, false to disable
     */
    void setVSync(bool enabled);
    
    /**
     * @brief Get VSync status
     * @return True if VSync is enabled
     */
    bool isVSyncEnabled() const;
    
    /**
     * @brief Set cursor visibility
     * @param visible True to show cursor, false to hide
     */
    void setCursorVisible(bool visible);
    
    /**
     * @brief Set cursor locked to window
     * @param locked True to lock cursor, false to unlock
     */
    void setCursorLocked(bool locked);
    
    /**
     * @brief Get cursor position
     * @param x Output: X position
     * @param y Output: Y position
     */
    void getCursorPosition(double& x, double& y) const;
    
    /**
     * @brief Set cursor position
     * @param x X position
     * @param y Y position
     */
    void setCursorPosition(double x, double y);
    
    /**
     * @brief Check if a key is pressed
     * @param key Key code
     * @return True if the key is pressed
     */
    bool isKeyPressed(int key) const;
    
    /**
     * @brief Check if a mouse button is pressed
     * @param button Mouse button code
     * @return True if the button is pressed
     */
    bool isMouseButtonPressed(int button) const;
    
    /**
     * @brief Get the time since the window was created
     * @return Time in seconds
     */
    double getTime() const;

private:
    GLFWwindow* m_handle;
    std::string m_title;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_framebufferWidth;
    uint32_t m_framebufferHeight;
    
    // Callbacks
    KeyCallback m_keyCallback;
    MouseButtonCallback m_mouseButtonCallback;
    MouseMoveCallback m_mouseMoveCallback;
    MouseScrollCallback m_mouseScrollCallback;
    FramebufferSizeCallback m_framebufferSizeCallback;
    
    // Previous window state for fullscreen toggle
    int32_t m_previousX;
    int32_t m_previousY;
    uint32_t m_previousWidth;
    uint32_t m_previousHeight;
    bool m_wasFullscreen;
    
    // Static callbacks (GLFW requires static functions)
    static void s_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void s_mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void s_mouseMoveCallback(GLFWwindow* window, double x, double y);
    static void s_mouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
    static void s_framebufferSizeCallback(GLFWwindow* window, int width, int height);
    
    /**
     * @brief Initialize GLFW
     * @return True if initialization succeeded
     */
    bool initGlfw();
    
    /**
     * @brief Create the GLFW window
     * @return True if window creation succeeded
     */
    bool createGlfwWindow();
    
    /**
     * @brief Set GLFW callbacks
     */
    void setGlfwCallbacks();
};

} // namespace renderbridge
