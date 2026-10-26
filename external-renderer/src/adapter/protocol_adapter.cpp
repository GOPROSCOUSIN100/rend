#include "protocol_adapter.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include "entity.hpp"
#include "particle.hpp"
#include "weather.hpp"
#include "utils/logger.hpp"

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <cstring>
#include <algorithm>

namespace renderbridge {

// Static logger
static Logger s_logger("ProtocolAdapter");

// Static members
std::unordered_map<std::string, TextureHandle> ProtocolAdapter::s_textureCache;
std::unordered_map<std::string, bgfx::UniformHandle> ProtocolAdapter::s_uniformCache;
std::unordered_map<uint64_t, ChunkHandle> ProtocolAdapter::s_chunkCache;
std::unordered_map<uint64_t, EntityHandle> ProtocolAdapter::s_entityCache;
std::unordered_map<uint64_t, ParticleSystemHandle> ProtocolAdapter::s_particleSystemCache;

int64_t ProtocolAdapter::s_currentFrameNumber = 0;
bool ProtocolAdapter::s_frameStarted = false;

bgfx::UniformHandle ProtocolAdapter::s_viewProjUniform;
bgfx::UniformHandle ProtocolAdapter::s_viewUniform;
bgfx::UniformHandle ProtocolAdapter::s_projUniform;
bgfx::UniformHandle ProtocolAdapter::s_cameraPosUniform;
bgfx::UniformHandle ProtocolAdapter::s_timeUniform;
bgfx::UniformHandle ProtocolAdapter::s_weatherUniform;

bgfx::VertexDecl ProtocolAdapter::s_voxelVertexDecl;

void ProtocolAdapter::initialize()
{
    s_logger.info("Initializing protocol adapter...");
    
    // Initialize vertex declarations
    initVoxelVertexDecl();
    
    // Create global uniforms
    createGlobalUniforms();
    
    s_logger.info("Protocol adapter initialized successfully");
}

void ProtocolAdapter::shutdown()
{
    s_logger.info("Shutting down protocol adapter...");
    
    // Destroy global uniforms
    destroyGlobalUniforms();
    
    // Clear caches
    s_textureCache.clear();
    s_uniformCache.clear();
    s_chunkCache.clear();
    s_entityCache.clear();
    s_particleSystemCache.clear();
    
    s_logger.info("Protocol adapter shut down");
}

// ========================================================================
// Camera conversion
// ========================================================================

std::shared_ptr<Camera> ProtocolAdapter::createCameraFromPayload(const uint8_t* payload, uint32_t payloadSize)
{
    auto camera = std::make_shared<Camera>();
    
    if (payload && payloadSize >= 104) {
        // Parse camera data
        // Format: positionX(float), positionY(float), positionZ(float),
        //        rotationPitch(float), rotationYaw(float), rotationRoll(float),
        //        fov(float), nearPlane(float), farPlane(float),
        //        viewportWidth(int), viewportHeight(int),
        //        viewMatrix(16 floats), projMatrix(16 floats), viewProjMatrix(16 floats),
        //        flags(int)
        
        uint32_t offset = 0;
        
        // Position
        camera->setPosition(
            *reinterpret_cast<const float*>(payload + offset),
            *reinterpret_cast<const float*>(payload + offset + 4),
            *reinterpret_cast<const float*>(payload + offset + 8)
        );
        offset += 12;
        
        // Rotation
        camera->setRotation(
            *reinterpret_cast<const float*>(payload + offset),
            *reinterpret_cast<const float*>(payload + offset + 4),
            *reinterpret_cast<const float*>(payload + offset + 8)
        );
        offset += 12;
        
        // FOV, near, far
        camera->setFov(*reinterpret_cast<const float*>(payload + offset));
        offset += 4;
        camera->setNearPlane(*reinterpret_cast<const float*>(payload + offset));
        offset += 4;
        camera->setFarPlane(*reinterpret_cast<const float*>(payload + offset));
        offset += 4;
        
        // Viewport
        int32_t width = *reinterpret_cast<const int32_t*>(payload + offset);
        offset += 4;
        int32_t height = *reinterpret_cast<const int32_t*>(payload + offset);
        offset += 4;
        camera->setViewport(width, height);
        
        // View matrix
        float viewMatrix[16];
        std::memcpy(viewMatrix, payload + offset, 64);
        offset += 64;
        camera->setViewMatrix(viewMatrix);
        
        // Projection matrix
        float projMatrix[16];
        std::memcpy(projMatrix, payload + offset, 64);
        offset += 64;
        camera->setProjectionMatrix(projMatrix);
        
        // View-projection matrix
        float viewProjMatrix[16];
        std::memcpy(viewProjMatrix, payload + offset, 64);
        offset += 64;
        camera->setViewProjectionMatrix(viewProjMatrix);
        
        // Flags
        if (offset + 4 <= payloadSize) {
            camera->setFlags(*reinterpret_cast<const uint32_t*>(payload + offset));
        }
    } else {
        s_logger.warn("Invalid camera payload size: %u", payloadSize);
    }
    
    return camera;
}

bgfx::ViewId ProtocolAdapter::createView(const std::shared_ptr<Camera>& camera, bgfx::ViewId viewId)
{
    if (!camera) {
        return viewId;
    }
    
    // Create view if not already created
    if (viewId.idx == bgfx::kInvalidHandle) {
        viewId = bgfx::createView("Main", 0, 0);
    }
    
    // Set view matrices
    updateViewMatrices(camera, viewId);
    
    return viewId;
}

void ProtocolAdapter::updateViewMatrices(const std::shared_ptr<Camera>& camera, bgfx::ViewId viewId)
{
    if (!camera || viewId.idx == bgfx::kInvalidHandle) {
        return;
    }
    
    // Set view transform
    const float* viewMatrix = camera->getViewMatrix();
    const float* projMatrix = camera->getProjectionMatrix();
    
    if (viewMatrix && projMatrix) {
        bgfx::setViewTransform(viewId, viewMatrix, projMatrix);
    }
    
    // Set view rect
    int32_t width = camera->getViewportWidth();
    int32_t height = camera->getViewportHeight();
    bgfx::setViewRect(viewId, 0, 0, width, height);
    
    // Update uniforms
    if (s_viewUniform.idx != bgfx::kInvalidHandle && viewMatrix) {
        bgfx::setUniform(s_viewUniform, viewMatrix);
    }
    if (s_projUniform.idx != bgfx::kInvalidHandle && projMatrix) {
        bgfx::setUniform(s_projUniform, projMatrix);
    }
    
    // Calculate and update view-proj matrix
    if (s_viewProjUniform.idx != bgfx::kInvalidHandle && viewMatrix && projMatrix) {
        float viewProjMatrix[16];
        bx::mtxMul(viewProjMatrix, projMatrix, viewMatrix);
        bgfx::setUniform(s_viewProjUniform, viewProjMatrix);
    }
    
    // Update camera position uniform
    if (s_cameraPosUniform.idx != bgfx::kInvalidHandle) {
        float cameraPos[4] = {
            camera->getPositionX(),
            camera->getPositionY(),
            camera->getPositionZ(),
            1.0f
        };
        bgfx::setUniform(s_cameraPosUniform, cameraPos);
    }
}

// ========================================================================
// Chunk conversion
// ========================================================================

std::shared_ptr<Chunk> ProtocolAdapter::createChunkFromPayload(const uint8_t* payload, uint32_t payloadSize)
{
    auto chunk = std::make_shared<Chunk>();
    
    if (payload && payloadSize >= 16) {
        // Parse chunk data
        // Format: x(int), y(int), z(int), flags(int),
        //        blockCount(int), blocks[] (each: type(byte), data(int))
        
        uint32_t offset = 0;
        
        // Position
        int32_t x = *reinterpret_cast<const int32_t*>(payload + offset);
        offset += 4;
        int32_t y = *reinterpret_cast<const int32_t*>(payload + offset);
        offset += 4;
        int32_t z = *reinterpret_cast<const int32_t*>(payload + offset);
        offset += 4;
        chunk->setPosition(x, y, z);
        
        // Flags
        uint32_t flags = *reinterpret_cast<const uint32_t*>(payload + offset);
        offset += 4;
        chunk->setFlags(flags);
        
        // Block count
        uint32_t blockCount = *reinterpret_cast<const uint32_t*>(payload + offset);
        offset += 4;
        
        // Blocks
        for (uint32_t i = 0; i < blockCount && offset + 5 <= payloadSize; ++i) {
            int32_t bx = *reinterpret_cast<const int16_t*>(payload + offset);
            offset += 2;
            int32_t by = *reinterpret_cast<const int16_t*>(payload + offset);
            offset += 2;
            int32_t bz = *reinterpret_cast<const int16_t*>(payload + offset);
            offset += 2;
            
            uint8_t blockType = *reinterpret_cast<const uint8_t*>(payload + offset);
            offset += 1;
            
            uint32_t blockData = *reinterpret_cast<const uint32_t*>(payload + offset);
            offset += 4;
            
            Chunk::BlockInfo block;
            block.type = static_cast<Chunk::BlockType>(blockType);
            block.data = blockData;
            chunk->setBlock(bx, by, bz, block);
        }
        
        // Generate mesh
        chunk->generateMesh();
    } else {
        s_logger.warn("Invalid chunk payload size: %u", payloadSize);
    }
    
    return chunk;
}

MeshHandle ProtocolAdapter::createMeshFromChunk(const std::shared_ptr<Chunk>& chunk)
{
    MeshHandle mesh;
    mesh.vbo.idx = bgfx::kInvalidHandle;
    mesh.ibo.idx = bgfx::kInvalidHandle;
    mesh.vertexCount = 0;
    mesh.indexCount = 0;
    
    if (!chunk) {
        return mesh;
    }
    
    // Generate mesh data
    std::vector<float> vertices;
    std::vector<uint16_t> indices;
    
    // Get chunk vertex data
    // This is a simplified version - in a real implementation, we would
    // use the Chunk::generateMesh method and convert to bgfx format
    
    // For now, create a simple cube
    const float cubeVertices[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    };
    
    const uint16_t cubeIndices[] = {
        0, 1, 2, 0, 2, 3
    };
    
    vertices.assign(cubeVertices, cubeVertices + 32);
    indices.assign(cubeIndices, cubeIndices + 6);
    
    // Create vertex buffer
    mesh.vbo = bgfx::createVertexBuffer(
        bgfx::makeRef(vertices.data(), vertices.size() * sizeof(float)),
        s_voxelVertexDecl
    );
    
    // Create index buffer
    mesh.ibo = bgfx::createIndexBuffer(
        bgfx::makeRef(indices.data(), indices.size() * sizeof(uint16_t))
    );
    
    mesh.vertexCount = static_cast<uint32_t>(vertices.size() / 8);
    mesh.indexCount = static_cast<uint32_t>(indices.size());
    
    return mesh;
}

void ProtocolAdapter::updateChunkMesh(ChunkHandle& chunkHandle, const std::shared_ptr<Chunk>& chunk)
{
    if (!chunk) return;
    
    // Destroy old mesh
    destroyChunkMesh(chunkHandle);
    
    // Create new mesh
    chunkHandle = createMeshFromChunk(chunk);
}

void ProtocolAdapter::destroyChunkMesh(ChunkHandle& chunkHandle)
{
    if (chunkHandle.vbo.idx != bgfx::kInvalidHandle) {
        bgfx::destroy(chunkHandle.vbo);
        chunkHandle.vbo.idx = bgfx::kInvalidHandle;
    }
    if (chunkHandle.ibo.idx != bgfx::kInvalidHandle) {
        bgfx::destroy(chunkHandle.ibo);
        chunkHandle.ibo.idx = bgfx::kInvalidHandle;
    }
    chunkHandle.vertexCount = 0;
    chunkHandle.indexCount = 0;
}

// ========================================================================
// Entity conversion
// ========================================================================

std::shared_ptr<Entity> ProtocolAdapter::createEntityFromPayload(const uint8_t* payload, uint32_t payloadSize)
{
    auto entity = std::make_shared<Entity>();
    
    if (payload && payloadSize >= 54) {
        // Parse entity data
        // Format: entityId(int64), type(byte), modelId(short),
        //        positionX(float), positionY(float), positionZ(float),
        //        rotationPitch(float), rotationYaw(float), rotationRoll(float),
        //        velocityX(float), velocityY(float), velocityZ(float),
        //        metadataSize(short), metadata(bytes),
        //        flags(byte)
        
        uint32_t offset = 0;
        
        // Entity ID
        entity->setEntityId(*reinterpret_cast<const uint64_t*>(payload + offset));
        offset += 8;
        
        // Type
        uint8_t type = *reinterpret_cast<const uint8_t*>(payload + offset);
        offset += 1;
        entity->setType(static_cast<Entity::Type>(type));
        
        // Model ID (skip for now)
        offset += 2;
        
        // Position
        entity->setPosition(
            *reinterpret_cast<const float*>(payload + offset),
            *reinterpret_cast<const float*>(payload + offset + 4),
            *reinterpret_cast<const float*>(payload + offset + 8)
        );
        offset += 12;
        
        // Rotation
        entity->setRotation(
            *reinterpret_cast<const float*>(payload + offset),
            *reinterpret_cast<const float*>(payload + offset + 4),
            *reinterpret_cast<const float*>(payload + offset + 8)
        );
        offset += 12;
        
        // Velocity
        entity->setVelocity(
            *reinterpret_cast<const float*>(payload + offset),
            *reinterpret_cast<const float*>(payload + offset + 4),
            *reinterpret_cast<const float*>(payload + offset + 8)
        );
        offset += 12;
        
        // Metadata size
        uint16_t metadataSize = *reinterpret_cast<const uint16_t*>(payload + offset);
        offset += 2;
        
        // Skip metadata for now
        offset += metadataSize;
        
        // Flags
        if (offset < payloadSize) {
            uint8_t flags = *reinterpret_cast<const uint8_t*>(payload + offset);
            entity->setFlags(flags);
        }
    } else {
        s_logger.warn("Invalid entity payload size: %u", payloadSize);
    }
    
    return entity;
}

EntityHandle ProtocolAdapter::createEntityHandle(const std::shared_ptr<Entity>& entity)
{
    EntityHandle handle;
    handle.id = entity ? entity->getEntityId() : 0;
    handle.mesh = createMeshFromChunk(nullptr); // Create a simple cube mesh
    handle.transformUniform.idx = bgfx::kInvalidHandle;
    handle.colorUniform.idx = bgfx::kInvalidHandle;
    
    if (entity && handle.mesh.vbo.idx != bgfx::kInvalidHandle) {
        // Create transform uniform
        char uniformName[64];
        snprintf(uniformName, sizeof(uniformName), "u_entityTransform[%llu]", entity->getEntityId());
        handle.transformUniform = bgfx::createUniform(uniformName, bgfx::UniformType::Mat4, 1);
    }
    
    return handle;
}

void ProtocolAdapter::updateEntity(EntityHandle& entityHandle, const std::shared_ptr<Entity>& entity)
{
    if (!entity) return;
    
    // Update transform uniform
    if (entityHandle.transformUniform.idx != bgfx::kInvalidHandle) {
        float modelMatrix[16];
        bx::mtxIdentity(modelMatrix);
        
        // Translate
        bx::mtxTranslate(modelMatrix,
            entity->getPositionX(),
            entity->getPositionY(),
            entity->getPositionZ());
        
        // Rotate
        float yaw = entity->getRotationYaw() * 0.0174532925f;
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
        
        bgfx::setUniform(entityHandle.transformUniform, modelMatrix);
    }
}

void ProtocolAdapter::destroyEntity(EntityHandle& entityHandle)
{
    if (entityHandle.transformUniform.idx != bgfx::kInvalidHandle) {
        bgfx::destroy(entityHandle.transformUniform);
        entityHandle.transformUniform.idx = bgfx::kInvalidHandle;
    }
    if (entityHandle.colorUniform.idx != bgfx::kInvalidHandle) {
        bgfx::destroy(entityHandle.colorUniform);
        entityHandle.colorUniform.idx = bgfx::kInvalidHandle;
    }
    destroyChunkMesh(entityHandle.mesh);
    entityHandle.id = 0;
}

// ========================================================================
// Particle conversion
// ========================================================================

std::vector<float> ProtocolAdapter::createParticleDataFromPayload(const uint8_t* payload, uint32_t payloadSize)
{
    std::vector<float> particleData;
    
    if (payload && payloadSize >= 20) {
        // Parse particle system data
        // Format: systemId(int64), type(short), positionX(float), positionY(float), positionZ(float),
        //        particleCount(int), particles[] (each: x,y,z,velocityX,velocityY,velocityZ,size,colorR,colorG,colorB,colorA)
        
        uint32_t offset = 0;
        
        // System ID
        uint64_t systemId = *reinterpret_cast<const uint64_t*>(payload + offset);
        offset += 8;
        
        // Type
        uint16_t type = *reinterpret_cast<const uint16_t*>(payload + offset);
        offset += 2;
        
        // Position
        float posX = *reinterpret_cast<const float*>(payload + offset);
        offset += 4;
        float posY = *reinterpret_cast<const float*>(payload + offset);
        offset += 4;
        float posZ = *reinterpret_cast<const float*>(payload + offset);
        offset += 4;
        
        // Particle count
        uint32_t particleCount = *reinterpret_cast<const uint32_t*>(payload + offset);
        offset += 4;
        
        // Particles
        for (uint32_t i = 0; i < particleCount && offset + 44 <= payloadSize; ++i) {
            // Position
            float x = *reinterpret_cast<const float*>(payload + offset);
            offset += 4;
            float y = *reinterpret_cast<const float*>(payload + offset);
            offset += 4;
            float z = *reinterpret_cast<const float*>(payload + offset);
            offset += 4;
            
            // Velocity
            offset += 12; // Skip velocity for now
            
            // Size
            float size = *reinterpret_cast<const float*>(payload + offset);
            offset += 4;
            
            // Color
            float r = *reinterpret_cast<const float*>(payload + offset);
            offset += 4;
            float g = *reinterpret_cast<const float*>(payload + offset);
            offset += 4;
            float b = *reinterpret_cast<const float*>(payload + offset);
            offset += 4;
            float a = *reinterpret_cast<const float*>(payload + offset);
            offset += 4;
            
            // Add to particle data
            particleData.push_back(x);
            particleData.push_back(y);
            particleData.push_back(z);
            particleData.push_back(size);
            particleData.push_back(r);
            particleData.push_back(g);
            particleData.push_back(b);
            particleData.push_back(a);
        }
    } else {
        s_logger.warn("Invalid particle payload size: %u", payloadSize);
    }
    
    return particleData;
}

void ProtocolAdapter::updateParticleSystem(ParticleSystemHandle& particleSystemHandle, const std::vector<float>& particleData)
{
    // In a real implementation, this would update the particle system's vertex buffer
    // For now, this is a placeholder
}

// ========================================================================
// Weather conversion
// ========================================================================

std::shared_ptr<Weather> ProtocolAdapter::createWeatherFromPayload(const uint8_t* payload, uint32_t payloadSize)
{
    auto weather = std::make_shared<Weather>();
    
    if (payload && payloadSize >= 53) {
        // Parse weather data
        // Format: type(byte), strength(float), time(int64),
        //        rainGradient(float), thunderGradient(float),
        //        fogDensity(float), fogColorR(float), fogColorG(float), fogColorB(float),
        //        windDirection(float), windStrength(float),
        //        temperature(float), wetness(float)
        
        uint32_t offset = 0;
        
        // Type
        uint8_t type = *reinterpret_cast<const uint8_t*>(payload + offset);
        offset += 1;
        weather->setType(static_cast<Weather::Type>(type));
        
        // Strength
        weather->setStrength(*reinterpret_cast<const float*>(payload + offset));
        offset += 4;
        
        // Time
        weather->setTime(*reinterpret_cast<const int64_t*>(payload + offset));
        offset += 8;
        
        // Rain gradient
        weather->setRainGradient(*reinterpret_cast<const float*>(payload + offset));
        offset += 4;
        
        // Thunder gradient
        weather->setThunderGradient(*reinterpret_cast<const float*>(payload + offset));
        offset += 4;
        
        // Fog density
        weather->setFogDensity(*reinterpret_cast<const float*>(payload + offset));
        offset += 4;
        
        // Fog color
        float r = *reinterpret_cast<const float*>(payload + offset);
        offset += 4;
        float g = *reinterpret_cast<const float*>(payload + offset);
        offset += 4;
        float b = *reinterpret_cast<const float*>(payload + offset);
        offset += 4;
        weather->setFogColor(r, g, b);
        
        // Wind direction
        weather->setWindDirection(*reinterpret_cast<const float*>(payload + offset));
        offset += 4;
        
        // Wind strength
        weather->setWindStrength(*reinterpret_cast<const float*>(payload + offset));
        offset += 4;
        
        // Temperature
        weather->setTemperature(*reinterpret_cast<const float*>(payload + offset));
        offset += 4;
        
        // Wetness
        weather->setWetness(*reinterpret_cast<const float*>(payload + offset));
        offset += 4;
    } else {
        s_logger.warn("Invalid weather payload size: %u", payloadSize);
    }
    
    return weather;
}

void ProtocolAdapter::updateWeatherUniforms(const std::shared_ptr<Weather>& weather)
{
    if (!weather || s_weatherUniform.idx == bgfx::kInvalidHandle) {
        return;
    }
    
    // Get weather uniforms
    float uniforms[16];
    weather->getUniforms(uniforms);
    
    // Set weather uniform
    bgfx::setUniform(s_weatherUniform, uniforms);
}

// ========================================================================
// GUI conversion
// ========================================================================

void ProtocolAdapter::processGuiMessage(const uint8_t* payload, uint32_t payloadSize)
{
    // GUI messages are handled separately
    // For now, this is a placeholder
}

// ========================================================================
// Frame management
// ========================================================================

void ProtocolAdapter::beginFrame(int64_t frameNumber, int64_t timestamp)
{
    s_currentFrameNumber = frameNumber;
    s_frameStarted = true;
}

void ProtocolAdapter::endFrame()
{
    s_frameStarted = false;
}

void ProtocolAdapter::processFrameStart(const uint8_t* payload, uint32_t payloadSize)
{
    if (payload && payloadSize >= 16) {
        int64_t frameNumber = *reinterpret_cast<const int64_t*>(payload);
        int64_t timestamp = *reinterpret_cast<const int64_t*>(payload + 8);
        beginFrame(frameNumber, timestamp);
    }
}

void ProtocolAdapter::processFrameEnd(const uint8_t* payload, uint32_t payloadSize)
{
    endFrame();
}

// ========================================================================
// Resource management
// ========================================================================

TextureHandle ProtocolAdapter::getOrCreateTexture(const std::string& name)
{
    auto it = s_textureCache.find(name);
    if (it != s_textureCache.end()) {
        return it->second;
    }
    
    // Create new texture
    TextureHandle handle;
    handle.handle.idx = bgfx::kInvalidHandle;
    handle.name = name;
    
    // In a real implementation, this would load the texture from a file
    // For now, create a placeholder texture
    handle.handle = bgfx::createTexture(
        bgfx::TextureInfo::create(1, 1, 1, false, false, 1, bgfx::TextureFormat::BGRA8)
    );
    
    if (handle.handle.idx != bgfx::kInvalidHandle) {
        // Create a solid color (magenta for missing textures)
        uint32_t color = 0xFFFF00FF; // ABGR format
        bgfx::updateTexture(handle.handle, 0, 0, 0, 0, 1, 1, &color);
    }
    
    s_textureCache[name] = handle;
    return handle;
}

void ProtocolAdapter::releaseTexture(TextureHandle& textureHandle)
{
    if (textureHandle.handle.idx != bgfx::kInvalidHandle) {
        bgfx::destroy(textureHandle.handle);
        textureHandle.handle.idx = bgfx::kInvalidHandle;
    }
    textureHandle.name.clear();
}

bgfx::UniformHandle ProtocolAdapter::getOrCreateUniform(const std::string& name, bgfx::UniformType::Enum type)
{
    auto it = s_uniformCache.find(name);
    if (it != s_uniformCache.end()) {
        return it->second;
    }
    
    // Create new uniform
    bgfx::UniformHandle handle = bgfx::createUniform(name.c_str(), type, 1);
    s_uniformCache[name] = handle;
    return handle;
}

void ProtocolAdapter::cleanupResources()
{
    // Release all textures
    for (auto& pair : s_textureCache) {
        releaseTexture(pair.second);
    }
    s_textureCache.clear();
    
    // Release all uniforms
    for (auto& pair : s_uniformCache) {
        bgfx::destroy(pair.second);
    }
    s_uniformCache.clear();
    
    // Release all chunk meshes
    for (auto& pair : s_chunkCache) {
        destroyChunkMesh(pair.second);
    }
    s_chunkCache.clear();
    
    // Release all entity handles
    for (auto& pair : s_entityCache) {
        destroyEntity(pair.second);
    }
    s_entityCache.clear();
    
    // Release all particle system handles
    for (auto& pair : s_particleSystemCache) {
        // In a real implementation, we would destroy particle system resources
    }
    s_particleSystemCache.clear();
}

// ========================================================================
// Private methods
// ========================================================================

void ProtocolAdapter::initVoxelVertexDecl()
{
    s_voxelVertexDecl
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();
}

void ProtocolAdapter::createGlobalUniforms()
{
    s_viewProjUniform = bgfx::createUniform("u_viewProj", bgfx::UniformType::Mat4, 1);
    s_viewUniform = bgfx::createUniform("u_view", bgfx::UniformType::Mat4, 1);
    s_projUniform = bgfx::createUniform("u_proj", bgfx::UniformType::Mat4, 1);
    s_cameraPosUniform = bgfx::createUniform("u_cameraPos", bgfx::UniformType::Vec4, 1);
    s_timeUniform = bgfx::createUniform("u_time", bgfx::UniformType::Vec4, 1);
    s_weatherUniform = bgfx::createUniform("u_weather", bgfx::UniformType::Vec4, 4); // 4 vec4s = 16 floats
}

void ProtocolAdapter::destroyGlobalUniforms()
{
    if (s_viewProjUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(s_viewProjUniform);
    if (s_viewUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(s_viewUniform);
    if (s_projUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(s_projUniform);
    if (s_cameraPosUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(s_cameraPosUniform);
    if (s_timeUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(s_timeUniform);
    if (s_weatherUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(s_weatherUniform);
    
    s_viewProjUniform.idx = bgfx::kInvalidHandle;
    s_viewUniform.idx = bgfx::kInvalidHandle;
    s_projUniform.idx = bgfx::kInvalidHandle;
    s_cameraPosUniform.idx = bgfx::kInvalidHandle;
    s_timeUniform.idx = bgfx::kInvalidHandle;
    s_weatherUniform.idx = bgfx::kInvalidHandle;
}

} // namespace renderbridge
