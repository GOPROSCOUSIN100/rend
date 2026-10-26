#include "scene.hpp"
#include "camera.hpp"
#include "chunk.hpp"
#include "entity.hpp"
#include "particle.hpp"
#include "weather.hpp"

#include <cstring>
#include <algorithm>

namespace renderbridge {

Scene::Scene()
    : m_frameNumber(0),
      m_timestamp(0),
      m_camera(std::make_shared<Camera>()),
      m_weather(std::make_shared<Weather>()),
      m_dirty(false),
      m_previousFrameNumber(-1),
      m_previousChunkCount(0),
      m_previousEntityCount(0)
{
}

Scene::~Scene()
{
    shutdown();
}

void Scene::initialize()
{
    m_camera = std::make_shared<Camera>();
    m_weather = std::make_shared<Weather>();
    m_frameNumber = 0;
    m_timestamp = 0;
    m_dirty = false;
    m_chunks.clear();
    m_entities.clear();
    m_particleSystems.clear();
}

void Scene::shutdown()
{
    clear();
}

void Scene::updateFromMessage(uint16_t messageType, const uint8_t* payload, uint32_t payloadSize)
{
    switch (messageType) {
        case 0x0101: // MessageType.CAMERA
            processCameraMessage(payload, payloadSize);
            break;
        case 0x0201: // MessageType.CHUNK
            processChunkMessage(payload, payloadSize);
            break;
        case 0x0301: // MessageType.ENTITY
            processEntityMessage(payload, payloadSize);
            break;
        case 0x0401: // MessageType.PARTICLE
            processParticleMessage(payload, payloadSize);
            break;
        case 0x0501: // MessageType.WEATHER
            processWeatherMessage(payload, payloadSize);
            break;
        case 0x0601: // MessageType.GUI
            processGuiMessage(payload, payloadSize);
            break;
        case 0x0002: // MessageType.FRAME_START
            processFrameStartMessage(payload, payloadSize);
            break;
        case 0x0003: // MessageType.FRAME_END
            processFrameEndMessage(payload, payloadSize);
            break;
        default:
            // Unknown message type
            break;
    }
}

void Scene::beginFrame(int64_t frameNumber, int64_t timestamp)
{
    m_frameNumber = frameNumber;
    m_timestamp = timestamp;
    
    // Mark previous counts
    m_previousFrameNumber = frameNumber - 1;
    m_previousChunkCount = static_cast<uint32_t>(m_chunks.size());
    m_previousEntityCount = static_cast<uint32_t>(m_entities.size());
}

void Scene::endFrame()
{
    // Check for changes
    if (m_chunks.size() != m_previousChunkCount ||
        m_entities.size() != m_previousEntityCount) {
        m_dirty = true;
    }
}

// Camera access
std::shared_ptr<Camera> Scene::getCamera() const
{
    return m_camera;
}

void Scene::setCamera(std::shared_ptr<Camera> camera)
{
    if (camera) {
        m_camera = camera;
        m_dirty = true;
    }
}

// Chunk access
std::shared_ptr<Chunk> Scene::getChunk(int32_t x, int32_t y, int32_t z) const
{
    ChunkKey key{x, y, z};
    auto it = m_chunks.find(key);
    if (it != m_chunks.end()) {
        return it->second;
    }
    return nullptr;
}

void Scene::addOrUpdateChunk(std::shared_ptr<Chunk> chunk)
{
    if (!chunk) return;
    
    ChunkKey key{
        static_cast<int32_t>(chunk->getPositionX()),
        static_cast<int32_t>(chunk->getPositionY()),
        static_cast<int32_t>(chunk->getPositionZ())
    };
    
    auto it = m_chunks.find(key);
    if (it != m_chunks.end()) {
        // Update existing chunk
        it->second->updateFrom(*chunk);
    } else {
        // Add new chunk
        m_chunks[key] = chunk;
    }
    
    m_dirty = true;
}

void Scene::removeChunk(int32_t x, int32_t y, int32_t z)
{
    ChunkKey key{x, y, z};
    auto it = m_chunks.find(key);
    if (it != m_chunks.end()) {
        m_chunks.erase(it);
        m_dirty = true;
    }
}

std::vector<std::shared_ptr<Chunk>> Scene::getAllChunks() const
{
    std::vector<std::shared_ptr<Chunk>> chunks;
    chunks.reserve(m_chunks.size());
    for (const auto& pair : m_chunks) {
        chunks.push_back(pair.second);
    }
    return chunks;
}

// Entity access
std::shared_ptr<Entity> Scene::getEntity(uint64_t entityId) const
{
    auto it = m_entities.find(entityId);
    if (it != m_entities.end()) {
        return it->second;
    }
    return nullptr;
}

void Scene::addOrUpdateEntity(std::shared_ptr<Entity> entity)
{
    if (!entity) return;
    
    uint64_t entityId = entity->getEntityId();
    auto it = m_entities.find(entityId);
    if (it != m_entities.end()) {
        // Update existing entity
        it->second->updateFrom(*entity);
    } else {
        // Add new entity
        m_entities[entityId] = entity;
    }
    
    m_dirty = true;
}

void Scene::removeEntity(uint64_t entityId)
{
    auto it = m_entities.find(entityId);
    if (it != m_entities.end()) {
        m_entities.erase(it);
        m_dirty = true;
    }
}

std::vector<std::shared_ptr<Entity>> Scene::getAllEntities() const
{
    std::vector<std::shared_ptr<Entity>> entities;
    entities.reserve(m_entities.size());
    for (const auto& pair : m_entities) {
        entities.push_back(pair.second);
    }
    return entities;
}

// Particle system access
std::shared_ptr<ParticleSystem> Scene::getParticleSystem(uint64_t systemId) const
{
    auto it = m_particleSystems.find(systemId);
    if (it != m_particleSystems.end()) {
        return it->second;
    }
    return nullptr;
}

void Scene::addOrUpdateParticleSystem(std::shared_ptr<ParticleSystem> particleSystem)
{
    if (!particleSystem) return;
    
    uint64_t systemId = particleSystem->getSystemId();
    auto it = m_particleSystems.find(systemId);
    if (it != m_particleSystems.end()) {
        // Update existing particle system
        it->second->updateFrom(*particleSystem);
    } else {
        // Add new particle system
        m_particleSystems[systemId] = particleSystem;
    }
    
    m_dirty = true;
}

void Scene::removeParticleSystem(uint64_t systemId)
{
    auto it = m_particleSystems.find(systemId);
    if (it != m_particleSystems.end()) {
        m_particleSystems.erase(it);
        m_dirty = true;
    }
}

// Weather access
std::shared_ptr<Weather> Scene::getWeather() const
{
    return m_weather;
}

void Scene::setWeather(std::shared_ptr<Weather> weather)
{
    if (weather) {
        m_weather = weather;
        m_dirty = true;
    }
}

// Frame information
int64_t Scene::getFrameNumber() const
{
    return m_frameNumber;
}

int64_t Scene::getTimestamp() const
{
    return m_timestamp;
}

// Statistics
uint32_t Scene::getChunkCount() const
{
    return static_cast<uint32_t>(m_chunks.size());
}

uint32_t Scene::getEntityCount() const
{
    return static_cast<uint32_t>(m_entities.size());
}

uint32_t Scene::getParticleSystemCount() const
{
    return static_cast<uint32_t>(m_particleSystems.size());
}

// Change detection
bool Scene::hasChanged() const
{
    return m_dirty;
}

void Scene::markUnchanged()
{
    m_dirty = false;
    
    // Mark all objects as unchanged
    if (m_camera) {
        m_camera->markUnchanged();
    }
    if (m_weather) {
        m_weather->markUnchanged();
    }
    
    for (auto& pair : m_chunks) {
        if (pair.second) {
            pair.second->markUnchanged();
        }
    }
    
    for (auto& pair : m_entities) {
        if (pair.second) {
            pair.second->markUnchanged();
        }
    }
    
    for (auto& pair : m_particleSystems) {
        if (pair.second) {
            pair.second->markUnchanged();
        }
    }
}

// Clear the scene
void Scene::clear()
{
    m_chunks.clear();
    m_entities.clear();
    m_particleSystems.clear();
    
    if (m_camera) {
        m_camera->markUnchanged();
    }
    if (m_weather) {
        m_weather->markUnchanged();
    }
    
    m_dirty = true;
}

// Process message handlers
void Scene::processCameraMessage(const uint8_t* payload, uint32_t payloadSize)
{
    if (!m_camera) {
        m_camera = std::make_shared<Camera>();
    }
    
    if (m_camera->parseFromPayload(payload, payloadSize)) {
        m_dirty = true;
    }
}

void Scene::processChunkMessage(const uint8_t* payload, uint32_t payloadSize)
{
    auto chunk = std::make_shared<Chunk>();
    if (chunk->parseFromPayload(payload, payloadSize)) {
        addOrUpdateChunk(chunk);
    }
}

void Scene::processEntityMessage(const uint8_t* payload, uint32_t payloadSize)
{
    auto entity = std::make_shared<Entity>();
    if (entity->parseFromPayload(payload, payloadSize)) {
        addOrUpdateEntity(entity);
    }
}

void Scene::processParticleMessage(const uint8_t* payload, uint32_t payloadSize)
{
    auto particleSystem = std::make_shared<ParticleSystem>();
    if (particleSystem->parseFromPayload(payload, payloadSize)) {
        addOrUpdateParticleSystem(particleSystem);
    }
}

void Scene::processWeatherMessage(const uint8_t* payload, uint32_t payloadSize)
{
    if (!m_weather) {
        m_weather = std::make_shared<Weather>();
    }
    
    if (m_weather->parseFromPayload(payload, payloadSize)) {
        m_dirty = true;
    }
}

void Scene::processGuiMessage(const uint8_t* payload, uint32_t payloadSize)
{
    // GUI messages are handled separately
    // For now, just mark as dirty
    m_dirty = true;
}

void Scene::processFrameStartMessage(const uint8_t* payload, uint32_t payloadSize)
{
    if (payload && payloadSize >= 16) {
        int64_t frameNumber = *reinterpret_cast<const int64_t*>(payload);
        int64_t timestamp = *reinterpret_cast<const int64_t*>(payload + 8);
        beginFrame(frameNumber, timestamp);
    }
}

void Scene::processFrameEndMessage(const uint8_t* payload, uint32_t payloadSize)
{
    endFrame();
}

} // namespace renderbridge
