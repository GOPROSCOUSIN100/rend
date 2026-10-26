#pragma once

#include <cstdint>
#include <array>
#include <string>

namespace renderbridge {

/**
 * @class Entity
 * @brief Represents an entity in the 3D scene
 * 
 * This class stores entity data extracted from Minecraft and
 * provides methods for working with entities in the renderer.
 */
class Entity {
public:
    // Entity types
    enum Type : uint8_t {
        UNKNOWN = 0,
        PLAYER = 1,
        MOB = 2,
        ITEM = 3,
        PROJECTILE = 4,
        VEHICLE = 5,
        ARMOR_STAND = 6
    };
    
    // Entity flags
    enum Flag : uint32_t {
        NONE = 0x00000000,
        ON_FIRE = 0x00000001,
        SNEAKING = 0x00000002,
        SPRINTING = 0x00000004,
        SWIMMING = 0x00000008,
        INVISIBLE = 0x00000010,
        GLOWING = 0x00000020,
        FLYING = 0x00000040,
        ON_GROUND = 0x00000080,
        RIDING = 0x00000100,
        BABY = 0x00000200
    };

public:
    /**
     * @brief Constructor
     */
    Entity();
    
    /**
     * @brief Constructor with ID
     * @param entityId Entity ID
     */
    explicit Entity(uint64_t entityId);
    
    /**
     * @brief Destructor
     */
    ~Entity();
    
    /**
     * @brief Parse entity data from a message payload
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     * @return True if parsing succeeded
     */
    bool parseFromPayload(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Serialize entity data to a buffer
     * @param buffer Output buffer
     * @param bufferSize Size of the buffer
     * @return Number of bytes written
     */
    uint32_t serializeToBuffer(uint8_t* buffer, uint32_t bufferSize) const;
    
    // ID accessors
    uint64_t getEntityId() const;
    void setEntityId(uint64_t entityId);
    
    // Type accessors
    Type getType() const;
    void setType(Type type);
    
    // Position accessors
    float getPositionX() const;
    float getPositionY() const;
    float getPositionZ() const;
    void setPosition(float x, float y, float z);
    void setPositionX(float x);
    void setPositionY(float y);
    void setPositionZ(float z);
    
    // Previous position accessors (for interpolation)
    float getPreviousPositionX() const;
    float getPreviousPositionY() const;
    float getPreviousPositionZ() const;
    void setPreviousPosition(float x, float y, float z);
    
    // Rotation accessors (in degrees)
    float getRotationPitch() const;
    float getRotationYaw() const;
    float getRotationRoll() const;
    void setRotation(float pitch, float yaw, float roll);
    void setRotationPitch(float pitch);
    void setRotationYaw(float yaw);
    void setRotationRoll(float roll);
    
    // Scale accessors
    float getScaleX() const;
    float getScaleY() const;
    float getScaleZ() const;
    void setScale(float x, float y, float z);
    void setScale(float scale);
    
    // Velocity accessors
    float getVelocityX() const;
    float getVelocityY() const;
    float getVelocityZ() const;
    void setVelocity(float x, float y, float z);
    
    // Bounding box accessors
    float getBoundingBoxMinX() const;
    float getBoundingBoxMinY() const;
    float getBoundingBoxMinZ() const;
    float getBoundingBoxMaxX() const;
    float getBoundingBoxMaxY() const;
    float getBoundingBoxMaxZ() const;
    void setBoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
    
    // Health accessors
    float getHealth() const;
    void setHealth(float health);
    
    // Name accessors
    const std::string& getName() const;
    void setName(const std::string& name);
    
    // Texture accessors
    const std::string& getTexture() const;
    void setTexture(const std::string& texture);
    
    // Model accessors
    const std::string& getModel() const;
    void setModel(const std::string& model);
    
    // Flag accessors
    uint32_t getFlags() const;
    void setFlags(uint32_t flags);
    void addFlag(Flag flag);
    void removeFlag(Flag flag);
    bool hasFlag(Flag flag) const;
    
    // Convenience methods for checking flags
    bool isOnFire() const;
    bool isSneaking() const;
    bool isSprinting() const;
    bool isSwimming() const;
    bool isInvisible() const;
    bool isGlowing() const;
    bool isFlying() const;
    bool isOnGround() const;
    bool isRiding() const;
    bool isBaby() const;
    
    // Animation accessors
    uint32_t getAnimationState() const;
    void setAnimationState(uint32_t state);
    
    // Tick accessors
    uint64_t getTick() const;
    void setTick(uint64_t tick);
    
    // Interpolation
    /**
     * @brief Get interpolated position
     * @param partialTick Partial tick (0.0 to 1.0)
     * @param output Output position (3 floats)
     */
    void getInterpolatedPosition(float partialTick, float* output) const;
    
    /**
     * @brief Get interpolated rotation
     * @param partialTick Partial tick (0.0 to 1.0)
     * @param output Output rotation (3 floats: pitch, yaw, roll)
     */
    void getInterpolatedRotation(float partialTick, float* output) const;
    
    // Check if entity has changed
    /**
     * @brief Check if the entity has changed since the last update
     * @return True if the entity has changed
     */
    bool hasChanged() const;
    
    /**
     * @brief Mark the entity as unchanged
     */
    void markUnchanged();
    
    /**
     * @brief Update the entity from another entity
     * @param other Other entity
     */
    void updateFrom(const Entity& other);

private:
    uint64_t m_entityId;
    Type m_type;
    
    // Position
    float m_positionX;
    float m_positionY;
    float m_positionZ;
    
    // Previous position (for interpolation)
    float m_previousPositionX;
    float m_previousPositionY;
    float m_previousPositionZ;
    
    // Rotation (in degrees)
    float m_rotationPitch;
    float m_rotationYaw;
    float m_rotationRoll;
    
    // Scale
    float m_scaleX;
    float m_scaleY;
    float m_scaleZ;
    
    // Velocity
    float m_velocityX;
    float m_velocityY;
    float m_velocityZ;
    
    // Bounding box
    float m_boundingBoxMinX;
    float m_boundingBoxMinY;
    float m_boundingBoxMinZ;
    float m_boundingBoxMaxX;
    float m_boundingBoxMaxY;
    float m_boundingBoxMaxZ;
    
    // Health
    float m_health;
    
    // Name
    std::string m_name;
    
    // Texture
    std::string m_texture;
    
    // Model
    std::string m_model;
    
    // Flags
    uint32_t m_flags;
    
    // Animation state
    uint32_t m_animationState;
    
    // Tick
    uint64_t m_tick;
    
    // Dirty flag for change detection
    mutable bool m_dirty;
    
    // Previous values for change detection
    mutable float m_previousRotationPitch;
    mutable float m_previousRotationYaw;
    mutable float m_previousRotationRoll;
    mutable float m_previousScaleX;
    mutable float m_previousScaleY;
    mutable float m_previousScaleZ;
    mutable uint32_t m_previousFlags;
};

} // namespace renderbridge
