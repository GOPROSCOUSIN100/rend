#include "camera.hpp"

#include <cmath>
#include <algorithm>
#include <cstring>

namespace renderbridge {

Camera::Camera()
    : m_frameNumber(-1),
      m_timestamp(0),
      m_positionX(0.0f), m_positionY(0.0f), m_positionZ(0.0f),
      m_rotationPitch(0.0f), m_rotationYaw(0.0f), m_rotationRoll(0.0f),
      m_fov(70.0f),
      m_nearPlane(0.1f),
      m_farPlane(1000.0f),
      m_viewportX(0), m_viewportY(0),
      m_viewportWidth(1280), m_viewportHeight(720),
      m_flags(0),
      m_dirty(false),
      m_previousFrameNumber(-1),
      m_previousTimestamp(0),
      m_previousPositionX(0.0f), m_previousPositionY(0.0f), m_previousPositionZ(0.0f),
      m_previousRotationPitch(0.0f), m_previousRotationYaw(0.0f), m_previousRotationRoll(0.0f),
      m_previousFov(70.0f),
      m_previousFlags(0) {
    
    // Initialize matrices
    updateViewMatrix();
    updateProjectionMatrix();
}

Camera::~Camera() {
}

bool Camera::parseFromPayload(const uint8_t* payload, uint32_t payloadSize) {
    // Expected payload size: 8 + 8 + 12 + 12 + 4 + 4 + 4 + 64 + 64 + 16 + 4 = 208 bytes
    if (payloadSize < 208) {
        return false;
    }
    
    uint32_t offset = 0;
    
    // Frame number (8 bytes)
    m_frameNumber = static_cast<int64_t>(
        static_cast<uint64_t>(payload[offset]) << 56 |
        static_cast<uint64_t>(payload[offset + 1]) << 48 |
        static_cast<uint64_t>(payload[offset + 2]) << 40 |
        static_cast<uint64_t>(payload[offset + 3]) << 32 |
        static_cast<uint64_t>(payload[offset + 4]) << 24 |
        static_cast<uint64_t>(payload[offset + 5]) << 16 |
        static_cast<uint64_t>(payload[offset + 6]) << 8 |
        static_cast<uint64_t>(payload[offset + 7])
    );
    offset += 8;
    
    // Timestamp (8 bytes)
    m_timestamp = static_cast<int64_t>(
        static_cast<uint64_t>(payload[offset]) << 56 |
        static_cast<uint64_t>(payload[offset + 1]) << 48 |
        static_cast<uint64_t>(payload[offset + 2]) << 40 |
        static_cast<uint64_t>(payload[offset + 3]) << 32 |
        static_cast<uint64_t>(payload[offset + 4]) << 24 |
        static_cast<uint64_t>(payload[offset + 5]) << 16 |
        static_cast<uint64_t>(payload[offset + 6]) << 8 |
        static_cast<uint64_t>(payload[offset + 7])
    );
    offset += 8;
    
    // Position (12 bytes = 3 floats)
    uint32_t floatBits;
    floatBits = static_cast<uint32_t>(payload[offset]) << 24 |
               static_cast<uint32_t>(payload[offset + 1]) << 16 |
               static_cast<uint32_t>(payload[offset + 2]) << 8 |
               static_cast<uint32_t>(payload[offset + 3]);
    std::memcpy(&m_positionX, &floatBits, sizeof(float));
    offset += 4;
    
    floatBits = static_cast<uint32_t>(payload[offset]) << 24 |
               static_cast<uint32_t>(payload[offset + 1]) << 16 |
               static_cast<uint32_t>(payload[offset + 2]) << 8 |
               static_cast<uint32_t>(payload[offset + 3]);
    std::memcpy(&m_positionY, &floatBits, sizeof(float));
    offset += 4;
    
    floatBits = static_cast<uint32_t>(payload[offset]) << 24 |
               static_cast<uint32_t>(payload[offset + 1]) << 16 |
               static_cast<uint32_t>(payload[offset + 2]) << 8 |
               static_cast<uint32_t>(payload[offset + 3]);
    std::memcpy(&m_positionZ, &floatBits, sizeof(float));
    offset += 4;
    
    // Rotation (12 bytes = 3 floats)
    floatBits = static_cast<uint32_t>(payload[offset]) << 24 |
               static_cast<uint32_t>(payload[offset + 1]) << 16 |
               static_cast<uint32_t>(payload[offset + 2]) << 8 |
               static_cast<uint32_t>(payload[offset + 3]);
    std::memcpy(&m_rotationPitch, &floatBits, sizeof(float));
    offset += 4;
    
    floatBits = static_cast<uint32_t>(payload[offset]) << 24 |
               static_cast<uint32_t>(payload[offset + 1]) << 16 |
               static_cast<uint32_t>(payload[offset + 2]) << 8 |
               static_cast<uint32_t>(payload[offset + 3]);
    std::memcpy(&m_rotationYaw, &floatBits, sizeof(float));
    offset += 4;
    
    floatBits = static_cast<uint32_t>(payload[offset]) << 24 |
               static_cast<uint32_t>(payload[offset + 1]) << 16 |
               static_cast<uint32_t>(payload[offset + 2]) << 8 |
               static_cast<uint32_t>(payload[offset + 3]);
    std::memcpy(&m_rotationRoll, &floatBits, sizeof(float));
    offset += 4;
    
    // FOV, near, far (12 bytes = 3 floats)
    floatBits = static_cast<uint32_t>(payload[offset]) << 24 |
               static_cast<uint32_t>(payload[offset + 1]) << 16 |
               static_cast<uint32_t>(payload[offset + 2]) << 8 |
               static_cast<uint32_t>(payload[offset + 3]);
    std::memcpy(&m_fov, &floatBits, sizeof(float));
    offset += 4;
    
    floatBits = static_cast<uint32_t>(payload[offset]) << 24 |
               static_cast<uint32_t>(payload[offset + 1]) << 16 |
               static_cast<uint32_t>(payload[offset + 2]) << 8 |
               static_cast<uint32_t>(payload[offset + 3]);
    std::memcpy(&m_nearPlane, &floatBits, sizeof(float));
    offset += 4;
    
    floatBits = static_cast<uint32_t>(payload[offset]) << 24 |
               static_cast<uint32_t>(payload[offset + 1]) << 16 |
               static_cast<uint32_t>(payload[offset + 2]) << 8 |
               static_cast<uint32_t>(payload[offset + 3]);
    std::memcpy(&m_farPlane, &floatBits, sizeof(float));
    offset += 4;
    
    // View matrix (64 bytes = 16 floats)
    for (int i = 0; i < 16; i++) {
        floatBits = static_cast<uint32_t>(payload[offset]) << 24 |
                   static_cast<uint32_t>(payload[offset + 1]) << 16 |
                   static_cast<uint32_t>(payload[offset + 2]) << 8 |
                   static_cast<uint32_t>(payload[offset + 3]);
        std::memcpy(&m_viewMatrix[i], &floatBits, sizeof(float));
        offset += 4;
    }
    
    // Projection matrix (64 bytes = 16 floats)
    for (int i = 0; i < 16; i++) {
        floatBits = static_cast<uint32_t>(payload[offset]) << 24 |
                   static_cast<uint32_t>(payload[offset + 1]) << 16 |
                   static_cast<uint32_t>(payload[offset + 2]) << 8 |
                   static_cast<uint32_t>(payload[offset + 3]);
        std::memcpy(&m_projectionMatrix[i], &floatBits, sizeof(float));
        offset += 4;
    }
    
    // Viewport (16 bytes = 4 ints)
    m_viewportX = static_cast<int32_t>(payload[offset]) << 24 |
                  static_cast<int32_t>(payload[offset + 1]) << 16 |
                  static_cast<int32_t>(payload[offset + 2]) << 8 |
                  static_cast<int32_t>(payload[offset + 3]);
    offset += 4;
    
    m_viewportY = static_cast<int32_t>(payload[offset]) << 24 |
                  static_cast<int32_t>(payload[offset + 1]) << 16 |
                  static_cast<int32_t>(payload[offset + 2]) << 8 |
                  static_cast<int32_t>(payload[offset + 3]);
    offset += 4;
    
    m_viewportWidth = static_cast<int32_t>(payload[offset]) << 24 |
                      static_cast<int32_t>(payload[offset + 1]) << 16 |
                      static_cast<int32_t>(payload[offset + 2]) << 8 |
                      static_cast<int32_t>(payload[offset + 3]);
    offset += 4;
    
    m_viewportHeight = static_cast<int32_t>(payload[offset]) << 24 |
                       static_cast<int32_t>(payload[offset + 1]) << 16 |
                       static_cast<int32_t>(payload[offset + 2]) << 8 |
                       static_cast<int32_t>(payload[offset + 3]);
    offset += 4;
    
    // Flags (4 bytes)
    m_flags = static_cast<uint32_t>(payload[offset]) << 24 |
              static_cast<uint32_t>(payload[offset + 1]) << 16 |
              static_cast<uint32_t>(payload[offset + 2]) << 8 |
              static_cast<uint32_t>(payload[offset + 3]);
    
    // Update matrices from position/rotation if needed
    updateViewMatrix();
    updateProjectionMatrix();
    
    return true;
}

uint32_t Camera::serializeToBuffer(uint8_t* buffer, uint32_t bufferSize) const {
    // Calculate required size
    uint32_t requiredSize = 208; // Same as parseFromPayload
    if (bufferSize < requiredSize) {
        return 0;
    }
    
    uint32_t offset = 0;
    
    // Frame number (8 bytes)
    uint64_t frameNumber = static_cast<uint64_t>(m_frameNumber);
    buffer[offset++] = static_cast<uint8_t>((frameNumber >> 56) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((frameNumber >> 48) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((frameNumber >> 40) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((frameNumber >> 32) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((frameNumber >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((frameNumber >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((frameNumber >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(frameNumber & 0xFF);
    
    // Timestamp (8 bytes)
    uint64_t timestamp = static_cast<uint64_t>(m_timestamp);
    buffer[offset++] = static_cast<uint8_t>((timestamp >> 56) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((timestamp >> 48) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((timestamp >> 40) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((timestamp >> 32) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((timestamp >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((timestamp >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((timestamp >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(timestamp & 0xFF);
    
    // Position (12 bytes)
    uint32_t floatBits;
    std::memcpy(&floatBits, &m_positionX, sizeof(float));
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(floatBits & 0xFF);
    
    std::memcpy(&floatBits, &m_positionY, sizeof(float));
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(floatBits & 0xFF);
    
    std::memcpy(&floatBits, &m_positionZ, sizeof(float));
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(floatBits & 0xFF);
    
    // Rotation (12 bytes)
    std::memcpy(&floatBits, &m_rotationPitch, sizeof(float));
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(floatBits & 0xFF);
    
    std::memcpy(&floatBits, &m_rotationYaw, sizeof(float));
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(floatBits & 0xFF);
    
    std::memcpy(&floatBits, &m_rotationRoll, sizeof(float));
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(floatBits & 0xFF);
    
    // FOV, near, far (12 bytes)
    std::memcpy(&floatBits, &m_fov, sizeof(float));
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(floatBits & 0xFF);
    
    std::memcpy(&floatBits, &m_nearPlane, sizeof(float));
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(floatBits & 0xFF);
    
    std::memcpy(&floatBits, &m_farPlane, sizeof(float));
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((floatBits >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(floatBits & 0xFF);
    
    // View matrix (64 bytes)
    for (int i = 0; i < 16; i++) {
        std::memcpy(&floatBits, &m_viewMatrix[i], sizeof(float));
        buffer[offset++] = static_cast<uint8_t>((floatBits >> 24) & 0xFF);
        buffer[offset++] = static_cast<uint8_t>((floatBits >> 16) & 0xFF);
        buffer[offset++] = static_cast<uint8_t>((floatBits >> 8) & 0xFF);
        buffer[offset++] = static_cast<uint8_t>(floatBits & 0xFF);
    }
    
    // Projection matrix (64 bytes)
    for (int i = 0; i < 16; i++) {
        std::memcpy(&floatBits, &m_projectionMatrix[i], sizeof(float));
        buffer[offset++] = static_cast<uint8_t>((floatBits >> 24) & 0xFF);
        buffer[offset++] = static_cast<uint8_t>((floatBits >> 16) & 0xFF);
        buffer[offset++] = static_cast<uint8_t>((floatBits >> 8) & 0xFF);
        buffer[offset++] = static_cast<uint8_t>(floatBits & 0xFF);
    }
    
    // Viewport (16 bytes)
    uint32_t intValue = static_cast<uint32_t>(m_viewportX);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(intValue & 0xFF);
    
    intValue = static_cast<uint32_t>(m_viewportY);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(intValue & 0xFF);
    
    intValue = static_cast<uint32_t>(m_viewportWidth);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(intValue & 0xFF);
    
    intValue = static_cast<uint32_t>(m_viewportHeight);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(intValue & 0xFF);
    
    // Flags (4 bytes)
    buffer[offset++] = static_cast<uint8_t>((m_flags >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((m_flags >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((m_flags >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(m_flags & 0xFF);
    
    return offset;
}

int64_t Camera::getFrameNumber() const {
    return m_frameNumber;
}

void Camera::setFrameNumber(int64_t frameNumber) {
    m_frameNumber = frameNumber;
}

int64_t Camera::getTimestamp() const {
    return m_timestamp;
}

void Camera::setTimestamp(int64_t timestamp) {
    m_timestamp = timestamp;
}

float Camera::getPositionX() const { return m_positionX; }
float Camera::getPositionY() const { return m_positionY; }
float Camera::getPositionZ() const { return m_positionZ; }

void Camera::setPosition(float x, float y, float z) {
    m_positionX = x;
    m_positionY = y;
    m_positionZ = z;
    updateViewMatrix();
}

void Camera::setPositionX(float x) { m_positionX = x; updateViewMatrix(); }
void Camera::setPositionY(float y) { m_positionY = y; updateViewMatrix(); }
void Camera::setPositionZ(float z) { m_positionZ = z; updateViewMatrix(); }

float Camera::getRotationPitch() const { return m_rotationPitch; }
float Camera::getRotationYaw() const { return m_rotationYaw; }
float Camera::getRotationRoll() const { return m_rotationRoll; }

void Camera::setRotation(float pitch, float yaw, float roll) {
    m_rotationPitch = pitch;
    m_rotationYaw = yaw;
    m_rotationRoll = roll;
    updateViewMatrix();
}

void Camera::setRotationPitch(float pitch) { m_rotationPitch = pitch; updateViewMatrix(); }
void Camera::setRotationYaw(float yaw) { m_rotationYaw = yaw; updateViewMatrix(); }
void Camera::setRotationRoll(float roll) { m_rotationRoll = roll; updateViewMatrix(); }

float Camera::getFov() const { return m_fov; }
float Camera::getNearPlane() const { return m_nearPlane; }
float Camera::getFarPlane() const { return m_farPlane; }

void Camera::setFov(float fov) { m_fov = fov; updateProjectionMatrix(); }
void Camera::setNearPlane(float nearPlane) { m_nearPlane = nearPlane; updateProjectionMatrix(); }
void Camera::setFarPlane(float farPlane) { m_farPlane = farPlane; updateProjectionMatrix(); }

const float* Camera::getViewMatrix() const { return m_viewMatrix.data(); }
const float* Camera::getProjectionMatrix() const { return m_projectionMatrix.data(); }

void Camera::setViewMatrix(const float* matrix) {
    if (matrix) {
        std::memcpy(m_viewMatrix.data(), matrix, 16 * sizeof(float));
    }
}

void Camera::setProjectionMatrix(const float* matrix) {
    if (matrix) {
        std::memcpy(m_projectionMatrix.data(), matrix, 16 * sizeof(float));
    }
}

uint32_t Camera::getFlags() const { return m_flags; }
void Camera::setFlags(uint32_t flags) { m_flags = flags; }

void Camera::addFlag(Flag flag) { m_flags |= static_cast<uint32_t>(flag); }
void Camera::removeFlag(Flag flag) { m_flags &= ~static_cast<uint32_t>(flag); }
bool Camera::hasFlag(Flag flag) const { return (m_flags & static_cast<uint32_t>(flag)) != 0; }

bool Camera::isThirdPerson() const { return hasFlag(FLAG_THIRD_PERSON); }
bool Camera::isInverseView() const { return hasFlag(FLAG_INVERSE_VIEW); }
bool Camera::isUnderwater() const { return hasFlag(FLAG_UNDERWATER); }
bool Camera::isInPortal() const { return hasFlag(FLAG_IN_PORTAL); }

int32_t Camera::getViewportX() const { return m_viewportX; }
int32_t Camera::getViewportY() const { return m_viewportY; }
int32_t Camera::getViewportWidth() const { return m_viewportWidth; }
int32_t Camera::getViewportHeight() const { return m_viewportHeight; }

void Camera::setViewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    m_viewportX = x;
    m_viewportY = y;
    m_viewportWidth = width;
    m_viewportHeight = height;
    updateProjectionMatrix();
}

void Camera::calculateViewProjectionMatrix(float* output) const {
    // Multiply view and projection matrices
    // This is a simplified implementation; in production, use a proper matrix library
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            output[i * 4 + j] = 0.0f;
            for (int k = 0; k < 4; k++) {
                output[i * 4 + j] += m_viewMatrix[i * 4 + k] * m_projectionMatrix[k * 4 + j];
            }
        }
    }
}

void Camera::calculateInverseViewMatrix(float* output) const {
    // Simple transpose for orthogonal matrices (assuming no scaling)
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            output[i * 4 + j] = m_viewMatrix[j * 4 + i];
        }
    }
    
    // Translate back
    output[12] = -m_positionX;
    output[13] = -m_positionY;
    output[14] = -m_positionZ;
}

void Camera::calculateDirectionVector(float* output) const {
    // Direction is the negative Z axis in view space
    output[0] = -m_viewMatrix[8];  // -Z.x
    output[1] = -m_viewMatrix[9];  // -Z.y
    output[2] = -m_viewMatrix[10]; // -Z.z
}

void Camera::calculateUpVector(float* output) const {
    // Up is the Y axis in view space
    output[0] = m_viewMatrix[4];  // Y.x
    output[1] = m_viewMatrix[5];  // Y.y
    output[2] = m_viewMatrix[6];  // Y.z
}

void Camera::calculateRightVector(float* output) const {
    // Right is the X axis in view space
    output[0] = m_viewMatrix[0];  // X.x
    output[1] = m_viewMatrix[1];  // X.y
    output[2] = m_viewMatrix[2];  // X.z
}

bool Camera::hasChanged() const {
    if (m_frameNumber != m_previousFrameNumber) return true;
    if (m_timestamp != m_previousTimestamp) return true;
    if (m_positionX != m_previousPositionX) return true;
    if (m_positionY != m_previousPositionY) return true;
    if (m_positionZ != m_previousPositionZ) return true;
    if (m_rotationPitch != m_previousRotationPitch) return true;
    if (m_rotationYaw != m_previousRotationYaw) return true;
    if (m_rotationRoll != m_previousRotationRoll) return true;
    if (m_fov != m_previousFov) return true;
    if (m_flags != m_previousFlags) return true;
    return false;
}

void Camera::markUnchanged() {
    m_previousFrameNumber = m_frameNumber;
    m_previousTimestamp = m_timestamp;
    m_previousPositionX = m_positionX;
    m_previousPositionY = m_positionY;
    m_previousPositionZ = m_positionZ;
    m_previousRotationPitch = m_rotationPitch;
    m_previousRotationYaw = m_rotationYaw;
    m_previousRotationRoll = m_rotationRoll;
    m_previousFov = m_fov;
    m_previousFlags = m_flags;
}

void Camera::updateFrom(const Camera& other) {
    m_frameNumber = other.m_frameNumber;
    m_timestamp = other.m_timestamp;
    m_positionX = other.m_positionX;
    m_positionY = other.m_positionY;
    m_positionZ = other.m_positionZ;
    m_rotationPitch = other.m_rotationPitch;
    m_rotationYaw = other.m_rotationYaw;
    m_rotationRoll = other.m_rotationRoll;
    m_fov = other.m_fov;
    m_nearPlane = other.m_nearPlane;
    m_farPlane = other.m_farPlane;
    std::memcpy(m_viewMatrix.data(), other.m_viewMatrix.data(), 16 * sizeof(float));
    std::memcpy(m_projectionMatrix.data(), other.m_projectionMatrix.data(), 16 * sizeof(float));
    m_viewportX = other.m_viewportX;
    m_viewportY = other.m_viewportY;
    m_viewportWidth = other.m_viewportWidth;
    m_viewportHeight = other.m_viewportHeight;
    m_flags = other.m_flags;
}

void Camera::updateViewMatrix() {
    // Convert Euler angles to view matrix
    // This is a simplified implementation
    float pitch = m_rotationPitch * 3.1415926535f / 180.0f;
    float yaw = m_rotationYaw * 3.1415926535f / 180.0f;
    float roll = m_rotationRoll * 3.1415926535f / 180.0f;
    
    float cosPitch = std::cos(pitch);
    float sinPitch = std::sin(pitch);
    float cosYaw = std::cos(yaw);
    float sinYaw = std::sin(yaw);
    float cosRoll = std::cos(roll);
    float sinRoll = std::sin(roll);
    
    // Create rotation matrix
    float m00 = cosYaw * cosPitch;
    float m01 = cosYaw * sinPitch * sinRoll - sinYaw * cosRoll;
    float m02 = cosYaw * sinPitch * cosRoll + sinYaw * sinRoll;
    float m10 = sinYaw * cosPitch;
    float m11 = sinYaw * sinPitch * sinRoll + cosYaw * cosRoll;
    float m12 = sinYaw * sinPitch * cosRoll - cosYaw * sinRoll;
    float m20 = -sinPitch;
    float m21 = cosPitch * sinRoll;
    float m22 = cosPitch * cosRoll;
    
    // Create view matrix (inverse of rotation * translation)
    m_viewMatrix[0] = m00;
    m_viewMatrix[1] = m10;
    m_viewMatrix[2] = m20;
    m_viewMatrix[3] = 0.0f;
    
    m_viewMatrix[4] = m01;
    m_viewMatrix[5] = m11;
    m_viewMatrix[6] = m21;
    m_viewMatrix[7] = 0.0f;
    
    m_viewMatrix[8] = m02;
    m_viewMatrix[9] = m12;
    m_viewMatrix[10] = m22;
    m_viewMatrix[11] = 0.0f;
    
    m_viewMatrix[12] = -(m00 * m_positionX + m01 * m_positionY + m02 * m_positionZ);
    m_viewMatrix[13] = -(m10 * m_positionX + m11 * m_positionY + m12 * m_positionZ);
    m_viewMatrix[14] = -(m20 * m_positionX + m21 * m_positionY + m22 * m_positionZ);
    m_viewMatrix[15] = 1.0f;
}

void Camera::updateProjectionMatrix() {
    // Create perspective projection matrix
    float aspect = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);
    float fovRad = m_fov * 3.1415926535f / 180.0f;
    float f = 1.0f / std::tan(fovRad / 2.0f);
    
    m_projectionMatrix[0] = f / aspect;
    m_projectionMatrix[1] = 0.0f;
    m_projectionMatrix[2] = 0.0f;
    m_projectionMatrix[3] = 0.0f;
    
    m_projectionMatrix[4] = 0.0f;
    m_projectionMatrix[5] = f;
    m_projectionMatrix[6] = 0.0f;
    m_projectionMatrix[7] = 0.0f;
    
    m_projectionMatrix[8] = 0.0f;
    m_projectionMatrix[9] = 0.0f;
    m_projectionMatrix[10] = (m_farPlane + m_nearPlane) / (m_nearPlane - m_farPlane);
    m_projectionMatrix[11] = -1.0f;
    
    m_projectionMatrix[12] = 0.0f;
    m_projectionMatrix[13] = 0.0f;
    m_projectionMatrix[14] = (2.0f * m_farPlane * m_nearPlane) / (m_nearPlane - m_farPlane);
    m_projectionMatrix[15] = 0.0f;
}

} // namespace renderbridge
