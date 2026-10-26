#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

// bgfx includes
#include <bgfx/bgfx.h>

namespace renderbridge {

// Forward declarations
class Scene;
class Camera;
class Chunk;
class Entity;
class ParticleSystem;
class Weather;

/**
 * @struct MeshHandle
 * @brief Handle to a mesh in the renderer
 */
struct MeshHandle {
    bgfx::VertexBufferHandle vbo;
    bgfx::IndexBufferHandle ibo;
    uint32_t vertexCount;
    uint32_t indexCount;
};

/**
 * @struct TextureHandle
 * @brief Handle to a texture in the renderer
 */
struct TextureHandle {
    bgfx::TextureHandle handle;
    std::string name;
};

/**
 * @struct EntityHandle
 * @brief Handle to an entity in the renderer
 */
struct EntityHandle {
    uint64_t id;
    MeshHandle mesh;
    bgfx::UniformHandle transformUniform;
    bgfx::UniformHandle colorUniform;
};

/**
 * @struct ChunkHandle
 * @brief Handle to a chunk in the renderer
 */
struct ChunkHandle {
    uint64_t id;
    MeshHandle mesh;
    bgfx::UniformHandle transformUniform;
};

/**
 * @struct ParticleSystemHandle
 * @brief Handle to a particle system in the renderer
 */
struct ParticleSystemHandle {
    uint64_t id;
    MeshHandle mesh;
    bgfx::VertexBufferHandle particleBuffer;
    bgfx::UniformHandle particleUniform;
};

/**
 * @class ProtocolAdapter
 * @brief Adapter class to convert bridge protocol messages to bgfx types
 * 
 * This class provides the translation layer between the Java bridge protocol
 * and the C++/bgfx renderer. It converts message payloads into bgfx-compatible
 * data structures (vertex buffers, index buffers, textures, uniforms).
 */
class ProtocolAdapter {
public:
    /**
     * @brief Initialize the protocol adapter
     * @param renderer bgfx renderer instance
     */
    static void initialize();
    
    /**
     * @brief Shutdown the protocol adapter
     */
    static void shutdown();
    
    // ========================================================================
    // Camera conversion
    // ========================================================================
    
