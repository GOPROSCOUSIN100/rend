#include "renderer.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include "entity.hpp"
#include "particle.hpp"
#include "weather.hpp"
#include "window.hpp"
#include "utils/logger.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

#include <cstring>
#include <algorithm>

namespace renderbridge {

// Static logger
static Logger s_logger("Renderer");

Renderer::Renderer(Window* window)
    : m_window(window),
      m_mainViewId(0),
      m_width(0),
      m_height(0),
      m_fov(70.0f),
      m_nearPlane(0.1f),
      m_farPlane(1000.0f),
      m_wireframe(false),
      m_debugRendering(false),
      m_frameCount(0),
      m_fps(0.0f),
      m_frameTime(0.0f),
      m_lastFrameTime(0.0f)
{
    // Initialize matrices
    std::memset(m_viewMatrix, 0, sizeof(m_viewMatrix));
    std::memset(m_projMatrix, 0, sizeof(m_projMatrix));
    std::memset(m_viewProjMatrix, 0, sizeof(m_viewProjMatrix));
    
    // Set default clear color (sky blue)
    m_clearColor[0] = 0.53f;
    m_clearColor[1] = 0.81f;
    m_clearColor[2] = 1.0f;
    m_clearColor[3] = 1.0f;
    
    // Initialize uniforms to invalid handles
    m_viewProjUniform.idx = bgfx::kInvalidHandle;
    m_viewUniform.idx = bgfx::kInvalidHandle;
    m_projUniform.idx = bgfx::kInvalidHandle;
    m_cameraPosUniform.idx = bgfx::kInvalidHandle;
    m_timeUniform.idx = bgfx::kInvalidHandle;
    m_lightDirUniform.idx = bgfx::kInvalidHandle;
    m_lightColorUniform.idx = bgfx::kInvalidHandle;
    m_fogColorUniform.idx = bgfx::kInvalidHandle;
    m_fogDensityUniform.idx = bgfx::kInvalidHandle;
    
    // Initialize textures to invalid handles
    m_whiteTexture.idx = bgfx::kInvalidHandle;
    m_blackTexture.idx = bgfx::kInvalidHandle;
    m_missingTexture.idx = bgfx::kInvalidHandle;
    
    // Initialize programs to invalid handles
    m_voxelProgram.idx = bgfx::kInvalidHandle;
    m_entityProgram.idx = bgfx::kInvalidHandle;
    m_particleProgram.idx = bgfx::kInvalidHandle;
}

Renderer::~Renderer()
{
    shutdown();
}

bool Renderer::initialize()
{
    s_logger.info("Initializing renderer...");
    
    if (!initBgfx()) {
        s_logger.error("Failed to initialize bgfx");
        return false;
    }
    
    if (!initResources()) {
        s_logger.error("Failed to initialize resources");
        return false;
    }
    
    s_logger.info("Renderer initialized successfully");
    return true;
}

void Renderer::shutdown()
{
    s_logger.info("Shutting down renderer...");
    
    destroyResources();
    
    // Shutdown bgfx
    if (bgfx::isInitialized()) {
        bgfx::shutdown();
    }
    
    s_logger.info("Renderer shut down");
}

void Renderer::renderFrame(const Scene* scene, float deltaTime)
{
    if (!scene || !bgfx::isInitialized()) {
        return;
    }
    
    // Update FPS counter
    updateFps();
    
    // Begin frame
    bgfx::touch(m_mainViewId);
    
    // Clear the frame buffer
    bgfx::setViewClear(m_mainViewId,
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL,
        0x000000FF,
        1.0f,
        0);
    
    // Setup view
    const Camera* camera = scene->getCamera().get();
    if (camera) {
        setupView(m_mainViewId, camera);
    }
    
    // Update uniforms
    if (camera) {
        updateUniforms(camera, deltaTime);
    }
    
    // Set view transform
    bgfx::setViewTransform(m_mainViewId, m_viewMatrix, m_projMatrix);
    
    // Set view rect
    bgfx::setViewRect(m_mainViewId, 0, 0, m_width, m_height);
    
    // Render scene
    renderScene(scene);
    
    // Render debug info if enabled
    if (m_debugRendering) {
        renderDebug();
    }
    
    // End frame
    bgfx::frame();
    
    // Increment frame counter
    m_frameCount++;
}

void Renderer::resize(uint32_t width, uint32_t height)
{
    if (m_width != width || m_height != height) {
        m_width = width;
        m_height = height;
        
        // Resize the frame buffer
        if (bgfx::isInitialized()) {
            bgfx::reset(m_width, m_height, BGFX_RESET_VSYNC);
        }
    }
}

void Renderer::setClearColor(float r, float g, float b, float a)
{
    m_clearColor[0] = r;
    m_clearColor[1] = g;
    m_clearColor[2] = b;
    m_clearColor[3] = a;
}

void Renderer::setWireframe(bool enabled)
{
    m_wireframe = enabled;
}

void Renderer::setDebugRendering(bool enabled)
{
    m_debugRendering = enabled;
}

void Renderer::setFov(float fov)
{
    m_fov = fov;
}

void Renderer::setNearPlane(float near)
{
    m_nearPlane = near;
}

void Renderer::setFarPlane(float far)
{
    m_farPlane = far;
}

bgfx::ViewId Renderer::getMainViewId() const
{
    return m_mainViewId;
}

float Renderer::getFps() const
{
    return m_fps;
}

float Renderer::getFrameTime() const
{
    return m_frameTime;
}

uint64_t Renderer::getFrameCount() const
{
    return m_frameCount;
}

std::string Renderer::getStatistics() const
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
        "Frames: %llu | FPS: %.1f | Frame Time: %.2fms",
        m_frameCount, m_fps, m_frameTime);
    return std::string(buffer);
}

