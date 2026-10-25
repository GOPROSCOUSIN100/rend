package renderbridge.mod.extractors;

import net.minecraft.client.renderer.RenderGlobal;
import net.minecraft.client.renderer.entity.RenderManager;
import net.minecraft.entity.Entity;
import net.minecraft.util.math.MathHelper;

import renderbridge.protocol.messages.EntityMessage;

import java.util.ArrayList;
import java.util.List;

/**
 * Extracts entity data from the RenderGlobal.
 * Converts Minecraft entities to EntityMessage for the bridge protocol.
 */
public class EntityExtractor {

    private final RenderGlobal renderGlobal;
    private final RenderManager renderManager;
    
    // Cached entity list
    private final List<Entity> trackedEntities = new ArrayList<>();
    
    // Frame counter
    private long lastFrameNumber = 0;

    /**
     * Create a new entity extractor.
     *
     * @param renderGlobal the RenderGlobal instance
     */
    public EntityExtractor(RenderGlobal renderGlobal) {
        this.renderGlobal = renderGlobal;
        this.renderManager = renderGlobal.renderManager;
    }

    /**
     * Update the entity extractor.
     *
     * @param partialTicks the current partial ticks
     */
    public void update(float partialTicks) {
        if (renderGlobal == null || renderManager == null) {
            return;
        }
        
        try {
            // Update tracked entities
            updateTrackedEntities();
        } catch (Exception e) {
            System.err.println("Failed to update entity extractor: " + e.getMessage());
        }
    }

    /**
     * Update the list of tracked entities.
     */
    private void updateTrackedEntities() {
        trackedEntities.clear();
        
        if (renderManager == null || renderManager.worldObj == null) {
            return;
        }
        
        try {
            // Get all entities from the world
            // In Minecraft 1.8.9, we can iterate through the entity lists
            if (renderManager.worldObj != null) {
                // Add all entities from the world
                // This is a simplified approach - in reality, we would:
                // 1. Only track entities that are visible
                // 2. Only track entities within a certain distance
                // 3. Handle entity additions/removals efficiently
                
                // For now, just add the player
                if (renderManager.viewer != null) {
                    trackedEntities.add(renderManager.viewer);
                }
            }
        } catch (Exception e) {
            System.err.println("Failed to update tracked entities: " + e.getMessage());
        }
    }

    /**
     * Extract entity data and create EntityMessages.
     *
     * @return a list of EntityMessages for all tracked entities
     */
    public List<EntityMessage> extract() {
        List<EntityMessage> messages = new ArrayList<>();
        
        if (renderGlobal == null || renderManager == null) {
            return messages;
        }
        
        try {
            for (Entity entity : trackedEntities) {
                if (entity == null || entity.isDead) {
                    continue;
                }
                
                EntityMessage message = extractEntityData(entity);
                if (message != null) {
                    messages.add(message);
                }
            }
        } catch (Exception e) {
            System.err.println("Failed to extract entity data: " + e.getMessage());
        }
        
        return messages;
    }

    /**
     * Extract data for a single entity.
     *
     * @param entity the entity
     * @return an EntityMessage with the entity data, or null if extraction fails
     */
    private EntityMessage extractEntityData(Entity entity) {
        try {
            EntityMessage message = new EntityMessage();
            
            // Set entity ID
            message.setEntityId(entity.getEntityId());
            
            // Set entity type
            // In Minecraft 1.8.9, entity types are not directly exposed
            // We'll use a simple mapping based on the entity class
            message.setEntityType(getEntityTypeId(entity));
            
            // Set position
            message.setPosition(
                    (float) entity.posX,
                    (float) entity.posY,
                    (float) entity.posZ
            );
            
            // Set rotation
            message.setRotation(
                    -entity.rotationPitch * 0.017453292F, // Convert to radians and flip pitch
                    entity.rotationYaw * 0.017453292F,    // Convert to radians
                    0
            );
            
            // Set velocity
            message.setVelocity(
                    (float) entity.motionX,
                    (float) entity.motionY,
                    (float) entity.motionZ
            );
            
            // Set model ID (for now, use entity type ID)
            message.setModelId(message.getEntityType());
            
            return message;
        } catch (Exception e) {
            System.err.println("Failed to extract entity data for entity " + entity + ": " + e.getMessage());
            return null;
        }
    }

    /**
     * Get the entity type ID for a given entity.
     *
     * @param entity the entity
     * @return the entity type ID
     */
    private short getEntityTypeId(Entity entity) {
        if (entity == null) {
            return 0;
        }
        
        // Simple mapping based on entity class name
        String className = entity.getClass().getSimpleName().toLowerCase();
        
        if (className.contains("player")) {
            return 0; // Player
        } else if (className.contains("zombie")) {
            return 1; // Zombie
        } else if (className.contains("skeleton")) {
            return 2; // Skeleton
        } else if (className.contains("cow")) {
            return 3; // Cow
        } else if (className.contains("pig")) {
            return 4; // Pig
        } else if (className.contains("sheep")) {
            return 5; // Sheep
        } else if (className.contains("chicken")) {
            return 6; // Chicken
        } else if (className.contains("item")) {
            return 7; // Item entity
        } else if (className.contains("xp")) {
            return 8; // XP orb
        } else {
            return -1; // Unknown
        }
    }

    /**
     * Called when an entity is added to the world.
     *
     * @param entity the added entity
     */
    public void onEntityAdded(Entity entity) {
        if (entity == null || entity.isDead) {
            return;
        }
        
        if (!trackedEntities.contains(entity)) {
            trackedEntities.add(entity);
        }
    }

    /**
     * Called when an entity is removed from the world.
     *
     * @param entity the removed entity
     */
    public void onEntityRemoved(Entity entity) {
        trackedEntities.remove(entity);
    }

    /**
     * Get the number of tracked entities.
     *
     * @return the number of tracked entities
     */
    public int getTrackedEntityCount() {
        return trackedEntities.size();
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
        return String.format(
                "EntityExtractor{renderGlobal=%s, trackedEntities=%d}",
                renderGlobal != null, trackedEntities.size()
        );
    }
}
