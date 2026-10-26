#include "entity.hpp"

#include <cstring>
#include <algorithm>
#include <cmath>

namespace renderbridge {

// Static constants
const uint32_t Entity::Flag[];

Entity::Entity()
    : m_entityId(0),
      m_type(UNKNOWN),
      m_positionX(0.0f), m_positionY(0.0f), m_positionZ(0.0f),
      m_previousPositionX(0.0f), m_previousPositionY(0.0f), m_previousPositionZ(0.0f),
      m_rotationPitch(0.0f), m_rotationYaw(0.0f), m_rotationRoll(0.0f),
      m_scaleX(1.0f), m_scaleY(1.0f), m_scaleZ(1.0f),
      m_velocityX(0.0f), m_velocityY(0.0f), m_velocityZ(0.0f),
      m_boundingBoxMinX(0.0f), m_boundingBoxMinY(0.0f), m_boundingBoxMinZ(0.0f),
      m_boundingBoxMaxX(0.0f), m_boundingBoxMaxY(0.0f), m_boundingBoxMaxZ(0.0f),
      m_health(1.0f),
      m_name(""),
      m_texture(""),
      m_model(""),
      m_flags(NONE),
      m_animationState(0),
      m_tick(0),
      m_dirty(true),
      m_previousRotationPitch(0.0f), m_previousRotationYaw(0.0f), m_previousRotationRoll(0.0f),
      m_previousScaleX(1.0f), m_previousScaleY(1.0f), m_previousScaleZ(1.0f),
      m_previousFlags(NONE)
{
}

Entity::Entity(uint64_t entityId)
    : m_entityId(entityId),
      m_type(UNKNOWN),
      m_positionX(0.0f), m_positionY(0.0f), m_positionZ(0.0f),
      m_previousPositionX(0.0f), m_previousPositionY(0.0f), m_previousPositionZ(0.0f),
      m_rotationPitch(0.0f), m_rotationYaw(0.0f), m_rotationRoll(0.0f),
      m_scaleX(1.0f), m_scaleY(1.0f), m_scaleZ(1.0f),
      m_velocityX(0.0f), m_velocityY(0.0f), m_velocityZ(0.0f),
      m_boundingBoxMinX(0.0f), m_boundingBoxMinY(0.0f), m_boundingBoxMinZ(0.0f),
      m_boundingBoxMaxX(0.0f), m_boundingBoxMaxY(0.0f), m_boundingBoxMaxZ(0.0f),
      m_health(1.0f),
      m_name(""),
      m_texture(""),
      m_model(""),
      m_flags(NONE),
      m_animationState(0),
      m_tick(0),
      m_dirty(true),
      m_previousRotationPitch(0.0f), m_previousRotationYaw(0.0f), m_previousRotationRoll(0.0f),
      m_previousScaleX(1.0f), m_previousScaleY(1.0f), m_previousScaleZ(1.0f),
      m_previousFlags(NONE)
{
}

Entity::~Entity()
{
}

// Parse entity data from a message payload
// Payload format matches EntityMessage.java:
// entityId(int32), entityType(int16), modelId(int16),
// positionX(float), positionY(float), positionZ(float),
// rotationPitch(float), rotationYaw(float), rotationRoll(float),
// velocityX(float), velocityY(float), velocityZ(float),
// metadataSize(uint16), metadata(bytes),
// flags(int8)
bool Entity::parseFromPayload(const uint8_t* payload, uint32_t payloadSize)
{
    if (payload == nullptr || payloadSize < 54) {
        // Minimum size: 4 + 2 + 2 + 12 + 12 + 12 + 2 + 1 = 54 bytes
        return false;
    }

    const uint8_t* ptr = payload;
    uint32_t offset = 0;

    // Read entity ID (int32 in Java, but we use uint64_t)
    m_entityId = static_cast<uint64_t>(*reinterpret_cast<const int32_t*>(ptr + offset));
    offset += 4;

    // Read entity type (int16)
    m_type = static_cast<Type>(*reinterpret_cast<const int16_t*>(ptr + offset));
    offset += 2;

    // Read model ID (int16) - stored as model name for now
    int16_t modelId = *reinterpret_cast<const int16_t*>(ptr + offset);
    offset += 2;
    m_model = "model_" + std::to_string(modelId);

    // Read position (3 floats)
    m_previousPositionX = m_positionX;
    m_previousPositionY = m_positionY;
    m_previousPositionZ = m_positionZ;
    
    m_positionX = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_positionY = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_positionZ = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    // Read rotation (3 floats)
    m_previousRotationPitch = m_rotationPitch;
    m_previousRotationYaw = m_rotationYaw;
    m_previousRotationRoll = m_rotationRoll;
    
    m_rotationPitch = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_rotationYaw = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_rotationRoll = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    // Read velocity (3 floats)
    m_velocityX = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_velocityY = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_velocityZ = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    // Read metadata size (uint16)
    uint16_t metadataSize = *reinterpret_cast<const uint16_t*>(ptr + offset);
    offset += 2;

    // Read metadata
    if (offset + metadataSize <= payloadSize) {
        // Parse metadata as key-value pairs
        const uint8_t* metadataPtr = ptr + offset;
        const uint8_t* metadataEnd = metadataPtr + metadataSize;
        
        while (metadataPtr < metadataEnd) {
            // Read key length (uint8)
            uint8_t keyLen = *metadataPtr++;
            if (metadataPtr + keyLen > metadataEnd) break;
            
            // Read key
            std::string key(reinterpret_cast<const char*>(metadataPtr), keyLen);
            metadataPtr += keyLen;
            
            // Read value type (uint8)
            uint8_t valueType = *metadataPtr++;
            
            // Read value based on type
            switch (valueType) {
                case 0: { // String
                    uint16_t strLen = *reinterpret_cast<const uint16_t*>(metadataPtr);
                    metadataPtr += 2;
                    if (metadataPtr + strLen <= metadataEnd) {
                        std::string value(reinterpret_cast<const char*>(metadataPtr), strLen);
                        metadataPtr += strLen;
                        
                        if (key == "name") {
                            m_name = value;
                        } else if (key == "texture") {
                            m_texture = value;
                        }
                    }
                    break;
                }
                case 1: { // Float
                    float value = *reinterpret_cast<const float*>(metadataPtr);
                    metadataPtr += 4;
                    
                    if (key == "health") {
                        m_health = value;
                    } else if (key == "scaleX") {
                        m_scaleX = value;
                    } else if (key == "scaleY") {
                        m_scaleY = value;
                    } else if (key == "scaleZ") {
                        m_scaleZ = value;
                    }
                    break;
                }
                case 2: { // Int32
                    int32_t value = *reinterpret_cast<const int32_t*>(metadataPtr);
                    metadataPtr += 4;
                    
                    if (key == "flags") {
                        m_flags = static_cast<uint32_t>(value);
                    } else if (key == "animationState") {
                        m_animationState = static_cast<uint32_t>(value);
                    }
                    break;
                }
                default:
                    // Skip unknown types
                    break;
            }
        }
        
        offset += metadataSize;
    }

    // Read flags (int8)
    if (offset < payloadSize) {
        m_previousFlags = m_flags;
        m_flags = static_cast<uint32_t>(*reinterpret_cast<const int8_t*>(ptr + offset));
    }

    // Mark as changed
    m_dirty = true;
    
    // Update previous values for interpolation
    m_previousPositionX = m_positionX;
    m_previousPositionY = m_positionY;
    m_previousPositionZ = m_positionZ;

    return true;
}

// Alternative parse method for simpler format (without metadata key-value parsing)
// This matches the Java EntityMessage more directly
bool Entity::parseFromPayloadSimple(const uint8_t* payload, uint32_t payloadSize)
{
    if (payload == nullptr || payloadSize < 54) {
        return false;
    }

    const uint8_t* ptr = payload;
    uint32_t offset = 0;

    // Read entity ID (int32)
    m_entityId = static_cast<uint64_t>(*reinterpret_cast<const int32_t*>(ptr + offset));
    offset += 4;

    // Read entity type (int16)
    m_type = static_cast<Type>(*reinterpret_cast<const int16_t*>(ptr + offset));
    offset += 2;

    // Read model ID (int16)
    int16_t modelId = *reinterpret_cast<const int16_t*>(ptr + offset);
    offset += 2;
    m_model = "model_" + std::to_string(modelId);

    // Read position
    m_previousPositionX = m_positionX;
    m_previousPositionY = m_positionY;
    m_previousPositionZ = m_positionZ;
    
    m_positionX = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_positionY = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_positionZ = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    // Read rotation
    m_previousRotationPitch = m_rotationPitch;
    m_previousRotationYaw = m_rotationYaw;
    m_previousRotationRoll = m_rotationRoll;
    
    m_rotationPitch = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_rotationYaw = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_rotationRoll = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    // Read velocity
    m_velocityX = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_velocityY = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_velocityZ = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    // Read metadata size (uint16)
    uint16_t metadataSize = *reinterpret_cast<const uint16_t*>(ptr + offset);
    offset += 2;

    // Skip metadata for now
    offset += metadataSize;

    // Read flags (int8)
    if (offset < payloadSize) {
        m_previousFlags = m_flags;
        m_flags = static_cast<uint32_t>(*reinterpret_cast<const uint8_t*>(ptr + offset));
    }

    m_dirty = true;
    return true;
}

// Serialize entity data to a buffer
uint32_t Entity::serializeToBuffer(uint8_t* buffer, uint32_t bufferSize) const
{
    if (buffer == nullptr || bufferSize < 54) {
        return 0;
    }

    uint8_t* ptr = buffer;
    uint32_t offset = 0;

    // Write entity ID (int32)
    *reinterpret_cast<int32_t*>(ptr + offset) = static_cast<int32_t>(m_entityId & 0xFFFFFFFF);
    offset += 4;

    // Write entity type (int16)
    *reinterpret_cast<int16_t*>(ptr + offset) = static_cast<int16_t>(m_type);
    offset += 2;

    // Write model ID (int16) - use 0 for now
    *reinterpret_cast<int16_t*>(ptr + offset) = 0;
    offset += 2;

    // Write position
    *reinterpret_cast<float*>(ptr + offset) = m_positionX;
    offset += 4;
    *reinterpret_cast<float*>(ptr + offset) = m_positionY;
    offset += 4;
    *reinterpret_cast<float*>(ptr + offset) = m_positionZ;
    offset += 4;

    // Write rotation
    *reinterpret_cast<float*>(ptr + offset) = m_rotationPitch;
    offset += 4;
    *reinterpret_cast<float*>(ptr + offset) = m_rotationYaw;
    offset += 4;
    *reinterpret_cast<float*>(ptr + offset) = m_rotationRoll;
    offset += 4;

    // Write velocity
    *reinterpret_cast<float*>(ptr + offset) = m_velocityX;
    offset += 4;
    *reinterpret_cast<float*>(ptr + offset) = m_velocityY;
    offset += 4;
    *reinterpret_cast<float*>(ptr + offset) = m_velocityZ;
    offset += 4;

    // Write metadata size (0 for now)
    *reinterpret_cast<uint16_t*>(ptr + offset) = 0;
    offset += 2;

    // Write flags (int8)
    *reinterpret_cast<int8_t*>(ptr + offset) = static_cast<int8_t>(m_flags & 0xFF);
    offset += 1;

    return offset;
}

// ID accessors
uint64_t Entity::getEntityId() const {
    return m_entityId;
}

void Entity::setEntityId(uint64_t entityId) {
    if (m_entityId != entityId) {
        m_entityId = entityId;
        m_dirty = true;
    }
}

// Type accessors
Entity::Type Entity::getType() const {
    return m_type;
}

void Entity::setType(Type type) {
    if (m_type != type) {
        m_type = type;
        m_dirty = true;
    }
}

// Position accessors
float Entity::getPositionX() const {
    return m_positionX;
}

float Entity::getPositionY() const {
    return m_positionY;
}

float Entity::getPositionZ() const {
    return m_positionZ;
}

void Entity::setPosition(float x, float y, float z) {
    if (m_positionX != x || m_positionY != y || m_positionZ != z) {
        m_positionX = x;
        m_positionY = y;
        m_positionZ = z;
        m_dirty = true;
    }
}

void Entity::setPositionX(float x) {
    if (m_positionX != x) {
        m_positionX = x;
        m_dirty = true;
    }
}

void Entity::setPositionY(float y) {
    if (m_positionY != y) {
        m_positionY = y;
        m_dirty = true;
    }
}

void Entity::setPositionZ(float z) {
    if (m_positionZ != z) {
        m_positionZ = z;
        m_dirty = true;
    }
}

// Previous position accessors
float Entity::getPreviousPositionX() const {
    return m_previousPositionX;
}

float Entity::getPreviousPositionY() const {
    return m_previousPositionY;
}

float Entity::getPreviousPositionZ() const {
    return m_previousPositionZ;
}

void Entity::setPreviousPosition(float x, float y, float z) {
    m_previousPositionX = x;
    m_previousPositionY = y;
    m_previousPositionZ = z;
}

// Rotation accessors
float Entity::getRotationPitch() const {
    return m_rotationPitch;
}

float Entity::getRotationYaw() const {
    return m_rotationYaw;
}

float Entity::getRotationRoll() const {
    return m_rotationRoll;
}

void Entity::setRotation(float pitch, float yaw, float roll) {
    if (m_rotationPitch != pitch || m_rotationYaw != yaw || m_rotationRoll != roll) {
        m_rotationPitch = pitch;
        m_rotationYaw = yaw;
        m_rotationRoll = roll;
        m_dirty = true;
    }
}

void Entity::setRotationPitch(float pitch) {
    if (m_rotationPitch != pitch) {
        m_rotationPitch = pitch;
        m_dirty = true;
    }
}

void Entity::setRotationYaw(float yaw) {
    if (m_rotationYaw != yaw) {
        m_rotationYaw = yaw;
        m_dirty = true;
    }
}

void Entity::setRotationRoll(float roll) {
    if (m_rotationRoll != roll) {
        m_rotationRoll = roll;
        m_dirty = true;
    }
}

// Scale accessors
float Entity::getScaleX() const {
    return m_scaleX;
}

float Entity::getScaleY() const {
    return m_scaleY;
}

float Entity::getScaleZ() const {
    return m_scaleZ;
}

void Entity::setScale(float x, float y, float z) {
    if (m_scaleX != x || m_scaleY != y || m_scaleZ != z) {
        m_scaleX = x;
        m_scaleY = y;
        m_scaleZ = z;
        m_dirty = true;
    }
}

void Entity::setScale(float scale) {
    setScale(scale, scale, scale);
}

// Velocity accessors
float Entity::getVelocityX() const {
    return m_velocityX;
}

float Entity::getVelocityY() const {
    return m_velocityY;
}

float Entity::getVelocityZ() const {
    return m_velocityZ;
}

void Entity::setVelocity(float x, float y, float z) {
    if (m_velocityX != x || m_velocityY != y || m_velocityZ != z) {
        m_velocityX = x;
        m_velocityY = y;
        m_velocityZ = z;
        m_dirty = true;
    }
}

// Bounding box accessors
float Entity::getBoundingBoxMinX() const {
    return m_boundingBoxMinX;
}

float Entity::getBoundingBoxMinY() const {
    return m_boundingBoxMinY;
}

float Entity::getBoundingBoxMinZ() const {
    return m_boundingBoxMinZ;
}

float Entity::getBoundingBoxMaxX() const {
    return m_boundingBoxMaxX;
}

float Entity::getBoundingBoxMaxY() const {
    return m_boundingBoxMaxY;
}

float Entity::getBoundingBoxMaxZ() const {
    return m_boundingBoxMaxZ;
}

void Entity::setBoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) {
    if (m_boundingBoxMinX != minX || m_boundingBoxMinY != minY || m_boundingBoxMinZ != minZ ||
        m_boundingBoxMaxX != maxX || m_boundingBoxMaxY != maxY || m_boundingBoxMaxZ != maxZ) {
        m_boundingBoxMinX = minX;
        m_boundingBoxMinY = minY;
        m_boundingBoxMinZ = minZ;
        m_boundingBoxMaxX = maxX;
        m_boundingBoxMaxY = maxY;
        m_boundingBoxMaxZ = maxZ;
        m_dirty = true;
    }
}

