#include "mesh_builder.hpp"
#include "scene.hpp"
#include "chunk.hpp"
#include "entity.hpp"
#include "utils/logger.hpp"

#include <bgfx/bgfx.h>
#include <bx/math.h>
#include <bx/hash.h>

#include <cstring>
#include <algorithm>

namespace renderbridge {

// Static logger
static Logger s_logger("MeshBuilder");

MeshBuilder::MeshBuilder()
    : m_voxelVertexDecl(),
      m_entityVertexDecl(),
      m_particleVertexDecl(),
      m_chunkMeshCount(0),
      m_entityMeshCount(0),
      m_totalVertices(0),
      m_totalIndices(0)
{
    // Initialize particle vertex declaration
    m_particleVertexDecl
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float) // UV
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float) // Color
        .add(bgfx::Attrib::TexCoord1, 1, bgfx::AttribType::Float) // Size
        .add(bgfx::Attrib::TexCoord2, 1, bgfx::AttribType::Float) // Rotation
        .add(bgfx::Attrib::TexCoord3, 1, bgfx::AttribType::Float) // Texture index
        .end();
}

MeshBuilder::~MeshBuilder()
{
    shutdown();
}

bool MeshBuilder::initialize(bgfx::VertexDecl voxelVertexDecl, bgfx::VertexDecl entityVertexDecl)
{
    s_logger.info("Initializing mesh builder...");
    
    m_voxelVertexDecl = voxelVertexDecl;
    m_entityVertexDecl = entityVertexDecl;
    
    // Validate vertex declarations
    if (m_voxelVertexDecl.getStride() == 0 || m_entityVertexDecl.getStride() == 0) {
        s_logger.error("Invalid vertex declarations");
        return false;
    }
    
    s_logger.info("Mesh builder initialized successfully");
    return true;
}

void MeshBuilder::shutdown()
{
    s_logger.info("Shutting down mesh builder...");
    
    clearCache();
    
    s_logger.info("Mesh builder shut down");
}

// ========================================================================
// Chunk mesh building
// ========================================================================

MeshBuilder::ChunkMeshData MeshBuilder::buildChunkMesh(const Chunk* chunk)
{
    ChunkMeshData meshData;
    meshData.vbo.idx = bgfx::kInvalidHandle;
    meshData.ibo.idx = bgfx::kInvalidHandle;
    meshData.vertexCount = 0;
    meshData.indexCount = 0;
    
    if (!chunk) {
        return meshData;
    }
    
    // Generate vertex and index data
    std::vector<float> vertices;
    std::vector<uint16_t> indices;
    float boundingBoxMin[3] = { 0.0f, 0.0f, 0.0f };
    float boundingBoxMax[3] = { 0.0f, 0.0f, 0.0f };
    
    generateChunkVertexData(chunk, vertices, indices, boundingBoxMin, boundingBoxMax);
    
    if (vertices.empty()) {
        return meshData;
    }
    
    // Create vertex buffer
    meshData.vbo = bgfx::createVertexBuffer(
        bgfx::makeRef(vertices.data(), vertices.size() * sizeof(float)),
        m_voxelVertexDecl
    );
    
    if (meshData.vbo.idx == bgfx::kInvalidHandle) {
        s_logger.error("Failed to create chunk vertex buffer");
        return meshData;
    }
    
    // Create index buffer
    if (!indices.empty()) {
        meshData.ibo = bgfx::createIndexBuffer(
            bgfx::makeRef(indices.data(), indices.size() * sizeof(uint16_t))
        );
        
        if (meshData.ibo.idx == bgfx::kInvalidHandle) {
            s_logger.error("Failed to create chunk index buffer");
            bgfx::destroy(meshData.vbo);
            meshData.vbo.idx = bgfx::kInvalidHandle;
            return meshData;
        }
    }
    
    meshData.vertexCount = static_cast<uint32_t>(vertices.size() / (m_voxelVertexDecl.getStride() / sizeof(float)));
    meshData.indexCount = static_cast<uint32_t>(indices.size());
    
    std::memcpy(meshData.boundingBoxMin, boundingBoxMin, sizeof(float) * 3);
    std::memcpy(meshData.boundingBoxMax, boundingBoxMax, sizeof(float) * 3);
    
    // Cache the mesh
    CachedChunkMesh cachedMesh;
    cachedMesh.meshData = meshData;
    cachedMesh.chunkHash = calculateChunkHash(chunk);
    m_chunkMeshCache[chunk] = cachedMesh;
    
    // Update statistics
    m_chunkMeshCount++;
    m_totalVertices += meshData.vertexCount;
    m_totalIndices += meshData.indexCount;
    
    return meshData;
}

