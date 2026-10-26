package renderbridge.renderer.gl;

import org.lwjgl.opengl.GL11;
import org.lwjgl.opengl.GL12;
import org.lwjgl.util.glu.GLU;

import renderbridge.protocol.messages.CameraMessage;
import renderbridge.protocol.messages.ChunkMessage;
import renderbridge.protocol.messages.EntityMessage;
import renderbridge.protocol.messages.ParticleMessage;
import renderbridge.protocol.messages.WeatherMessage;
import renderbridge.renderer.bridge.RendererBridgeClient;

import java.nio.FloatBuffer;
import java.util.List;

/**
 * OpenGL renderer for the external renderer.
 * Renders the scene using OpenGL 2.1 (compatible with LWJGL 2).
 */
public class GLRenderer {

    // Renderer settings
    private int width;
    private int height;
    private float fov = 70.0f;
    private float nearPlane = 0.1f;
    private float farPlane = 1000.0f;
    
    // Bridge client
    private RendererBridgeClient bridgeClient;
    
    // State
    private boolean initialized = false;
    
    // Shaders (for OpenGL 2.1, we use fixed-function pipeline)
    // In a real implementation, we would use GLSL shaders
    
    // Matrices
    private final float[] viewMatrix = new float[16];
    private final float[] projectionMatrix = new float[16];
    
    // Timing
    private long lastTime;
    private float rotation = 0;

    /**
     * Create a new GL renderer.
     *
     * @param width the viewport width
     * @param height the viewport height
     */
    public GLRenderer(int width, int height) {
        this.width = width;
        this.height = height;
        this.lastTime = System.nanoTime();
    }

    /**
     * Initialize the renderer.
     */
    public void initialize() {
        if (initialized) {
            return;
        }
        
        try {
            // Enable features
            GL11.glEnable(GL11.GL_DEPTH_TEST);
            GL11.glEnable(GL11.GL_CULL_FACE);
            GL11.glCullFace(GL11.GL_BACK);
            GL11.glEnable(GL11.GL_BLEND);
            GL11.glBlendFunc(GL11.GL_SRC_ALPHA, GL11.GL_ONE_MINUS_SRC_ALPHA);
            
            // Set clear color
            GL11.glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            
            // Set viewport
            GL11.glViewport(0, 0, width, height);
            
            // Set up projection matrix
            updateProjectionMatrix();
            
            initialized = true;
            System.out.println("GLRenderer initialized");
        } catch (Exception e) {
            System.err.println("Failed to initialize GLRenderer: " + e.getMessage());
        }
    }

    /**
     * Update the renderer.
     */
    public void update() {
        if (!initialized) {
            initialize();
        }
        
        // Update animation
        long currentTime = System.nanoTime();
        long elapsed = currentTime - lastTime;
        lastTime = currentTime;
        
        rotation += (elapsed / 1_000_000_000.0f) * 360.0f; // 1 rotation per second
        if (rotation > 360) {
            rotation -= 360;
        }
    }

    /**
     * Render the scene.
     */
    public void render() {
        if (!initialized) {
            initialize();
        }
        
        // Clear buffers
        GL11.glClear(GL11.GL_COLOR_BUFFER_BIT | GL11.GL_DEPTH_BUFFER_BIT);
        
        // Set up view matrix from camera
        updateViewMatrix();
        
        // Render scene
        renderScene();
    }

    /**
     * Render the scene.
     */
    private void renderScene() {
        if (bridgeClient == null || !bridgeClient.isConnected()) {
            // Render fallback scene
            renderFallbackScene();
            return;
        }
        
        // Get current camera
        CameraMessage camera = bridgeClient.getCurrentCamera();
        if (camera == null) {
            renderFallbackScene();
            return;
        }
        
        // Apply camera matrices
        applyCameraMatrices(camera);
        
        // Render chunks
        renderChunks();
        
        // Render entities
        renderEntities();
        
        // Render particles
        renderParticles();
    }

    /**
     * Render a fallback scene when no camera data is available.
     */
    private void renderFallbackScene() {
        // Draw a simple rotating cube
        GL11.glMatrixMode(GL11.GL_MODELVIEW);
        GL11.glLoadIdentity();
        
        // Position camera
        GLU.gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);
        
        // Rotate the cube
        GL11.glRotatef(rotation, 0, 1, 0);
        
        // Draw cube
        drawCube();
        
