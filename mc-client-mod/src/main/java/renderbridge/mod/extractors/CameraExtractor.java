package renderbridge.mod.extractors;

import net.minecraft.client.renderer.EntityRenderer;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3d;

import org.lwjgl.opengl.GL11;
import org.lwjgl.util.glu.GLU;

import java.nio.FloatBuffer;

/**
 * Extracts camera data from the EntityRenderer.
 * Includes position, rotation, FOV, and projection/view matrices.
 */
public class CameraExtractor {

    private final EntityRenderer entityRenderer;
    
    // Cached values
    private float lastPartialTicks = 0;
    private long lastFrameNumber = 0;
    
    // Temporary buffers for matrix extraction
    private final float[] viewMatrix = new float[16];
    private final float[] projectionMatrix = new float[16];

    /**
     * Create a new camera extractor.
     *
     * @param entityRenderer the EntityRenderer instance
     */
    public CameraExtractor(EntityRenderer entityRenderer) {
        this.entityRenderer = entityRenderer;
    }

    /**
     * Update the extractor with the current partial ticks.
     *
     * @param partialTicks the current partial ticks
     */
    public void update(float partialTicks) {
        this.lastPartialTicks = partialTicks;
    }

    /**
     * Extract camera data and create a CameraMessage.
     *
     * @return a CameraMessage with the current camera data, or null if extraction fails
     */
    public renderbridge.protocol.messages.CameraMessage extract() {
        try {
            renderbridge.protocol.messages.CameraMessage message = 
                    new renderbridge.protocol.messages.CameraMessage();
            
            message.setFrameNumber(++lastFrameNumber);
            message.setTimestamp(System.nanoTime());
            
            // Extract camera position and rotation
            extractPositionAndRotation(message);
            
            // Extract camera settings
            extractCameraSettings(message);
            
            // Extract matrices
            extractMatrices(message);
            
            // Extract viewport
            extractViewport(message);
            
            return message;
        } catch (Exception e) {
            System.err.println("Failed to extract camera data: " + e.getMessage());
            return null;
        }
    }

    /**
     * Extract camera position and rotation.
     *
     * @param message the message to populate
     */
    private void extractPositionAndRotation(renderbridge.protocol.messages.CameraMessage message) {
        if (entityRenderer == null) {
            return;
        }
        
        try {
            // Get camera position (from entityRenderer.getCameraPosition() in newer versions)
            // For 1.8.9, we need to use reflection or access the fields directly
            
            // In Minecraft 1.8.9, camera position can be obtained from:
            // - entityRenderer.getRenderViewEntity() for the view entity
            // - The actual camera position is stored in various fields
            
            // For now, use the player position as a fallback
            if (entityRenderer.mc != null && entityRenderer.mc.thePlayer != null) {
                Vec3d pos = entityRenderer.mc.thePlayer.getPositionVector();
                message.setPosition((float) pos.xCoord, (float) pos.yCoord, (float) pos.zCoord);
            }
            
            // Get camera rotation
            if (entityRenderer.mc != null && entityRenderer.mc.thePlayer != null) {
                float pitch = entityRenderer.mc.thePlayer.rotationPitch;
                float yaw = entityRenderer.mc.thePlayer.rotationYaw;
                message.setRotation(-pitch * 0.017453292F, yaw * 0.017453292F, 0);
            }
            
            // Set flags based on camera state
            if (entityRenderer != null) {
                // Check for third person view
                if (entityRenderer.gameSettings.thirdPersonView > 0) {
                    message.addFlag(renderbridge.protocol.messages.CameraMessage.FLAG_THIRD_PERSON);
                }
            }
        } catch (Exception e) {
            System.err.println("Failed to extract position/rotation: " + e.getMessage());
        }
    }

