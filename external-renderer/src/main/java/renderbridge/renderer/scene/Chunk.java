package renderbridge.renderer.scene;

import renderbridge.protocol.messages.ChunkMessage;

import java.nio.ByteBuffer;

/**
 * Represents a chunk in the 3D world.
 * Contains block data, light data, and biome data.
 */
public class Chunk {

    // Chunk position
    private final int x;
    private final byte y;
    private final int z;
    
    // Section mask (which sections are present)
    private short sectionMask;
    
    // Block data
    private BlockData[] sections;
    
    // Light data
    private LightData[] lightSections;
    
    // Biome data
    private byte[] biomeData;
    
    // Flags
    private byte flags;

    /**
     * Create a new chunk.
     *
     * @param x the X position
     * @param y the Y position
     * @param z the Z position
     */
    public Chunk(int x, byte y, int z) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.sections = new BlockData[16]; // 16 sections (Y: 0-15)
        this.lightSections = new LightData[16];
    }

    /**
     * Create a chunk from a ChunkMessage.
     *
     * @param message the chunk message
     */
    public Chunk(ChunkMessage message) {
        this(message.getChunkX(), message.getChunkY(), message.getChunkZ());
        
        this.sectionMask = message.getSectionMask();
        this.flags = message.getFlags();
        
        // Parse block data
        parseBlockData(message.getBlockData());
        
        // Parse light data
        parseLightData(message.getLightData());
        
        // Parse biome data
        this.biomeData = message.getBiomeData();
    }

    /**
     * Parse block data from the message.
     *
     * @param blockData the block data from the message
     */
    private void parseBlockData(byte[] blockData) {
        if (blockData == null || blockData.length == 0) {
            return;
        }
        
        // Each block is 3 bytes: [blockId: uint16][blockMeta: uint8]
        // Total blocks per section: 16*16*16 = 4096
        // Total bytes per section: 4096 * 3 = 12288
        
        int sectionSize = 16 * 16 * 16; // 4096 blocks per section
        
        for (int sectionY = 0; sectionY < 16; sectionY++) {
            if (!hasSection(sectionY)) {
                continue;
            }
            
            int sectionIndex = sectionY;
            int startIndex = sectionIndex * sectionSize * 3;
            int endIndex = startIndex + sectionSize * 3;
            
            if (endIndex > blockData.length) {
                continue;
            }
            
            // Create block data for this section
            BlockData blockDataSection = new BlockData();
            
            for (int i = 0; i < sectionSize; i++) {
                int dataIndex = startIndex + i * 3;
                
                // Read block ID (2 bytes, big-endian)
                int blockId = ((blockData[dataIndex] & 0xFF) << 8) | (blockData[dataIndex + 1] & 0xFF);
                
                // Read metadata (1 byte)
                int metadata = blockData[dataIndex + 2] & 0xFF;
                
                // Calculate position within section
                int x = i & 0xF; // i % 16
                int y = (i >> 4) & 0xF; // (i / 16) % 16
                int z = (i >> 8) & 0xF; // (i / 256) % 16
                
                blockDataSection.setBlock(x, y, z, blockId, metadata);
            }
            
            sections[sectionY] = blockDataSection;
        }
    }

    /**
     * Parse light data from the message.
     *
     * @param lightData the light data from the message
     */
    private void parseLightData(byte[] lightData) {
        if (lightData == null || lightData.length == 0) {
            return;
        }
        
        // Each light value is 2 bytes: [skyLight: uint8][blockLight: uint8]
        // Total blocks per section: 16*16*16 = 4096
        // Total bytes per section: 4096 * 2 = 8192
        
        int sectionSize = 16 * 16 * 16; // 4096 blocks per section
        
        for (int sectionY = 0; sectionY < 16; sectionY++) {
            if (!hasSection(sectionY)) {
                continue;
            }
            
            int sectionIndex = sectionY;
            int startIndex = sectionIndex * sectionSize * 2;
            int endIndex = startIndex + sectionSize * 2;
            
            if (endIndex > lightData.length) {
                continue;
            }
            
            // Create light data for this section
            LightData lightDataSection = new LightData();
            
            for (int i = 0; i < sectionSize; i++) {
                int dataIndex = startIndex + i * 2;
                
                // Read sky light (1 byte)
                int skyLight = lightData[dataIndex] & 0xFF;
                
                // Read block light (1 byte)
                int blockLight = lightData[dataIndex + 1] & 0xFF;
                
                // Calculate position within section
                int x = i & 0xF;
                int y = (i >> 4) & 0xF;
                int z = (i >> 8) & 0xF;
                
                lightDataSection.setLight(x, y, z, skyLight, blockLight);
            }
            
            lightSections[sectionY] = lightDataSection;
        }
    }

    /**
     * Check if a section is present.
     *
     * @param sectionY the section Y coordinate (0-15)
     * @return true if the section is present
     */
    public boolean hasSection(int sectionY) {
        return (sectionMask & (1 << sectionY)) != 0;
    }

    /**
     * Get the block at a specific position within the chunk.
     *
     * @param x the X position within the chunk (0-15)
     * @param y the Y position within the chunk (0-255)
     * @param z the Z position within the chunk (0-15)
     * @return the block at the position, or null if not available
     */
    public Block getBlock(int x, int y, int z) {
        int sectionY = y >> 4; // y / 16
        int sectionYInChunk = y & 0xF; // y % 16
        
        if (sectionY < 0 || sectionY >= 16) {
            return null;
        }
        
        BlockData section = sections[sectionY];
        if (section == null) {
            return null;
        }
        
        return section.getBlock(x, sectionYInChunk, z);
    }

    /**
     * Get the light values at a specific position within the chunk.
     *
     * @param x the X position within the chunk (0-15)
     * @param y the Y position within the chunk (0-255)
     * @param z the Z position within the chunk (0-15)
     * @return the light values at the position, or null if not available
     */
    public LightValues getLight(int x, int y, int z) {
        int sectionY = y >> 4;
        int sectionYInChunk = y & 0xF;
        
        if (sectionY < 0 || sectionY >= 16) {
            return null;
        }
        
        LightData section = lightSections[sectionY];
        if (section == null) {
            return null;
        }
        
        return section.getLight(x, sectionYInChunk, z);
    }

    /**
     * Get the biome at a specific position within the chunk.
     *
     * @param x the X position within the chunk (0-15)
     * @param z the Z position within the chunk (0-15)
     * @return the biome ID at the position
     */
    public int getBiome(int x, int z) {
        if (biomeData == null || x < 0 || x >= 16 || z < 0 || z >= 16) {
            return 0;
        }
        
        return biomeData[(z << 4) | x] & 0xFF;
    }

    // Getters and setters

    public int getX() {
        return x;
    }

    public byte getY() {
        return y;
    }

    public int getZ() {
        return z;
    }

    public short getSectionMask() {
        return sectionMask;
    }

    public void setSectionMask(short sectionMask) {
        this.sectionMask = sectionMask;
    }

    public BlockData[] getSections() {
        return sections;
    }

    public BlockData getSection(int sectionY) {
        return sections[sectionY];
    }

    public void setSection(int sectionY, BlockData section) {
        sections[sectionY] = section;
    }

    public LightData[] getLightSections() {
        return lightSections;
    }

    public LightData getLightSection(int sectionY) {
        return lightSections[sectionY];
    }

    public void setLightSection(int sectionY, LightData lightSection) {
        lightSections[sectionY] = lightSection;
    }

    public byte[] getBiomeData() {
        return biomeData;
    }

    public void setBiomeData(byte[] biomeData) {
        this.biomeData = biomeData;
    }

    public byte getFlags() {
        return flags;
    }

    public void setFlags(byte flags) {
        this.flags = flags;
    }

    @Override
    public String toString() {
        return String.format(
                "Chunk{position=(%d, %d, %d), sectionMask=0x%04X}",
                x, y, z, sectionMask
        );
    }

    /**
     * Represents a block in the world.
     */
    public static class Block {
        private final int id;
        private final int metadata;

        public Block(int id, int metadata) {
            this.id = id;
            this.metadata = metadata;
        }

        public int getId() {
            return id;
        }

        public int getMetadata() {
            return metadata;
        }

        @Override
        public String toString() {
            return String.format("Block{id=%d, metadata=%d}", id, metadata);
        }
    }

    /**
     * Stores block data for a section.
     */
    public static class BlockData {
        private static final int SECTION_SIZE = 16 * 16 * 16;
        
        private final Block[] blocks = new Block[SECTION_SIZE];

        public BlockData() {
            // Initialize with air blocks
            for (int i = 0; i < SECTION_SIZE; i++) {
                blocks[i] = new Block(0, 0); // Air
            }
        }

        public void setBlock(int x, int y, int z, int id, int metadata) {
            int index = (z << 8) | (y << 4) | x; // z*256 + y*16 + x
            blocks[index] = new Block(id, metadata);
        }

        public Block getBlock(int x, int y, int z) {
            int index = (z << 8) | (y << 4) | x;
            return blocks[index];
        }
    }

    /**
     * Stores light data for a section.
     */
    public static class LightData {
        private static final int SECTION_SIZE = 16 * 16 * 16;
        
        private final LightValues[] lights = new LightValues[SECTION_SIZE];

        public LightData() {
            // Initialize with full brightness
            for (int i = 0; i < SECTION_SIZE; i++) {
                lights[i] = new LightValues(15, 15);
            }
        }

        public void setLight(int x, int y, int z, int skyLight, int blockLight) {
            int index = (z << 8) | (y << 4) | x;
            lights[index] = new LightValues(skyLight, blockLight);
        }

        public LightValues getLight(int x, int y, int z) {
            int index = (z << 8) | (y << 4) | x;
            return lights[index];
        }
    }

    /**
     * Represents light values (sky light and block light).
     */
    public static class LightValues {
        private final int skyLight;
        private final int blockLight;

        public LightValues(int skyLight, int blockLight) {
            this.skyLight = skyLight;
            this.blockLight = blockLight;
        }

        public int getSkyLight() {
            return skyLight;
        }

        public int getBlockLight() {
            return blockLight;
        }

        public int getCombinedLight() {
            return Math.max(skyLight, blockLight);
        }

        @Override
        public String toString() {
            return String.format("LightValues{sky=%d, block=%d}", skyLight, blockLight);
        }
    }
}