void MeshBuilder::updateChunkMesh(const Chunk* chunk, ChunkMeshData& meshData)
{
    if (!chunk) return;
    
    // Check if mesh needs update
    auto it = m_chunkMeshCache.find(chunk);
    if (it != m_chunkMeshCache.end()) {
        uint64_t currentHash = calculateChunkHash(chunk);
        if (currentHash == it->second.chunkHash) {
            return; // No change
        }
    }
    
    // Generate new vertex and index data
    std::vector<float> vertices;
    std::vector<uint16_t> indices;
    float boundingBoxMin[3] = { 0.0f, 0.0f, 0.0f };
    float boundingBoxMax[3] = { 0.0f, 0.0f, 0.0f };
    
    generateChunkVertexData(chunk, vertices, indices, boundingBoxMin, boundingBoxMax);
    
    if (vertices.empty()) {
        return;
    }
    
    // Update vertex buffer
    if (meshData.vbo.idx != bgfx::kInvalidHandle) {
        bgfx::updateVertexBuffer(
            meshData.vbo,
            0,
            bgfx::makeRef(vertices.data(), vertices.size() * sizeof(float))
        );
    }
    
    // Update index buffer
    if (meshData.ibo.idx != bgfx::kInvalidHandle && !indices.empty()) {
        bgfx::updateIndexBuffer(
            meshData.ibo,
            0,
            bgfx::makeRef(indices.data(), indices.size() * sizeof(uint16_t))
        );
    }
    
    meshData.vertexCount = static_cast<uint32_t>(vertices.size() / (m_voxelVertexDecl.getStride() / sizeof(float)));
    meshData.indexCount = static_cast<uint32_t>(indices.size());
    
    std::memcpy(meshData.boundingBoxMin, boundingBoxMin, sizeof(float) * 3);
    std::memcpy(meshData.boundingBoxMax, boundingBoxMax, sizeof(float) * 3);
    
    // Update cache
    if (it != m_chunkMeshCache.end()) {
        it->second.meshData = meshData;
        it->second.chunkHash = calculateChunkHash(chunk);
    } else {
        CachedChunkMesh cachedMesh;
        cachedMesh.meshData = meshData;
        cachedMesh.chunkHash = calculateChunkHash(chunk);
        m_chunkMeshCache[chunk] = cachedMesh;
        m_chunkMeshCount++;
    }
    
    // Update statistics
    m_totalVertices = 0;
    m_totalIndices = 0;
    for (auto& pair : m_chunkMeshCache) {
        m_totalVertices += pair.second.meshData.vertexCount;
        m_totalIndices += pair.second.meshData.indexCount;
    }
    for (auto& pair : m_entityMeshCache) {
        m_totalVertices += pair.second.meshData.vertexCount;
        m_totalIndices += pair.second.meshData.indexCount;
    }
}

void MeshBuilder::destroyChunkMesh(ChunkMeshData& meshData)
{
    if (meshData.vbo.idx != bgfx::kInvalidHandle) {
        bgfx::destroy(meshData.vbo);
        meshData.vbo.idx = bgfx::kInvalidHandle;
    }
    if (meshData.ibo.idx != bgfx::kInvalidHandle) {
        bgfx::destroy(meshData.ibo);
        meshData.ibo.idx = bgfx::kInvalidHandle;
    }
    meshData.vertexCount = 0;
    meshData.indexCount = 0;
}

// ========================================================================
// Entity mesh building
// ========================================================================

