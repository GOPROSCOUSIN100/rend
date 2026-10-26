#include "entity_renderer.hpp"
#include "scene.hpp"
#include "entity.hpp"
#include "utils/logger.hpp"

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <cstring>
#include <algorithm>

namespace renderbridge {

// Static logger
static Logger s_logger("EntityRenderer");

EntityRenderer::EntityRenderer()
    : m_vertexDecl(),
      m_program(bgfx::kInvalidHandle),
      m_texture(bgfx::kInvalidHandle),
      m_totalVertices(0),
      m_totalIndices(0),
      m_entityCount(0)
{
}

EntityRenderer::~EntityRenderer()
{
    shutdown();
}

bool EntityRenderer::initialize(bgfx::VertexDecl vertexDecl, bgfx::ProgramHandle program)
{
    s_logger.info("Initializing entity renderer...");
    
    m_vertexDecl = vertexDecl;
    m_program = program;
    
    if (m_program.idx == bgfx::kInvalidHandle) {
        s_logger.error("Invalid program handle");
        return false;
    }
    
    s_logger.info("Entity renderer initialized successfully");
    return true;
}

void EntityRenderer::shutdown()
{
    s_logger.info("Shutting down entity renderer...");
    
    // Destroy all entity meshes
    for (auto& pair : m_entityMeshes) {
        destroyEntityMesh(pair.second);
    }
    m_entityMeshes.clear();
    
    // Reset statistics
    m_totalVertices = 0;
    m_totalIndices = 0;
    m_entityCount = 0;
    
    s_logger.info("Entity renderer shut down");
}

void EntityRenderer::render(const Scene* scene, bgfx::ViewId viewId, const float* viewMatrix, const float* projMatrix)
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
        auto it = m_uniforms.find("u_texColor");
        if (it != m_uniforms.end()) {
            bgfx::setTexture(0, it->second, m_texture);
        }
    }
    
    // Set view and projection matrices as uniforms if available
    auto viewIt = m_uniforms.find("u_view");
    auto projIt = m_uniforms.find("u_proj");
    if (viewIt != m_uniforms.end() && viewMatrix) {
        bgfx::setUniform(viewIt->second, viewMatrix);
    }
    if (projIt != m_uniforms.end() && projMatrix) {
        bgfx::setUniform(projIt->second, projMatrix);
    }
    
    // Get all entities
    auto entities = scene->getAllEntities();
    
    // Render each entity
    for (auto& entity : entities) {
        if (!entity) continue;
        
        // Get or create mesh for entity
        auto meshIt = m_entityMeshes.find(entity.get());
        if (meshIt == m_entityMeshes.end()) {
            // Create new mesh
            if (createEntityMesh(entity.get())) {
                meshIt = m_entityMeshes.find(entity.get());
            } else {
                continue;
            }
        }
        
        EntityMesh& entityMesh = meshIt->second;
        
        // Update mesh if dirty
        if (entityMesh.dirty) {
            updateMeshBuffers(entity.get(), entityMesh);
            entityMesh.dirty = false;
        }
        
        // Submit entity for rendering
        submitEntity(entity.get(), entityMesh, viewId, viewMatrix, projMatrix);
    }
    
    // Update statistics
    m_entityCount = static_cast<uint32_t>(m_entityMeshes.size());
}

void EntityRenderer::updateEntityMesh(const Entity* entity)
{
    if (!entity) return;
    
    auto it = m_entityMeshes.find(entity);
    if (it != m_entityMeshes.end()) {
        it->second.dirty = true;
    } else {
        createEntityMesh(entity);
    }
}

void EntityRenderer::removeEntityMesh(const Entity* entity)
{
    if (!entity) return;
    
    auto it = m_entityMeshes.find(entity);
    if (it != m_entityMeshes.end()) {
        destroyEntityMesh(it->second);
        m_entityMeshes.erase(it);
    }
}

void EntityRenderer::setTexture(bgfx::TextureHandle texture)
{
    m_texture = texture;
}

void EntityRenderer::setUniform(const std::string& name, bgfx::UniformHandle handle)
{
    m_uniforms[name] = handle;
}

std::string EntityRenderer::getStatistics() const
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
        "Entity Renderer: %u entities, %u vertices, %u indices",
        m_entityCount, m_totalVertices, m_totalIndices);
    return std::string(buffer);
}

bool EntityRenderer::createEntityMesh(const Entity* entity)
{
    if (!entity) return false;
    
    EntityMesh mesh;
    mesh.vbo.idx = bgfx::kInvalidHandle;
    mesh.ibo.idx = bgfx::kInvalidHandle;
    mesh.vertexCount = 0;
    mesh.indexCount = 0;
    mesh.dirty = true;
    mesh.transformUniform.idx = bgfx::kInvalidHandle;
    
    // Create transform uniform for this entity
    char uniformName[64];
    snprintf(uniformName, sizeof(uniformName), "u_entityTransform[%llu]", entity->getEntityId());
    mesh.transformUniform = bgfx::createUniform(uniformName, bgfx::UniformType::Mat4, 1);
    
    // Generate initial vertex data
    std::vector<float> vertices;
    std::vector<uint16_t> indices;
    generateEntityVertexData(entity, vertices, indices);
    
    mesh.vertexCount = static_cast<uint32_t>(vertices.size() / 8); // 8 floats per vertex
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
        destroyEntityMesh(mesh);
        return false;
    }
    
    // Add to mesh map
    m_entityMeshes[entity] = mesh;
    
    // Update statistics
    m_totalVertices += mesh.vertexCount;
    m_totalIndices += mesh.indexCount;
    
    return true;
}

