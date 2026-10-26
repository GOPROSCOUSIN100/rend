#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <array>

namespace renderbridge {

/**
 * @struct Particle
 * @brief Represents a single particle
 */
struct Particle {
    // Position
    float x, y, z;
    
    // Previous position (for interpolation)
    float prevX, prevY, prevZ;
    
    // Velocity
    float velocityX, velocityY, velocityZ;
    
    // Size
    float size;
    
    // Scale
    float scale;
    
    // Rotation (in degrees)
    float rotation;
    
    // Color (RGBA, 0.0 to 1.0)
    float colorR, colorG, colorB, colorA;
    
    // Age (in ticks)
    int32_t age;
    
    // Max age (in ticks)
    int32_t maxAge;
    
    // Type ID
    uint16_t typeId;
    
    // Flags
    uint8_t flags;
    
    // Texture index
    uint16_t textureIndex;
    
    // Constructor
    Particle() 
        : x(0), y(0), z(0),
          prevX(0), prevY(0), prevZ(0),
          velocityX(0), velocityY(0), velocityZ(0),
          size(1.0f), scale(1.0f), rotation(0),
          colorR(1.0f), colorG(1.0f), colorB(1.0f), colorA(1.0f),
          age(0), maxAge(20),
          typeId(0), flags(0), textureIndex(0) {}
};

/**
 * @struct ParticleVertex
 * @brief Vertex data for particle rendering
 */
struct ParticleVertex {
    float x, y, z;          // Position
    float u, v;            // Texture coordinates
    float r, g, b, a;      // Color
    float size;           // Size
    float rotation;        // Rotation
    float texIndex;       // Texture index
};

/**
 * @class ParticleSystem
 * @brief Represents a particle system in the scene
 * 
 * This class manages a collection of particles and provides
 * methods for updating and rendering them.
 */
class ParticleSystem {
public:
    // Particle system flags
    enum Flag : uint8_t {
        NONE = 0x00,
        ACTIVE = 0x01,
        PAUSED = 0x02,
        VISIBLE = 0x04
    };

public:
    /**
     * @brief Constructor
     */
    ParticleSystem();
    
    /**
     * @brief Constructor with ID
     * @param systemId System ID
     */
    explicit ParticleSystem(uint64_t systemId);
    
    /**
     * @brief Destructor
     */
    ~ParticleSystem();
    
    /**
     * @brief Parse particle system data from a message payload
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     * @return True if parsing succeeded
     */
    bool parseFromPayload(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Serialize particle system data to a buffer
     * @param buffer Output buffer
     * @param bufferSize Size of the buffer
     * @return Number of bytes written
     */
    uint32_t serializeToBuffer(uint8_t* buffer, uint32_t bufferSize) const;
    
    // ID accessors
    uint64_t getSystemId() const;
    void setSystemId(uint64_t systemId);
    
    // Type accessors
    uint16_t getType() const;
    void setType(uint16_t type);
    
    // Position accessors
    float getPositionX() const;
    float getPositionY() const;
    float getPositionZ() const;
    void setPosition(float x, float y, float z);
    
    // Particle count accessors
    uint32_t getParticleCount() const;
    
    // Max particles accessors
    uint32_t getMaxParticles() const;
    void setMaxParticles(uint32_t maxParticles);
    
    // Particle access
    const Particle& getParticle(uint32_t index) const;
    Particle& getParticle(uint32_t index);
    
    // Add a particle
    /**
     * @brief Add a particle to the system
     * @param particle Particle to add
     */
    void addParticle(const Particle& particle);
    
    // Remove a particle
    /**
     * @brief Remove a particle from the system
     * @param index Index of the particle to remove
     */
    void removeParticle(uint32_t index);
    
    // Clear all particles
    /**
     * @brief Clear all particles from the system
     */
    void clearParticles();
    
    // Flag accessors
    uint8_t getFlags() const;
    void setFlags(uint8_t flags);
    void addFlag(Flag flag);
    void removeFlag(Flag flag);
    bool hasFlag(Flag flag) const;
    
    // Texture accessors
    const std::string& getTexture() const;
    void setTexture(const std::string& texture);
    
    // Blend mode accessors
    uint8_t getBlendMode() const;
    void setBlendMode(uint8_t blendMode);
    
    // Gravity accessors
    float getGravity() const;
    void setGravity(float gravity);
    
    // Size range accessors
    float getMinSize() const;
    float getMaxSize() const;
    void setSizeRange(float minSize, float maxSize);
    
    // Lifetime range accessors
    int32_t getMinLifetime() const;
    int32_t getMaxLifetime() const;
    void setLifetimeRange(int32_t minLifetime, int32_t maxLifetime);
    
    // Speed range accessors
    float getMinSpeed() const;
    float getMaxSpeed() const;
    void setSpeedRange(float minSpeed, float maxSpeed);
    
    // Color accessors
    float getColorR() const;
    float getColorG() const;
    float getColorB() const;
    float getColorA() const;
    void setColor(float r, float g, float b, float a);
    
    // Generate vertex data for rendering
    /**
     * @brief Generate vertex data for all particles
     * @param vertices Output: vertex data
     * @param partialTick Partial tick for interpolation
     */
    void generateVertexData(std::vector<ParticleVertex>& vertices, float partialTick) const;
    
    // Update particles
    /**
     * @brief Update all particles
     * @param deltaTime Delta time in seconds
     */
    void update(float deltaTime);
    
    // Check if system has changed
    /**
     * @brief Check if the particle system has changed since the last update
     * @return True if the particle system has changed
     */
    bool hasChanged() const;
    
    /**
     * @brief Mark the particle system as unchanged
     */
    void markUnchanged();
    
    /**
     * @brief Update the particle system from another
     * @param other Other particle system
     */
    void updateFrom(const ParticleSystem& other);

private:
    uint64_t m_systemId;
    uint16_t m_type;
    
    // Position
    float m_positionX;
    float m_positionY;
    float m_positionZ;
    
    // Particles
    std::vector<Particle> m_particles;
    uint32_t m_maxParticles;
    
    // Flags
    uint8_t m_flags;
    
    // Texture
    std::string m_texture;
    
    // Blend mode
    uint8_t m_blendMode;
    
    // Physics
    float m_gravity;
    
    // Ranges
    float m_minSize;
    float m_maxSize;
    int32_t m_minLifetime;
    int32_t m_maxLifetime;
    float m_minSpeed;
    float m_maxSpeed;
    
    // Color
    float m_colorR;
    float m_colorG;
    float m_colorB;
    float m_colorA;
    
    // Dirty flag for change detection
    mutable bool m_dirty;
};

} // namespace renderbridge
