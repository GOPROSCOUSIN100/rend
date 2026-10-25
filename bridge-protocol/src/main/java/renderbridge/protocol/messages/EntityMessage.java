package renderbridge.protocol.messages;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageType;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * Message containing entity data for rendering.
 * Includes position, rotation, velocity, and metadata.
 */
public class EntityMessage extends Message {

    // Maximum metadata size
    private static final int MAX_METADATA_SIZE = 4096;

    private int entityId;
    private short entityType;
    private short modelId;
    
    // Position and rotation
    private float positionX;
    private float positionY;
    private float positionZ;
    private float rotationPitch;
    private float rotationYaw;
    private float rotationRoll;
    
    // Velocity
    private float velocityX;
    private float velocityY;
    private float velocityZ;
    
    // Metadata (NBT-like data)
    private byte[] metadata;
    
    // Flags
    private byte flags;

    /**
     * Create a new entity message.
     */
    public EntityMessage() {
        super(MessageType.ENTITY);
    }

    /**
     * Create an entity message with the specified version.
     *
     * @param version the protocol version
     */
    public EntityMessage(int version) {
        super(MessageType.ENTITY);
    }

    /**
     * Create an entity message from a header and payload buffer.
     *
     * @param version the protocol version
     * @param payload the payload buffer
     */
    public EntityMessage(int version, ByteBuffer payload) {
        super(MessageType.ENTITY);
        readPayload(payload);
    }

    @Override
    protected void readPayload(ByteBuffer buffer) {
        entityId = buffer.getInt();
        entityType = buffer.getShort();
        modelId = buffer.getShort();
        
        positionX = buffer.getFloat();
        positionY = buffer.getFloat();
        positionZ = buffer.getFloat();
        
        rotationPitch = buffer.getFloat();
        rotationYaw = buffer.getFloat();
        rotationRoll = buffer.getFloat();
        
        velocityX = buffer.getFloat();
        velocityY = buffer.getFloat();
        velocityZ = buffer.getFloat();
        
        // Read metadata size (uint16)
        int metadataSize = buffer.getShort() & 0xFFFF;
        metadata = new byte[Math.min(metadataSize, MAX_METADATA_SIZE)];
        buffer.get(metadata);
        
        flags = buffer.get();
    }

    @Override
    protected void writePayload(ByteBuffer buffer) {
        buffer.putInt(entityId);
        buffer.putShort(entityType);
        buffer.putShort(modelId);
        
        buffer.putFloat(positionX);
        buffer.putFloat(positionY);
        buffer.putFloat(positionZ);
        
        buffer.putFloat(rotationPitch);
        buffer.putFloat(rotationYaw);
        buffer.putFloat(rotationRoll);
        
        buffer.putFloat(velocityX);
        buffer.putFloat(velocityY);
        buffer.putFloat(velocityZ);
        
        // Write metadata
        if (metadata != null) {
            int metadataSize = Math.min(metadata.length, MAX_METADATA_SIZE);
            buffer.putShort((short) metadataSize);
            buffer.put(metadata, 0, metadataSize);
        } else {
            buffer.putShort((short) 0);
        }
        
        buffer.put(flags);
    }

    @Override
    public int getPayloadSize() {
        // entityId(4) + entityType(2) + modelId(2) +
        // position(12) + rotation(12) + velocity(12) +
        // metadataSize(2) + metadata.length + flags(1)
        int metadataSize = metadata != null ? Math.min(metadata.length, MAX_METADATA_SIZE) : 0;
        return 4 + 2 + 2 + 12 + 12 + 12 + 2 + metadataSize + 1;
    }

    // Getters and setters

    public int getEntityId() {
        return entityId;
    }

    public void setEntityId(int entityId) {
        this.entityId = entityId;
    }

    public short getEntityType() {
        return entityType;
    }

    public void setEntityType(short entityType) {
        this.entityType = entityType;
    }

    public short getModelId() {
        return modelId;
    }

    public void setModelId(short modelId) {
        this.modelId = modelId;
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

    public float getVelocityX() {
        return velocityX;
    }

    public void setVelocityX(float velocityX) {
        this.velocityX = velocityX;
    }

    public float getVelocityY() {
        return velocityY;
    }

    public void setVelocityY(float velocityY) {
        this.velocityY = velocityY;
    }

    public float getVelocityZ() {
        return velocityZ;
    }

    public void setVelocityZ(float velocityZ) {
        this.velocityZ = velocityZ;
    }

    public void setVelocity(float x, float y, float z) {
        this.velocityX = x;
        this.velocityY = y;
        this.velocityZ = z;
    }

    public byte[] getMetadata() {
        return metadata;
    }

    public void setMetadata(byte[] metadata) {
        this.metadata = metadata;
    }

    public byte getFlags() {
        return flags;
    }

    public void setFlags(byte flags) {
        this.flags = flags;
    }

    public void addFlag(byte flag) {
        this.flags |= flag;
    }

    public boolean hasFlag(byte flag) {
        return (this.flags & flag) != 0;
    }

    @Override
    public String toString() {
        return String.format(
                "EntityMessage{entityId=%d, entityType=%d, modelId=%d, position=(%.2f, %.2f, %.2f), rotation=(%.2f, %.2f, %.2f), velocity=(%.2f, %.2f, %.2f), flags=0x%02X}",
                entityId, entityType, modelId,
                positionX, positionY, positionZ,
                rotationPitch, rotationYaw, rotationRoll,
                velocityX, velocityY, velocityZ,
                flags
        );
    }
}