MeshBuilder::EntityMeshData MeshBuilder::buildEntityMesh(const Entity* entity)
{
    EntityMeshData meshData;
    meshData.vbo.idx = bgfx::kInvalidHandle;
    meshData.ibo.idx = bgfx::kInvalidHandle;
    meshData.vertexCount = 0;
    meshData.indexCount = 0;
    
    if (!entity) {
        return meshData;
    }
    
    // Generate vertex and index data
    std::vector<float> vertices;
    std::vector<uint16_t> indices;
    float boundingBoxMin[3] = { 0.0f, 0.0f, 0.0f };
    float boundingBoxMax[3] = { 0.0f, 0.0f, 0.0f };
    
    generateEntityVertexData(entity, vertices, indices, boundingBoxMin, boundingBoxMax);
    
    if (vertices.empty()) {
        // Create a simple cube mesh for entities without custom meshes
        const float cubeVertices[] = {
            // Front face
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        };
        
        const uint16_t cubeIndices[] = {
            0, 1, 2, 0, 2, 3
        };
        
        vertices.assign(cubeVertices, cubeVertices + 32);
        indices.assign(cubeIndices, cubeIndices + 6);
        
        boundingBoxMin[0] = -0.5f; boundingBoxMin[1] = -0.5f; boundingBoxMin[2] = -0.5f;
        boundingBoxMax[0] = 0.5f; boundingBoxMax[1] = 0.5f; boundingBoxMax[2] = 0.5f;
    }
    
    // Create vertex buffer
    meshData.vbo = bgfx::createVertexBuffer(
        bgfx::makeRef(vertices.data(), vertices.size() * sizeof(float)),
        m_entityVertexDecl
    );
    
    if (meshData.vbo.idx == bgfx::kInvalidHandle) {
        s_logger.error("Failed to create entity vertex buffer");
        return meshData;
    }
    
    // Create index buffer
    if (!indices.empty()) {
        meshData.ibo = bgfx::createIndexBuffer(
            bgfx::makeRef(indices.data(), indices.size() * sizeof(uint16_t))
        );
        
        if (meshData.ibo.idx == bgfx::kInvalidHandle) {
            s_logger.error("Failed to create entity index buffer");
            bgfx::destroy(meshData.vbo);
            meshData.vbo.idx = bgfx::kInvalidHandle;
            return meshData;
        }
    }
    
    meshData.vertexCount = static_cast<uint32_t>(vertices.size() / (m_entityVertexDecl.getStride() / sizeof(float)));
    meshData.indexCount = static_cast<uint32_t>(indices.size());
    
    std::memcpy(meshData.boundingBoxMin, boundingBoxMin, sizeof(float) * 3);
    std::memcpy(meshData.boundingBoxMax, boundingBoxMax, sizeof(float) * 3);
    
    // Cache the mesh
    CachedEntityMesh cachedMesh;
    cachedMesh.meshData = meshData;
    cachedMesh.entityHash = calculateEntityHash(entity);
    m_entityMeshCache[entity] = cachedMesh;
    
    // Update statistics
    m_entityMeshCount++;
    m_totalVertices += meshData.vertexCount;
    m_totalIndices += meshData.indexCount;
    
    return meshData;
}

void MeshBuilder::updateEntityMesh(const Entity* entity, EntityMeshData& meshData)
{
    if (!entity) return;
    
    // Check if mesh needs update
    auto it = m_entityMeshCache.find(entity);
    if (it != m_entityMeshCache.end()) {
        uint64_t currentHash = calculateEntityHash(entity);
        if (currentHash == it->second.entityHash) {
            return; // No change
        }
    }
    
    // Generate new vertex and index data
    std::vector<float> vertices;
    std::vector<uint16_t> indices;
    float boundingBoxMin[3] = { 0.0f, 0.0f, 0.0f };
    float boundingBoxMax[3] = { 0.0f, 0.0f, 0.0f };
    
    generateEntityVertexData(entity, vertices, indices, boundingBoxMin, boundingBoxMax);
    
    if (vertices.empty()) {
        return;
    }
    
    // Update vertex buffer
    if (meshData.vbo.idx != bgfx::kInvalidHandle) {
        bgfx::updateVertexBuffer(
            meshData.vbo,
            0,
            bgfx::makeRef(vertices.data(), vertices.size() * sizeof(float))
        );
    }
    
    // Update index buffer
    if (meshData.ibo.idx != bgfx::kInvalidHandle && !indices.empty()) {
        bgfx::updateIndexBuffer(
            meshData.ibo,
            0,
            bgfx::makeRef(indices.data(), indices.size() * sizeof(uint16_t))
        );
    }
    
    meshData.vertexCount = static_cast<uint32_t>(vertices.size() / (m_entityVertexDecl.getStride() / sizeof(float)));
    meshData.indexCount = static_cast<uint32_t>(indices.size());
    
    std::memcpy(meshData.boundingBoxMin, boundingBoxMin, sizeof(float) * 3);
    std::memcpy(meshData.boundingBoxMax, boundingBoxMax, sizeof(float) * 3);
    
    // Update cache
    if (it != m_entityMeshCache.end()) {
        it->second.meshData = meshData;
        it->second.entityHash = calculateEntityHash(entity);
    } else {
        CachedEntityMesh cachedMesh;
        cachedMesh.meshData = meshData;
        cachedMesh.entityHash = calculateEntityHash(entity);
        m_entityMeshCache[entity] = cachedMesh;
        m_entityMeshCount++;
    }
    
    // Update statistics
    m_totalVertices = 0;
    m_totalIndices = 0;
    for (auto& pair : m_chunkMeshCache) {
        m_totalVertices += pair.second.meshData.vertexCount;
        m_totalIndices += pair.second.meshData.indexCount;
    }
    for (auto& pair : m_entityMeshCache) {
        m_totalVertices += pair.second.meshData.vertexCount;
        m_totalIndices += pair.second.meshData.indexCount;
    }
}

