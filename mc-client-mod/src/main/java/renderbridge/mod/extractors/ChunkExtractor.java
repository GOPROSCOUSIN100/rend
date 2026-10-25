package renderbridge.mod.extractors;

import net.minecraft.client.renderer.RenderGlobal;
import net.minecraft.client.renderer.chunk.ChunkRenderContainer;
import net.minecraft.client.renderer.chunk.RenderChunk;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;
import net.minecraft.world.chunk.Chunk;

import renderbridge.protocol.messages.ChunkMessage;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * Extracts chunk data from the RenderGlobal.
 * Converts Minecraft chunk data to ChunkMessage for the bridge protocol.
 */
public class ChunkExtractor {

    private final RenderGlobal renderGlobal;
    private final int maxChunkDistance;
    
    // Tracked chunks
    private final Set<BlockPos> trackedChunks = new HashSet<>();
    private final Set<BlockPos> dirtyChunks = new HashSet<>();
    
    // Cache for chunk data
    private long lastUpdateTime = 0;
    private int updateInterval = 100; // Update every 100ms

    /**
     * Create a new chunk extractor.
     *
     * @param renderGlobal the RenderGlobal instance
     * @param maxChunkDistance the maximum distance for chunk extraction
     */
    public ChunkExtractor(RenderGlobal renderGlobal, int maxChunkDistance) {
        this.renderGlobal = renderGlobal;
        this.maxChunkDistance = maxChunkDistance;
    }

    /**
     * Update the chunk extractor.
     */
    public void update() {
        if (renderGlobal == null || renderGlobal.worldObj == null) {
            return;
        }
        
        long currentTime = System.currentTimeMillis();
        if (currentTime - lastUpdateTime < updateInterval) {
            return;
        }
        
        lastUpdateTime = currentTime;
        
        try {
            // Update tracked chunks based on camera position
            updateTrackedChunks();
            
            // Extract dirty chunks
            extractDirtyChunks();
        } catch (Exception e) {
            System.err.println("Failed to update chunk extractor: " + e.getMessage());
        }
    }

    /**
     * Update the set of tracked chunks based on camera position.
     */
    private void updateTrackedChunks() {
        if (renderGlobal == null || renderGlobal.worldObj == null) {
            return;
        }
        
        World world = renderGlobal.worldObj;
        
        // Get camera position (for now, use player position)
        // In a real implementation, we would use the actual camera position
        // from the EntityRenderer
        
        // For simplicity, we'll track chunks around the player
        // This is a placeholder implementation
        trackedChunks.clear();
        
        // Add chunks in a radius around the player
        // This is a simplified version - in reality, we would use the
        // actual visible chunks from the RenderGlobal
    }

    /**
     * Extract dirty chunks and create ChunkMessages.
     */
    private void extractDirtyChunks() {
        if (renderGlobal == null || renderGlobal.worldObj == null) {
            return;
        }
        
        World world = renderGlobal.worldObj;
        
        // Extract data for dirty chunks
        for (BlockPos chunkPos : dirtyChunks) {
            extractChunkData(chunkPos, world);
        }
        
        dirtyChunks.clear();
    }

    /**
     * Extract chunk data for a specific chunk position.
     *
     * @param chunkPos the chunk position
     * @param world the world
     */
    private void extractChunkData(BlockPos chunkPos, World world) {
        try {
            Chunk chunk = world.getChunkFromBlockCoords(chunkPos);
            if (chunk == null) {
                return;
            }
            
            // Create chunk message
            ChunkMessage message = new ChunkMessage();
            message.setChunkPosition(chunkPos.getX(), (byte) chunkPos.getY(), chunkPos.getZ());
            
            // Extract block data for all sections
            extractBlockData(chunk, message);
            
            // Extract light data
            extractLightData(chunk, message);
            
            // Extract biome data
            extractBiomeData(chunk, message);
            
            // Add to tracked chunks
            trackedChunks.add(chunkPos);
        } catch (Exception e) {
            System.err.println("Failed to extract chunk data for " + chunkPos + ": " + e.getMessage());
        }
    }

    /**
     * Extract block data from a chunk.
     *
     * @param chunk the chunk
     * @param message the message to populate
     */
    private void extractBlockData(Chunk chunk, ChunkMessage message) {
        try {
            // Minecraft 1.8.9 chunks have 16 sections (Y: 0-15)
            // Each section is 16x16x16 blocks
            
            // For now, we'll use a simplified approach
            // In a real implementation, we would iterate through all sections
            // and extract the block IDs and metadata
            
            // This is a placeholder - actual implementation would need to:
            // 1. Iterate through all 16 sections
            // 2. For each section, check if it exists
            // 3. If it exists, extract the block data
            // 4. Serialize the data in our format
            
            // For now, just mark all sections as present
            message.setSectionMask((short) 0xFFFF);
            
            // Create dummy block data (all air for now)
            // In reality, this would be the actual block data from the chunk
            byte[] blockData = new byte[ChunkMessage.SECTION_BLOCKS * ChunkMessage.MAX_SECTIONS * 3];
            // Fill with air (block ID 0)
            for (int i = 0; i < blockData.length; i += 3) {
                blockData[i] = 0;     // Block ID high byte
                blockData[i + 1] = 0; // Block ID low byte
                blockData[i + 2] = 0; // Metadata
            }
            
            message.setBlockData(blockData);
        } catch (Exception e) {
            System.err.println("Failed to extract block data: " + e.getMessage());
        }
    }