bool Renderer::initBgfx()
{
    if (bgfx::isInitialized()) {
        return true;
    }
    
    if (!m_window) {
        s_logger.error("No window provided for bgfx initialization");
        return false;
    }
    
    // Get window handle
    void* nativeWindowHandle = m_window->getNativeHandle();
    if (!nativeWindowHandle) {
        s_logger.error("Failed to get native window handle");
        return false;
    }
    
    // Get window dimensions
    uint32_t width, height;
    m_window->getDimensions(width, height);
    m_width = width;
    m_height = height;
    
    // Initialize bgfx platform data
    bgfx::PlatformData platformData;
    #if BGFX_CONFIG_RENDERER_TYPE == BGFX_RENDERER_TYPE_DIRECT3D11
        platformData.nwh = nativeWindowHandle;
        platformData.ndt = nullptr;
        platformData.context = nullptr;
        platformData.backBuffer = nullptr;
        platformData.backBufferDS = nullptr;
    #elif BGFX_CONFIG_RENDERER_TYPE == BGFX_RENDERER_TYPE_DIRECT3D12
        platformData.nwh = nativeWindowHandle;
        platformData.ndt = nullptr;
        platformData.context = nullptr;
        platformData.backBuffer = nullptr;
        platformData.backBufferDS = nullptr;
    #elif BGFX_CONFIG_RENDERER_TYPE == BGFX_RENDERER_TYPE_METAL
        platformData.nwh = nativeWindowHandle;
        platformData.ndt = nullptr;
        platformData.context = nullptr;
        platformData.backBuffer = nullptr;
        platformData.backBufferDS = nullptr;
    #elif BGFX_CONFIG_RENDERER_TYPE == BGFX_RENDERER_TYPE_OPENGL
        platformData.nwh = nativeWindowHandle;
        platformData.ndt = nullptr;
        platformData.context = nullptr;
        platformData.backBuffer = nullptr;
        platformData.backBufferDS = nullptr;
    #elif BGFX_CONFIG_RENDERER_TYPE == BGFX_RENDERER_TYPE_VULKAN
        platformData.nwh = nativeWindowHandle;
        platformData.ndt = nullptr;
        platformData.context = nullptr;
        platformData.backBuffer = nullptr;
        platformData.backBufferDS = nullptr;
    #else
        platformData.nwh = nativeWindowHandle;
        platformData.ndt = nullptr;
        platformData.context = nullptr;
        platformData.backBuffer = nullptr;
        platformData.backBufferDS = nullptr;
    #endif
    
    platformData.type = bgfx::RendererType::Count; // Auto-detect
    platformData.debug = false;
    platformData.profile = false;
    
    // Initialize bgfx
    if (!bgfx::init(platformData)) {
        s_logger.error("bgfx::init failed");
        return false;
    }
    
    // Set platform data
    bgfx::setPlatformData(platformData);
    
    // Create main view
    m_mainViewId = bgfx::createView("Main", m_width, m_height);
    
    // Enable debug features
    bgfx::setDebug(BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS | BGFX_DEBUG_IFH);
    
    s_logger.info("bgfx initialized successfully");
    s_logger.info("Renderer: %s", bgfx::getRendererName(bgfx::getRendererType()));
    
    return true;
}

