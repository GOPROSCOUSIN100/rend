package renderbridge.protocol.messages;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageType;

import java.nio.ByteBuffer;

/**
 * Message containing camera data for rendering.
 * Includes position, rotation, FOV, and projection matrices.
 */
public class CameraMessage extends Message {

    // Camera flags
    public static final int FLAG_THIRD_PERSON = 0x00000001;
    public static final int FLAG_INVERSE_VIEW = 0x00000002;
    public static final int FLAG_UNDERWATER = 0x00000004;
    public static final int FLAG_IN_PORTAL = 0x00000008;

    private long frameNumber;
    private long timestamp;
    
    // Position and rotation
    private float positionX;
    private float positionY;
    private float positionZ;
    private float rotationPitch;
    private float rotationYaw;
    private float rotationRoll;
    
    // Camera settings
    private float fov;
    private float nearPlane;
    private float farPlane;
    
    // Matrices (4x4, column-major)
    private final float[] viewMatrix = new float[16];
    private final float[] projectionMatrix = new float[16];
    
    // Viewport
    private int viewportX;
    private int viewportY;
    private int viewportWidth;
    private int viewportHeight;
    
    private int flags;

    /**
     * Create a new camera message.
     */
    public CameraMessage() {
        super(MessageType.CAMERA);
    }

    /**
     * Create a camera message with the specified version.
     *
     * @param version the protocol version
     */
    public CameraMessage(int version) {
        super(MessageType.CAMERA);
    }

    /**
     * Create a camera message from a header and payload buffer.
     *
     * @param version the protocol version
     * @param payload the payload buffer
     */
    public CameraMessage(int version, ByteBuffer payload) {
        super(MessageType.CAMERA);
        readPayload(payload);
    }

    @Override
    protected void readPayload(ByteBuffer buffer) {
        frameNumber = buffer.getLong();
        timestamp = buffer.getLong();
        
        positionX = buffer.getFloat();
        positionY = buffer.getFloat();
        positionZ = buffer.getFloat();
        
        rotationPitch = buffer.getFloat();
        rotationYaw = buffer.getFloat();
        rotationRoll = buffer.getFloat();
        
        fov = buffer.getFloat();
        nearPlane = buffer.getFloat();
        farPlane = buffer.getFloat();
        
        // Read view matrix (16 floats)
        for (int i = 0; i < 16; i++) {
            viewMatrix[i] = buffer.getFloat();
        }
        
        // Read projection matrix (16 floats)
        for (int i = 0; i < 16; i++) {
            projectionMatrix[i] = buffer.getFloat();
        }
        
        viewportX = buffer.getInt();
        viewportY = buffer.getInt();
        viewportWidth = buffer.getInt();
        viewportHeight = buffer.getInt();
        
        flags = buffer.getInt();
    }

    @Override
    protected void writePayload(ByteBuffer buffer) {
        buffer.putLong(frameNumber);
        buffer.putLong(timestamp);
        
        buffer.putFloat(positionX);
        buffer.putFloat(positionY);
        buffer.putFloat(positionZ);
        
        buffer.putFloat(rotationPitch);
        buffer.putFloat(rotationYaw);
        buffer.putFloat(rotationRoll);
        
        buffer.putFloat(fov);
        buffer.putFloat(nearPlane);
        buffer.putFloat(farPlane);
        
        // Write view matrix
        for (float value : viewMatrix) {
            buffer.putFloat(value);
        }
        
        // Write projection matrix
        for (float value : projectionMatrix) {
            buffer.putFloat(value);
        }
        
        buffer.putInt(viewportX);
        buffer.putInt(viewportY);
        buffer.putInt(viewportWidth);
        buffer.putInt(viewportHeight);
        
        buffer.putInt(flags);
    }

    @Override
    public int getPayloadSize() {
        // frameNumber(8) + timestamp(8) +
        // position(12) + rotation(12) +
        // fov(4) + nearPlane(4) + farPlane(4) +
        // viewMatrix(64) + projectionMatrix(64) +
        // viewport(16) + flags(4)
        return 8 + 8 + 12 + 12 + 4 + 4 + 4 + 64 + 64 + 16 + 4;
    }

    // Getters and setters

    public long getFrameNumber() {
        return frameNumber;
    }

    public void setFrameNumber(long frameNumber) {
        this.frameNumber = frameNumber;
    }