    /**
     * Extract light data from a chunk.
     *
     * @param chunk the chunk
     * @param message the message to populate
     */
    private void extractLightData(Chunk chunk, ChunkMessage message) {
        try {
            // In Minecraft 1.8.9, light data is stored separately
            // For now, create dummy light data (full brightness)
            byte[] lightData = new byte[ChunkMessage.SECTION_BLOCKS * ChunkMessage.MAX_SECTIONS * 2];
            // Fill with full brightness
            for (int i = 0; i < lightData.length; i++) {
                lightData[i] = (byte) 15; // Full light
            }
            
            message.setLightData(lightData);
        } catch (Exception e) {
            System.err.println("Failed to extract light data: " + e.getMessage());
        }
    }

    /**
     * Extract biome data from a chunk.
     *
     * @param chunk the chunk
     * @param message the message to populate
     */
    private void extractBiomeData(Chunk chunk, ChunkMessage message) {
        try {
            // In Minecraft 1.8.9, biome data is stored in the chunk
            // For now, create dummy biome data
            byte[] biomeData = new byte[256]; // 16x16 biomes
            // Fill with forest biome (ID 18)
            for (int i = 0; i < biomeData.length; i++) {
                biomeData[i] = 18;
            }
            
            message.setBiomeData(biomeData);
        } catch (Exception e) {
            System.err.println("Failed to extract biome data: " + e.getMessage());
        }
    }

    /**
     * Extract all visible chunks and create ChunkMessages.
     *
     * @return a list of ChunkMessages for all visible chunks
     */
    public List<ChunkMessage> extract() {
        List<ChunkMessage> messages = new ArrayList<>();
        
        if (renderGlobal == null || renderGlobal.worldObj == null) {
            return messages;
        }
        
        try {
            World world = renderGlobal.worldObj;
            
            // Iterate through all render chunks
            // In Minecraft 1.8.9, we can access the render chunks through
            // renderGlobal.renderChunkOffsets and renderGlobal.renderChunks
            
            // This is a placeholder - actual implementation would need to:
            // 1. Iterate through all visible render chunks
            // 2. For each render chunk, get the corresponding world chunk
            // 3. Extract the data and create a ChunkMessage
            
            // For now, just create a dummy chunk at origin
            ChunkMessage message = new ChunkMessage();
            message.setChunkPosition(0, (byte) 0, 0);
            message.setSectionMask((short) 0xFFFF);
            
            // Create dummy block data
            byte[] blockData = new byte[ChunkMessage.SECTION_BLOCKS * 3];
            for (int i = 0; i < blockData.length; i += 3) {
                // Stone block (ID 1)
                blockData[i] = 0;
                blockData[i + 1] = 1;
                blockData[i + 2] = 0;
            }
            message.setBlockData(blockData);
            
            // Create dummy light data
            byte[] lightData = new byte[ChunkMessage.SECTION_BLOCKS * 2];
            for (int i = 0; i < lightData.length; i++) {
                lightData[i] = (byte) 15;
            }
            message.setLightData(lightData);
            
            messages.add(message);
        } catch (Exception e) {
            System.err.println("Failed to extract chunks: " + e.getMessage());
        }
        
        return messages;
    }

    /**
     * Called when a chunk is loaded.
     *
     * @param chunk the loaded chunk
     */
    public void onChunkLoaded(Chunk chunk) {
        if (chunk == null) {
            return;
        }
        
        BlockPos chunkPos = chunk.getChunkCoordIntPair();
        dirtyChunks.add(new BlockPos(chunkPos.getX(), 0, chunkPos.getZ()));
    }

    /**
     * Called when a chunk is unloaded.
     *
     * @param chunk the unloaded chunk
     */
    public void onChunkUnloaded(Chunk chunk) {
        if (chunk == null) {
            return;
        }
        
        BlockPos chunkPos = chunk.getChunkCoordIntPair();
        trackedChunks.remove(new BlockPos(chunkPos.getX(), 0, chunkPos.getZ()));
        dirtyChunks.remove(new BlockPos(chunkPos.getX(), 0, chunkPos.getZ()));
    }

    /**
     * Get the maximum chunk distance.
     *
     * @return the maximum chunk distance
     */
    public int getMaxChunkDistance() {
        return maxChunkDistance;
    }

    /**
     * Get the number of tracked chunks.
     *
     * @return the number of tracked chunks
     */
    public int getTrackedChunkCount() {
        return trackedChunks.size();
    }

    /**
     * Get the number of dirty chunks.
     *
     * @return the number of dirty chunks
     */
    public int getDirtyChunkCount() {
        return dirtyChunks.size();
    }

    @Override
    public String toString() {
        return String.format(
                "ChunkExtractor{renderGlobal=%s, maxChunkDistance=%d, trackedChunks=%d, dirtyChunks=%d}",
                renderGlobal != null, maxChunkDistance, trackedChunks.size(), dirtyChunks.size()
        );
    }
}
