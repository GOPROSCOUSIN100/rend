#pragma once

#include <cstdint>
#include <vector>
#include <memory>

// bgfx includes
#include <bgfx/bgfx.h>

namespace renderbridge {

// Forward declarations
class Chunk;
class Entity;

/**
 * @class MeshBuilder
 * @brief Builds bgfx-compatible meshes from scene data
 */
class MeshBuilder {
public:
    /**
     * @brief Constructor
     */
    MeshBuilder();
    
    /**
     * @brief Destructor
     */
    ~MeshBuilder();
    
    /**
     * @brief Initialize the mesh builder
     * @param voxelVertexDecl Vertex declaration for voxels
     * @param entityVertexDecl Vertex declaration for entities
     * @return True if initialization succeeded
     */
    bool initialize(bgfx::VertexDecl voxelVertexDecl, bgfx::VertexDecl entityVertexDecl);
    
    /**
     * @brief Shutdown the mesh builder
     */
    void shutdown();
    
    // ========================================================================
    // Chunk mesh building
    // ========================================================================
    
    /**
     * @brief Build a mesh from a chunk
     * @param chunk Chunk to build mesh from
     * @return Mesh data (vertex buffer, index buffer, counts)
     */
    struct ChunkMeshData {
        bgfx::VertexBufferHandle vbo;
        bgfx::IndexBufferHandle ibo;
        uint32_t vertexCount;
        uint32_t indexCount;
        float boundingBoxMin[3];
        float boundingBoxMax[3];
    };
    
    ChunkMeshData buildChunkMesh(const Chunk* chunk);
    
    /**
     * @brief Update a chunk mesh
     * @param chunk Chunk with updated data
     * @param meshData Mesh data to update
     */
    void updateChunkMesh(const Chunk* chunk, ChunkMeshData& meshData);
    
    /**
     * @brief Destroy a chunk mesh
     * @param meshData Mesh data to destroy
     */
    void destroyChunkMesh(ChunkMeshData& meshData);
    
    // ========================================================================
    // Entity mesh building
    // ========================================================================
    
    /**
     * @brief Build a mesh from an entity
     * @param entity Entity to build mesh from
     * @return Mesh data (vertex buffer, index buffer, counts)
     */
    struct EntityMeshData {
        bgfx::VertexBufferHandle vbo;
        bgfx::IndexBufferHandle ibo;
        uint32_t vertexCount;
        uint32_t indexCount;
        float boundingBoxMin[3];
        float boundingBoxMax[3];
    };
    
    EntityMeshData buildEntityMesh(const Entity* entity);
    
    /**
     * @brief Update an entity mesh
     * @param entity Entity with updated data
     * @param meshData Mesh data to update
     */
    void updateEntityMesh(const Entity* entity, EntityMeshData& meshData);
    
    /**
     * @brief Destroy an entity mesh
     * @param meshData Mesh data to destroy
     */
    void destroyEntityMesh(EntityMeshData& meshData);
    
    // ========================================================================
    // Particle mesh building
    // ========================================================================
    
    /**
     * @brief Build a vertex buffer from particle data
     * @param particles Particle data (array of floats)
     * @param particleCount Number of particles
     * @return Vertex buffer handle
     */
    bgfx::VertexBufferHandle buildParticleBuffer(const float* particles, uint32_t particleCount);
    
    /**
     * @brief Update a particle buffer
     * @param vbo Vertex buffer to update
     * @param particles Particle data
     * @param particleCount Number of particles
     */
    void updateParticleBuffer(bgfx::VertexBufferHandle vbo, const float* particles, uint32_t particleCount);
    
    /**
     * @brief Destroy a particle buffer
     * @param vbo Vertex buffer to destroy
     */
    void destroyParticleBuffer(bgfx::VertexBufferHandle vbo);
    
    // ========================================================================
    // Mesh caching
    // ========================================================================
    
    /**
     * @brief Clear all cached meshes
     */
    void clearCache();
    
    /**
     * @brief Get cache statistics
     * @return Statistics string
     */
    std::string getCacheStatistics() const;

private:
    bgfx::VertexDecl m_voxelVertexDecl;
    bgfx::VertexDecl m_entityVertexDecl;
    
    // Vertex declaration for particles
    bgfx::VertexDecl m_particleVertexDecl;
    
    // Cache for chunk meshes
    struct CachedChunkMesh {
        ChunkMeshData meshData;
        uint64_t chunkHash;
    };
    std::unordered_map<const Chunk*, CachedChunkMesh> m_chunkMeshCache;
    
    // Cache for entity meshes
    struct CachedEntityMesh {
        EntityMeshData meshData;
        uint64_t entityHash;
    };
    std::unordered_map<const Entity*, CachedEntityMesh> m_entityMeshCache;
    
    // Statistics
    uint32_t m_chunkMeshCount;
    uint32_t m_entityMeshCount;
    uint32_t m_totalVertices;
    uint32_t m_totalIndices;
    
    /**
     * @brief Generate vertex data for a chunk
     * @param chunk Chunk
     * @param vertices Output: vertex data
     * @param indices Output: index data
     * @param boundingBoxMin Output: bounding box minimum
     * @param boundingBoxMax Output: bounding box maximum
     */
    void generateChunkVertexData(const Chunk* chunk, 
                                  std::vector<float>& vertices, 
                                  std::vector<uint16_t>& indices,
                                  float boundingBoxMin[3],
                                  float boundingBoxMax[3]);
    
    /**
     * @brief Generate vertex data for an entity
     * @param entity Entity
     * @param vertices Output: vertex data
     * @param indices Output: index data
     * @param boundingBoxMin Output: bounding box minimum
     * @param boundingBoxMax Output: bounding box maximum
     */
    void generateEntityVertexData(const Entity* entity,
                                   std::vector<float>& vertices,
                                   std::vector<uint16_t>& indices,
                                   float boundingBoxMin[3],
                                   float boundingBoxMax[3]);
    
    /**
     * @brief Calculate hash for a chunk
     * @param chunk Chunk
     * @return Hash value
     */
    uint64_t calculateChunkHash(const Chunk* chunk) const;
    
    /**
     * @brief Calculate hash for an entity
     * @param entity Entity
     * @return Hash value
     */
    uint64_t calculateEntityHash(const Entity* entity) const;
};

} // namespace renderbridge
