#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>

namespace renderbridge {

// Forward declarations
class Camera;
class Chunk;
class Entity;
class ParticleSystem;
class Weather;

/**
 * @class Scene
 * @brief Represents the complete 3D scene for rendering
 * 
 * This class manages all scene objects (camera, chunks, entities, particles, weather)
 * and provides methods for updating and rendering the scene.
 */
class Scene {
public:
    /**
     * @brief Constructor
     */
    Scene();
    
    /**
     * @brief Destructor
     */
    ~Scene();
    
    /**
     * @brief Initialize the scene
     */
    void initialize();
    
    /**
     * @brief Shutdown the scene
     */
    void shutdown();
    
    /**
     * @brief Update the scene with new data from a message
     * @param messageType Type of the message
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     */
    void updateFromMessage(uint16_t messageType, const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Begin a new frame
     * @param frameNumber Frame number
     * @param timestamp Timestamp
     */
    void beginFrame(int64_t frameNumber, int64_t timestamp);
    
    /**
     * @brief End the current frame
     */
    void endFrame();
    
    // Camera access
    /**
     * @brief Get the current camera
     * @return Shared pointer to the camera
     */
    std::shared_ptr<Camera> getCamera() const;
    
    /**
     * @brief Set the camera
     * @param camera Camera to set
     */
    void setCamera(std::shared_ptr<Camera> camera);
    
    // Chunk access
    /**
     * @brief Get a chunk by position
     * @param x X position
     * @param y Y position
     * @param z Z position
     * @return Shared pointer to the chunk, or nullptr if not found
     */
    std::shared_ptr<Chunk> getChunk(int32_t x, int32_t y, int32_t z) const;
    
    /**
     * @brief Add or update a chunk
     * @param chunk Chunk to add or update
     */
    void addOrUpdateChunk(std::shared_ptr<Chunk> chunk);
    
    /**
     * @brief Remove a chunk
     * @param x X position
     * @param y Y position
     * @param z Z position
     */
    void removeChunk(int32_t x, int32_t y, int32_t z);
    
    /**
     * @brief Get all chunks
     * @return Vector of all chunks
     */
    std::vector<std::shared_ptr<Chunk>> getAllChunks() const;
    
    // Entity access
    /**
     * @brief Get an entity by ID
     * @param entityId Entity ID
     * @return Shared pointer to the entity, or nullptr if not found
     */
    std::shared_ptr<Entity> getEntity(uint64_t entityId) const;
    
    /**
     * @brief Add or update an entity
     * @param entity Entity to add or update
     */
    void addOrUpdateEntity(std::shared_ptr<Entity> entity);
    
    /**
     * @brief Remove an entity
     * @param entityId Entity ID
     */
    void removeEntity(uint64_t entityId);
    
    /**
     * @brief Get all entities
     * @return Vector of all entities
     */
    std::vector<std::shared_ptr<Entity>> getAllEntities() const;
    
    // Particle system access
    /**
     * @brief Get a particle system by ID
     * @param systemId System ID
     * @return Shared pointer to the particle system, or nullptr if not found
     */
    std::shared_ptr<ParticleSystem> getParticleSystem(uint64_t systemId) const;
    
    /**
     * @brief Add or update a particle system
     * @param particleSystem Particle system to add or update
     */
    void addOrUpdateParticleSystem(std::shared_ptr<ParticleSystem> particleSystem);
    
    /**
     * @brief Remove a particle system
     * @param systemId System ID
     */
    void removeParticleSystem(uint64_t systemId);
    
    // Weather access
    /**
     * @brief Get the weather
     * @return Shared pointer to the weather
     */
    std::shared_ptr<Weather> getWeather() const;
    
    /**
     * @brief Set the weather
     * @param weather Weather to set
     */
    void setWeather(std::shared_ptr<Weather> weather);
    
    // Frame information
    /**
     * @brief Get the current frame number
     * @return Frame number
     */
    int64_t getFrameNumber() const;
    
    /**
     * @brief Get the current timestamp
     * @return Timestamp
     */
    int64_t getTimestamp() const;
    
    // Statistics
    /**
     * @brief Get the number of chunks
     * @return Number of chunks
     */
    uint32_t getChunkCount() const;
    
    /**
     * @brief Get the number of entities
     * @return Number of entities
     */
    uint32_t getEntityCount() const;
    
    /**
     * @brief Get the number of particle systems
     * @return Number of particle systems
     */
    uint32_t getParticleSystemCount() const;
    
    /**
     * @brief Check if the scene has changed since the last frame
     * @return True if the scene has changed
     */
    bool hasChanged() const;
    
    /**
     * @brief Mark the scene as unchanged
     */
    void markUnchanged();
    
    /**
     * @brief Clear the scene
     */
    void clear();
    
    /**
     * @brief Process a camera message
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     */
    void processCameraMessage(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Process a chunk message
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     */
    void processChunkMessage(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Process an entity message
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     */
    void processEntityMessage(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Process a particle message
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     */
    void processParticleMessage(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Process a weather message
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     */
    void processWeatherMessage(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Process a GUI message
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     */
    void processGuiMessage(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Process a frame start message
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     */
    void processFrameStartMessage(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Process a frame end message
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     */
    void processFrameEndMessage(const uint8_t* payload, uint32_t payloadSize);

private:
    int64_t m_frameNumber;
    int64_t m_timestamp;
    
    // Scene objects
    std::shared_ptr<Camera> m_camera;
    std::shared_ptr<Weather> m_weather;
    
    // Chunk storage (key: x,y,z)
    struct ChunkKey {
        int32_t x, y, z;
        
        bool operator==(const ChunkKey& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };
    
    struct ChunkKeyHash {
        size_t operator()(const ChunkKey& key) const {
            return ((size_t)key.x << 20) ^ ((size_t)key.y << 10) ^ (size_t)key.z;
        }
    };
    
    std::unordered_map<ChunkKey, std::shared_ptr<Chunk>, ChunkKeyHash> m_chunks;
    
    // Entity storage
    std::unordered_map<uint64_t, std::shared_ptr<Entity>> m_entities;
    
    // Particle system storage
    std::unordered_map<uint64_t, std::shared_ptr<ParticleSystem>> m_particleSystems;
    
    // Dirty flag for change detection
    mutable bool m_dirty;
    
    // Previous frame info for change detection
    mutable int64_t m_previousFrameNumber;
    mutable uint32_t m_previousChunkCount;
    mutable uint32_t m_previousEntityCount;
};

} // namespace renderbridge
