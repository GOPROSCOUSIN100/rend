#include "main.hpp"

#include "window.hpp"
#include "renderer/renderer.hpp"
#include "scene/scene.hpp"
#include "bridge/renderer_bridge_client.hpp"
#include "utils/logger.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

// Platform-specific includes for GLFW native access
#if defined(_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
#elif defined(__APPLE__)
    #define GLFW_EXPOSE_NATIVE_COCOA
    #include <GLFW/glfw3native.h>
#elif defined(__linux__)
    #define GLFW_EXPOSE_NATIVE_X11
    #include <GLFW/glfw3native.h>
#endif

namespace renderbridge {

// Static callback handlers for GLFW
static void glfwErrorCallback(int error, const char* description) {
    RB_LOG_ERROR_FMT("GLFW Error [{}]: {}", error, description);
}

RenderBridgeApp::RenderBridgeApp() 
    : m_running(false), m_initialized(false), m_frameCount(0), m_fps(0.0f), m_frameTime(0.0f) {
}

RenderBridgeApp::~RenderBridgeApp() {
    shutdown();
}

bool RenderBridgeApp::initialize(uint32_t windowWidth, uint32_t windowHeight, const std::string& windowTitle) {
    RB_LOG_INFO("Initializing RenderBridge C++ Renderer...");
    
    // Initialize logger
    initializeLogger("RenderBridge-Renderer", LogLevel::INFO);
    
    // Initialize GLFW
    if (!glfwInit()) {
        RB_LOG_ERROR("Failed to initialize GLFW");
        return false;
    }
    
    // Set GLFW error callback
    glfwSetErrorCallback(glfwErrorCallback);
    
    // Create window
    m_window = std::make_unique<Window>(windowTitle, windowWidth, windowHeight);
    if (!m_window->create()) {
        RB_LOG_ERROR("Failed to create window");
        glfwTerminate();
        return false;
    }
    
    // Initialize bgfx
    if (!initBgfx()) {
        RB_LOG_ERROR("Failed to initialize bgfx");
        return false;
    }
    
    // Create scene
    m_scene = std::make_unique<Scene>();
    m_scene->initialize();
    
    // Create renderer
    m_renderer = std::make_unique<Renderer>(m_window.get());
    if (!m_renderer->initialize()) {
        RB_LOG_ERROR("Failed to initialize renderer");
        return false;
    }
    
    // Initialize bridge client
    if (!initBridge()) {
        RB_LOG_ERROR("Failed to initialize bridge client");
        return false;
    }
    
    // Connect to mod
    if (!connectToMod()) {
        RB_LOG_WARN("Failed to connect to mod, will retry...");
    }
    
    m_initialized = true;
    m_running = true;
    
    RB_LOG_INFO("RenderBridge C++ Renderer initialized successfully");
    return true;
}

bool RenderBridgeApp::initBgfx() {
    RB_LOG_INFO("Initializing bgfx...");
    
    // Get GLFW window handle
    GLFWwindow* glfwWindow = m_window->getHandle();
    
    // Platform-specific setup
    bgfx::PlatformData pd = {};
    
#if defined(_WIN32)
    pd.nwh = glfwGetWin32Window(glfwWindow);
    pd.ndt = nullptr;
#elif defined(__APPLE__)
    pd.nwh = glfwGetCocoaWindow(glfwWindow);
    pd.ndt = nullptr;
#elif defined(__linux__)
    pd.nwh = (void*)(uintptr_t)glfwGetX11Window(glfwWindow);
    pd.ndt = glfwGetX11Display();
#endif
    
    pd.context = nullptr;
    pd.backBuffer = nullptr;
    pd.backBufferDS = nullptr;
    
    // Initialize bgfx
    bgfx::Init init;
    init.type = bgfx::RendererType::Count; // Auto-detect
    init.vendorId = BGFX_PCI_ID_SOFTWARE_RASTERIZER;
    init.deviceId = 0;
    init.debug = true;
    init.profile = true;
    init.platformData = pd;
    
    if (!bgfx::init(init)) {
        RB_LOG_ERROR("bgfx::init() failed");
        return false;
    }
    
    // Set view
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, m_window->getFramebufferWidth(), m_window->getFramebufferHeight());
    
    RB_LOG_INFO("bgfx initialized successfully");
    return true;
}

bool RenderBridgeApp::initBridge() {
    RB_LOG_INFO("Initializing bridge client...");
    
    m_bridgeClient = std::make_unique<RendererBridgeClient>();
    
    // Initialize with renderer info
    if (!m_bridgeClient->initialize("RenderBridge-CPP-Renderer", 0x00020000)) {
        RB_LOG_ERROR("Failed to initialize bridge client");
        return false;
    }
    
    // Set preference for shared memory
    m_bridgeClient->setPreferSharedMemory(true);
    
    // Register callbacks
    m_bridgeClient->registerMessageCallback([this](uint16_t messageType, const uint8_t* payload, uint32_t payloadSize) {
        // Forward messages to scene
        if (m_scene) {
            m_scene->updateFromMessage(messageType, payload, payloadSize);
        }
    });
    
    m_bridgeClient->registerConnectionStatusCallback([this](bool connected, const std::string& errorMessage) {
        if (connected) {
            RB_LOG_INFO_FMT("Connected to mod: {}", errorMessage);
        } else {
            RB_LOG_WARN_FMT("Disconnected from mod: {}", errorMessage);
        }
    });
    
    RB_LOG_INFO("Bridge client initialized successfully");
    return true;
}

