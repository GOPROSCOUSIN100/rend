#include "voxel_renderer.hpp"
#include "scene.hpp"
#include "chunk.hpp"
#include "utils/logger.hpp"

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <cstring>
#include <algorithm>

namespace renderbridge {

// Static logger
static Logger s_logger("VoxelRenderer");

VoxelRenderer::VoxelRenderer()
    : m_vertexDecl(),
      m_program(bgfx::kInvalidHandle),
      m_texture(bgfx::kInvalidHandle),
      m_totalVertices(0),
      m_totalIndices(0),
      m_chunkCount(0)
{
}

VoxelRenderer::~VoxelRenderer()
{
    shutdown();
}

bool VoxelRenderer::initialize(bgfx::VertexDecl vertexDecl, bgfx::ProgramHandle program)
{
    s_logger.info("Initializing voxel renderer...");
    
    m_vertexDecl = vertexDecl;
    m_program = program;
    
    if (m_program.idx == bgfx::kInvalidHandle) {
        s_logger.error("Invalid program handle");
        return false;
    }
    
    s_logger.info("Voxel renderer initialized successfully");
    return true;
}

void VoxelRenderer::shutdown()
{
    s_logger.info("Shutting down voxel renderer...");
    
    // Destroy all chunk meshes
    for (auto& pair : m_chunkMeshes) {
        destroyChunkMesh(pair.second);
    }
    m_chunkMeshes.clear();
    
    // Reset statistics
    m_totalVertices = 0;
    m_totalIndices = 0;
    m_chunkCount = 0;
    
    s_logger.info("Voxel renderer shut down");
}

void VoxelRenderer::render(const Scene* scene, bgfx::ViewId viewId, const float* viewProjMatrix)
{
    if (!scene || m_program.idx == bgfx::kInvalidHandle) {
        return;
    }
    
    // Set shader program
    bgfx::setState(
        BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z |
        BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW |
        BGFX_STATE_MSAA
    );
    
    // Set shader
    bgfx::setProgram(m_program);
    
    // Set vertex declaration
    bgfx::setVertexDecl(m_vertexDecl);
    
    // Set texture
    if (m_texture.idx != bgfx::kInvalidHandle) {
        bgfx::setTexture(0, m_uniforms["u_texColor"], m_texture);
    }
    
    // Set view-proj matrix uniform if available
    auto it = m_uniforms.find("u_viewProj");
    if (it != m_uniforms.end() && viewProjMatrix) {
        bgfx::setUniform(it->second, viewProjMatrix);
    }
    
    // Get all chunks
    auto chunks = scene->getAllChunks();
    
    // Render each chunk
    for (auto& chunk : chunks) {
        if (!chunk) continue;
        
        // Get or create mesh for chunk
        auto meshIt = m_chunkMeshes.find(chunk.get());
        if (meshIt == m_chunkMeshes.end()) {
            // Create new mesh
            if (createChunkMesh(chunk.get())) {
                meshIt = m_chunkMeshes.find(chunk.get());
            } else {
                continue;
            }
        }
        
        ChunkMesh& chunkMesh = meshIt->second;
        
        // Update mesh if dirty
        if (chunkMesh.dirty) {
            updateMeshBuffers(chunk.get(), chunkMesh);
            chunkMesh.dirty = false;
        }
        
        // Submit chunk for rendering
        submitChunk(chunk.get(), chunkMesh, viewId, viewProjMatrix);
    }
    
    // Update statistics
    m_chunkCount = static_cast<uint32_t>(m_chunkMeshes.size());
}

void VoxelRenderer::updateChunkMesh(const Chunk* chunk)
{
    if (!chunk) return;
    
    auto it = m_chunkMeshes.find(chunk);
    if (it != m_chunkMeshes.end()) {
        it->second.dirty = true;
    } else {
        createChunkMesh(chunk);
    }
}

void VoxelRenderer::removeChunkMesh(const Chunk* chunk)
{
    if (!chunk) return;
    
    auto it = m_chunkMeshes.find(chunk);
    if (it != m_chunkMeshes.end()) {
        destroyChunkMesh(it->second);
        m_chunkMeshes.erase(it);
    }
}

void VoxelRenderer::setTexture(bgfx::TextureHandle texture)
{
    m_texture = texture;
}

void VoxelRenderer::setUniform(const std::string& name, bgfx::UniformHandle handle)
{
    m_uniforms[name] = handle;
}

std::string VoxelRenderer::getStatistics() const
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
        "Voxel Renderer: %u chunks, %u vertices, %u indices",
        m_chunkCount, m_totalVertices, m_totalIndices);
    return std::string(buffer);
}