void EntityRenderer::updateMeshBuffers(const Entity* entity, EntityMesh& entityMesh)
{
    if (!entity) return;
    
    // Generate new vertex data
    std::vector<float> vertices;
    std::vector<uint16_t> indices;
    generateEntityVertexData(entity, vertices, indices);
    
    // Update vertex buffer
    if (!vertices.empty() && entityMesh.vbo.idx != bgfx::kInvalidHandle) {
        bgfx::updateVertexBuffer(
            entityMesh.vbo,
            0,
            bgfx::makeRef(vertices.data(), vertices.size() * sizeof(float))
        );
        entityMesh.vertexCount = static_cast<uint32_t>(vertices.size() / 8);
    }
    
    // Update index buffer
    if (!indices.empty() && entityMesh.ibo.idx != bgfx::kInvalidHandle) {
        bgfx::updateIndexBuffer(
            entityMesh.ibo,
            0,
            bgfx::makeRef(indices.data(), indices.size() * sizeof(uint16_t))
        );
        entityMesh.indexCount = static_cast<uint32_t>(indices.size());
    }
    
    // Update statistics
    m_totalVertices = 0;
    m_totalIndices = 0;
    for (auto& pair : m_entityMeshes) {
        m_totalVertices += pair.second.vertexCount;
        m_totalIndices += pair.second.indexCount;
    }
}

void EntityRenderer::destroyEntityMesh(EntityMesh& entityMesh)
{
    if (entityMesh.transformUniform.idx != bgfx::kInvalidHandle) {
        bgfx::destroy(entityMesh.transformUniform);
        entityMesh.transformUniform.idx = bgfx::kInvalidHandle;
    }
    if (entityMesh.vbo.idx != bgfx::kInvalidHandle) {
        bgfx::destroy(entityMesh.vbo);
        entityMesh.vbo.idx = bgfx::kInvalidHandle;
    }
    if (entityMesh.ibo.idx != bgfx::kInvalidHandle) {
        bgfx::destroy(entityMesh.ibo);
        entityMesh.ibo.idx = bgfx::kInvalidHandle;
    }
    entityMesh.vertexCount = 0;
    entityMesh.indexCount = 0;
    entityMesh.dirty = false;
}

void EntityRenderer::generateEntityVertexData(const Entity* entity, std::vector<float>& vertices, std::vector<uint16_t>& indices)
{
    vertices.clear();
    indices.clear();
    
    if (!entity) return;
    
    // For now, create a simple cube mesh for each entity
    // In a real implementation, this would load the actual entity model
    
    // Define cube vertices
    struct EntityVertex {
        float x, y, z;
        float nx, ny, nz;
        float u, v;
    };
    
    const EntityVertex cubeVertices[24] = {
        // Front face
        {-0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
        { 0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
        { 0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f},
        {-0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
        
        // Back face
        {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f},
        { 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f},
        { 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f},
        {-0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f},
        
        // Left face
        {-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {-0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f},
        {-0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
        {-0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f},
        
        // Right face
        { 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        { 0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f},
        { 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},
        { 0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f},
        
        // Top face
        {-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
        { 0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f},
        { 0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f},
        {-0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
        
        // Bottom face
        {-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f},
        { 0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f},
        { 0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f},
        {-0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f},
    };
    
    const uint16_t cubeIndices[36] = {
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
    
    // Add vertices with position, normal, and texture coordinates
    for (const auto& v : cubeVertices) {
        vertices.push_back(v.x);
        vertices.push_back(v.y);
        vertices.push_back(v.z);
        vertices.push_back(v.nx);
        vertices.push_back(v.ny);
        vertices.push_back(v.nz);
        vertices.push_back(v.u);
        vertices.push_back(v.v);
    }
    
    // Add indices
    for (const auto& i : cubeIndices) {
        indices.push_back(i);
    }
}

void EntityRenderer::submitEntity(const Entity* entity, const EntityMesh& entityMesh, bgfx::ViewId viewId, const float* viewMatrix, const float* projMatrix)
{
    if (!entity || entityMesh.vbo.idx == bgfx::kInvalidHandle || entityMesh.ibo.idx == bgfx::kInvalidHandle) {
        return;
    }
    
    // Calculate model matrix
    float modelMatrix[16];
    bx::mtxIdentity(modelMatrix);
    
    // Translate
    bx::mtxTranslate(modelMatrix,
        entity->getPositionX(),
        entity->getPositionY(),
        entity->getPositionZ());
    
    // Rotate (Yaw, Pitch, Roll)
    float yaw = entity->getRotationYaw() * 0.0174532925f; // degrees to radians
    float pitch = entity->getRotationPitch() * 0.0174532925f;
    float roll = entity->getRotationRoll() * 0.0174532925f;
    
    float rotMatrix[16];
    bx::mtxRotateXYZ(rotMatrix, pitch, yaw, roll);
    bx::mtxMul(modelMatrix, modelMatrix, rotMatrix);
    
    // Scale
    float scaleMatrix[16];
    bx::mtxScale(scaleMatrix,
        entity->getScaleX(),
        entity->getScaleY(),
        entity->getScaleZ());
    bx::mtxMul(modelMatrix, modelMatrix, scaleMatrix);
    
    // Set transform uniform
    if (entityMesh.transformUniform.idx != bgfx::kInvalidHandle) {
        bgfx::setUniform(entityMesh.transformUniform, modelMatrix);
    }
    
    // Set vertex and index buffers
    bgfx::setVertexBuffer(0, entityMesh.vbo);
    bgfx::setIndexBuffer(entityMesh.ibo);
    
    // Submit for rendering
    bgfx::submit(viewId, m_program);
}

} // namespace renderbridge