// Health accessors
float Entity::getHealth() const {
    return m_health;
}

void Entity::setHealth(float health) {
    if (m_health != health) {
        m_health = health;
        m_dirty = true;
    }
}

// Name accessors
const std::string& Entity::getName() const {
    return m_name;
}

void Entity::setName(const std::string& name) {
    if (m_name != name) {
        m_name = name;
        m_dirty = true;
    }
}

// Texture accessors
const std::string& Entity::getTexture() const {
    return m_texture;
}

void Entity::setTexture(const std::string& texture) {
    if (m_texture != texture) {
        m_texture = texture;
        m_dirty = true;
    }
}

// Model accessors
const std::string& Entity::getModel() const {
    return m_model;
}

void Entity::setModel(const std::string& model) {
    if (m_model != model) {
        m_model = model;
        m_dirty = true;
    }
}

// Flag accessors
uint32_t Entity::getFlags() const {
    return m_flags;
}

void Entity::setFlags(uint32_t flags) {
    if (m_flags != flags) {
        m_flags = flags;
        m_dirty = true;
    }
}

void Entity::addFlag(Flag flag) {
    if ((m_flags & flag) == 0) {
        m_flags |= flag;
        m_dirty = true;
    }
}

void Entity::removeFlag(Flag flag) {
    if ((m_flags & flag) != 0) {
        m_flags &= ~flag;
        m_dirty = true;
    }
}

