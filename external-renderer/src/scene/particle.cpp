#include "particle.hpp"

#include <cstring>
#include <algorithm>
#include <cmath>

namespace renderbridge {

// Static constants
const ParticleSystem::Flag ParticleSystem::Flag[];

ParticleSystem::ParticleSystem()
    : m_systemId(0),
      m_type(0),
      m_positionX(0.0f), m_positionY(0.0f), m_positionZ(0.0f),
      m_maxParticles(1000),
      m_flags(ACTIVE | VISIBLE),
      m_texture(""),
      m_blendMode(0),
      m_gravity(0.0f),
      m_minSize(0.1f), m_maxSize(1.0f),
      m_minLifetime(20), m_maxLifetime(100),
      m_minSpeed(0.1f), m_maxSpeed(1.0f),
      m_colorR(1.0f), m_colorG(1.0f), m_colorB(1.0f), m_colorA(1.0f),
      m_dirty(true)
{
    m_particles.reserve(m_maxParticles);
}

ParticleSystem::ParticleSystem(uint64_t systemId)
    : m_systemId(systemId),
      m_type(0),
      m_positionX(0.0f), m_positionY(0.0f), m_positionZ(0.0f),
      m_maxParticles(1000),
      m_flags(ACTIVE | VISIBLE),
      m_texture(""),
      m_blendMode(0),
      m_gravity(0.0f),
      m_minSize(0.1f), m_maxSize(1.0f),
      m_minLifetime(20), m_maxLifetime(100),
      m_minSpeed(0.1f), m_maxSpeed(1.0f),
      m_colorR(1.0f), m_colorG(1.0f), m_colorB(1.0f), m_colorA(1.0f),
      m_dirty(true)
{
    m_particles.reserve(m_maxParticles);
}

ParticleSystem::~ParticleSystem()
{
}

// Parse particle system data from a message payload
// Payload format:
// systemId(int64), type(uint16), positionX(float), positionY(float), positionZ(float),
// particleCount(uint32), particles[] (each: x,y,z,velocityX,velocityY,velocityZ,size,scale,rotation,
//     colorR,colorG,colorB,colorA,age,maxAge,typeId,flags,textureIndex),
// textureSize(uint16), texture(string),
// flags(uint8), blendMode(uint8), gravity(float),
// minSize(float), maxSize(float), minLifetime(int32), maxLifetime(int32),
// minSpeed(float), maxSpeed(float),
// colorR(float), colorG(float), colorB(float), colorA(float)
bool ParticleSystem::parseFromPayload(const uint8_t* payload, uint32_t payloadSize)
{
    if (payload == nullptr || payloadSize < 8) {
        return false;
    }

    const uint8_t* ptr = payload;
    uint32_t offset = 0;

    // Read system ID (int64)
    m_systemId = *reinterpret_cast<const uint64_t*>(ptr + offset);
    offset += 8;

    // Read type (uint16)
    m_type = *reinterpret_cast<const uint16_t*>(ptr + offset);
    offset += 2;

    // Read position (3 floats)
    m_positionX = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_positionY = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_positionZ = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    // Read particle count (uint32)
    uint32_t particleCount = *reinterpret_cast<const uint32_t*>(ptr + offset);
    offset += 4;

    // Read particles
    m_particles.clear();
    for (uint32_t i = 0; i < particleCount && offset + 72 <= payloadSize; ++i) {
        Particle particle;
        
        // Position (3 floats)
        particle.prevX = particle.x;
        particle.prevY = particle.y;
        particle.prevZ = particle.z;
        particle.x = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        particle.y = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        particle.z = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        
        // Velocity (3 floats)
        particle.velocityX = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        particle.velocityY = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        particle.velocityZ = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        
        // Size and scale (2 floats)
        particle.size = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        particle.scale = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        
        // Rotation (1 float)
        particle.rotation = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        
        // Color (4 floats)
        particle.colorR = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        particle.colorG = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        particle.colorB = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        particle.colorA = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        
        // Age and max age (2 int32)
        particle.age = *reinterpret_cast<const int32_t*>(ptr + offset);
        offset += 4;
        particle.maxAge = *reinterpret_cast<const int32_t*>(ptr + offset);
        offset += 4;
        
        // Type ID (uint16)
        particle.typeId = *reinterpret_cast<const uint16_t*>(ptr + offset);
        offset += 2;
        
        // Flags (uint8)
        particle.flags = *reinterpret_cast<const uint8_t*>(ptr + offset);
        offset += 1;
        
        // Texture index (uint16)
        particle.textureIndex = *reinterpret_cast<const uint16_t*>(ptr + offset);
        offset += 2;
        
        m_particles.push_back(particle);
    }

    // Read texture size (uint16)
    if (offset + 2 <= payloadSize) {
        uint16_t textureSize = *reinterpret_cast<const uint16_t*>(ptr + offset);
        offset += 2;
        
        // Read texture string
        if (offset + textureSize <= payloadSize) {
            m_texture.assign(reinterpret_cast<const char*>(ptr + offset), textureSize);
            offset += textureSize;
        }
    }

    // Read flags (uint8)
    if (offset + 1 <= payloadSize) {
        m_flags = *reinterpret_cast<const uint8_t*>(ptr + offset);
        offset += 1;
    }

    // Read blend mode (uint8)
    if (offset + 1 <= payloadSize) {
        m_blendMode = *reinterpret_cast<const uint8_t*>(ptr + offset);
        offset += 1;
    }

    // Read gravity (float)
    if (offset + 4 <= payloadSize) {
        m_gravity = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
    }

    // Read size range (2 floats)
    if (offset + 8 <= payloadSize) {
        m_minSize = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        m_maxSize = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
    }

    // Read lifetime range (2 int32)
    if (offset + 8 <= payloadSize) {
        m_minLifetime = *reinterpret_cast<const int32_t*>(ptr + offset);
        offset += 4;
        m_maxLifetime = *reinterpret_cast<const int32_t*>(ptr + offset);
        offset += 4;
    }

    // Read speed range (2 floats)
    if (offset + 8 <= payloadSize) {
        m_minSpeed = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        m_maxSpeed = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
    }

    // Read color (4 floats)
    if (offset + 16 <= payloadSize) {
        m_colorR = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        m_colorG = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        m_colorB = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        m_colorA = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
    }

    m_dirty = true;
    return true;
}

// Alternative simpler parse for protocol messages
// This matches the Java ParticleMessage format more closely
bool ParticleSystem::parseFromPayloadSimple(const uint8_t* payload, uint32_t payloadSize)
{
    if (payload == nullptr || payloadSize < 20) {
        return false;
    }

    const uint8_t* ptr = payload;
    uint32_t offset = 0;

    // Read system ID (int64)
    m_systemId = *reinterpret_cast<const uint64_t*>(ptr + offset);
    offset += 8;

    // Read type (uint16)
    m_type = *reinterpret_cast<const uint16_t*>(ptr + offset);
    offset += 2;

    // Read position (3 floats)
    m_positionX = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_positionY = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_positionZ = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    // Read particle count (uint32)
    uint32_t particleCount = *reinterpret_cast<const uint32_t*>(ptr + offset);
    offset += 4;

    // Read particles (simplified format: position, velocity, color, size, type)
    m_particles.clear();
    for (uint32_t i = 0; i < particleCount && offset + 48 <= payloadSize; ++i) {
        Particle particle;
        
        particle.x = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        particle.y = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        particle.z = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        
        particle.velocityX = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        particle.velocityY = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        particle.velocityZ = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        
        particle.colorR = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        particle.colorG = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        particle.colorB = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        particle.colorA = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        
        particle.size = *reinterpret_cast<const float*>(ptr + offset);
        offset += 4;
        
        particle.typeId = *reinterpret_cast<const uint16_t*>(ptr + offset);
        offset += 2;
        
        particle.age = 0;
        particle.maxAge = 20;
        particle.scale = 1.0f;
        particle.rotation = 0.0f;
        particle.flags = 0;
        particle.textureIndex = 0;
        
        m_particles.push_back(particle);
    }

    m_dirty = true;
    return true;
}

// Serialize particle system data to a buffer
uint32_t ParticleSystem::serializeToBuffer(uint8_t* buffer, uint32_t bufferSize) const
{
    if (buffer == nullptr || bufferSize < 20) {
        return 0;
    }

    uint8_t* ptr = buffer;
    uint32_t offset = 0;

    // Write system ID (int64)
    *reinterpret_cast<uint64_t*>(ptr + offset) = m_systemId;
    offset += 8;

    // Write type (uint16)
    *reinterpret_cast<uint16_t*>(ptr + offset) = m_type;
    offset += 2;

    // Write position (3 floats)
    *reinterpret_cast<float*>(ptr + offset) = m_positionX;
    offset += 4;
    *reinterpret_cast<float*>(ptr + offset) = m_positionY;
    offset += 4;
    *reinterpret_cast<float*>(ptr + offset) = m_positionZ;
    offset += 4;

    // Write particle count (uint32)
    uint32_t particleCount = static_cast<uint32_t>(std::min(m_particles.size(), m_maxParticles));
    *reinterpret_cast<uint32_t*>(ptr + offset) = particleCount;
    offset += 4;

    // Write particles
    for (uint32_t i = 0; i < particleCount && offset + 72 <= bufferSize; ++i) {
        const Particle& particle = m_particles[i];
        
        *reinterpret_cast<float*>(ptr + offset) = particle.x;
        offset += 4;
        *reinterpret_cast<float*>(ptr + offset) = particle.y;
        offset += 4;
        *reinterpret_cast<float*>(ptr + offset) = particle.z;
        offset += 4;
        
        *reinterpret_cast<float*>(ptr + offset) = particle.velocityX;
        offset += 4;
        *reinterpret_cast<float*>(ptr + offset) = particle.velocityY;
        offset += 4;
        *reinterpret_cast<float*>(ptr + offset) = particle.velocityZ;
        offset += 4;
        
        *reinterpret_cast<float*>(ptr + offset) = particle.size;
        offset += 4;
        *reinterpret_cast<float*>(ptr + offset) = particle.scale;
        offset += 4;
        
        *reinterpret_cast<float*>(ptr + offset) = particle.rotation;
        offset += 4;
        
        *reinterpret_cast<float*>(ptr + offset) = particle.colorR;
        offset += 4;
        *reinterpret_cast<float*>(ptr + offset) = particle.colorG;
        offset += 4;
        *reinterpret_cast<float*>(ptr + offset) = particle.colorB;
        offset += 4;
        *reinterpret_cast<float*>(ptr + offset) = particle.colorA;
        offset += 4;
        
        *reinterpret_cast<int32_t*>(ptr + offset) = particle.age;
        offset += 4;
        *reinterpret_cast<int32_t*>(ptr + offset) = particle.maxAge;
        offset += 4;
        
        *reinterpret_cast<uint16_t*>(ptr + offset) = particle.typeId;
        offset += 2;
        
        *reinterpret_cast<uint8_t*>(ptr + offset) = particle.flags;
        offset += 1;
        
        *reinterpret_cast<uint16_t*>(ptr + offset) = particle.textureIndex;
        offset += 2;
    }

    // Write texture size (uint16)
    uint16_t textureSize = static_cast<uint16_t>(std::min(m_texture.size(), static_cast<size_t>(65535)));
    *reinterpret_cast<uint16_t*>(ptr + offset) = textureSize;
    offset += 2;

    // Write texture string
    if (offset + textureSize <= bufferSize) {
        std::memcpy(ptr + offset, m_texture.data(), textureSize);
        offset += textureSize;
    }

    // Write flags (uint8)
    *reinterpret_cast<uint8_t*>(ptr + offset) = m_flags;
    offset += 1;

    // Write blend mode (uint8)
    *reinterpret_cast<uint8_t*>(ptr + offset) = m_blendMode;
    offset += 1;

    // Write gravity (float)
    *reinterpret_cast<float*>(ptr + offset) = m_gravity;
    offset += 4;

    // Write size range (2 floats)
    *reinterpret_cast<float*>(ptr + offset) = m_minSize;
    offset += 4;
    *reinterpret_cast<float*>(ptr + offset) = m_maxSize;
    offset += 4;

    // Write lifetime range (2 int32)
    *reinterpret_cast<int32_t*>(ptr + offset) = m_minLifetime;
    offset += 4;
    *reinterpret_cast<int32_t*>(ptr + offset) = m_maxLifetime;
    offset += 4;

    // Write speed range (2 floats)
    *reinterpret_cast<float*>(ptr + offset) = m_minSpeed;
    offset += 4;
    *reinterpret_cast<float*>(ptr + offset) = m_maxSpeed;
    offset += 4;

    // Write color (4 floats)
    *reinterpret_cast<float*>(ptr + offset) = m_colorR;
    offset += 4;
    *reinterpret_cast<float*>(ptr + offset) = m_colorG;
    offset += 4;
    *reinterpret_cast<float*>(ptr + offset) = m_colorB;
    offset += 4;
    *reinterpret_cast<float*>(ptr + offset) = m_colorA;
    offset += 4;

    return offset;
}

// ID accessors
uint64_t ParticleSystem::getSystemId() const {
    return m_systemId;
}

void ParticleSystem::setSystemId(uint64_t systemId) {
    if (m_systemId != systemId) {
        m_systemId = systemId;
        m_dirty = true;
    }
}

// Type accessors
uint16_t ParticleSystem::getType() const {
    return m_type;
}

void ParticleSystem::setType(uint16_t type) {
    if (m_type != type) {
        m_type = type;
        m_dirty = true;
    }
}

// Position accessors
float ParticleSystem::getPositionX() const {
    return m_positionX;
}

float ParticleSystem::getPositionY() const {
    return m_positionY;
}

float ParticleSystem::getPositionZ() const {
    return m_positionZ;
}

void ParticleSystem::setPosition(float x, float y, float z) {
    if (m_positionX != x || m_positionY != y || m_positionZ != z) {
        m_positionX = x;
        m_positionY = y;
        m_positionZ = z;
        m_dirty = true;
    }
}

// Particle count accessors
uint32_t ParticleSystem::getParticleCount() const {
    return static_cast<uint32_t>(m_particles.size());
}

// Max particles accessors
uint32_t ParticleSystem::getMaxParticles() const {
    return m_maxParticles;
}

void ParticleSystem::setMaxParticles(uint32_t maxParticles) {
    if (m_maxParticles != maxParticles) {
        m_maxParticles = maxParticles;
        m_particles.reserve(m_maxParticles);
        m_dirty = true;
    }
}

// Particle access
const Particle& ParticleSystem::getParticle(uint32_t index) const {
    if (index < m_particles.size()) {
        return m_particles[index];
    }
    static Particle nullParticle;
    return nullParticle;
}

Particle& ParticleSystem::getParticle(uint32_t index) {
    if (index < m_particles.size()) {
        return m_particles[index];
    }
    static Particle nullParticle;
    return nullParticle;
}

// Add a particle
void ParticleSystem::addParticle(const Particle& particle) {
    if (m_particles.size() < m_maxParticles) {
        m_particles.push_back(particle);
        m_dirty = true;
    }
}

// Remove a particle
void ParticleSystem::removeParticle(uint32_t index) {
    if (index < m_particles.size()) {
        m_particles.erase(m_particles.begin() + index);
        m_dirty = true;
    }
}

// Clear all particles
void ParticleSystem::clearParticles() {
    if (!m_particles.empty()) {
        m_particles.clear();
        m_dirty = true;
    }
}

// Flag accessors
uint8_t ParticleSystem::getFlags() const {
    return m_flags;
}

void ParticleSystem::setFlags(uint8_t flags) {
    if (m_flags != flags) {
        m_flags = flags;
        m_dirty = true;
    }
}

void ParticleSystem::addFlag(Flag flag) {
    if ((m_flags & flag) == 0) {
        m_flags |= flag;
        m_dirty = true;
    }
}

void ParticleSystem::removeFlag(Flag flag) {
    if ((m_flags & flag) != 0) {
        m_flags &= ~flag;
        m_dirty = true;
    }
}

bool ParticleSystem::hasFlag(Flag flag) const {
    return (m_flags & flag) != 0;
}

// Texture accessors
const std::string& ParticleSystem::getTexture() const {
    return m_texture;
}

void ParticleSystem::setTexture(const std::string& texture) {
    if (m_texture != texture) {
        m_texture = texture;
        m_dirty = true;
    }
}

// Blend mode accessors
uint8_t ParticleSystem::getBlendMode() const {
    return m_blendMode;
}

void ParticleSystem::setBlendMode(uint8_t blendMode) {
    if (m_blendMode != blendMode) {
        m_blendMode = blendMode;
        m_dirty = true;
    }
}

// Gravity accessors
float ParticleSystem::getGravity() const {
    return m_gravity;
}

void ParticleSystem::setGravity(float gravity) {
    if (m_gravity != gravity) {
        m_gravity = gravity;
        m_dirty = true;
    }
}

// Size range accessors
float ParticleSystem::getMinSize() const {
    return m_minSize;
}

float ParticleSystem::getMaxSize() const {
    return m_maxSize;
}

void ParticleSystem::setSizeRange(float minSize, float maxSize) {
    if (m_minSize != minSize || m_maxSize != maxSize) {
        m_minSize = minSize;
        m_maxSize = maxSize;
        m_dirty = true;
    }
}

// Lifetime range accessors
int32_t ParticleSystem::getMinLifetime() const {
    return m_minLifetime;
}

int32_t ParticleSystem::getMaxLifetime() const {
    return m_maxLifetime;
}

void ParticleSystem::setLifetimeRange(int32_t minLifetime, int32_t maxLifetime) {
    if (m_minLifetime != minLifetime || m_maxLifetime != maxLifetime) {
        m_minLifetime = minLifetime;
        m_maxLifetime = maxLifetime;
        m_dirty = true;
    }
}

// Speed range accessors
float ParticleSystem::getMinSpeed() const {
    return m_minSpeed;
}

float ParticleSystem::getMaxSpeed() const {
    return m_maxSpeed;
}

void ParticleSystem::setSpeedRange(float minSpeed, float maxSpeed) {
    if (m_minSpeed != minSpeed || m_maxSpeed != maxSpeed) {
        m_minSpeed = minSpeed;
        m_maxSpeed = maxSpeed;
        m_dirty = true;
    }
}

// Color accessors
float ParticleSystem::getColorR() const {
    return m_colorR;
}

float ParticleSystem::getColorG() const {
    return m_colorG;
}

float ParticleSystem::getColorB() const {
    return m_colorB;
}

float ParticleSystem::getColorA() const {
    return m_colorA;
}

void ParticleSystem::setColor(float r, float g, float b, float a) {
    if (m_colorR != r || m_colorG != g || m_colorB != b || m_colorA != a) {
        m_colorR = r;
        m_colorG = g;
        m_colorB = b;
        m_colorA = a;
        m_dirty = true;
    }
}

// Generate vertex data for rendering
void ParticleSystem::generateVertexData(std::vector<ParticleVertex>& vertices, float partialTick) const
{
    vertices.clear();
    vertices.reserve(m_particles.size() * 4); // 4 vertices per particle (quad)
    
    for (const Particle& particle : m_particles) {
        // Interpolate position
        float interpX = particle.prevX + (particle.x - particle.prevX) * partialTick;
        float interpY = particle.prevY + (particle.y - particle.prevY) * partialTick;
        float interpZ = particle.prevZ + (particle.z - particle.prevZ) * partialTick;
        
        // Calculate size with scale
        float size = particle.size * particle.scale * m_colorA;
        float halfSize = size * 0.5f;
        
        // Calculate rotation in radians
        float rotationRad = particle.rotation * 0.0174532925f; // degrees to radians
        float cosRot = std::cos(rotationRad);
        float sinRot = std::sin(rotationRad);
        
        // Create quad vertices (billboard facing camera)
        // For now, create a simple quad facing the camera
        // In a real implementation, this would be oriented based on camera view
        
        // Vertex 0: Bottom-left
        ParticleVertex v0;
        v0.x = interpX - halfSize;
        v0.y = interpY - halfSize;
        v0.z = interpZ;
        v0.u = 0.0f;
        v0.v = 0.0f;
        v0.r = particle.colorR * m_colorR;
        v0.g = particle.colorG * m_colorG;
        v0.b = particle.colorB * m_colorB;
        v0.a = particle.colorA * m_colorA;
        v0.size = size;
        v0.rotation = rotationRad;
        v0.texIndex = static_cast<float>(particle.textureIndex);
        vertices.push_back(v0);
        
        // Vertex 1: Bottom-right
        ParticleVertex v1;
        v1.x = interpX + halfSize;
        v1.y = interpY - halfSize;
        v1.z = interpZ;
        v1.u = 1.0f;
        v1.v = 0.0f;
        v1.r = particle.colorR * m_colorR;
        v1.g = particle.colorG * m_colorG;
        v1.b = particle.colorB * m_colorB;
        v1.a = particle.colorA * m_colorA;
        v1.size = size;
        v1.rotation = rotationRad;
        v1.texIndex = static_cast<float>(particle.textureIndex);
        vertices.push_back(v1);
        
        // Vertex 2: Top-right
        ParticleVertex v2;
        v2.x = interpX + halfSize;
        v2.y = interpY + halfSize;
        v2.z = interpZ;
        v2.u = 1.0f;
        v2.v = 1.0f;
        v2.r = particle.colorR * m_colorR;
        v2.g = particle.colorG * m_colorG;
        v2.b = particle.colorB * m_colorB;
        v2.a = particle.colorA * m_colorA;
        v2.size = size;
        v2.rotation = rotationRad;
        v2.texIndex = static_cast<float>(particle.textureIndex);
        vertices.push_back(v2);
        
        // Vertex 3: Top-left
        ParticleVertex v3;
        v3.x = interpX - halfSize;
        v3.y = interpY + halfSize;
        v3.z = interpZ;
        v3.u = 0.0f;
        v3.v = 1.0f;
        v3.r = particle.colorR * m_colorR;
        v3.g = particle.colorG * m_colorG;
        v3.b = particle.colorB * m_colorB;
        v3.a = particle.colorA * m_colorA;
        v3.size = size;
        v3.rotation = rotationRad;
        v3.texIndex = static_cast<float>(particle.textureIndex);
        vertices.push_back(v3);
    }
}

// Update particles
void ParticleSystem::update(float deltaTime)
{
    for (Particle& particle : m_particles) {
        // Update previous position
        particle.prevX = particle.x;
        particle.prevY = particle.y;
        particle.prevZ = particle.z;
        
        // Apply velocity
        particle.x += particle.velocityX * deltaTime;
        particle.y += particle.velocityY * deltaTime;
        particle.z += particle.velocityZ * deltaTime;
        
        // Apply gravity
        particle.velocityY -= m_gravity * deltaTime;
        
        // Update age
        particle.age++;
        
        // Check if particle is dead
        if (particle.age >= particle.maxAge) {
            particle.age = 0; // Reset for reuse
        }
    }
    
    m_dirty = false;
}

// Change detection
bool ParticleSystem::hasChanged() const {
    return m_dirty;
}

void ParticleSystem::markUnchanged() {
    m_dirty = false;
}

// Update from another particle system
void ParticleSystem::updateFrom(const ParticleSystem& other) {
    if (m_systemId != other.m_systemId) {
        return;
    }

    m_type = other.m_type;
    m_positionX = other.m_positionX;
    m_positionY = other.m_positionY;
    m_positionZ = other.m_positionZ;
    
    m_particles = other.m_particles;
    m_maxParticles = other.m_maxParticles;
    
    m_flags = other.m_flags;
    m_texture = other.m_texture;
    m_blendMode = other.m_blendMode;
    m_gravity = other.m_gravity;
    
    m_minSize = other.m_minSize;
    m_maxSize = other.m_maxSize;
    m_minLifetime = other.m_minLifetime;
    m_maxLifetime = other.m_maxLifetime;
    m_minSpeed = other.m_minSpeed;
    m_maxSpeed = other.m_maxSpeed;
    
    m_colorR = other.m_colorR;
    m_colorG = other.m_colorG;
    m_colorB = other.m_colorB;
    m_colorA = other.m_colorA;
    
    m_dirty = true;
}

} // namespace renderbridge