bool VoxelRenderer::createChunkMesh(const Chunk* chunk)
{
    if (!chunk) return false;
    
    ChunkMesh mesh;
    mesh.vbo.idx = bgfx::kInvalidHandle;
    mesh.ibo.idx = bgfx::kInvalidHandle;
    mesh.vertexCount = 0;
    mesh.indexCount = 0;
    mesh.dirty = true;
    
    // Generate initial vertex data
    std::vector<float> vertices;
    std::vector<uint16_t> indices;
    generateChunkVertexData(chunk, vertices, indices);
    
    mesh.vertexCount = static_cast<uint32_t>(vertices.size() / 8); // 8 floats per vertex (pos, normal, uv, color)
    mesh.indexCount = static_cast<uint32_t>(indices.size());
    
    // Create vertex buffer
    if (!vertices.empty()) {
        mesh.vbo = bgfx::createVertexBuffer(
            bgfx::makeRef(vertices.data(), vertices.size() * sizeof(float)),
            m_vertexDecl
        );
    }
    
    // Create index buffer
    if (!indices.empty()) {
        mesh.ibo = bgfx::createIndexBuffer(
            bgfx::makeRef(indices.data(), indices.size() * sizeof(uint16_t))
        );
    }
    
    if (mesh.vbo.idx == bgfx::kInvalidHandle || mesh.ibo.idx == bgfx::kInvalidHandle) {
        destroyChunkMesh(mesh);
        return false;
    }
    
    // Add to mesh map
    m_chunkMeshes[chunk] = mesh;
    
    // Update statistics
    m_totalVertices += mesh.vertexCount;
    m_totalIndices += mesh.indexCount;
    
    return true;
}

void VoxelRenderer::updateMeshBuffers(const Chunk* chunk, ChunkMesh& chunkMesh)
{
    if (!chunk) return;
    
    // Generate new vertex data
    std::vector<float> vertices;
    std::vector<uint16_t> indices;
    generateChunkVertexData(chunk, vertices, indices);
    
    // Update vertex buffer
    if (!vertices.empty() && chunkMesh.vbo.idx != bgfx::kInvalidHandle) {
        bgfx::updateVertexBuffer(
            chunkMesh.vbo,
            0,
            bgfx::makeRef(vertices.data(), vertices.size() * sizeof(float))
        );
        chunkMesh.vertexCount = static_cast<uint32_t>(vertices.size() / 8);
    }
    
    // Update index buffer
    if (!indices.empty() && chunkMesh.ibo.idx != bgfx::kInvalidHandle) {
        bgfx::updateIndexBuffer(
            chunkMesh.ibo,
            0,
            bgfx::makeRef(indices.data(), indices.size() * sizeof(uint16_t))
        );
        chunkMesh.indexCount = static_cast<uint32_t>(indices.size());
    }
    
    // Update statistics
    m_totalVertices = 0;
    m_totalIndices = 0;
    for (auto& pair : m_chunkMeshes) {
        m_totalVertices += pair.second.vertexCount;
        m_totalIndices += pair.second.indexCount;
    }
}

void VoxelRenderer::destroyChunkMesh(ChunkMesh& chunkMesh)
{
    if (chunkMesh.vbo.idx != bgfx::kInvalidHandle) {
        bgfx::destroy(chunkMesh.vbo);
        chunkMesh.vbo.idx = bgfx::kInvalidHandle;
    }
    if (chunkMesh.ibo.idx != bgfx::kInvalidHandle) {
        bgfx::destroy(chunkMesh.ibo);
        chunkMesh.ibo.idx = bgfx::kInvalidHandle;
    }
    chunkMesh.vertexCount = 0;
    chunkMesh.indexCount = 0;
    chunkMesh.dirty = false;
}

void VoxelRenderer::generateChunkVertexData(const Chunk* chunk, std::vector<float>& vertices, std::vector<uint16_t>& indices)
{
    vertices.clear();
    indices.clear();
    
    if (!chunk) return;
    
    // Get chunk data
    const auto& blocks = chunk->getBlocks();
    int32_t chunkX = static_cast<int32_t>(chunk->getPositionX());
    int32_t chunkY = static_cast<int32_t>(chunk->getPositionY());
    int32_t chunkZ = static_cast<int32_t>(chunk->getPositionZ());
    
    // Reserve space for vertices and indices
    // Each visible face has 4 vertices and 6 indices
    // Maximum: 16*16*16 blocks * 6 faces * 4 vertices = 65536 vertices
    // Maximum indices: 65536 * 6 / 4 = 98304 (but we use 16-bit indices, so max 65535)
    vertices.reserve(65536 * 8); // 8 floats per vertex
    indices.reserve(65536 * 6);
    
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
                
                // Define face vertices
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

void VoxelRenderer::submitChunk(const Chunk* chunk, const ChunkMesh& chunkMesh, bgfx::ViewId viewId, const float* viewProjMatrix)
{
    if (!chunk || chunkMesh.vbo.idx == bgfx::kInvalidHandle || chunkMesh.ibo.idx == bgfx::kInvalidHandle) {
        return;
    }
    
    // Set model matrix
    float modelMatrix[16];
    bx::mtxIdentity(modelMatrix);
    bx::mtxTranslate(modelMatrix,
        chunk->getPositionX(),
        chunk->getPositionY(),
        chunk->getPositionZ());
    
    // Set model matrix uniform if available
    auto it = m_uniforms.find("u_model");
    if (it != m_uniforms.end()) {
        bgfx::setUniform(it->second, modelMatrix);
    }
    
    // Set vertex and index buffers
    bgfx::setVertexBuffer(0, chunkMesh.vbo);
    bgfx::setIndexBuffer(chunkMesh.ibo);
    
    // Submit for rendering
    bgfx::submit(viewId, m_program);
}

} // namespace renderbridge
