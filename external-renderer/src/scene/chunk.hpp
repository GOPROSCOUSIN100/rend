#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <array>

namespace renderbridge {

/**
 * @struct BlockInfo
 * @brief Information about a single block
 */
struct BlockInfo {
    uint16_t blockId;      // Block ID
    uint8_t blockData;     // Block metadata/data
    uint8_t light;         // Light level (0-15)
    uint8_t skyLight;      // Sky light level (0-15)
    
    // Default constructor
    BlockInfo() : blockId(0), blockData(0), light(0), skyLight(0) {}
    
    // Parameterized constructor
    BlockInfo(uint16_t id, uint8_t data, uint8_t l, uint8_t sl) 
        : blockId(id), blockData(data), light(l), skyLight(sl) {}
};

/**
 * @struct VoxelVertex
 * @brief Vertex data for voxel rendering
 */
struct VoxelVertex {
    float x, y, z;          // Position
    float nx, ny, nz;      // Normal
    float u, v;            // Texture coordinates
    uint8_t r, g, b, a;    // Color
    
    // Default constructor
    VoxelVertex() 
        : x(0), y(0), z(0), nx(0), ny(0), nz(0), u(0), v(0), r(255), g(255), b(255), a(255) {}
};

/**
 * @class Chunk
 * @brief Represents a chunk in the Minecraft world
 * 
 * A chunk is a 16x256x16 section of the world containing block data.
 * This class stores the block data and provides methods for mesh generation.
 */
class Chunk {
public:
    // Chunk dimensions
    static constexpr int WIDTH = 16;
    static constexpr int HEIGHT = 256;
    static constexpr int DEPTH = 16;
    static constexpr int SIZE = WIDTH * HEIGHT * DEPTH;
    
    // Chunk flags
    enum Flag : uint32_t {
        NONE = 0x00000000,
        EMPTY = 0x00000001,      // Chunk is empty (all air)
        LOADED = 0x00000002,     // Chunk is loaded
        GENERATED = 0x00000004,  // Chunk mesh is generated
        DIRTY = 0x00000008,      // Chunk needs mesh regeneration
        VISIBLE = 0x00000010      // Chunk is visible
    };

public:
    /**
     * @brief Constructor
     */
    Chunk();
    
    /**
     * @brief Constructor with position
     * @param x X position
     * @param y Y position (chunk Y)
     * @param z Z position
     */
    Chunk(int32_t x, int32_t y, int32_t z);
    
    /**
     * @brief Destructor
     */
    ~Chunk();
    
    /**
     * @brief Parse chunk data from a message payload
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     * @return True if parsing succeeded
     */
    bool parseFromPayload(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Serialize chunk data to a buffer
     * @param buffer Output buffer
     * @param bufferSize Size of the buffer
     * @return Number of bytes written
     */
    uint32_t serializeToBuffer(uint8_t* buffer, uint32_t bufferSize) const;
    
    // Position accessors
    int32_t getX() const;
    int32_t getY() const;
    int32_t getZ() const;
    void setPosition(int32_t x, int32_t y, int32_t z);
    
    // Block data accessors
    const BlockInfo& getBlock(int32_t x, int32_t y, int32_t z) const;
    void setBlock(int32_t x, int32_t y, int32_t z, const BlockInfo& block);
    void setBlock(int32_t x, int32_t y, int32_t z, uint16_t blockId, uint8_t blockData);
    
    // Flag accessors
    uint32_t getFlags() const;
    void setFlags(uint32_t flags);
    void addFlag(Flag flag);
    void removeFlag(Flag flag);
    bool hasFlag(Flag flag) const;
    
    // Mesh generation
    /**
     * @brief Generate the mesh for this chunk
     * @param vertices Output: vertex data
     * @param indices Output: index data
     * @return True if mesh generation succeeded
     */
    bool generateMesh(std::vector<VoxelVertex>& vertices, std::vector<uint16_t>& indices) const;
    
    /**
     * @brief Generate a simple mesh (for debugging)
     * @param vertices Output: vertex data
     * @param indices Output: index data
     */
    void generateSimpleMesh(std::vector<VoxelVertex>& vertices, std::vector<uint16_t>& indices) const;
    
    /**
     * @brief Check if the chunk is empty
     * @return True if the chunk is empty
     */
    bool isEmpty() const;
    
    /**
     * @brief Check if the chunk has changed since the last mesh generation
     * @return True if the chunk has changed
     */
    bool hasChanged() const;
    
    /**
     * @brief Mark the chunk as unchanged
     */
    void markUnchanged();
    
    /**
     * @brief Get the world position of a block in this chunk
     * @param x Block X position (0-15)
     * @param y Block Y position (0-255)
     * @param z Block Z position (0-15)
     * @param output Output world position (3 floats)
     */
    void getWorldPosition(int32_t x, int32_t y, int32_t z, float* output) const;
    
    /**
     * @brief Check if a block is visible (not air)
     * @param x Block X position
     * @param y Block Y position
     * @param z Block Z position
     * @return True if the block is visible
     */
    bool isBlockVisible(int32_t x, int32_t y, int32_t z) const;
    
    /**
     * @brief Get the light level at a position
     * @param x Block X position
     * @param y Block Y position
     * @param z Block Z position
     * @return Light level (0-15)
     */
    uint8_t getLightLevel(int32_t x, int32_t y, int32_t z) const;
    
    /**
     * @brief Get the combined light level at a position
     * @param x Block X position
     * @param y Block Y position
     * @param z Block Z position
     * @return Combined light level (0-30)
     */
    uint8_t getCombinedLightLevel(int32_t x, int32_t y, int32_t z) const;
    
    /**
     * @brief Update the chunk from another chunk
     * @param other Other chunk
     */
    void updateFrom(const Chunk& other);
    
    /**
     * @brief Clear the chunk data
     */
    void clear();

private:
    int32_t m_x;
    int32_t m_y;
    int32_t m_z;
    
    // Block data
    std::vector<BlockInfo> m_blocks;
    
    // Flags
    uint32_t m_flags;
    
    // Dirty flag for change detection
    mutable bool m_dirty;
    
    // Previous block data for change detection
    mutable std::vector<BlockInfo> m_previousBlocks;
    
    /**
     * @brief Check if a block is air
     * @param block Block to check
     * @return True if the block is air
     */
    static bool isAirBlock(const BlockInfo& block);
    
    /**
     * @brief Check if a block is transparent
     * @param block Block to check
     * @return True if the block is transparent
     */
    static bool isTransparentBlock(const BlockInfo& block);
    
    /**
     * @brief Get the texture coordinates for a block face
     * @param block Block to get texture for
     * @param face Face index (0-5: front, back, left, right, top, bottom)
     * @param output Output texture coordinates (u, v)
     */
    static void getBlockTextureCoordinates(const BlockInfo& block, int32_t face, float* output);
    
    /**
     * @brief Get the color for a block based on light level
     * @param block Block to get color for
     * @param light Light level (0-30)
     * @param output Output color (r, g, b, a)
     */
    static void getBlockColor(const BlockInfo& block, uint8_t light, uint8_t* output);
};

} // namespace renderbridge
