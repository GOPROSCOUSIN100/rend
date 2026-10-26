package renderbridge.renderer.scene;

import org.lwjgl.util.vector.Matrix4f;
import org.lwjgl.util.vector.Vector3f;

/**
 * Represents a camera in 3D space.
 * Contains position, rotation, and projection information.
 */
public class Camera {

    // Position
    private Vector3f position;
    
    // Rotation (in radians)
    private float pitch;
    private float yaw;
    private float roll;
    
    // Matrices
    private Matrix4f viewMatrix;
    private Matrix4f projectionMatrix;
    
    // Settings
    private float fov;
    private float nearPlane;
    private float farPlane;
    
    // Viewport
    private int viewportX;
    private int viewportY;
    private int viewportWidth;
    private int viewportHeight;
    
    // Flags
    private int flags;

    /**
     * Create a new camera at the origin.
     */
    public Camera() {
        this.position = new Vector3f(0, 0, 0);
        this.viewMatrix = new Matrix4f();
        this.projectionMatrix = new Matrix4f();
        this.fov = 70.0f;
        this.nearPlane = 0.1f;
        this.farPlane = 1000.0f;
    }

    /**
     * Create a new camera with the specified position.
     *
     * @param x the X position
     * @param y the Y position
     * @param z the Z position
     */
    public Camera(float x, float y, float z) {
        this();
        this.position = new Vector3f(x, y, z);
    }

    /**
     * Update the camera matrices.
     */
    public void updateMatrices() {
        // Update view matrix
        updateViewMatrix();
        
        // Update projection matrix
        updateProjectionMatrix();
    }

    /**
     * Update the view matrix based on position and rotation.
     */
    private void updateViewMatrix() {
        viewMatrix.setIdentity();
        
        // Apply rotation
        viewMatrix.rotate(roll, 0, 0, 1);
        viewMatrix.rotate(yaw, 0, 1, 0);
        viewMatrix.rotate(pitch, 1, 0, 0);
        
        // Apply translation
        viewMatrix.translate(-position.x, -position.y, -position.z);
    }

    /**
     * Update the projection matrix.
     */
    private void updateProjectionMatrix() {
        float aspect = (float) viewportWidth / viewportHeight;
        float fovRadians = (float) Math.toRadians(fov);
        
        projectionMatrix.setIdentity();
        
        float f = 1.0f / (float) Math.tan(fovRadians / 2.0f);
        
        projectionMatrix.m00 = f / aspect;
        projectionMatrix.m11 = f;
        projectionMatrix.m22 = (farPlane + nearPlane) / (nearPlane - farPlane);
        projectionMatrix.m23 = -1;
        projectionMatrix.m32 = (2 * farPlane * nearPlane) / (nearPlane - farPlane);
        projectionMatrix.m33 = 0;
    }

    // Getters and setters

    public Vector3f getPosition() {
        return position;
    }

    public void setPosition(Vector3f position) {
        this.position = position;
    }

    public void setPosition(float x, float y, float z) {
        this.position = new Vector3f(x, y, z);
    }

    public float getPitch() {
        return pitch;
    }

    public void setPitch(float pitch) {
        this.pitch = pitch;
    }

    public float getYaw() {
        return yaw;
    }

    public void setYaw(float yaw) {
        this.yaw = yaw;
    }

    public float getRoll() {
        return roll;
    }

    public void setRoll(float roll) {
        this.roll = roll;
    }

    public void setRotation(float pitch, float yaw, float roll) {
        this.pitch = pitch;
        this.yaw = yaw;
        this.roll = roll;
    }

    public Matrix4f getViewMatrix() {
        return viewMatrix;
    }

    public void setViewMatrix(Matrix4f viewMatrix) {
        this.viewMatrix = viewMatrix;
    }

    public Matrix4f getProjectionMatrix() {
        return projectionMatrix;
    }

    public void setProjectionMatrix(Matrix4f projectionMatrix) {
        this.projectionMatrix = projectionMatrix;
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

    @Override
    public String toString() {
        return String.format(
                "Camera{position=(%.2f, %.2f, %.2f), rotation=(%.2f, %.2f, %.2f), fov=%.1f, viewport=(%d, %d, %d, %d)}",
                position.x, position.y, position.z,
                pitch, yaw, roll,
                fov,
                viewportX, viewportY, viewportWidth, viewportHeight
        );
    }
}