    public long getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(long timestamp) {
        this.timestamp = timestamp;
    }

    public float getPositionX() {
        return positionX;
    }

    public void setPositionX(float positionX) {
        this.positionX = positionX;
    }

    public float getPositionY() {
        return positionY;
    }

    public void setPositionY(float positionY) {
        this.positionY = positionY;
    }

    public float getPositionZ() {
        return positionZ;
    }

    public void setPositionZ(float positionZ) {
        this.positionZ = positionZ;
    }

    public void setPosition(float x, float y, float z) {
        this.positionX = x;
        this.positionY = y;
        this.positionZ = z;
    }

    public float getRotationPitch() {
        return rotationPitch;
    }

    public void setRotationPitch(float rotationPitch) {
        this.rotationPitch = rotationPitch;
    }

    public float getRotationYaw() {
        return rotationYaw;
    }

    public void setRotationYaw(float rotationYaw) {
        this.rotationYaw = rotationYaw;
    }

    public float getRotationRoll() {
        return rotationRoll;
    }

    public void setRotationRoll(float rotationRoll) {
        this.rotationRoll = rotationRoll;
    }

    public void setRotation(float pitch, float yaw, float roll) {
        this.rotationPitch = pitch;
        this.rotationYaw = yaw;
        this.rotationRoll = roll;
    }

    public float getFov() {
        return fov;
    }

    public void setFov(float fov) {
        this.fov = fov;
    }

    public float getNearPlane() {
        return nearPlane;
    }

    public void setNearPlane(float nearPlane) {
        this.nearPlane = nearPlane;
    }

    public float getFarPlane() {
        return farPlane;
    }

    public void setFarPlane(float farPlane) {
        this.farPlane = farPlane;
    }

    public float[] getViewMatrix() {
        return viewMatrix;
    }

    public void setViewMatrix(float[] viewMatrix) {
        System.arraycopy(viewMatrix, 0, this.viewMatrix, 0, 16);
    }

    public float[] getProjectionMatrix() {
        return projectionMatrix;
    }

    public void setProjectionMatrix(float[] projectionMatrix) {
        System.arraycopy(projectionMatrix, 0, this.projectionMatrix, 0, 16);
    }

    public int getViewportX() {
        return viewportX;
    }

    public void setViewportX(int viewportX) {
        this.viewportX = viewportX;
    }

    public int getViewportY() {
        return viewportY;
    }

    public void setViewportY(int viewportY) {
        this.viewportY = viewportY;
    }

    public int getViewportWidth() {
        return viewportWidth;
    }

    public void setViewportWidth(int viewportWidth) {
        this.viewportWidth = viewportWidth;
    }

    public int getViewportHeight() {
        return viewportHeight;
    }

    public void setViewportHeight(int viewportHeight) {
        this.viewportHeight = viewportHeight;
    }

    public void setViewport(int x, int y, int width, int height) {
        this.viewportX = x;
        this.viewportY = y;
        this.viewportWidth = width;
        this.viewportHeight = height;
    }

    public int getFlags() {
        return flags;
    }

    public void setFlags(int flags) {
        this.flags = flags;
    }

    public void addFlag(int flag) {
        this.flags |= flag;
    }

    public boolean hasFlag(int flag) {
        return (this.flags & flag) != 0;
    }

    // Convenience methods for checking specific flags

    public boolean isThirdPerson() {
        return hasFlag(FLAG_THIRD_PERSON);
    }

    public boolean isInverseView() {
        return hasFlag(FLAG_INVERSE_VIEW);
    }

    public boolean isUnderwater() {
        return hasFlag(FLAG_UNDERWATER);
    }

    public boolean isInPortal() {
        return hasFlag(FLAG_IN_PORTAL);
    }

    @Override
    public String toString() {
        return String.format(
                "CameraMessage{frameNumber=%d, timestamp=%d, position=(%.2f, %.2f, %.2f), rotation=(%.2f, %.2f, %.2f), fov=%.2f, viewport=(%d, %d, %d, %d), flags=0x%08X}",
                frameNumber, timestamp,
                positionX, positionY, positionZ,
                rotationPitch, rotationYaw, rotationRoll,
                fov,
                viewportX, viewportY, viewportWidth, viewportHeight,
                flags
        );
    }
}