void MeshBuilder::destroyEntityMesh(EntityMeshData& meshData)
{
    if (meshData.vbo.idx != bgfx::kInvalidHandle) {
        bgfx::destroy(meshData.vbo);
        meshData.vbo.idx = bgfx::kInvalidHandle;
    }
    if (meshData.ibo.idx != bgfx::kInvalidHandle) {
        bgfx::destroy(meshData.ibo);
        meshData.ibo.idx = bgfx::kInvalidHandle;
    }
    meshData.vertexCount = 0;
    meshData.indexCount = 0;
}

// ========================================================================
// Particle mesh building
// ========================================================================

bgfx::VertexBufferHandle MeshBuilder::buildParticleBuffer(const float* particles, uint32_t particleCount)
{
    if (!particles || particleCount == 0) {
        return bgfx::VertexBufferHandle();
    }
    
    // Each particle has 9 floats (position, uv, color, size, rotation, texIndex)
    // But we're creating a quad for each particle (4 vertices)
    // For now, assume particles array already contains the expanded vertex data
    
    return bgfx::createVertexBuffer(
        bgfx::makeRef(particles, particleCount * 9 * sizeof(float)),
        m_particleVertexDecl
    );
}

void MeshBuilder::updateParticleBuffer(bgfx::VertexBufferHandle vbo, const float* particles, uint32_t particleCount)
{
    if (vbo.idx == bgfx::kInvalidHandle || !particles || particleCount == 0) {
        return;
    }
    
    bgfx::updateVertexBuffer(
        vbo,
        0,
        bgfx::makeRef(particles, particleCount * 9 * sizeof(float))
    );
}

void MeshBuilder::destroyParticleBuffer(bgfx::VertexBufferHandle vbo)
{
    if (vbo.idx != bgfx::kInvalidHandle) {
        bgfx::destroy(vbo);
    }
}

// ========================================================================
// Mesh caching
// ========================================================================

void MeshBuilder::clearCache()
{
    // Destroy all chunk meshes
    for (auto& pair : m_chunkMeshCache) {
        destroyChunkMesh(pair.second.meshData);
    }
    m_chunkMeshCache.clear();
    
    // Destroy all entity meshes
    for (auto& pair : m_entityMeshCache) {
        destroyEntityMesh(pair.second.meshData);
    }
    m_entityMeshCache.clear();
    
    // Reset statistics
    m_chunkMeshCount = 0;
    m_entityMeshCount = 0;
    m_totalVertices = 0;
    m_totalIndices = 0;
}

std::string MeshBuilder::getCacheStatistics() const
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
        "MeshBuilder Cache: %u chunks, %u entities, %u vertices, %u indices",
        m_chunkMeshCount, m_entityMeshCount, m_totalVertices, m_totalIndices);
    return std::string(buffer);
}

// ========================================================================
// Private methods
// ========================================================================