bool Renderer::initResources()
{
    s_logger.info("Initializing resources...");
    
    // Initialize vertex declarations
    initVertexDeclarations();
    
    // Initialize uniforms
    initUniforms();
    
    // Initialize textures
    initTextures();
    
    // Initialize shaders
    if (!initShaders()) {
        s_logger.error("Failed to initialize shaders");
        return false;
    }
    
    s_logger.info("Resources initialized successfully");
    return true;
}

void Renderer::initVertexDeclarations()
{
    s_logger.info("Initializing vertex declarations...");
    
    // Voxel vertex declaration
    m_voxelVertexDecl
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Float) // Lightmap UV
        .add(bgfx::Attrib::TexCoord2, 1, bgfx::AttribType::Float) // Block ID
        .end();
    
    // Entity vertex declaration
    m_entityVertexDecl
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Tangent, 4, bgfx::AttribType::Float) // Bone indices/weights
        .end();
    
    // Particle vertex declaration
    m_particleVertexDecl
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float) // UV
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float) // Color
        .add(bgfx::Attrib::TexCoord1, 1, bgfx::AttribType::Float) // Size
        .add(bgfx::Attrib::TexCoord2, 1, bgfx::AttribType::Float) // Rotation
        .add(bgfx::Attrib::TexCoord3, 1, bgfx::AttribType::Float) // Texture index
        .end();
}

void Renderer::initUniforms()
{
    s_logger.info("Initializing uniforms...");
    
    // Create uniforms
    m_viewProjUniform = bgfx::createUniform("u_viewProj", bgfx::UniformType::Mat4, 1);
    m_viewUniform = bgfx::createUniform("u_view", bgfx::UniformType::Mat4, 1);
    m_projUniform = bgfx::createUniform("u_proj", bgfx::UniformType::Mat4, 1);
    m_cameraPosUniform = bgfx::createUniform("u_cameraPos", bgfx::UniformType::Vec4, 1);
    m_timeUniform = bgfx::createUniform("u_time", bgfx::UniformType::Vec4, 1);
    m_lightDirUniform = bgfx::createUniform("u_lightDir", bgfx::UniformType::Vec4, 1);
    m_lightColorUniform = bgfx::createUniform("u_lightColor", bgfx::UniformType::Vec4, 1);
    m_fogColorUniform = bgfx::createUniform("u_fogColor", bgfx::UniformType::Vec4, 1);
    m_fogDensityUniform = bgfx::createUniform("u_fogDensity", bgfx::UniformType::Vec4, 1);
    
    // Create default textures
    m_whiteTexture = createSolidColorTexture(1.0f, 1.0f, 1.0f, 1.0f);
    m_blackTexture = createSolidColorTexture(0.0f, 0.0f, 0.0f, 1.0f);
    m_missingTexture = createSolidColorTexture(1.0f, 0.0f, 1.0f, 1.0f); // Magenta for missing textures
}

void Renderer::initTextures()
{
    s_logger.info("Initializing textures...");
    // Textures are loaded on demand
}

bool Renderer::initShaders()
{
    s_logger.info("Initializing shaders...");
    
    // Load shader programs
    m_voxelProgram = loadShaderProgram("vs_voxel");
    m_entityProgram = loadShaderProgram("vs_entity");
    m_particleProgram = loadShaderProgram("vs_particle");
    
    if (m_voxelProgram.idx == bgfx::kInvalidHandle ||
        m_entityProgram.idx == bgfx::kInvalidHandle ||
        m_particleProgram.idx == bgfx::kInvalidHandle) {
        s_logger.error("Failed to load one or more shader programs");
        return false;
    }
    
    s_logger.info("Shaders initialized successfully");
    return true;
}