bool Entity::hasFlag(Flag flag) const {
    return (m_flags & flag) != 0;
}

// Convenience methods for checking flags
bool Entity::isOnFire() const {
    return hasFlag(ON_FIRE);
}

bool Entity::isSneaking() const {
    return hasFlag(SNEAKING);
}

bool Entity::isSprinting() const {
    return hasFlag(SPRINTING);
}

bool Entity::isSwimming() const {
    return hasFlag(SWIMMING);
}

bool Entity::isInvisible() const {
    return hasFlag(INVISIBLE);
}

bool Entity::isGlowing() const {
    return hasFlag(GLOWING);
}

bool Entity::isFlying() const {
    return hasFlag(FLYING);
}

bool Entity::isOnGround() const {
    return hasFlag(ON_GROUND);
}

bool Entity::isRiding() const {
    return hasFlag(RIDING);
}

bool Entity::isBaby() const {
    return hasFlag(BABY);
}

// Animation accessors
uint32_t Entity::getAnimationState() const {
    return m_animationState;
}

void Entity::setAnimationState(uint32_t state) {
    if (m_animationState != state) {
        m_animationState = state;
        m_dirty = true;
    }
}

// Tick accessors
uint64_t Entity::getTick() const {
    return m_tick;
}

void Entity::setTick(uint64_t tick) {
    if (m_tick != tick) {
        m_tick = tick;
        m_dirty = true;
    }
}