void MeshBuilder::generateChunkVertexData(const Chunk* chunk, 
                                         std::vector<float>& vertices, 
                                         std::vector<uint16_t>& indices,
                                         float boundingBoxMin[3],
                                         float boundingBoxMax[3])
{
    vertices.clear();
    indices.clear();
    
    if (!chunk) return;
    
    // Initialize bounding box
    boundingBoxMin[0] = 0.0f; boundingBoxMin[1] = 0.0f; boundingBoxMin[2] = 0.0f;
    boundingBoxMax[0] = 0.0f; boundingBoxMax[1] = 0.0f; boundingBoxMax[2] = 0.0f;
    
    const auto& blocks = chunk->getBlocks();
    int32_t chunkX = static_cast<int32_t>(chunk->getPositionX());
    int32_t chunkY = static_cast<int32_t>(chunk->getPositionY());
    int32_t chunkZ = static_cast<int32_t>(chunk->getPositionZ());
    
    // Vertex offset for this chunk
    uint32_t vertexOffset = 0;
    
    // Iterate through all blocks in the chunk
    for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
        for (int z = 0; z < Chunk::CHUNK_DEPTH; ++z) {
            for (int x = 0; x < Chunk::CHUNK_WIDTH; ++x) {
                const Chunk::BlockInfo& block = blocks[x + y * Chunk::CHUNK_WIDTH + z * Chunk::CHUNK_WIDTH * Chunk::CHUNK_HEIGHT];
                
                // Skip air blocks
                if (block.type == Chunk::BlockType::AIR) continue;
                
                // Calculate world position
                float worldX = chunkX + x;
                float worldY = chunkY + y;
                float worldZ = chunkZ + z;
                
                // Update bounding box
                if (x == 0 && y == 0 && z == 0) {
                    boundingBoxMin[0] = worldX - 0.5f;
                    boundingBoxMin[1] = worldY - 0.5f;
                    boundingBoxMin[2] = worldZ - 0.5f;
                    boundingBoxMax[0] = worldX + 0.5f;
                    boundingBoxMax[1] = worldY + 0.5f;
                    boundingBoxMax[2] = worldZ + 0.5f;
                } else {
                    boundingBoxMin[0] = std::min(boundingBoxMin[0], worldX - 0.5f);
                    boundingBoxMin[1] = std::min(boundingBoxMin[1], worldY - 0.5f);
                    boundingBoxMin[2] = std::min(boundingBoxMin[2], worldZ - 0.5f);
                    boundingBoxMax[0] = std::max(boundingBoxMax[0], worldX + 0.5f);
                    boundingBoxMax[1] = std::max(boundingBoxMax[1], worldY + 0.5f);
                    boundingBoxMax[2] = std::max(boundingBoxMax[2], worldZ + 0.5f);
                }
                
                // Define face vertices and normals
                const float faceVertices[6][4][3] = {
                    // Front face (Z+)
                    {{-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}},
                    // Back face (Z-)
                    {{-0.5f, -0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f}},
                    // Left face (X-)
                    {{-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f, -0.5f}},
                    // Right face (X+)
                    {{ 0.5f, -0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f}},
                    // Top face (Y+)
                    {{-0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}},
                    // Bottom face (Y-)
                    {{-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f, -0.5f}},
                };
                
                const float faceNormals[6][3] = {
                    {0.0f, 0.0f, 1.0f},  // Front
                    {0.0f, 0.0f, -1.0f}, // Back
                    {-1.0f, 0.0f, 0.0f}, // Left
                    {1.0f, 0.0f, 0.0f},  // Right
                    {0.0f, 1.0f, 0.0f},  // Top
                    {0.0f, -1.0f, 0.0f}, // Bottom
                };
                
                // Check each face for visibility
                for (int face = 0; face < 6; ++face) {
                    // Check if face is visible (not adjacent to another block)
                    bool visible = true;
                    
                    int nx = x, ny = y, nz = z;
                    switch (face) {
                        case 0: nz++; break; // Front
                        case 1: nz--; break; // Back
                        case 2: nx--; break; // Left
                        case 3: nx++; break; // Right
                        case 4: ny++; break; // Top
                        case 5: ny--; break; // Bottom
                    }
                    
                    // Check if adjacent block is solid
                    if (nx >= 0 && nx < Chunk::CHUNK_WIDTH &&
                        ny >= 0 && ny < Chunk::CHUNK_HEIGHT &&
                        nz >= 0 && nz < Chunk::CHUNK_DEPTH) {
                        const Chunk::BlockInfo& adjacent = blocks[nx + ny * Chunk::CHUNK_WIDTH + nz * Chunk::CHUNK_WIDTH * Chunk::CHUNK_HEIGHT];
                        if (adjacent.type != Chunk::BlockType::AIR) {
                            visible = false;
                        }
                    }
                    
                    if (!visible) continue;
                    
                    // Add vertices for this face
                    uint32_t startVertex = vertexOffset;
                    
                    for (int i = 0; i < 4; ++i) {
                        // Position
                        vertices.push_back(worldX + faceVertices[face][i][0]);
                        vertices.push_back(worldY + faceVertices[face][i][1]);
                        vertices.push_back(worldZ + faceVertices[face][i][2]);
                        
                        // Normal
                        vertices.push_back(faceNormals[face][0]);
                        vertices.push_back(faceNormals[face][1]);
                        vertices.push_back(faceNormals[face][2]);
                        
                        // Texture coordinates
                        float u = (i == 0 || i == 3) ? 0.0f : 1.0f;
                        float v = (i == 0 || i == 1) ? 0.0f : 1.0f;
                        vertices.push_back(u);
                        vertices.push_back(v);
                        
                        vertexOffset++;
                    }
                    
                    // Add indices for this face (two triangles)
                    indices.push_back(static_cast<uint16_t>(startVertex));
                    indices.push_back(static_cast<uint16_t>(startVertex + 1));
                    indices.push_back(static_cast<uint16_t>(startVertex + 2));
                    indices.push_back(static_cast<uint16_t>(startVertex));
                    indices.push_back(static_cast<uint16_t>(startVertex + 2));
                    indices.push_back(static_cast<uint16_t>(startVertex + 3));
                }
            }
        }
    }
}

