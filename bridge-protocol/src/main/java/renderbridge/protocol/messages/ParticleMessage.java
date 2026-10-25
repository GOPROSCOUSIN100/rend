package renderbridge.protocol.messages;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageType;

import java.nio.ByteBuffer;
import java.util.Arrays;

/**
 * Message containing particle data for rendering.
 * Includes particle positions, velocities, types, and lifetimes.
 */
public class ParticleMessage extends Message {

    // Maximum particles per message
    public static final int MAX_PARTICLES = 1024;

    // Particle types (Minecraft 1.8.9)
    public static final int PARTICLE_EXPLOSION_NORMAL = 0;
    public static final int PARTICLE_EXPLOSION_LARGE = 1;
    public static final int PARTICLE_EXPLOSION_HUGE = 2;
    public static final int PARTICLE_FIREWORKS_SPARK = 3;
    public static final int PARTICLE_WATER_BUBBLE = 4;
    public static final int PARTICLE_WATER_SPLASH = 5;
    public static final int PARTICLE_WATER_WAKE = 6;
    public static final int PARTICLE_WATER_DROP = 7;
    public static final int PARTICLE_BLOCK_DUST = 8;
    public static final int PARTICLE_BLOCK_CRACK = 9;
    public static final int PARTICLE_BLOCK_BREAK = 10;
    public static final int PARTICLE_SMOKE_NORMAL = 11;
    public static final int PARTICLE_SMOKE_LARGE = 12;
    public static final int PARTICLE_SPELL = 13;
    public static final int PARTICLE_SPELL_INSTANT = 14;
    public static final int PARTICLE_SPELL_MOB = 15;
    public static final int PARTICLE_SPELL_MOB_AMBIENT = 16;
    public static final int PARTICLE_SPELL_WITCH = 17;
    public static final int PARTICLE_DRIP_WATER = 18;
    public static final int PARTICLE_DRIP_LAVA = 19;
    public static final int PARTICLE_ANGRY_VILLAGER = 20;
    public static final int PARTICLE_HAPPY_VILLAGER = 21;

    private short particleType;
    private short particleCount;
    
    // Arrays for particle data (x, y, z for each particle)
    private final float[] positionsX = new float[MAX_PARTICLES];
    private final float[] positionsY = new float[MAX_PARTICLES];
    private final float[] positionsZ = new float[MAX_PARTICLES];
    
    private final float[] velocitiesX = new float[MAX_PARTICLES];
    private final float[] velocitiesY = new float[MAX_PARTICLES];
    private final float[] velocitiesZ = new float[MAX_PARTICLES];
    
    private final float[] lifetimes = new float[MAX_PARTICLES];
    private final float[] scales = new float[MAX_PARTICLES];

    /**
     * Create a new particle message.
     */
    public ParticleMessage() {
        super(MessageType.PARTICLE);
    }

    /**
     * Create a particle message with the specified version.
     *
     * @param version the protocol version
     */
    public ParticleMessage(int version) {
        super(MessageType.PARTICLE);
    }

    /**
     * Create a particle message from a header and payload buffer.
     *
     * @param version the protocol version
     * @param payload the payload buffer
     */
    public ParticleMessage(int version, ByteBuffer payload) {
        super(MessageType.PARTICLE);
        readPayload(payload);
    }

    @Override
    protected void readPayload(ByteBuffer buffer) {
        particleType = buffer.getShort();
        particleCount = buffer.getShort();
        
        int count = Math.min(particleCount, MAX_PARTICLES);
        
        for (int i = 0; i < count; i++) {
            positionsX[i] = buffer.getFloat();
            positionsY[i] = buffer.getFloat();
            positionsZ[i] = buffer.getFloat();
        }
        
        for (int i = 0; i < count; i++) {
            velocitiesX[i] = buffer.getFloat();
            velocitiesY[i] = buffer.getFloat();
            velocitiesZ[i] = buffer.getFloat();
        }
        
        for (int i = 0; i < count; i++) {
            lifetimes[i] = buffer.getFloat();
        }
        
        for (int i = 0; i < count; i++) {
            scales[i] = buffer.getFloat();
        }
    }

    @Override
    protected void writePayload(ByteBuffer buffer) {
        buffer.putShort(particleType);
        buffer.putShort(particleCount);
        
        int count = Math.min(particleCount, MAX_PARTICLES);
        
        for (int i = 0; i < count; i++) {
            buffer.putFloat(positionsX[i]);
            buffer.putFloat(positionsY[i]);
            buffer.putFloat(positionsZ[i]);
        }
        
        for (int i = 0; i < count; i++) {
            buffer.putFloat(velocitiesX[i]);
            buffer.putFloat(velocitiesY[i]);
            buffer.putFloat(velocitiesZ[i]);
        }
        
        for (int i = 0; i < count; i++) {
            buffer.putFloat(lifetimes[i]);
        }
        
        for (int i = 0; i < count; i++) {
            buffer.putFloat(scales[i]);
        }
    }