// Interpolation
void Entity::getInterpolatedPosition(float partialTick, float* output) const {
    if (output == nullptr) return;
    
    output[0] = m_previousPositionX + (m_positionX - m_previousPositionX) * partialTick;
    output[1] = m_previousPositionY + (m_positionY - m_previousPositionY) * partialTick;
    output[2] = m_previousPositionZ + (m_positionZ - m_previousPositionZ) * partialTick;
}

void Entity::getInterpolatedRotation(float partialTick, float* output) const {
    if (output == nullptr) return;
    
    // Slerp for rotation
    float prevPitch = m_previousRotationPitch;
    float prevYaw = m_previousRotationYaw;
    float prevRoll = m_previousRotationRoll;
    
    // Simple linear interpolation for rotation (good enough for most cases)
    output[0] = prevPitch + (m_rotationPitch - prevPitch) * partialTick;
    output[1] = prevYaw + (m_rotationYaw - prevYaw) * partialTick;
    output[2] = prevRoll + (m_rotationRoll - prevRoll) * partialTick;
}

// Change detection
bool Entity::hasChanged() const {
    return m_dirty;
}

void Entity::markUnchanged() {
    m_dirty = false;
    m_previousPositionX = m_positionX;
    m_previousPositionY = m_positionY;
    m_previousPositionZ = m_positionZ;
    m_previousRotationPitch = m_rotationPitch;
    m_previousRotationYaw = m_rotationYaw;
    m_previousRotationRoll = m_rotationRoll;
    m_previousScaleX = m_scaleX;
    m_previousScaleY = m_scaleY;
    m_previousScaleZ = m_scaleZ;
    m_previousFlags = m_flags;
}

