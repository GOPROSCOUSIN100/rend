package renderbridge.protocol.messages;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageType;

import java.nio.ByteBuffer;

/**
 * Message containing chunk data for rendering.
 * Includes block data, light data, and metadata for a specific chunk.
 */
public class ChunkMessage extends Message {

    // Chunk section size (16x16x16 blocks)
    public static final int SECTION_SIZE = 16;
    public static final int SECTION_BLOCKS = SECTION_SIZE * SECTION_SIZE * SECTION_SIZE;
    
    // Maximum sections per chunk (16 in Y direction for Minecraft)
    public static final int MAX_SECTIONS = 16;

    // Compression flags
    public static final int FLAG_NO_COMPRESSION = 0x00;
    public static final int FLAG_ZSTD_COMPRESSION = 0x01;

    private int chunkX;
    private byte chunkY;
    private int chunkZ;
    private short sectionMask;
    private byte compression;
    private byte flags;
    
    // Block data: [blockId: uint16][blockMeta: uint8] per block
    // Size: 3 bytes per block * 4096 blocks = 12KB per section
    private byte[] blockData;
    
    // Light data: [skyLight: uint8][blockLight: uint8] per block
    // Size: 2 bytes per block * 4096 blocks = 8KB per section
    private byte[] lightData;
    
    // Biome data for the chunk (optional)
    private byte[] biomeData;

    /**
     * Create a new chunk message.
     */
    public ChunkMessage() {
        super(MessageType.CHUNK);
    }

    /**
     * Create a chunk message with the specified version.
     *
     * @param version the protocol version
     */
    public ChunkMessage(int version) {
        super(MessageType.CHUNK);
    }

    /**
     * Create a chunk message from a header and payload buffer.
     *
     * @param version the protocol version
     * @param payload the payload buffer
     */
    public ChunkMessage(int version, ByteBuffer payload) {
        super(MessageType.CHUNK);
        readPayload(payload);
    }

    @Override
    protected void readPayload(ByteBuffer buffer) {
        chunkX = buffer.getInt();
        chunkY = buffer.get();
        chunkZ = buffer.getInt();
        sectionMask = buffer.getShort();
        compression = buffer.get();
        flags = buffer.get();
        
        // Read block data size (uint16)
        int blockDataSize = buffer.getShort() & 0xFFFF;
        blockData = new byte[blockDataSize];
        buffer.get(blockData);
        
        // Read light data size (uint16)
        int lightDataSize = buffer.getShort() & 0xFFFF;
        lightData = new byte[lightDataSize];
        buffer.get(lightData);
        
        // Read biome data size (uint16)
        int biomeDataSize = buffer.getShort() & 0xFFFF;
        biomeData = new byte[biomeDataSize];
        buffer.get(biomeData);
    }

    @Override
    protected void writePayload(ByteBuffer buffer) {
        buffer.putInt(chunkX);
        buffer.put(chunkY);
        buffer.putInt(chunkZ);
        buffer.putShort(sectionMask);
        buffer.put(compression);
        buffer.put(flags);
        
        // Write block data
        if (blockData != null) {
            buffer.putShort((short) blockData.length);
            buffer.put(blockData);
        } else {
            buffer.putShort((short) 0);
        }
        
        // Write light data
        if (lightData != null) {
            buffer.putShort((short) lightData.length);
            buffer.put(lightData);
        } else {
            buffer.putShort((short) 0);
        }
        
        // Write biome data
        if (biomeData != null) {
            buffer.putShort((short) biomeData.length);
            buffer.put(biomeData);
        } else {
            buffer.putShort((short) 0);
        }
    }

    @Override
    public int getPayloadSize() {
        // chunkX(4) + chunkY(1) + chunkZ(4) + sectionMask(2) + compression(1) + flags(1) +
        // blockDataSize(2) + blockData.length +
        // lightDataSize(2) + lightData.length +
        // biomeDataSize(2) + biomeData.length
        int size = 4 + 1 + 4 + 2 + 1 + 1;
        size += 2 + (blockData != null ? blockData.length : 0);
        size += 2 + (lightData != null ? lightData.length : 0);
        size += 2 + (biomeData != null ? biomeData.length : 0);
        return size;
    }

