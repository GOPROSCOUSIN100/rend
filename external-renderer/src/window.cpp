#include "window.hpp"

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <algorithm>

namespace renderbridge {

// Static callback forwarders
void Window::s_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->m_keyCallback) {
        self->m_keyCallback(key, scancode, action, mods);
    }
}

void Window::s_mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->m_mouseButtonCallback) {
        self->m_mouseButtonCallback(button, action, mods);
    }
}

void Window::s_mouseMoveCallback(GLFWwindow* window, double x, double y) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->m_mouseMoveCallback) {
        self->m_mouseMoveCallback(x, y);
    }
}

void Window::s_mouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->m_mouseScrollCallback) {
        self->m_mouseScrollCallback(xOffset, yOffset);
    }
}

void Window::s_framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self) {
        self->m_framebufferWidth = static_cast<uint32_t>(width);
        self->m_framebufferHeight = static_cast<uint32_t>(height);
        
        if (self->m_framebufferSizeCallback) {
            self->m_framebufferSizeCallback(width, height);
        }
    }
}

Window::Window(const std::string& title, uint32_t width, uint32_t height)
    : m_handle(nullptr),
      m_title(title),
      m_width(width),
      m_height(height),
      m_framebufferWidth(width),
      m_framebufferHeight(height),
      m_previousX(0),
      m_previousY(0),
      m_previousWidth(width),
      m_previousHeight(height),
      m_wasFullscreen(false) {
}

Window::~Window() {
    destroy();
}

bool Window::create() {
    // Initialize GLFW if not already initialized
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // bgfx will create its own context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Start hidden
    
    // Create window
    m_handle = glfwCreateWindow(static_cast<int>(m_width), static_cast<int>(m_height), m_title.c_str(), nullptr, nullptr);
    if (!m_handle) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    
    // Set user pointer
    glfwSetWindowUserPointer(m_handle, this);
    
    // Set callbacks
    setGlfwCallbacks();
    
    // Center window
    center();
    
    // Show window
    glfwShowWindow(m_handle);
    
    return true;
}

void Window::destroy() {
    if (m_handle) {
        glfwDestroyWindow(m_handle);
        m_handle = nullptr;
    }
}

void Window::swapBuffers() {
    if (m_handle) {
        glfwSwapBuffers(m_handle);
    }
}

void Window::pollEvents() {
    glfwPollEvents();
}

bool Window::shouldClose() const {
    return m_handle ? glfwWindowShouldClose(m_handle) : true;
}

void Window::setTitle(const std::string& title) {
    m_title = title;
    if (m_handle) {
        glfwSetWindowTitle(m_handle, title.c_str());
    }
}

const std::string& Window::getTitle() const {
    return m_title;
}

uint32_t Window::getWidth() const {
    return m_width;
}

uint32_t Window::getHeight() const {
    return m_height;
}

uint32_t Window::getFramebufferWidth() const {
    return m_framebufferWidth;
}

uint32_t Window::getFramebufferHeight() const {
    return m_framebufferHeight;
}

float Window::getAspectRatio() const {
    return m_framebufferWidth > 0 && m_framebufferHeight > 0 
        ? static_cast<float>(m_framebufferWidth) / static_cast<float>(m_framebufferHeight)
        : 1.0f;
}

GLFWwindow* Window::getHandle() const {
    return m_handle;
}

void Window::setKeyCallback(KeyCallback callback) {
    m_keyCallback = callback;
}

void Window::setMouseButtonCallback(MouseButtonCallback callback) {
    m_mouseButtonCallback = callback;
}

void Window::setMouseMoveCallback(MouseMoveCallback callback) {
    m_mouseMoveCallback = callback;
}

void Window::setMouseScrollCallback(MouseScrollCallback callback) {
    m_mouseScrollCallback = callback;
}

void Window::setFramebufferSizeCallback(FramebufferSizeCallback callback) {
    m_framebufferSizeCallback = callback;
}