bool RenderBridgeApp::connectToMod() {
    RB_LOG_INFO("Connecting to mod...");
    
    // Try shared memory first
    if (m_bridgeClient->isSharedMemoryAvailable() && m_bridgeClient->connectSharedMemory()) {
        RB_LOG_INFO("Connected via shared memory");
        return true;
    }
    
    // Fall back to socket
    if (m_bridgeClient->isSocketAvailable() && m_bridgeClient->connectSocket()) {
        RB_LOG_INFO("Connected via socket");
        return true;
    }
    
    RB_LOG_WARN("Failed to connect to mod");
    return false;
}

void RenderBridgeApp::run() {
    RB_LOG_INFO("Starting main loop...");
    
    double lastTime = m_window->getTime();
    
    while (m_running && !m_window->shouldClose()) {
        double currentTime = m_window->getTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;
        
        // Update frame time
        m_frameTime = deltaTime * 1000.0f; // Convert to milliseconds
        
        // Process window events
        m_window->pollEvents();
        
        // Process bridge messages
        processBridgeMessages();
        
        // Process frame
        processFrame();
        
        // Swap buffers
        m_window->swapBuffers();
        
        // Frame
        bgfx::frame();
        
        // Update FPS
        updateFps();
        
        // Increment frame counter
        m_frameCount++;
    }
    
    RB_LOG_INFO("Main loop ended");
}

void RenderBridgeApp::processFrame() {
    if (!m_initialized || !m_renderer || !m_scene) {
        return;
    }
    
    // Get delta time
    static double lastTime = m_window->getTime();
    double currentTime = m_window->getTime();
    float deltaTime = static_cast<float>(currentTime - lastTime);
    lastTime = currentTime;
    
    // Render frame
    if (m_renderer) {
        m_renderer->renderFrame(m_scene.get(), deltaTime);
    }
}

void RenderBridgeApp::processBridgeMessages() {
    if (!m_bridgeClient) {
        return;
    }
    
    // Poll for messages
    uint32_t messageCount = m_bridgeClient->pollMessages();
    
    // If not connected, try to reconnect
    if (!m_bridgeClient->isConnected()) {
        connectToMod();
    }
}

void RenderBridgeApp::onResize(uint32_t width, uint32_t height) {
    if (m_renderer) {
        m_renderer->resize(width, height);
    }
}

void RenderBridgeApp::onKey(int key, int scancode, int action, int mods) {
    // Handle key events
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        m_running = false;
    }
}

void RenderBridgeApp::onMouseButton(int button, int action, int mods) {
    // Handle mouse button events
}

void RenderBridgeApp::onMouseMove(double x, double y) {
    // Handle mouse movement
}

void RenderBridgeApp::onMouseScroll(double xOffset, double yOffset) {
    // Handle mouse scroll
}

bool RenderBridgeApp::shouldClose() const {
    return !m_running || (m_window && m_window->shouldClose());
}

Window* RenderBridgeApp::getWindow() {
    return m_window.get();
}

RendererBridgeClient* RenderBridgeApp::getBridgeClient() {
    return m_bridgeClient.get();
}

Scene* RenderBridgeApp::getScene() {
    return m_scene.get();
}

Renderer* RenderBridgeApp::getRenderer() {
    return m_renderer.get();
}

void RenderBridgeApp::shutdown() {
    RB_LOG_INFO("Shutting down RenderBridge C++ Renderer...");
    
    m_running = false;
    
    // Shutdown renderer
    if (m_renderer) {
        m_renderer->shutdown();
        m_renderer.reset();
    }
    
    // Shutdown scene
    if (m_scene) {
        m_scene->shutdown();
        m_scene.reset();
    }
    
    // Disconnect bridge
    if (m_bridgeClient) {
        m_bridgeClient->disconnect();
        m_bridgeClient.reset();
    }
    
    // Shutdown bgfx
    bgfx::shutdown();
    
    // Destroy window
    if (m_window) {
        m_window->destroy();
        m_window.reset();
    }
    
    // Terminate GLFW
    glfwTerminate();
    
    // Shutdown logger
    shutdownLogger();
    
    m_initialized = false;
    
    RB_LOG_INFO("RenderBridge C++ Renderer shut down successfully");
}

void RenderBridgeApp::updateFps() {
    static double lastFpsTime = 0;
    static uint32_t frameCount = 0;
    
    double currentTime = m_window->getTime();
    frameCount++;
    
    if (currentTime - lastFpsTime >= 1.0) {
        m_fps = static_cast<float>(frameCount) / (currentTime - lastFpsTime);
        frameCount = 0;
        lastFpsTime = currentTime;
    }
}

float RenderBridgeApp::calculateAverageFrameTime() const {
    return m_frameTime;
}

} // namespace renderbridge

// Main entry point
int main(int argc, char* argv[]) {
    using namespace renderbridge;
    
    // Create application
    RenderBridgeApp app;
    
    // Initialize
    if (!app.initialize(1280, 720, "RenderBridge - bgfx Renderer")) {
        RB_LOG_FATAL("Failed to initialize application");
        return 1;
    }
    
    // Run
    app.run();
    
    // Shutdown
    app.shutdown();
    
    return 0;
}
