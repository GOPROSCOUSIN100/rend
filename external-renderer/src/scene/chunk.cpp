#include "chunk.hpp"

#include <cstring>
#include <algorithm>

namespace renderbridge {

Chunk::Chunk()
    : m_x(0), m_y(0), m_z(0),
      m_flags(FLAG_EMPTY),
      m_dirty(false) {
    m_blocks.resize(SIZE, BlockInfo());
    m_previousBlocks = m_blocks;
}

Chunk::Chunk(int32_t x, int32_t y, int32_t z)
    : m_x(x), m_y(y), m_z(z),
      m_flags(FLAG_EMPTY),
      m_dirty(false) {
    m_blocks.resize(SIZE, BlockInfo());
    m_previousBlocks = m_blocks;
}

Chunk::~Chunk() {
}

bool Chunk::parseFromPayload(const uint8_t* payload, uint32_t payloadSize) {
    // Expected payload: x(4) + y(4) + z(4) + flags(4) + block data (SIZE * 4)
    // Block data: blockId(2) + blockData(1) + light(1) = 4 bytes per block
    uint32_t expectedSize = 16 + (SIZE * 4); // 16 + 16*16*256*4 = 16 + 262144 = 262160 bytes
    
    if (payloadSize < expectedSize) {
        return false;
    }
    
    uint32_t offset = 0;
    
    // Position (3 * 4 bytes)
    m_x = static_cast<int32_t>(payload[offset]) << 24 |
          static_cast<int32_t>(payload[offset + 1]) << 16 |
          static_cast<int32_t>(payload[offset + 2]) << 8 |
          static_cast<int32_t>(payload[offset + 3]);
    offset += 4;
    
    m_y = static_cast<int32_t>(payload[offset]) << 24 |
          static_cast<int32_t>(payload[offset + 1]) << 16 |
          static_cast<int32_t>(payload[offset + 2]) << 8 |
          static_cast<int32_t>(payload[offset + 3]);
    offset += 4;
    
    m_z = static_cast<int32_t>(payload[offset]) << 24 |
          static_cast<int32_t>(payload[offset + 1]) << 16 |
          static_cast<int32_t>(payload[offset + 2]) << 8 |
          static_cast<int32_t>(payload[offset + 3]);
    offset += 4;
    
    // Flags (4 bytes)
    m_flags = static_cast<uint32_t>(payload[offset]) << 24 |
              static_cast<uint32_t>(payload[offset + 1]) << 16 |
              static_cast<uint32_t>(payload[offset + 2]) << 8 |
              static_cast<uint32_t>(payload[offset + 3]);
    offset += 4;
    
    // Block data
    for (int i = 0; i < SIZE; i++) {
        uint16_t blockId = static_cast<uint16_t>(payload[offset]) << 8 |
                          static_cast<uint16_t>(payload[offset + 1]);
        offset += 2;
        
        uint8_t blockData = payload[offset++];
        uint8_t light = payload[offset++];
        uint8_t skyLight = payload[offset++];
        
        m_blocks[i] = BlockInfo(blockId, blockData, light, skyLight);
    }
    
    // Update previous blocks
    m_previousBlocks = m_blocks;
    m_dirty = false;
    
    // Check if empty
    bool isEmpty = true;
    for (const auto& block : m_blocks) {
        if (!isAirBlock(block)) {
            isEmpty = false;
            break;
        }
    }
    
    if (isEmpty) {
        addFlag(FLAG_EMPTY);
    } else {
        removeFlag(FLAG_EMPTY);
    }
    
    return true;
}

uint32_t Chunk::serializeToBuffer(uint8_t* buffer, uint32_t bufferSize) const {
    uint32_t requiredSize = 16 + (SIZE * 4);
    if (bufferSize < requiredSize) {
        return 0;
    }
    
    uint32_t offset = 0;
    
    // Position (3 * 4 bytes)
    uint32_t intValue = static_cast<uint32_t>(m_x);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(intValue & 0xFF);
    
    intValue = static_cast<uint32_t>(m_y);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(intValue & 0xFF);
    
    intValue = static_cast<uint32_t>(m_z);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((intValue >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(intValue & 0xFF);
    
    // Flags (4 bytes)
    buffer[offset++] = static_cast<uint8_t>((m_flags >> 24) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((m_flags >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((m_flags >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(m_flags & 0xFF);
    
    // Block data
    for (const auto& block : m_blocks) {
        buffer[offset++] = static_cast<uint8_t>((block.blockId >> 8) & 0xFF);
        buffer[offset++] = static_cast<uint8_t>(block.blockId & 0xFF);
        buffer[offset++] = block.blockData;
        buffer[offset++] = block.light;
        buffer[offset++] = block.skyLight;
    }
    
    return offset;
}

int32_t Chunk::getX() const { return m_x; }
int32_t Chunk::getY() const { return m_y; }
int32_t Chunk::getZ() const { return m_z; }

void Chunk::setPosition(int32_t x, int32_t y, int32_t z) {
    m_x = x;
    m_y = y;
    m_z = z;
}

const BlockInfo& Chunk::getBlock(int32_t x, int32_t y, int32_t z) const {
    int index = x + (y * WIDTH) + (z * WIDTH * HEIGHT);
    if (index >= 0 && index < SIZE) {
        return m_blocks[index];
    }
    static BlockInfo airBlock(0, 0, 0, 0);
    return airBlock;
}

void Chunk::setBlock(int32_t x, int32_t y, int32_t z, const BlockInfo& block) {
    int index = x + (y * WIDTH) + (z * WIDTH * HEIGHT);
    if (index >= 0 && index < SIZE) {
        m_blocks[index] = block;
        m_dirty = true;
    }
}

void Chunk::setBlock(int32_t x, int32_t y, int32_t z, uint16_t blockId, uint8_t blockData) {
    BlockInfo block(blockId, blockData, 0, 0);
    setBlock(x, y, z, block);
}

uint32_t Chunk::getFlags() const { return m_flags; }
void Chunk::setFlags(uint32_t flags) { m_flags = flags; }
void Chunk::addFlag(Flag flag) { m_flags |= static_cast<uint32_t>(flag); }
void Chunk::removeFlag(Flag flag) { m_flags &= ~static_cast<uint32_t>(flag); }
bool Chunk::hasFlag(Flag flag) const { return (m_flags & static_cast<uint32_t>(flag)) != 0; }

bool Chunk::isEmpty() const {
    return hasFlag(FLAG_EMPTY);
}

bool Chunk::hasChanged() const {
    return m_dirty || (m_blocks != m_previousBlocks);
}

void Chunk::markUnchanged() {
    m_dirty = false;
    m_previousBlocks = m_blocks;
}

void Chunk::getWorldPosition(int32_t x, int32_t y, int32_t z, float* output) const {
    output[0] = static_cast<float>(m_x * WIDTH + x);
    output[1] = static_cast<float>(m_y * HEIGHT + y);
    output[2] = static_cast<float>(m_z * DEPTH + z);
}

bool Chunk::isBlockVisible(int32_t x, int32_t y, int32_t z) const {
    return !isAirBlock(getBlock(x, y, z));
}

uint8_t Chunk::getLightLevel(int32_t x, int32_t y, int32_t z) const {
    return getBlock(x, y, z).light;
}

uint8_t Chunk::getCombinedLightLevel(int32_t x, int32_t y, int32_t z) const {
    const BlockInfo& block = getBlock(x, y, z);
    return std::min(block.light + block.skyLight, static_cast<uint8_t>(30));
}

void Chunk::updateFrom(const Chunk& other) {
    m_x = other.m_x;
    m_y = other.m_y;
    m_z = other.m_z;
    m_blocks = other.m_blocks;
    m_flags = other.m_flags;
    m_previousBlocks = other.m_previousBlocks;
    m_dirty = other.m_dirty;
}

void Chunk::clear() {
    m_blocks.assign(SIZE, BlockInfo());
    m_flags = FLAG_EMPTY;
    m_dirty = true;
}

bool Chunk::isAirBlock(const BlockInfo& block) {
    // Air block ID is typically 0
    return block.blockId == 0;
}

bool Chunk::isTransparentBlock(const BlockInfo& block) {
    // Common transparent blocks: air, glass, water, etc.
    // This is a simplified implementation
    if (block.blockId == 0) return true; // Air
    if (block.blockId == 20) return true; // Glass
    if (block.blockId == 8) return true; // Water
    if (block.blockId == 9) return true; // Still water
    return false;
}

void Chunk::getBlockTextureCoordinates(const BlockInfo& block, int32_t face, float* output) {
    // Simplified texture coordinate calculation
    // In a real implementation, this would use block texture mappings
    float u = 0.0f;
    float v = 0.0f;
    
    // Different faces have different texture coordinates
    switch (face) {
        case 0: // Front
            u = 0.0f; v = 0.0f;
            break;
        case 1: // Back
            u = 0.0f; v = 0.0f;
            break;
        case 2: // Left
            u = 0.0f; v = 0.0f;
            break;
        case 3: // Right
            u = 0.0f; v = 0.0f;
            break;
        case 4: // Top
            u = 0.0f; v = 0.0f;
            break;
        case 5: // Bottom
            u = 0.0f; v = 0.0f;
            break;
    }
    
    output[0] = u;
    output[1] = v;
}

void Chunk::getBlockColor(const BlockInfo& block, uint8_t light, uint8_t* output) {
    // Simplified color calculation based on light level
    // In a real implementation, this would use block color mappings
    float lightFactor = static_cast<float>(light) / 15.0f;
    
    // Base color (white for now)
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    
    // Apply light
    r = static_cast<uint8_t>(r * lightFactor);
    g = static_cast<uint8_t>(g * lightFactor);
    b = static_cast<uint8_t>(b * lightFactor);
    
    output[0] = r;
    output[1] = g;
    output[2] = b;
    output[3] = 255; // Alpha
}

bool Chunk::generateMesh(std::vector<VoxelVertex>& vertices, std::vector<uint16_t>& indices) const {
    // Check if chunk is empty
    if (isEmpty()) {
        return false;
    }
    
    // Clear output
    vertices.clear();
    indices.clear();
    
    // Iterate through all blocks
    for (int z = 0; z < DEPTH; z++) {
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                if (!isBlockVisible(x, y, z)) {
                    continue;
                }
                
                const BlockInfo& block = getBlock(x, y, z);
                uint8_t light = getCombinedLightLevel(x, y, z);
                
                // Get block color
                uint8_t color[4];
                getBlockColor(block, light, color);
                
                // Get world position
                float worldPos[3];
                getWorldPosition(x, y, z, worldPos);
                
                // Generate vertices for each visible face
                // Front face
                if (x > 0 && !isBlockVisible(x - 1, y, z)) {
                    // Add vertices for front face
                    size_t baseIndex = vertices.size();
                    
                    // Bottom-left
                    vertices.push_back({worldPos[0], worldPos[1], worldPos[2],
                                        0.0f, 0.0f, -1.0f,
                                        0.0f, 0.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    // Top-left
                    vertices.push_back({worldPos[0], worldPos[1] + 1.0f, worldPos[2],
                                        0.0f, 0.0f, -1.0f,
                                        0.0f, 1.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    // Top-right
                    vertices.push_back({worldPos[0] + 1.0f, worldPos[1] + 1.0f, worldPos[2],
                                        0.0f, 0.0f, -1.0f,
                                        1.0f, 1.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    // Bottom-right
                    vertices.push_back({worldPos[0] + 1.0f, worldPos[1], worldPos[2],
                                        0.0f, 0.0f, -1.0f,
                                        1.0f, 0.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    // Add indices (two triangles)
                    indices.push_back(static_cast<uint16_t>(baseIndex));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 1));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 2));
                    
                    indices.push_back(static_cast<uint16_t>(baseIndex));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 2));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 3));
                }
                
                // Back face
                if (x < WIDTH - 1 && !isBlockVisible(x + 1, y, z)) {
                    size_t baseIndex = vertices.size();
                    
                    vertices.push_back({worldPos[0] + 1.0f, worldPos[1], worldPos[2] + 1.0f,
                                        0.0f, 0.0f, 1.0f,
                                        0.0f, 0.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    vertices.push_back({worldPos[0] + 1.0f, worldPos[1] + 1.0f, worldPos[2] + 1.0f,
                                        0.0f, 0.0f, 1.0f,
                                        0.0f, 1.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    vertices.push_back({worldPos[0], worldPos[1] + 1.0f, worldPos[2] + 1.0f,
                                        0.0f, 0.0f, 1.0f,
                                        1.0f, 1.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    vertices.push_back({worldPos[0], worldPos[1], worldPos[2] + 1.0f,
                                        0.0f, 0.0f, 1.0f,
                                        1.0f, 0.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    indices.push_back(static_cast<uint16_t>(baseIndex));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 1));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 2));
                    
                    indices.push_back(static_cast<uint16_t>(baseIndex));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 2));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 3));
                }
                
                // Left face
                if (z > 0 && !isBlockVisible(x, y, z - 1)) {
                    size_t baseIndex = vertices.size();
                    
                    vertices.push_back({worldPos[0], worldPos[1], worldPos[2],
                                        -1.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    vertices.push_back({worldPos[0], worldPos[1] + 1.0f, worldPos[2],
                                        -1.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    vertices.push_back({worldPos[0], worldPos[1] + 1.0f, worldPos[2] + 1.0f,
                                        -1.0f, 0.0f, 0.0f,
                                        1.0f, 1.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    vertices.push_back({worldPos[0], worldPos[1], worldPos[2] + 1.0f,
                                        -1.0f, 0.0f, 0.0f,
                                        1.0f, 0.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    indices.push_back(static_cast<uint16_t>(baseIndex));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 1));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 2));
                    
                    indices.push_back(static_cast<uint16_t>(baseIndex));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 2));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 3));
                }
                
                // Right face
                if (z < DEPTH - 1 && !isBlockVisible(x, y, z + 1)) {
                    size_t baseIndex = vertices.size();
                    
                    vertices.push_back({worldPos[0] + 1.0f, worldPos[1], worldPos[2] + 1.0f,
                                        1.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    vertices.push_back({worldPos[0] + 1.0f, worldPos[1] + 1.0f, worldPos[2] + 1.0f,
                                        1.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    vertices.push_back({worldPos[0] + 1.0f, worldPos[1] + 1.0f, worldPos[2],
                                        1.0f, 0.0f, 0.0f,
                                        1.0f, 1.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    vertices.push_back({worldPos[0] + 1.0f, worldPos[1], worldPos[2],
                                        1.0f, 0.0f, 0.0f,
                                        1.0f, 0.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    indices.push_back(static_cast<uint16_t>(baseIndex));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 1));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 2));
                    
                    indices.push_back(static_cast<uint16_t>(baseIndex));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 2));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 3));
                }
                
                // Top face
                if (y < HEIGHT - 1 && !isBlockVisible(x, y + 1, z)) {
                    size_t baseIndex = vertices.size();
                    
                    vertices.push_back({worldPos[0], worldPos[1] + 1.0f, worldPos[2] + 1.0f,
                                        0.0f, 1.0f, 0.0f,
                                        0.0f, 0.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    vertices.push_back({worldPos[0], worldPos[1] + 1.0f, worldPos[2],
                                        0.0f, 1.0f, 0.0f,
                                        0.0f, 1.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    vertices.push_back({worldPos[0] + 1.0f, worldPos[1] + 1.0f, worldPos[2],
                                        0.0f, 1.0f, 0.0f,
                                        1.0f, 1.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    vertices.push_back({worldPos[0] + 1.0f, worldPos[1] + 1.0f, worldPos[2] + 1.0f,
                                        0.0f, 1.0f, 0.0f,
                                        1.0f, 0.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    indices.push_back(static_cast<uint16_t>(baseIndex));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 1));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 2));
                    
                    indices.push_back(static_cast<uint16_t>(baseIndex));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 2));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 3));
                }
                
                // Bottom face
                if (y > 0 && !isBlockVisible(x, y - 1, z)) {
                    size_t baseIndex = vertices.size();
                    
                    vertices.push_back({worldPos[0] + 1.0f, worldPos[1], worldPos[2] + 1.0f,
                                        0.0f, -1.0f, 0.0f,
                                        0.0f, 0.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    vertices.push_back({worldPos[0] + 1.0f, worldPos[1], worldPos[2],
                                        0.0f, -1.0f, 0.0f,
                                        0.0f, 1.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    vertices.push_back({worldPos[0], worldPos[1], worldPos[2],
                                        0.0f, -1.0f, 0.0f,
                                        1.0f, 1.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    vertices.push_back({worldPos[0], worldPos[1], worldPos[2] + 1.0f,
                                        0.0f, -1.0f, 0.0f,
                                        1.0f, 0.0f,
                                        color[0], color[1], color[2], color[3]});
                    
                    indices.push_back(static_cast<uint16_t>(baseIndex));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 1));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 2));
                    
                    indices.push_back(static_cast<uint16_t>(baseIndex));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 2));
                    indices.push_back(static_cast<uint16_t>(baseIndex + 3));
                }
            }
        }
    }
    
    return !vertices.empty();
}

void Chunk::generateSimpleMesh(std::vector<VoxelVertex>& vertices, std::vector<uint16_t>& indices) const {
    // Generate a simple cube for the entire chunk
    vertices.clear();
    indices.clear();
    
    float worldPos[3];
    getWorldPosition(0, 0, 0, worldPos);
    
    // Simple cube vertices (8 vertices)
    // Front face
    vertices.push_back({worldPos[0], worldPos[1], worldPos[2],
                        0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 255, 255, 255, 255});
    vertices.push_back({worldPos[0], worldPos[1] + 16.0f, worldPos[2],
                        0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 255, 255, 255, 255});
    vertices.push_back({worldPos[0] + 16.0f, worldPos[1] + 16.0f, worldPos[2],
                        0.0f, 0.0f, -1.0f, 1.0f, 1.0f, 255, 255, 255, 255});
    vertices.push_back({worldPos[0] + 16.0f, worldPos[1], worldPos[2],
                        0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 255, 255, 255, 255});
    
    // Back face
    vertices.push_back({worldPos[0] + 16.0f, worldPos[1], worldPos[2] + 16.0f,
                        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 255, 255, 255, 255});
    vertices.push_back({worldPos[0] + 16.0f, worldPos[1] + 16.0f, worldPos[2] + 16.0f,
                        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 255, 255, 255, 255});
    vertices.push_back({worldPos[0], worldPos[1] + 16.0f, worldPos[2] + 16.0f,
                        0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 255, 255, 255, 255});
    vertices.push_back({worldPos[0], worldPos[1], worldPos[2] + 16.0f,
                        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 255, 255, 255, 255});
    
    // Create indices for all faces
    // Front face
    indices.push_back(0); indices.push_back(1); indices.push_back(2);
    indices.push_back(0); indices.push_back(2); indices.push_back(3);
    
    // Back face
    indices.push_back(4); indices.push_back(5); indices.push_back(6);
    indices.push_back(4); indices.push_back(6); indices.push_back(7);
    
    // Left face
    indices.push_back(7); indices.push_back(6); indices.push_back(2);
    indices.push_back(7); indices.push_back(2); indices.push_back(1);
    
    // Right face
    indices.push_back(3); indices.push_back(2); indices.push_back(5);
    indices.push_back(3); indices.push_back(5); indices.push_back(4);
    
    // Top face
    indices.push_back(1); indices.push_back(6); indices.push_back(5);
    indices.push_back(1); indices.push_back(5); indices.push_back(2);
    
    // Bottom face
    indices.push_back(0); indices.push_back(3); indices.push_back(4);
    indices.push_back(0); indices.push_back(4); indices.push_back(7);
}

} // namespace renderbridge