    // Getters and setters

    public int getChunkX() {
        return chunkX;
    }

    public void setChunkX(int chunkX) {
        this.chunkX = chunkX;
    }

    public byte getChunkY() {
        return chunkY;
    }

    public void setChunkY(byte chunkY) {
        this.chunkY = chunkY;
    }

    public int getChunkZ() {
        return chunkZ;
    }

    public void setChunkZ(int chunkZ) {
        this.chunkZ = chunkZ;
    }

    public void setChunkPosition(int x, byte y, int z) {
        this.chunkX = x;
        this.chunkY = y;
        this.chunkZ = z;
    }

    public short getSectionMask() {
        return sectionMask;
    }

    public void setSectionMask(short sectionMask) {
        this.sectionMask = sectionMask;
    }

    public boolean hasSection(int sectionY) {
        return (sectionMask & (1 << sectionY)) != 0;
    }

    public void setSectionPresent(int sectionY, boolean present) {
        if (present) {
            sectionMask |= (1 << sectionY);
        } else {
            sectionMask &= ~(1 << sectionY);
        }
    }

    public byte getCompression() {
        return compression;
    }

    public void setCompression(byte compression) {
        this.compression = compression;
    }

    public byte getFlags() {
        return flags;
    }

    public void setFlags(byte flags) {
        this.flags = flags;
    }

    public byte[] getBlockData() {
        return blockData;
    }

    public void setBlockData(byte[] blockData) {
        this.blockData = blockData;
    }

    public byte[] getLightData() {
        return lightData;
    }

    public void setLightData(byte[] lightData) {
        this.lightData = lightData;
    }

    public byte[] getBiomeData() {
        return biomeData;
    }

    public void setBiomeData(byte[] biomeData) {
        this.biomeData = biomeData;
    }

    /**
     * Get the block ID at a specific position within the chunk.
     *
     * @param x the X position within the chunk (0-15)
     * @param y the Y position within the chunk (0-255)
     * @param z the Z position within the chunk (0-15)
     * @return the block ID, or -1 if the section is not present
     */
    public int getBlockId(int x, int y, int z) {
        int sectionY = y >> 4; // y / 16
        if (!hasSection(sectionY)) {
            return -1;
        }
        
        int sectionIndex = y & 0xF; // y % 16
        int index = (sectionY * SECTION_BLOCKS) + (sectionIndex * SECTION_SIZE * SECTION_SIZE) + (z * SECTION_SIZE) + x;
        
        if (blockData == null || index * 3 + 2 >= blockData.length) {
            return -1;
        }
        
        // Block data is stored as [blockId: uint16][blockMeta: uint8]
        return ((blockData[index * 3] & 0xFF) << 8) | (blockData[index * 3 + 1] & 0xFF);
    }

    /**
     * Get the block metadata at a specific position within the chunk.
     *
     * @param x the X position within the chunk (0-15)
     * @param y the Y position within the chunk (0-255)
     * @param z the Z position within the chunk (0-15)
     * @return the block metadata, or -1 if the section is not present
     */
    public int getBlockMeta(int x, int y, int z) {
        int sectionY = y >> 4;
        if (!hasSection(sectionY)) {
            return -1;
        }
        
        int sectionIndex = y & 0xF;
        int index = (sectionY * SECTION_BLOCKS) + (sectionIndex * SECTION_SIZE * SECTION_SIZE) + (z * SECTION_SIZE) + x;
        
        if (blockData == null || index * 3 + 2 >= blockData.length) {
            return -1;
        }
        
        return blockData[index * 3 + 2] & 0xFF;
    }

    @Override
    public String toString() {
        return String.format(
                "ChunkMessage{chunkPos=(%d, %d, %d), sectionMask=0x%04X, compression=%d, flags=0x%02X, blockDataSize=%d, lightDataSize=%d, biomeDataSize=%d}",
                chunkX, chunkY, chunkZ, sectionMask, compression, flags,
                blockData != null ? blockData.length : 0,
                lightData != null ? lightData.length : 0,
                biomeData != null ? biomeData.length : 0
        );
    }
}