    @Override
    public int getPayloadSize() {
        // particleType(2) + particleCount(2) +
        // positions(12 * count) + velocities(12 * count) + lifetimes(4 * count) + scales(4 * count)
        int count = Math.min(particleCount, MAX_PARTICLES);
        return 2 + 2 + (12 * count) + (12 * count) + (4 * count) + (4 * count);
    }

    // Getters and setters

    public short getParticleType() {
        return particleType;
    }

    public void setParticleType(short particleType) {
        this.particleType = particleType;
    }

    public short getParticleCount() {
        return particleCount;
    }

    public void setParticleCount(short particleCount) {
        this.particleCount = Math.min(particleCount, MAX_PARTICLES);
    }

    public float getPositionX(int index) {
        return index >= 0 && index < MAX_PARTICLES ? positionsX[index] : 0;
    }

    public void setPositionX(int index, float value) {
        if (index >= 0 && index < MAX_PARTICLES) {
            positionsX[index] = value;
        }
    }

    public float getPositionY(int index) {
        return index >= 0 && index < MAX_PARTICLES ? positionsY[index] : 0;
    }

    public void setPositionY(int index, float value) {
        if (index >= 0 && index < MAX_PARTICLES) {
            positionsY[index] = value;
        }
    }

    public float getPositionZ(int index) {
        return index >= 0 && index < MAX_PARTICLES ? positionsZ[index] : 0;
    }

    public void setPositionZ(int index, float value) {
        if (index >= 0 && index < MAX_PARTICLES) {
            positionsZ[index] = value;
        }
    }

    public void setPosition(int index, float x, float y, float z) {
        if (index >= 0 && index < MAX_PARTICLES) {
            positionsX[index] = x;
            positionsY[index] = y;
            positionsZ[index] = z;
        }
    }

    public float getVelocityX(int index) {
        return index >= 0 && index < MAX_PARTICLES ? velocitiesX[index] : 0;
    }

    public void setVelocityX(int index, float value) {
        if (index >= 0 && index < MAX_PARTICLES) {
            velocitiesX[index] = value;
        }
    }

    public float getVelocityY(int index) {
        return index >= 0 && index < MAX_PARTICLES ? velocitiesY[index] : 0;
    }

    public void setVelocityY(int index, float value) {
        if (index >= 0 && index < MAX_PARTICLES) {
            velocitiesY[index] = value;
        }
    }

    public float getVelocityZ(int index) {
        return index >= 0 && index < MAX_PARTICLES ? velocitiesZ[index] : 0;
    }

    public void setVelocityZ(int index, float value) {
        if (index >= 0 && index < MAX_PARTICLES) {
            velocitiesZ[index] = value;
        }
    }

    public void setVelocity(int index, float x, float y, float z) {
        if (index >= 0 && index < MAX_PARTICLES) {
            velocitiesX[index] = x;
            velocitiesY[index] = y;
            velocitiesZ[index] = z;
        }
    }

    public float getLifetime(int index) {
        return index >= 0 && index < MAX_PARTICLES ? lifetimes[index] : 0;
    }

    public void setLifetime(int index, float value) {
        if (index >= 0 && index < MAX_PARTICLES) {
            lifetimes[index] = value;
        }
    }

    public float getScale(int index) {
        return index >= 0 && index < MAX_PARTICLES ? scales[index] : 1.0f;
    }

    public void setScale(int index, float value) {
        if (index >= 0 && index < MAX_PARTICLES) {
            scales[index] = value;
        }
    }

    /**
     * Add a particle to the message.
     *
     * @param index the particle index
     * @param x the X position
     * @param y the Y position
     * @param z the Z position
     * @param vx the X velocity
     * @param vy the Y velocity
     * @param vz the Z velocity
     * @param lifetime the particle lifetime
     * @param scale the particle scale
     */
    public void addParticle(int index, float x, float y, float z, 
                           float vx, float vy, float vz, 
                           float lifetime, float scale) {
        if (index >= 0 && index < MAX_PARTICLES) {
            positionsX[index] = x;
            positionsY[index] = y;
            positionsZ[index] = z;
            velocitiesX[index] = vx;
            velocitiesY[index] = vy;
            velocitiesZ[index] = vz;
            lifetimes[index] = lifetime;
            scales[index] = scale;
            
            if (index >= particleCount) {
                particleCount = (short) (index + 1);
            }
        }
    }

    @Override
    public String toString() {
        return String.format(
                "ParticleMessage{particleType=%d, particleCount=%d}",
                particleType, particleCount
        );
    }
}