void Renderer::destroyResources()
{
    s_logger.info("Destroying resources...");
    
    // Destroy uniforms
    if (m_viewProjUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_viewProjUniform);
    if (m_viewUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_viewUniform);
    if (m_projUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_projUniform);
    if (m_cameraPosUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_cameraPosUniform);
    if (m_timeUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_timeUniform);
    if (m_lightDirUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_lightDirUniform);
    if (m_lightColorUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_lightColorUniform);
    if (m_fogColorUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_fogColorUniform);
    if (m_fogDensityUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_fogDensityUniform);
    
    // Destroy textures
    if (m_whiteTexture.idx != bgfx::kInvalidHandle) bgfx::destroy(m_whiteTexture);
    if (m_blackTexture.idx != bgfx::kInvalidHandle) bgfx::destroy(m_blackTexture);
    if (m_missingTexture.idx != bgfx::kInvalidHandle) bgfx::destroy(m_missingTexture);
    
    // Destroy programs
    if (m_voxelProgram.idx != bgfx::kInvalidHandle) bgfx::destroy(m_voxelProgram);
    if (m_entityProgram.idx != bgfx::kInvalidHandle) bgfx::destroy(m_entityProgram);
    if (m_particleProgram.idx != bgfx::kInvalidHandle) bgfx::destroy(m_particleProgram);
    
    // Destroy view
    if (m_mainViewId.idx != bgfx::kInvalidHandle) bgfx::destroy(m_mainViewId);
    
    s_logger.info("Resources destroyed");
}

void Renderer::setupView(bgfx::ViewId viewId, const Camera* camera)
{
    if (!camera) return;
    
    // Get camera matrices
    const float* viewMatrix = camera->getViewMatrix();
    const float* projMatrix = camera->getProjectionMatrix();
    const float* viewProjMatrix = camera->getViewProjectionMatrix();
    
    if (viewMatrix) {
        std::memcpy(m_viewMatrix, viewMatrix, sizeof(m_viewMatrix));
    }
    if (projMatrix) {
        std::memcpy(m_projMatrix, projMatrix, sizeof(m_projMatrix));
    }
    if (viewProjMatrix) {
        std::memcpy(m_viewProjMatrix, viewProjMatrix, sizeof(m_viewProjMatrix));
    }
    
    // Set view transform
    bgfx::setViewTransform(viewId, m_viewMatrix, m_projMatrix);
    
    // Set view rect
    bgfx::setViewRect(viewId, 0, 0, m_width, m_height);
}

void Renderer::updateUniforms(const Camera* camera, float deltaTime)
{
    if (!camera) return;
    
    // Update time uniform
    float timeValues[4] = {
        deltaTime,
        static_cast<float>(m_frameCount),
        m_frameTime / 1000.0f, // Frame time in seconds
        0.0f
    };
    bgfx::setUniform(m_timeUniform, timeValues);
    
    // Update camera position uniform
    float cameraPos[4] = {
        camera->getPositionX(),
        camera->getPositionY(),
        camera->getPositionZ(),
        1.0f
    };
    bgfx::setUniform(m_cameraPosUniform, cameraPos);
    
    // Update light direction uniform (sun light)
    float lightDir[4] = { 0.5f, -1.0f, 0.5f, 0.0f };
    bx::vec3Normalize(reinterpret_cast<float*>(lightDir));
    bgfx::setUniform(m_lightDirUniform, lightDir);
    
    // Update light color uniform
    float lightColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    bgfx::setUniform(m_lightColorUniform, lightColor);
    
    // Update view-proj matrix uniform
    bgfx::setUniform(m_viewProjUniform, m_viewProjMatrix);
}

void Renderer::renderScene(const Scene* scene)
{
    if (!scene) return;
    
    // Render voxels (chunks)
    renderVoxels(scene);
    
    // Render entities
    renderEntities(scene);
    
    // Render particles
    renderParticles(scene);
}