void MeshBuilder::generateEntityVertexData(const Entity* entity,
                                          std::vector<float>& vertices,
                                          std::vector<uint16_t>& indices,
                                          float boundingBoxMin[3],
                                          float boundingBoxMax[3])
{
    vertices.clear();
    indices.clear();
    
    if (!entity) return;
    
    // For now, create a simple cube mesh for all entities
    // In a real implementation, this would load the actual entity model
    
    // Initialize bounding box
    boundingBoxMin[0] = -0.5f; boundingBoxMin[1] = -0.5f; boundingBoxMin[2] = -0.5f;
    boundingBoxMax[0] = 0.5f; boundingBoxMax[1] = 0.5f; boundingBoxMax[2] = 0.5f;
    
    const float cubeVertices[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        
        // Back face
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        
        // Left face
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        
        // Right face
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        
        // Top face
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        
        // Bottom face
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
    };
    
    const uint16_t cubeIndices[] = {
        // Front face
        0, 1, 2, 0, 2, 3,
        // Back face
        4, 6, 5, 4, 7, 6,
        // Left face
        8, 9, 10, 8, 10, 11,
        // Right face
        12, 14, 13, 12, 15, 14,
        // Top face
        16, 18, 17, 16, 19, 18,
        // Bottom face
        20, 21, 22, 20, 22, 23,
    };
    
    vertices.assign(cubeVertices, cubeVertices + sizeof(cubeVertices) / sizeof(float));
    indices.assign(cubeIndices, cubeIndices + sizeof(cubeIndices) / sizeof(uint16_t));
}

uint64_t MeshBuilder::calculateChunkHash(const Chunk* chunk) const
{
    if (!chunk) return 0;
    
    uint64_t hash = 0;
    const auto& blocks = chunk->getBlocks();
    
    for (const auto& block : blocks) {
        hash = bx::hashCombine(hash, static_cast<uint32_t>(block.type));
        hash = bx::hashCombine(hash, block.data);
    }
    
    return hash;
}

uint64_t MeshBuilder::calculateEntityHash(const Entity* entity) const
{
    if (!entity) return 0;
    
    uint64_t hash = entity->getEntityId();
    hash = bx::hashCombine(hash, static_cast<uint32_t>(entity->getType()));
    hash = bx::hashCombine(hash, *reinterpret_cast<const uint32_t*>(&entity->getPositionX()));
    hash = bx::hashCombine(hash, *reinterpret_cast<const uint32_t*>(&entity->getPositionY()));
    hash = bx::hashCombine(hash, *reinterpret_cast<const uint32_t*>(&entity->getPositionZ()));
    hash = bx::hashCombine(hash, *reinterpret_cast<const uint32_t*>(&entity->getRotationPitch()));
    hash = bx::hashCombine(hash, *reinterpret_cast<const uint32_t*>(&entity->getRotationYaw()));
    hash = bx::hashCombine(hash, *reinterpret_cast<const uint32_t*>(&entity->getRotationRoll()));
    
    return hash;
}

} // namespace renderbridge
