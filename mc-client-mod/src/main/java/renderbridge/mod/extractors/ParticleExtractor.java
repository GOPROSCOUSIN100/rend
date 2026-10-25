package renderbridge.mod.extractors;

import net.minecraft.client.particle.EffectRenderer;
import net.minecraft.client.particle.Particle;
import net.minecraft.client.particle.ParticleManager;

import renderbridge.protocol.messages.ParticleMessage;

import java.util.ArrayList;
import java.util.List;

/**
 * Extracts particle data from the EffectRenderer.
 * Converts Minecraft particles to ParticleMessage for the bridge protocol.
 */
public class ParticleExtractor {

    private final EffectRenderer effectRenderer;
    private final ParticleManager particleManager;
    
    // Frame counter
    private long lastFrameNumber = 0;

    /**
     * Create a new particle extractor.
     *
     * @param effectRenderer the EffectRenderer instance
     */
    public ParticleExtractor(EffectRenderer effectRenderer) {
        this.effectRenderer = effectRenderer;
        this.particleManager = effectRenderer;
    }

    /**
     * Update the particle extractor.
     *
     * @param partialTicks the current partial ticks
     */
    public void update(float partialTicks) {
        if (effectRenderer == null) {
            return;
        }
        
        try {
            // Update particle positions
            effectRenderer.updateEffects();
        } catch (Exception e) {
            System.err.println("Failed to update particle extractor: " + e.getMessage());
        }
    }

    /**
     * Extract particle data and create ParticleMessages.
     *
     * @return a list of ParticleMessages for all particle systems
     */
    public List<ParticleMessage> extract() {
        List<ParticleMessage> messages = new ArrayList<>();
        
        if (effectRenderer == null) {
            return messages;
        }
        
        try {
            // Group particles by type
            // In Minecraft 1.8.9, particles are stored in the EffectRenderer
            // We need to iterate through all active particles
            
            // For now, create a single particle message with all particles
            ParticleMessage message = new ParticleMessage();
            message.setParticleType(ParticleMessage.PARTICLE_EXPLOSION_NORMAL); // Default type
            
            // Extract particle data
            int particleCount = extractParticleData(message);
            message.setParticleCount((short) particleCount);
            
            if (particleCount > 0) {
                messages.add(message);
            }
        } catch (Exception e) {
            System.err.println("Failed to extract particle data: " + e.getMessage());
        }
        
        return messages;
    }

    /**
     * Extract particle data and populate the message.
     *
     * @param message the message to populate
     * @return the number of particles extracted
     */
    private int extractParticleData(ParticleMessage message) {
        int count = 0;
        
        if (effectRenderer == null) {
            return 0;
        }
        
        try {
            // In Minecraft 1.8.9, particles are stored in lists in the EffectRenderer
            // We would iterate through these lists and extract data
            
            // For now, create some dummy particles
            // In a real implementation, we would:
            // 1. Iterate through all particle lists
            // 2. For each particle, extract position, velocity, type, etc.
            // 3. Group particles by type for efficiency
            
            // Create a few dummy particles
            for (int i = 0; i < 10 && i < ParticleMessage.MAX_PARTICLES; i++) {
                message.addParticle(
                        i, // index
                        (float) (Math.random() * 10 - 5), // x
                        (float) (Math.random() * 10),    // y
                        (float) (Math.random() * 10 - 5), // z
                        0, 0, 0, // velocity
                        1.0f, // lifetime
                        1.0f  // scale
                );
                count++;
            }
        } catch (Exception e) {
            System.err.println("Failed to extract particle data: " + e.getMessage());
        }
        
        return count;
    }

    /**
     * Get the particle type ID for a given particle.
     *
     * @param particle the particle
     * @return the particle type ID
     */
    private short getParticleTypeId(Particle particle) {
        if (particle == null) {
            return 0;
        }
        
        // Simple mapping based on particle class name
        String className = particle.getClass().getSimpleName().toLowerCase();
        
        if (className.contains("explosion")) {
            return ParticleMessage.PARTICLE_EXPLOSION_NORMAL;
        } else if (className.contains("fireworks")) {
            return ParticleMessage.PARTICLE_FIREWORKS_SPARK;
        } else if (className.contains("bubble")) {
            return ParticleMessage.PARTICLE_WATER_BUBBLE;
        } else if (className.contains("splash")) {
            return ParticleMessage.PARTICLE_WATER_SPLASH;
        } else if (className.contains("drip")) {
            return ParticleMessage.PARTICLE_DRIP_WATER;
        } else if (className.contains("smoke")) {
            return ParticleMessage.PARTICLE_SMOKE_NORMAL;
        } else if (className.contains("spell")) {
            return ParticleMessage.PARTICLE_SPELL;
        } else {
            return 0; // Unknown
        }
    }

    /**
     * Get the number of active particles.
     *
     * @return the number of active particles
     */
    public int getParticleCount() {
        if (effectRenderer == null) {
            return 0;
        }
        
        // In a real implementation, we would return the actual particle count
        return 0;
    }

    @Override
    public String toString() {
        return String.format(
                "ParticleExtractor{effectRenderer=%s}",
                effectRenderer != null
        );
    }
}