    /**
     * @brief Convert a camera message payload to a Camera object
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     * @return Shared pointer to the Camera object
     */
    static std::shared_ptr<Camera> createCameraFromPayload(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Create a bgfx view from a Camera object
     * @param camera Camera object
     * @param viewId View ID to use
     * @return View ID
     */
    static bgfx::ViewId createView(const std::shared_ptr<Camera>& camera, bgfx::ViewId viewId = 0);
    
    /**
     * @brief Update the camera view matrices
     * @param camera Camera object
     * @param viewId View ID
     */
    static void updateViewMatrices(const std::shared_ptr<Camera>& camera, bgfx::ViewId viewId);
    
    // ========================================================================
    // Chunk conversion
    // ========================================================================
    
    /**
     * @brief Convert a chunk message payload to a Chunk object
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     * @return Shared pointer to the Chunk object
     */
    static std::shared_ptr<Chunk> createChunkFromPayload(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Create a mesh from a Chunk object
     * @param chunk Chunk object
     * @return MeshHandle
     */
    static MeshHandle createMeshFromChunk(const std::shared_ptr<Chunk>& chunk);
    
    /**
     * @brief Update a chunk mesh
     * @param chunkHandle Chunk handle
     * @param chunk Chunk object with new data
     */
    static void updateChunkMesh(ChunkHandle& chunkHandle, const std::shared_ptr<Chunk>& chunk);
    
    /**
     * @brief Destroy a chunk mesh
     * @param chunkHandle Chunk handle
     */
    static void destroyChunkMesh(ChunkHandle& chunkHandle);
    
    // ========================================================================
    // Entity conversion
    // ========================================================================
    
    /**
     * @brief Convert an entity message payload to an Entity object
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     * @return Shared pointer to the Entity object
     */
    static std::shared_ptr<Entity> createEntityFromPayload(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Create an entity handle from an Entity object
     * @param entity Entity object
     * @return EntityHandle
     */
    static EntityHandle createEntityHandle(const std::shared_ptr<Entity>& entity);
    
    /**
     * @brief Update an entity
     * @param entityHandle Entity handle
     * @param entity Entity object with new data
     */
    static void updateEntity(EntityHandle& entityHandle, const std::shared_ptr<Entity>& entity);
    
    /**
     * @brief Destroy an entity
     * @param entityHandle Entity handle
     */
    static void destroyEntity(EntityHandle& entityHandle);
    
    // ========================================================================
    // Particle conversion
    // ========================================================================
    
    /**
     * @brief Convert a particle message payload to particle data
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     * @return Vector of particle data
     */
    static std::vector<float> createParticleDataFromPayload(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Update particle system
     * @param particleSystemHandle Particle system handle
     * @param particleData Particle data
     */
    static void updateParticleSystem(ParticleSystemHandle& particleSystemHandle, const std::vector<float>& particleData);
    
    // ========================================================================
    // Weather conversion
    // ========================================================================
    
    /**
     * @brief Convert a weather message payload to a Weather object
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     * @return Shared pointer to the Weather object
     */
    static std::shared_ptr<Weather> createWeatherFromPayload(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Update weather uniforms
     * @param weather Weather object
     */
    static void updateWeatherUniforms(const std::shared_ptr<Weather>& weather);
    
    // ========================================================================
    // GUI conversion
    // ========================================================================
    
    /**
     * @brief Convert a GUI message payload to GUI data
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     * @return GUI data
     */
    static void processGuiMessage(const uint8_t* payload, uint32_t payloadSize);
    
    // ========================================================================
    // Frame management
    // ========================================================================
    
    /**
     * @brief Begin a new frame
     * @param frameNumber Frame number
     * @param timestamp Timestamp
     */
    static void beginFrame(int64_t frameNumber, int64_t timestamp);
    
    /**
     * @brief End the current frame
     */
    static void endFrame();
    
    /**
     * @brief Process a frame start message
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     */
    static void processFrameStart(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Process a frame end message
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     */
    static void processFrameEnd(const uint8_t* payload, uint32_t payloadSize);
    
    // ========================================================================
    // Resource management
    // ========================================================================
    
    /**
     * @brief Get or create a texture
     * @param name Texture name
     * @return TextureHandle
     */
    static TextureHandle getOrCreateTexture(const std::string& name);
    
    /**
     * @brief Release a texture
     * @param textureHandle Texture handle
     */
    static void releaseTexture(TextureHandle& textureHandle);
    
    /**
     * @brief Get or create a uniform
     * @param name Uniform name
     * @param type bgfx uniform type
     * @return bgfx::UniformHandle
     */
    static bgfx::UniformHandle getOrCreateUniform(const std::string& name, bgfx::UniformType::Enum type);
    
    /**
     * @brief Cleanup unused resources
     */
    static void cleanupResources();

private:
    // Resource caches
    static std::unordered_map<std::string, TextureHandle> s_textureCache;
    static std::unordered_map<std::string, bgfx::UniformHandle> s_uniformCache;
    static std::unordered_map<uint64_t, ChunkHandle> s_chunkCache;
    static std::unordered_map<uint64_t, EntityHandle> s_entityCache;
    static std::unordered_map<uint64_t, ParticleSystemHandle> s_particleSystemCache;
    
    // Current state
    static int64_t s_currentFrameNumber;
    static bool s_frameStarted;
    
    // Uniform handles for global state
    static bgfx::UniformHandle s_viewProjUniform;
    static bgfx::UniformHandle s_viewUniform;
    static bgfx::UniformHandle s_projUniform;
    static bgfx::UniformHandle s_cameraPosUniform;
    static bgfx::UniformHandle s_timeUniform;
    static bgfx::UniformHandle s_weatherUniform;
    
    // Vertex declaration for voxel rendering
    static bgfx::VertexDecl s_voxelVertexDecl;
    
    /**
     * @brief Initialize the voxel vertex declaration
     */
    static void initVoxelVertexDecl();
    
    /**
     * @brief Create global uniforms
     */
    static void createGlobalUniforms();
    
    /**
     * @brief Destroy global uniforms
     */
    static void destroyGlobalUniforms();
};

} // namespace renderbridge
