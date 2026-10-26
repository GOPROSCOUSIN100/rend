#pragma once

#include <cstdint>
#include <array>

namespace renderbridge {

/**
 * @class Camera
 * @brief Represents a camera in the 3D scene
 * 
 * This class stores camera data extracted from Minecraft and
 * provides methods for working with the camera in the renderer.
 */
class Camera {
public:
    // Camera flags
    enum Flag : uint32_t {
        NONE = 0x00000000,
        THIRD_PERSON = 0x00000001,
        INVERSE_VIEW = 0x00000002,
        UNDERWATER = 0x00000004,
        IN_PORTAL = 0x00000008
    };

public:
    /**
     * @brief Constructor
     */
    Camera();
    
    /**
     * @brief Destructor
     */
    ~Camera();
    
    /**
     * @brief Parse camera data from a message payload
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     * @return True if parsing succeeded
     */
    bool parseFromPayload(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Serialize camera data to a buffer
     * @param buffer Output buffer
     * @param bufferSize Size of the buffer
     * @return Number of bytes written
     */
    uint32_t serializeToBuffer(uint8_t* buffer, uint32_t bufferSize) const;
    
    /**
     * @brief Get the frame number
     * @return Frame number
     */
    int64_t getFrameNumber() const;
    
    /**
     * @brief Set the frame number
     * @param frameNumber Frame number
     */
    void setFrameNumber(int64_t frameNumber);
    
    /**
     * @brief Get the timestamp
     * @return Timestamp
     */
    int64_t getTimestamp() const;
    
    /**
     * @brief Set the timestamp
     * @param timestamp Timestamp
     */
    void setTimestamp(int64_t timestamp);
    
    // Position accessors
    float getPositionX() const;
    float getPositionY() const;
    float getPositionZ() const;
    void setPosition(float x, float y, float z);
    void setPositionX(float x);
    void setPositionY(float y);
    void setPositionZ(float z);
    
    // Rotation accessors
    float getRotationPitch() const;
    float getRotationYaw() const;
    float getRotationRoll() const;
    void setRotation(float pitch, float yaw, float roll);
    void setRotationPitch(float pitch);
    void setRotationYaw(float yaw);
    void setRotationRoll(float roll);
    
    // Camera settings accessors
    float getFov() const;
    float getNearPlane() const;
    float getFarPlane() const;
    void setFov(float fov);
    void setNearPlane(float nearPlane);
    void setFarPlane(float farPlane);
    
    // Viewport accessors
    int32_t getViewportX() const;
    int32_t getViewportY() const;
    int32_t getViewportWidth() const;
    int32_t getViewportHeight() const;
    void setViewport(int32_t x, int32_t y, int32_t width, int32_t height);
    
    // Matrix accessors
    const float* getViewMatrix() const;
    const float* getProjectionMatrix() const;
    void setViewMatrix(const float* matrix);
    void setProjectionMatrix(const float* matrix);
    
    // Flags accessors
    uint32_t getFlags() const;
    void setFlags(uint32_t flags);
    void addFlag(Flag flag);
    void removeFlag(Flag flag);
    bool hasFlag(Flag flag) const;
    
    // Convenience methods
    bool isThirdPerson() const;
    bool isInverseView() const;
    bool isUnderwater() const;
    bool isInPortal() const;
    
    /**
     * @brief Calculate the view-projection matrix
     * @param output Output matrix (16 floats)
     */
    void calculateViewProjectionMatrix(float* output) const;
    
    /**
     * @brief Calculate the inverse view matrix
     * @param output Output matrix (16 floats)
     */
    void calculateInverseViewMatrix(float* output) const;
    
    /**
     * @brief Calculate the camera direction vector
     * @param output Output vector (3 floats)
     */
    void calculateDirectionVector(float* output) const;
    
    /**
     * @brief Calculate the camera up vector
     * @param output Output vector (3 floats)
     */
    void calculateUpVector(float* output) const;
    
    /**
     * @brief Calculate the camera right vector
     * @param output Output vector (3 floats)
     */
    void calculateRightVector(float* output) const;
    
    /**
     * @brief Check if the camera has changed since the last update
     * @return True if the camera has changed
     */
    bool hasChanged() const;
    
    /**
     * @brief Mark the camera as unchanged
     */
    void markUnchanged();
    
    /**
     * @brief Update the camera from another camera
     * @param other Other camera
     */
    void updateFrom(const Camera& other);

private:
    int64_t m_frameNumber;
    int64_t m_timestamp;
    
    // Position
    float m_positionX;
    float m_positionY;
    float m_positionZ;
    
    // Rotation (in degrees)
    float m_rotationPitch;
    float m_rotationYaw;
    float m_rotationRoll;
    
    // Camera settings
    float m_fov;
    float m_nearPlane;
    float m_farPlane;
    
    // Matrices (4x4, column-major)
    std::array<float, 16> m_viewMatrix;
    std::array<float, 16> m_projectionMatrix;
    
    // Viewport
    int32_t m_viewportX;
    int32_t m_viewportY;
    int32_t m_viewportWidth;
    int32_t m_viewportHeight;
    
    // Flags
    uint32_t m_flags;
    
    // Dirty flag for change detection
    mutable bool m_dirty;
    
    // Previous values for change detection
    mutable int64_t m_previousFrameNumber;
    mutable int64_t m_previousTimestamp;
    mutable float m_previousPositionX;
    mutable float m_previousPositionY;
    mutable float m_previousPositionZ;
    mutable float m_previousRotationPitch;
    mutable float m_previousRotationYaw;
    mutable float m_previousRotationRoll;
    mutable float m_previousFov;
    mutable uint32_t m_previousFlags;
    
    /**
     * @brief Initialize the view matrix from position and rotation
     */
    void updateViewMatrix();
    
    /**
     * @brief Initialize the projection matrix from FOV and planes
     */
    void updateProjectionMatrix();
};

} // namespace renderbridge
