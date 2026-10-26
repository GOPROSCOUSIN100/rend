package renderbridge.renderer.scene;

import renderbridge.protocol.messages.CameraMessage;
import renderbridge.protocol.messages.ChunkMessage;
import renderbridge.protocol.messages.EntityMessage;
import renderbridge.protocol.messages.ParticleMessage;
import renderbridge.protocol.messages.WeatherMessage;

import java.util.ArrayList;
import java.util.List;

/**
 * Represents the 3D scene to be rendered.
 * Contains all the data needed to render a frame.
 */
public class Scene {

    // Camera
    private CameraMessage camera;
    
    // World
    private final List<ChunkMessage> chunks = new ArrayList<>();
    
    // Entities
    private final List<EntityMessage> entities = new ArrayList<>();
    
    // Particles
    private final List<ParticleMessage> particles = new ArrayList<>();
    
    // Weather
    private WeatherMessage weather;
    
    // Frame information
    private long frameNumber;
    private long timestamp;
    private float processingTimeMs;

    /**
     * Create a new empty scene.
     */
    public Scene() {
    }

    /**
     * Create a new scene with the specified frame number.
     *
     * @param frameNumber the frame number
     */
    public Scene(long frameNumber) {
        this.frameNumber = frameNumber;
    }

    /**
     * Clear the scene.
     */
    public void clear() {
        camera = null;
        chunks.clear();
        entities.clear();
        particles.clear();
        weather = null;
        frameNumber = -1;
        timestamp = 0;
        processingTimeMs = 0;
    }

    /**
     * Update the scene with new data.
     *
     * @param camera the camera message
     * @param chunks the chunk messages
     * @param entities the entity messages
     * @param particles the particle messages
     * @param weather the weather message
     * @param frameNumber the frame number
     * @param timestamp the timestamp
     * @param processingTimeMs the processing time in milliseconds
     */
    public void update(CameraMessage camera, List<ChunkMessage> chunks, 
                      List<EntityMessage> entities, List<ParticleMessage> particles,
                      WeatherMessage weather, long frameNumber, long timestamp, 
                      float processingTimeMs) {
        this.camera = camera;
        this.chunks.clear();
        this.chunks.addAll(chunks);
        this.entities.clear();
        this.entities.addAll(entities);
        this.particles.clear();
        this.particles.addAll(particles);
        this.weather = weather;
        this.frameNumber = frameNumber;
        this.timestamp = timestamp;
        this.processingTimeMs = processingTimeMs;
    }

    // Getters and setters

    public CameraMessage getCamera() {
        return camera;
    }

    public void setCamera(CameraMessage camera) {
        this.camera = camera;
    }

    public List<ChunkMessage> getChunks() {
        return new ArrayList<>(chunks);
    }

    public void addChunk(ChunkMessage chunk) {
        chunks.add(chunk);
    }

    public void removeChunk(ChunkMessage chunk) {
        chunks.remove(chunk);
    }

    public List<EntityMessage> getEntities() {
        return new ArrayList<>(entities);
    }

    public void addEntity(EntityMessage entity) {
        entities.add(entity);
    }

    public void removeEntity(EntityMessage entity) {
        entities.remove(entity);
    }

    public List<ParticleMessage> getParticles() {
        return new ArrayList<>(particles);
    }

    public void addParticle(ParticleMessage particle) {
        particles.add(particle);
    }

    public void removeParticle(ParticleMessage particle) {
        particles.remove(particle);
    }

    public WeatherMessage getWeather() {
        return weather;
    }

    public void setWeather(WeatherMessage weather) {
        this.weather = weather;
    }

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

    public float getProcessingTimeMs() {
        return processingTimeMs;
    }

    public void setProcessingTimeMs(float processingTimeMs) {
        this.processingTimeMs = processingTimeMs;
    }

    /**
     * Get the number of chunks in the scene.
     *
     * @return the number of chunks
     */
    public int getChunkCount() {
        return chunks.size();
    }

    /**
     * Get the number of entities in the scene.
     *
     * @return the number of entities
     */
    public int getEntityCount() {
        return entities.size();
    }

    /**
     * Get the number of particle systems in the scene.
     *
     * @return the number of particle systems
     */
    public int getParticleCount() {
        return particles.size();
    }

    /**
     * Check if the scene has a camera.
     *
     * @return true if the scene has a camera
     */
    public boolean hasCamera() {
        return camera != null;
    }

    /**
     * Check if the scene has any chunks.
     *
     * @return true if the scene has chunks
     */
    public boolean hasChunks() {
        return !chunks.isEmpty();
    }

    /**
     * Check if the scene has any entities.
     *
     * @return true if the scene has entities
     */
    public boolean hasEntities() {
        return !entities.isEmpty();
    }

    /**
     * Check if the scene has any particles.
     *
     * @return true if the scene has particles
     */
    public boolean hasParticles() {
        return !particles.isEmpty();
    }

    /**
     * Check if the scene has weather.
     *
     * @return true if the scene has weather
     */
    public boolean hasWeather() {
        return weather != null;
    }

    @Override
    public String toString() {
        return String.format(
                "Scene{frameNumber=%d, timestamp=%d, processingTime=%.2fms, chunks=%d, entities=%d, particles=%d, hasCamera=%s}",
                frameNumber, timestamp, processingTimeMs, 
                chunks.size(), entities.size(), particles.size(), hasCamera()
        );
    }
}
