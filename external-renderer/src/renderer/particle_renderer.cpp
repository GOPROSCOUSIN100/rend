#include "particle_renderer.hpp"
#include "scene.hpp"
#include "particle.hpp"
#include "utils/logger.hpp"

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <cstring>
#include <algorithm>

namespace renderbridge {

// Static logger
static Logger s_logger("ParticleRenderer");

ParticleRenderer::ParticleRenderer()
    : m_vertexDecl(),
      m_program(bgfx::kInvalidHandle),
      m_textureArray(bgfx::kInvalidHandle),
      m_totalParticles(0),
      m_particleSystemCount(0)
{
}

ParticleRenderer::~ParticleRenderer()
{
    shutdown();
}

bool ParticleRenderer::initialize(bgfx::VertexDecl vertexDecl, bgfx::ProgramHandle program)
{
    s_logger.info("Initializing particle renderer...");
    
    m_vertexDecl = vertexDecl;
    m_program = program;
    
    if (m_program.idx == bgfx::kInvalidHandle) {
        s_logger.error("Invalid program handle");
        return false;
    }
    
    s_logger.info("Particle renderer initialized successfully");
    return true;
}

void ParticleRenderer::shutdown()
{
    s_logger.info("Shutting down particle renderer...");
    
    // Destroy all particle system buffers
    for (auto& pair : m_particleSystemData) {
        destroyParticleSystemBuffers(pair.second);
    }
    m_particleSystemData.clear();
    
    // Reset statistics
    m_totalParticles = 0;
    m_particleSystemCount = 0;
    
    s_logger.info("Particle renderer shut down");
}

void ParticleRenderer::render(const Scene* scene, bgfx::ViewId viewId, const float* viewMatrix, const float* projMatrix)
{
    if (!scene || m_program.idx == bgfx::kInvalidHandle) {
        return;
    }
    
    // Set shader program
    bgfx::setState(
        BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z |
        BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_BLEND_ALPHA |
        BGFX_STATE_POINT_SIZE
    );
    
    // Set shader
    bgfx::setProgram(m_program);
    
    // Set vertex declaration
    bgfx::setVertexDecl(m_vertexDecl);
    
    // Set texture array
    if (m_textureArray.idx != bgfx::kInvalidHandle) {
        auto it = m_uniforms.find("u_texArray");
        if (it != m_uniforms.end()) {
            bgfx::setTexture(0, it->second, m_textureArray);
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
    
    // Get all particle systems
    auto particleSystems = scene->getAllParticleSystems();
    
    // Render each particle system
    for (auto& particleSystem : particleSystems) {
        if (!particleSystem) continue;
        
        // Get or create buffers for particle system
        auto dataIt = m_particleSystemData.find(particleSystem.get());
        if (dataIt == m_particleSystemData.end()) {
            // Create new buffers
            if (createParticleSystemBuffers(particleSystem.get())) {
                dataIt = m_particleSystemData.find(particleSystem.get());
            } else {
                continue;
            }
        }
        
        ParticleSystemData& particleSystemData = dataIt->second;
        
        // Update buffers if dirty
        if (particleSystemData.dirty) {
            updateParticleBuffers(particleSystem.get(), particleSystemData);
            particleSystemData.dirty = false;
        }
        
        // Submit particle system for rendering
        submitParticleSystem(particleSystem.get(), particleSystemData, viewId, viewMatrix, projMatrix);
    }
    
    // Update statistics
    m_particleSystemCount = static_cast<uint32_t>(m_particleSystemData.size());
}

void ParticleRenderer::updateParticleSystem(const ParticleSystem* particleSystem)
{
    if (!particleSystem) return;
    
    auto it = m_particleSystemData.find(particleSystem);
    if (it != m_particleSystemData.end()) {
        it->second.dirty = true;
    } else {
        createParticleSystemBuffers(particleSystem);
    }
}

void ParticleRenderer::removeParticleSystem(const ParticleSystem* particleSystem)
{
    if (!particleSystem) return;
    
    auto it = m_particleSystemData.find(particleSystem);
    if (it != m_particleSystemData.end()) {
        destroyParticleSystemBuffers(it->second);
        m_particleSystemData.erase(it);
    }
}

void ParticleRenderer::setTextureArray(bgfx::TextureHandle textureArray)
{
    m_textureArray = textureArray;
}

void ParticleRenderer::setUniform(const std::string& name, bgfx::UniformHandle handle)
{
    m_uniforms[name] = handle;
}

std::string ParticleRenderer::getStatistics() const
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
        "Particle Renderer: %u particle systems, %u particles",
        m_particleSystemCount, m_totalParticles);
    return std::string(buffer);
}

bool ParticleRenderer::createParticleSystemBuffers(const ParticleSystem* particleSystem)
{
    if (!particleSystem) return false;
    
    ParticleSystemData data;
    data.vbo.idx = bgfx::kInvalidHandle;
    data.particleCount = 0;
    data.dirty = true;
    
    // Generate initial vertex data
    std::vector<float> vertices;
    generateParticleVertexData(particleSystem, vertices);
    
    data.particleCount = static_cast<uint32_t>(vertices.size() / 9); // 9 floats per particle vertex
    
    // Create vertex buffer
    if (!vertices.empty()) {
        data.vbo = bgfx::createVertexBuffer(
            bgfx::makeRef(vertices.data(), vertices.size() * sizeof(float)),
            m_vertexDecl
        );
    }
    
    if (data.vbo.idx == bgfx::kInvalidHandle) {
        destroyParticleSystemBuffers(data);
        return false;
    }
    
    // Add to data map
    m_particleSystemData[particleSystem] = data;
    
    // Update statistics
    m_totalParticles += data.particleCount;
    
    return true;
}

void ParticleRenderer::updateParticleBuffers(const ParticleSystem* particleSystem, ParticleSystemData& particleSystemData)
{
    if (!particleSystem) return;
    
    // Generate new vertex data
    std::vector<float> vertices;
    generateParticleVertexData(particleSystem, vertices);
    
    // Update vertex buffer
    if (!vertices.empty() && particleSystemData.vbo.idx != bgfx::kInvalidHandle) {
        bgfx::updateVertexBuffer(
            particleSystemData.vbo,
            0,
            bgfx::makeRef(vertices.data(), vertices.size() * sizeof(float))
        );
        particleSystemData.particleCount = static_cast<uint32_t>(vertices.size() / 9);
    }
    
    // Update statistics
    m_totalParticles = 0;
    for (auto& pair : m_particleSystemData) {
        m_totalParticles += pair.second.particleCount;
    }
}

void ParticleRenderer::destroyParticleSystemBuffers(ParticleSystemData& particleSystemData)
{
    if (particleSystemData.vbo.idx != bgfx::kInvalidHandle) {
        bgfx::destroy(particleSystemData.vbo);
        particleSystemData.vbo.idx = bgfx::kInvalidHandle;
    }
    particleSystemData.particleCount = 0;
    particleSystemData.dirty = false;
}

void ParticleRenderer::generateParticleVertexData(const ParticleSystem* particleSystem, std::vector<float>& vertices)
{
    vertices.clear();
    
    if (!particleSystem) return;
    
    // Generate vertex data for all particles
    // Each particle is rendered as a quad (4 vertices)
    std::vector<ParticleVertex> particleVertices;
    particleSystem->generateVertexData(particleVertices, 0.0f); // No interpolation for now
    
    // Convert to flat array
    vertices.reserve(particleVertices.size() * 9); // 9 floats per vertex
    
    for (const auto& pv : particleVertices) {
        // Position
        vertices.push_back(pv.x);
        vertices.push_back(pv.y);
        vertices.push_back(pv.z);
        
        // Texture coordinates
        vertices.push_back(pv.u);
        vertices.push_back(pv.v);
        
        // Color
        vertices.push_back(pv.r);
        vertices.push_back(pv.g);
        vertices.push_back(pv.b);
        vertices.push_back(pv.a);
        
        // Size, rotation, texture index
        vertices.push_back(pv.size);
    }
}

void ParticleRenderer::submitParticleSystem(const ParticleSystem* particleSystem, const ParticleSystemData& particleSystemData, bgfx::ViewId viewId, const float* viewMatrix, const float* projMatrix)
{
    if (!particleSystem || particleSystemData.vbo.idx == bgfx::kInvalidHandle) {
        return;
    }
    
    // Set vertex buffer
    bgfx::setVertexBuffer(0, particleSystemData.vbo);
    
    // Calculate model matrix (particle system position)
    float modelMatrix[16];
    bx::mtxIdentity(modelMatrix);
    bx::mtxTranslate(modelMatrix,
        particleSystem->getPositionX(),
        particleSystem->getPositionY(),
        particleSystem->getPositionZ());
    
    // Set model matrix uniform if available
    auto it = m_uniforms.find("u_model");
    if (it != m_uniforms.end()) {
        bgfx::setUniform(it->second, modelMatrix);
    }
    
    // Submit for rendering (using draw call with vertex count)
    // Each particle is 4 vertices (quad), but we're using a geometry shader or instancing would be better
    // For now, we'll just submit the vertices as-is
    bgfx::submit(viewId, m_program, 0, BGFX_DISCARD_ALL);
}

} // namespace renderbridge