        // Draw status text
        drawStatusText();
    }

    /**
     * Render chunks.
     */
    private void renderChunks() {
        List<ChunkMessage> chunks = bridgeClient.getCurrentChunks();
        
        for (ChunkMessage chunk : chunks) {
            renderChunk(chunk);
        }
    }

    /**
     * Render a single chunk.
     *
     * @param chunk the chunk to render
     */
    private void renderChunk(ChunkMessage chunk) {
        // For now, just draw a simple cube at the chunk position
        GL11.glPushMatrix();
        
        // Position the chunk
        GL11.glTranslatef(chunk.getChunkX() * 16, 0, chunk.getChunkZ() * 16);
        
        // Draw a simple representation of the chunk
        drawChunkRepresentation();
        
        GL11.glPopMatrix();
    }

    /**
     * Draw a simple representation of a chunk.
     */
    private void drawChunkRepresentation() {
        // Draw a simple grid of blocks
        GL11.glColor3f(0.5f, 0.5f, 0.5f);
        
        for (int x = 0; x < 16; x += 2) {
            for (int z = 0; z < 16; z += 2) {
                GL11.glPushMatrix();
                GL11.glTranslatef(x - 8, 0, z - 8);
                drawCube();
                GL11.glPopMatrix();
            }
        }
    }

    /**
     * Render entities.
     */
    private void renderEntities() {
        List<EntityMessage> entities = bridgeClient.getCurrentEntities();
        
        for (EntityMessage entity : entities) {
            renderEntity(entity);
        }
    }

    /**
     * Render a single entity.
     *
     * @param entity the entity to render
     */
    private void renderEntity(EntityMessage entity) {
        GL11.glPushMatrix();
        
        // Position and rotate the entity
        GL11.glTranslatef(entity.getPositionX(), entity.getPositionY(), entity.getPositionZ());
        GL11.glRotatef(entity.getRotationYaw() * (180.0f / (float) Math.PI), 0, 1, 0);
        
        // Draw a simple representation of the entity
        drawEntityRepresentation(entity);
        
        GL11.glPopMatrix();
    }

    /**
     * Draw a simple representation of an entity.
     *
     * @param entity the entity
     */
    private void drawEntityRepresentation(EntityMessage entity) {
        // Different colors for different entity types
        switch (entity.getEntityType()) {
            case 0: // Player
                GL11.glColor3f(0.0f, 0.0f, 1.0f); // Blue
                break;
            case 1: // Zombie
                GL11.glColor3f(0.0f, 1.0f, 0.0f); // Green
                break;
            default:
                GL11.glColor3f(1.0f, 0.0f, 0.0f); // Red
                break;
        }
        
        // Draw a simple cube for the entity
        drawCube();
    }

    /**
     * Render particles.
     */
    private void renderParticles() {
        List<ParticleMessage> particles = bridgeClient.getCurrentParticles();
        
        for (ParticleMessage particleSystem : particles) {
            renderParticleSystem(particleSystem);
        }
    }

    /**
     * Render a particle system.
     *
     * @param particleSystem the particle system to render
     */
    private void renderParticleSystem(ParticleMessage particleSystem) {
        GL11.glDisable(GL11.GL_CULL_FACE);
        GL11.glEnable(GL11.GL_BLEND);
        GL11.glBlendFunc(GL11.GL_SRC_ALPHA, GL11.GL_ONE);
        
        for (int i = 0; i < particleSystem.getParticleCount(); i++) {
            float x = particleSystem.getPositionX(i);
            float y = particleSystem.getPositionY(i);
            float z = particleSystem.getPositionZ(i);
            float scale = particleSystem.getScale(i);
            
            GL11.glPushMatrix();
            GL11.glTranslatef(x, y, z);
            GL11.glScalef(scale, scale, scale);
            
            // Draw a simple particle (quad)
            drawParticle();
            
            GL11.glPopMatrix();
        }
        
        GL11.glEnable(GL11.GL_CULL_FACE);
        GL11.glBlendFunc(GL11.GL_SRC_ALPHA, GL11.GL_ONE_MINUS_SRC_ALPHA);
    }

    /**
     * Draw a simple particle (textured quad).
     */
    private void drawParticle() {
        GL11.glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
        GL11.glBegin(GL11.GL_QUADS);
        GL11.glVertex3f(-0.5f, -0.5f, 0);
        GL11.glVertex3f(0.5f, -0.5f, 0);
        GL11.glVertex3f(0.5f, 0.5f, 0);
        GL11.glVertex3f(-0.5f, 0.5f, 0);
        GL11.glEnd();
    }

    /**
     * Apply camera matrices for rendering.
     *
     * @param camera the camera message
     */
    private void applyCameraMatrices(CameraMessage camera) {
        // Set up projection matrix
        GL11.glMatrixMode(GL11.GL_PROJECTION);
        GL11.glLoadIdentity();
        
        // Use the camera's projection matrix
        // For now, we'll use a simple perspective projection
        GLU.gluPerspective(camera.getFov(), (float) width / height, camera.getNearPlane(), camera.getFarPlane());
        
        // Set up view matrix
        GL11.glMatrixMode(GL11.GL_MODELVIEW);
        GL11.glLoadIdentity();
        
        // Apply camera position and rotation
        GL11.glTranslatef(-camera.getPositionX(), -camera.getPositionY(), -camera.getPositionZ());
        GL11.glRotatef(-camera.getRotationPitch() * (180.0f / (float) Math.PI), 1, 0, 0);
        GL11.glRotatef(-camera.getRotationYaw() * (180.0f / (float) Math.PI), 0, 1, 0);
    }

    /**
     * Update the projection matrix.
     */
    private void updateProjectionMatrix() {
        float aspect = (float) width / height;
        float fovRadians = (float) Math.toRadians(fov);
        
        // Create perspective projection matrix
        float f = 1.0f / (float) Math.tan(fovRadians / 2.0f);
        
        for (int i = 0; i < 16; i++) {
            projectionMatrix[i] = 0;
        }
        
        projectionMatrix[0] = f / aspect;
        projectionMatrix[5] = f;
        projectionMatrix[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
        projectionMatrix[11] = -1;
        projectionMatrix[14] = (2 * farPlane * nearPlane) / (nearPlane - farPlane);
        projectionMatrix[15] = 0;
    }

    /**
     * Update the view matrix from the current camera.
     */
    private void updateViewMatrix() {
        // For now, use a simple view matrix
        for (int i = 0; i < 16; i++) {
            viewMatrix[i] = 0;
        }
        
        viewMatrix[0] = 1;
        viewMatrix[5] = 1;
        viewMatrix[10] = 1;
        viewMatrix[15] = 1;
    }

    /**
     * Draw a simple cube.
     */
    private void drawCube() {
        GL11.glBegin(GL11.GL_QUADS);
        
        // Front face
        GL11.glVertex3f(-0.5f, -0.5f, -0.5f);
        GL11.glVertex3f(0.5f, -0.5f, -0.5f);
        GL11.glVertex3f(0.5f, 0.5f, -0.5f);
        GL11.glVertex3f(-0.5f, 0.5f, -0.5f);
        
        // Back face
        GL11.glVertex3f(-0.5f, -0.5f, 0.5f);
        GL11.glVertex3f(-0.5f, 0.5f, 0.5f);
        GL11.glVertex3f(0.5f, 0.5f, 0.5f);
        GL11.glVertex3f(0.5f, -0.5f, 0.5f);
        
        // Top face
        GL11.glVertex3f(-0.5f, 0.5f, -0.5f);
        GL11.glVertex3f(0.5f, 0.5f, -0.5f);
        GL11.glVertex3f(0.5f, 0.5f, 0.5f);
        GL11.glVertex3f(-0.5f, 0.5f, 0.5f);
        
        // Bottom face
        GL11.glVertex3f(-0.5f, -0.5f, -0.5f);
        GL11.glVertex3f(-0.5f, -0.5f, 0.5f);
        GL11.glVertex3f(0.5f, -0.5f, 0.5f);
        GL11.glVertex3f(0.5f, -0.5f, -0.5f);
        
        // Left face
        GL11.glVertex3f(-0.5f, -0.5f, -0.5f);
        GL11.glVertex3f(-0.5f, -0.5f, 0.5f);
        GL11.glVertex3f(-0.5f, 0.5f, 0.5f);
        GL11.glVertex3f(-0.5f, 0.5f, -0.5f);
        
        // Right face
        GL11.glVertex3f(0.5f, -0.5f, -0.5f);
        GL11.glVertex3f(0.5f, 0.5f, -0.5f);
        GL11.glVertex3f(0.5f, 0.5f, 0.5f);
        GL11.glVertex3f(0.5f, -0.5f, 0.5f);
        
        GL11.glEnd();
    }

    /**
     * Draw status text.
     */
    private void drawStatusText() {
        if (bridgeClient == null) {
            return;
        }
        
        // For now, just print to console
        // In a real implementation, we would render text using a font renderer
        if (!bridgeClient.isConnected()) {
            System.out.println("Waiting for connection...");
        }
    }

    /**
     * Resize the renderer.
     *
     * @param width the new width
     * @param height the new height
     */
    public void resize(int width, int height) {
        this.width = width;
        this.height = height;
        
        if (initialized) {
            GL11.glViewport(0, 0, width, height);
            updateProjectionMatrix();
        }
    }

    /**
     * Cleanup the renderer.
     */
    public void cleanup() {
        initialized = false;
    }

    /**
     * Set the bridge client.
     *
     * @param bridgeClient the bridge client
     */
    public void setBridgeClient(RendererBridgeClient bridgeClient) {
        this.bridgeClient = bridgeClient;
    }

    /**
     * Get the bridge client.
     *
     * @return the bridge client
     */
    public RendererBridgeClient getBridgeClient() {
        return bridgeClient;
    }

    /**
     * Get the width.
     *
     * @return the width
     */
    public int getWidth() {
        return width;
    }

    /**
     * Get the height.
     *
     * @return the height
     */
    public int getHeight() {
        return height;
    }

    /**
     * Set the FOV.
     *
     * @param fov the FOV in degrees
     */
    public void setFov(float fov) {
        this.fov = fov;
        updateProjectionMatrix();
    }

    /**
     * Get the FOV.
     *
     * @return the FOV in degrees
     */
    public float getFov() {
        return fov;
    }

    @Override
    public String toString() {
        return String.format(
                "GLRenderer{size=%dx%d, fov=%.1f, initialized=%s}",
                width, height, fov, initialized
        );
    }
}