void Renderer::renderVoxels(const Scene* scene)
{
    if (!scene || m_voxelProgram.idx == bgfx::kInvalidHandle) return;
    
    // Set shader program
    bgfx::setState(
        BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z |
        BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW |
        BGFX_STATE_MSAA
    );
    
    // Set shader
    bgfx::setProgram(m_voxelProgram);
    
    // Set vertex declaration
    bgfx::setVertexDecl(m_voxelVertexDecl);
    
    // Get all chunks
    auto chunks = scene->getAllChunks();
    
    // For now, render a test cube if no chunks are available
    if (chunks.empty()) {
        createTestCube();
    }
    
    // Render each chunk
    for (auto& chunk : chunks) {
        if (!chunk) continue;
        
        // Get chunk mesh data
        // In a real implementation, this would be converted to bgfx vertex/index buffers
        // For now, we'll use the test cube
        
        // Set model matrix (chunk position)
        float modelMatrix[16];
        bx::mtxIdentity(modelMatrix);
        bx::mtxTranslate(modelMatrix,
            chunk->getPositionX(),
            chunk->getPositionY(),
            chunk->getPositionZ());
        
        // Set model matrix as a uniform (if we had one)
        // bgfx::setUniform(m_modelUniform, modelMatrix);
        
        // Submit for rendering
        // In a real implementation, we would submit the chunk's vertex/index buffers
    }
}

void Renderer::renderEntities(const Scene* scene)
{
    if (!scene || m_entityProgram.idx == bgfx::kInvalidHandle) return;
    
    // Set shader program
    bgfx::setState(
        BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z |
        BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW |
        BGFX_STATE_MSAA
    );
    
    // Set shader
    bgfx::setProgram(m_entityProgram);
    
    // Set vertex declaration
    bgfx::setVertexDecl(m_entityVertexDecl);
    
    // Get all entities
    auto entities = scene->getAllEntities();
    
    // Render each entity
    for (auto& entity : entities) {
        if (!entity) continue;
        
        // Set model matrix (entity position and rotation)
        float modelMatrix[16];
        bx::mtxIdentity(modelMatrix);
        
        // Translate
        bx::mtxTranslate(modelMatrix,
            entity->getPositionX(),
            entity->getPositionY(),
            entity->getPositionZ());
        
        // Rotate (Yaw, Pitch, Roll)
        float yaw = entity->getRotationYaw() * 0.0174532925f; // degrees to radians
        float pitch = entity->getRotationPitch() * 0.0174532925f;
        float roll = entity->getRotationRoll() * 0.0174532925f;
        
        float rotMatrix[16];
        bx::mtxRotateXYZ(rotMatrix, pitch, yaw, roll);
        bx::mtxMul(modelMatrix, modelMatrix, rotMatrix);
        
        // Scale
        float scaleMatrix[16];
        bx::mtxScale(scaleMatrix,
            entity->getScaleX(),
            entity->getScaleY(),
            entity->getScaleZ());
        bx::mtxMul(modelMatrix, modelMatrix, scaleMatrix);
        
        // In a real implementation, we would submit the entity's mesh
    }
}

void Renderer::renderParticles(const Scene* scene)
{
    if (!scene || m_particleProgram.idx == bgfx::kInvalidHandle) return;
    
    // Set shader program for particles
    bgfx::setState(
        BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z |
        BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_BLEND_ALPHA |
        BGFX_STATE_POINT_SIZE
    );
    
    // Set shader
    bgfx::setProgram(m_particleProgram);
    
    // Set vertex declaration
    bgfx::setVertexDecl(m_particleVertexDecl);
    
    // Get all particle systems
    auto particleSystems = scene->getAllParticleSystems();
    
    // Render each particle system
    for (auto& particleSystem : particleSystems) {
        if (!particleSystem) continue;
        
        // Generate vertex data
        std::vector<ParticleVertex> vertices;
        particleSystem->generateVertexData(vertices, 0.0f); // No interpolation for now
        
        // In a real implementation, we would submit the vertices
    }
}

void Renderer::renderDebug()
{
    // Draw debug information
    bgfx::dbgTextClear();
    bgfx::dbgTextPrintf(0, 0, 0x0f, "FPS: %.1f", m_fps);
    bgfx::dbgTextPrintf(0, 1, 0x0f, "Frame: %llu", m_frameCount);
    bgfx::dbgTextPrintf(0, 2, 0x0f, "Frame Time: %.2fms", m_frameTime);
}