    /**
     * Extract camera settings (FOV, near/far planes).
     *
     * @param message the message to populate
     */
    private void extractCameraSettings(renderbridge.protocol.messages.CameraMessage message) {
        if (entityRenderer == null) {
            return;
        }
        
        try {
            // Get FOV
            // In 1.8.9, FOV is calculated in EntityRenderer.updateCameraAndRender
            if (entityRenderer.mc != null && entityRenderer.mc.gameSettings != null) {
                float fov = entityRenderer.mc.gameSettings.fovSetting;
                message.setFov(fov);
            }
            
            // Set near and far planes (default Minecraft values)
            message.setNearPlane(0.05f);
            message.setFarPlane(256.0f);
        } catch (Exception e) {
            System.err.println("Failed to extract camera settings: " + e.getMessage());
        }
    }

    /**
     * Extract view and projection matrices.
     * Uses OpenGL state to get the current matrices.
     *
     * @param message the message to populate
     */
    private void extractMatrices(renderbridge.protocol.messages.CameraMessage message) {
        try {
            // Get the current modelview matrix (this is the view matrix in OpenGL)
            FloatBuffer modelviewBuffer = FloatBuffer.allocate(16);
            GL11.glGetFloat(GL11.GL_MODELVIEW_MATRIX, modelviewBuffer);
            modelviewBuffer.rewind();
            modelviewBuffer.get(viewMatrix);
            
            // Get the current projection matrix
            FloatBuffer projectionBuffer = FloatBuffer.allocate(16);
            GL11.glGetFloat(GL11.GL_PROJECTION_MATRIX, projectionBuffer);
            projectionBuffer.rewind();
            projectionBuffer.get(projectionMatrix);
            
            // Convert from OpenGL column-major to our storage format
            // OpenGL matrices are column-major, which is what we want
            message.setViewMatrix(viewMatrix);
            message.setProjectionMatrix(projectionMatrix);
        } catch (Exception e) {
            System.err.println("Failed to extract matrices: " + e.getMessage());
            // Set identity matrices as fallback
            float[] identity = new float[16];
            for (int i = 0; i < 16; i++) {
                identity[i] = i % 5 == 0 ? 1.0f : 0.0f;
            }
            message.setViewMatrix(identity);
            message.setProjectionMatrix(identity);
        }
    }

    /**
     * Extract viewport information.
     *
     * @param message the message to populate
     */
    private void extractViewport(renderbridge.protocol.messages.CameraMessage message) {
        try {
            if (entityRenderer != null && entityRenderer.mc != null) {
                int width = entityRenderer.mc.displayWidth;
                int height = entityRenderer.mc.displayHeight;
                message.setViewport(0, 0, width, height);
            }
        } catch (Exception e) {
            System.err.println("Failed to extract viewport: " + e.getMessage());
            message.setViewport(0, 0, 800, 600); // Default viewport
        }
    }

    /**
     * Get the current camera position as a Vec3d.
     *
     * @return the camera position, or null if not available
     */
    public Vec3d getCameraPosition() {
        if (entityRenderer == null || entityRenderer.mc == null || entityRenderer.mc.thePlayer == null) {
            return null;
        }
        return entityRenderer.mc.thePlayer.getPositionVector();
    }

    /**
     * Get the current camera rotation.
     *
     * @return the camera rotation as [pitch, yaw, roll] in radians, or null if not available
     */
    public float[] getCameraRotation() {
        if (entityRenderer == null || entityRenderer.mc == null || entityRenderer.mc.thePlayer == null) {
            return null;
        }
        
        float pitch = entityRenderer.mc.thePlayer.rotationPitch * 0.017453292F;
        float yaw = entityRenderer.mc.thePlayer.rotationYaw * 0.017453292F;
        return new float[] { -pitch, yaw, 0 };
    }

    /**
     * Get the current FOV.
     *
     * @return the current FOV in degrees, or 70 if not available
     */
    public float getFov() {
        if (entityRenderer == null || entityRenderer.mc == null || entityRenderer.mc.gameSettings == null) {
            return 70.0f;
        }
        return entityRenderer.mc.gameSettings.fovSetting;
    }

    /**
     * Get the last frame number.
     *
     * @return the last frame number
     */
    public long getLastFrameNumber() {
        return lastFrameNumber;
    }

    @Override
    public String toString() {
        return "CameraExtractor{entityRenderer=" + (entityRenderer != null) + "}";
    }
}