// Update from another entity
void Entity::updateFrom(const Entity& other) {
    if (m_entityId != other.m_entityId) {
        return; // Don't update if entity IDs don't match
    }

    m_type = other.m_type;
    
    m_previousPositionX = m_positionX;
    m_previousPositionY = m_positionY;
    m_previousPositionZ = m_positionZ;
    m_positionX = other.m_positionX;
    m_positionY = other.m_positionY;
    m_positionZ = other.m_positionZ;
    
    m_previousRotationPitch = m_rotationPitch;
    m_previousRotationYaw = m_rotationYaw;
    m_previousRotationRoll = m_rotationRoll;
    m_rotationPitch = other.m_rotationPitch;
    m_rotationYaw = other.m_rotationYaw;
    m_rotationRoll = other.m_rotationRoll;
    
    m_scaleX = other.m_scaleX;
    m_scaleY = other.m_scaleY;
    m_scaleZ = other.m_scaleZ;
    
    m_velocityX = other.m_velocityX;
    m_velocityY = other.m_velocityY;
    m_velocityZ = other.m_velocityZ;
    
    m_boundingBoxMinX = other.m_boundingBoxMinX;
    m_boundingBoxMinY = other.m_boundingBoxMinY;
    m_boundingBoxMinZ = other.m_boundingBoxMinZ;
    m_boundingBoxMaxX = other.m_boundingBoxMaxX;
    m_boundingBoxMaxY = other.m_boundingBoxMaxY;
    m_boundingBoxMaxZ = other.m_boundingBoxMaxZ;
    
    m_health = other.m_health;
    m_name = other.m_name;
    m_texture = other.m_texture;
    m_model = other.m_model;
    
    m_previousFlags = m_flags;
    m_flags = other.m_flags;
    
    m_animationState = other.m_animationState;
    m_tick = other.m_tick;
    
    m_dirty = true;
}

} // namespace renderbridge