void Window::setSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    if (m_handle) {
        glfwSetWindowSize(m_handle, static_cast<int>(width), static_cast<int>(height));
    }
}

void Window::getPosition(int32_t& x, int32_t& y) const {
    if (m_handle) {
        glfwGetWindowPos(m_handle, &x, &y);
    } else {
        x = 0;
        y = 0;
    }
}

void Window::setPosition(int32_t x, int32_t y) {
    if (m_handle) {
        glfwSetWindowPos(m_handle, x, y);
    }
}

void Window::center() {
    if (!m_handle) {
        return;
    }
    
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (!monitor) {
        return;
    }
    
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (!mode) {
        return;
    }
    
    int x = (mode->width - static_cast<int>(m_width)) / 2;
    int y = (mode->height - static_cast<int>(m_height)) / 2;
    glfwSetWindowPos(m_handle, x, y);
}

void Window::setVisible(bool visible) {
    if (m_handle) {
        if (visible) {
            glfwShowWindow(m_handle);
        } else {
            glfwHideWindow(m_handle);
        }
    }
}

void Window::setFullscreen(bool fullscreen) {
    if (!m_handle) {
        return;
    }
    
    if (fullscreen && !m_wasFullscreen) {
        // Save current window state
        glfwGetWindowPos(m_handle, &m_previousX, &m_previousY);
        glfwGetWindowSize(m_handle, &m_previousWidth, &m_previousHeight);
        
        // Get monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        if (monitor) {
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            if (mode) {
                glfwSetWindowMonitor(m_handle, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            }
        }
        
        m_wasFullscreen = true;
    } else if (!fullscreen && m_wasFullscreen) {
        // Restore windowed mode
        glfwSetWindowMonitor(m_handle, nullptr, m_previousX, m_previousY, m_previousWidth, m_previousHeight, GLFW_DONT_CARE);
        m_wasFullscreen = false;
    }
}

void Window::toggleFullscreen() {
    setFullscreen(!m_wasFullscreen);
}

void Window::setVSync(bool enabled) {
    if (m_handle) {
        glfwSwapInterval(enabled ? 1 : 0);
    }
}

bool Window::isVSyncEnabled() const {
    return glfwGetSwapInterval() != 0;
}

void Window::setCursorVisible(bool visible) {
    if (m_handle) {
        glfwSetInputMode(m_handle, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
    }
}

void Window::setCursorLocked(bool locked) {
    if (m_handle) {
        glfwSetInputMode(m_handle, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

void Window::getCursorPosition(double& x, double& y) const {
    if (m_handle) {
        glfwGetCursorPos(m_handle, &x, &y);
    } else {
        x = 0;
        y = 0;
    }
}

void Window::setCursorPosition(double x, double y) {
    if (m_handle) {
        glfwSetCursorPos(m_handle, x, y);
    }
}

bool Window::isKeyPressed(int key) const {
    if (m_handle) {
        return glfwGetKey(m_handle, key) == GLFW_PRESS;
    }
    return false;
}

bool Window::isMouseButtonPressed(int button) const {
    if (m_handle) {
        return glfwGetMouseButton(m_handle, button) == GLFW_PRESS;
    }
    return false;
}

double Window::getTime() const {
    return glfwGetTime();
}

void Window::setGlfwCallbacks() {
    if (!m_handle) {
        return;
    }
    
    glfwSetWindowCloseCallback(m_handle, [](GLFWwindow* window) {
        Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    });
    
    glfwSetKeyCallback(m_handle, s_keyCallback);
    glfwSetMouseButtonCallback(m_handle, s_mouseButtonCallback);
    glfwSetCursorPosCallback(m_handle, s_mouseMoveCallback);
    glfwSetScrollCallback(m_handle, s_mouseScrollCallback);
    glfwSetFramebufferSizeCallback(m_handle, s_framebufferSizeCallback);
}

} // namespace renderbridge