void Renderer::createTestCube()
{
    // Create a simple test cube mesh
    // This is a temporary implementation for testing
    
    // Define cube vertices
    struct TestVertex {
        float x, y, z;
        float nx, ny, nz;
        float u, v;
    };
    
    const TestVertex vertices[24] = {
        // Front face
        {-0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
        { 0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
        { 0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f},
        {-0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
        
        // Back face
        {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f},
        { 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f},
        { 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f},
        {-0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f},
        
        // Left face
        {-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {-0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f},
        {-0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
        {-0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f},
        
        // Right face
        { 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        { 0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f},
        { 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
        { 0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f},
        
        // Top face
        {-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
        { 0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f},
        { 0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f},
        {-0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        
        // Bottom face
        {-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f},
        { 0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f},
        { 0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f},
        {-0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f},
    };
    
    const uint16_t indices[36] = {
        // Front face
        0, 1, 2, 0, 2, 3,
        // Back face
        4, 6, 5, 4, 7, 6,
        // Left face
        8, 9, 10, 8, 10, 11,
        // Right face
        12, 14, 13, 12, 15, 14,
        // Top face
        16, 18, 17, 16, 19, 18,
        // Bottom face
        20, 21, 22, 20, 22, 23,
    };
    
    // In a real implementation, we would create bgfx vertex and index buffers
    // For now, this is just a placeholder
}

bgfx::ProgramHandle Renderer::loadShaderProgram(const std::string& name)
{
    // Try to load the shader
    const char* shaderName = name.c_str();
    
    // Check if the shader exists
    bgfx::ShaderHandle vertexShader = bgfx::createShader(bgfx::makeRef(
        bgfx::readFile(shaderName, nullptr, nullptr, nullptr)
    ));
    
    if (vertexShader.idx == bgfx::kInvalidHandle) {
        s_logger.error("Failed to load vertex shader: %s", shaderName);
        return bgfx::ProgramHandle();
    }
    
    // Fragment shader name
    std::string fragmentShaderName = "fs_" + name.substr(3); // Replace "vs_" with "fs_"
    
    bgfx::ShaderHandle fragmentShader = bgfx::createShader(bgfx::makeRef(
        bgfx::readFile(fragmentShaderName.c_str(), nullptr, nullptr, nullptr)
    ));
    
    if (fragmentShader.idx == bgfx::kInvalidHandle) {
        s_logger.error("Failed to load fragment shader: %s", fragmentShaderName.c_str());
        bgfx::destroy(vertexShader);
        return bgfx::ProgramHandle();
    }
    
    // Create program
    bgfx::ProgramHandle program = bgfx::createProgram(vertexShader, fragmentShader, true);
    
    // Destroy individual shaders (they're kept alive by the program)
    bgfx::destroy(vertexShader);
    bgfx::destroy(fragmentShader);
    
    return program;
}

bgfx::TextureHandle Renderer::loadTexture(const std::string& name)
{
    // Try to load the texture
    // In a real implementation, this would load from a file
    // For now, return the missing texture
    return m_missingTexture;
}

bgfx::TextureHandle Renderer::createSolidColorTexture(float r, float g, float b, float a)
{
    // Create a 1x1 texture with the specified color
    uint32_t color = (
        (static_cast<uint8_t>(a * 255) << 24) |
        (static_cast<uint8_t>(b * 255) << 16) |
        (static_cast<uint8_t>(g * 255) << 8) |
        (static_cast<uint8_t>(r * 255) << 0)
    );
    
    bgfx::TextureHandle handle = bgfx::createTexture(
        bgfx::TextureInfo::create(1, 1, 1, false, false, 1, bgfx::TextureFormat::BGRA8)
    );
    
    if (handle.idx != bgfx::kInvalidHandle) {
        bgfx::updateTexture(handle, 0, 0, 0, 0, 1, 1, &color);
    }
    
    return handle;
}

void Renderer::updateFps()
{
    static uint64_t lastTime = 0;
    static uint64_t frameCounter = 0;
    
    uint64_t currentTime = bx::getHPCounter();
    uint64_t elapsed = currentTime - lastTime;
    
    if (elapsed > 0) {
        float elapsedSeconds = static_cast<float>(elapsed) / static_cast<float>(bx::getHPFrequency());
        m_frameTime = elapsedSeconds * 1000.0f; // Convert to milliseconds
        
        frameCounter++;
        
        // Update FPS every second
        static float fpsTimer = 0.0f;
        fpsTimer += elapsedSeconds;
        
        if (fpsTimer >= 1.0f) {
            m_fps = static_cast<float>(frameCounter) / fpsTimer;
            frameCounter = 0;
            fpsTimer = 0.0f;
        }
    }
    
    lastTime = currentTime;
}

} // namespace renderbridge
