#include "uniform_manager.hpp"
#include "utils/logger.hpp"

#include <bgfx/bgfx.h>

#include <cstring>
#include <algorithm>

namespace renderbridge {

// Static logger
static Logger s_logger("UniformManager");

UniformManager::UniformManager()
    : m_viewProjUniform(bgfx::kInvalidHandle),
      m_viewUniform(bgfx::kInvalidHandle),
      m_projUniform(bgfx::kInvalidHandle),
      m_cameraPosUniform(bgfx::kInvalidHandle),
      m_timeUniform(bgfx::kInvalidHandle),
      m_lightDirUniform(bgfx::kInvalidHandle),
      m_lightColorUniform(bgfx::kInvalidHandle),
      m_fogColorUniform(bgfx::kInvalidHandle),
      m_fogDensityUniform(bgfx::kInvalidHandle),
      m_totalUniforms(0)
{
}

UniformManager::~UniformManager()
{
    shutdown();
}

bool UniformManager::initialize()
{
    s_logger.info("Initializing uniform manager...");
    
    createCommonUniforms();
    
    s_logger.info("Uniform manager initialized successfully");
    return true;
}

void UniformManager::shutdown()
{
    s_logger.info("Shutting down uniform manager...");
    
    clearAllUniforms();
    destroyCommonUniforms();
    
    s_logger.info("Uniform manager shut down");
}

bgfx::UniformHandle UniformManager::getUniform(const std::string& name, bgfx::UniformType::Enum type, uint16_t num)
{
    // Check if uniform is already created
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) {
        it->second.referenceCount++;
        s_logger.debug("Uniform cache hit: %s", name.c_str());
        return it->second.handle;
    }
    
    // Create new uniform
    bgfx::UniformHandle handle = bgfx::createUniform(name.c_str(), type, num);
    
    if (handle.idx == bgfx::kInvalidHandle) {
        s_logger.error("Failed to create uniform: %s", name.c_str());
        return bgfx::UniformHandle();
    }
    
    // Store in cache
    UniformInfo info;
    info.handle = handle;
    info.name = name;
    info.type = type;
    info.num = num;
    info.referenceCount = 1;
    m_uniformCache[name] = info;
    
    // Update statistics
    m_totalUniforms++;
    
    s_logger.info("Created uniform: %s (type: %d, num: %d)", name.c_str(), type, num);
    
    return handle;
}

void UniformManager::releaseUniform(const std::string& name)
{
    auto it = m_uniformCache.find(name);
    if (it != m_uniformCache.end()) {
        it->second.referenceCount--;
        
        if (it->second.referenceCount == 0) {
            bgfx::destroy(it->second.handle);
            m_totalUniforms--;
            m_uniformCache.erase(it);
        }
    }
}

void UniformManager::releaseUniform(bgfx::UniformHandle handle)
{
    if (handle.idx == bgfx::kInvalidHandle) {
        return;
    }
    
    // Find and release the uniform
    for (auto it = m_uniformCache.begin(); it != m_uniformCache.end(); ++it) {
        if (it->second.handle.idx == handle.idx) {
            it->second.referenceCount--;
            
            if (it->second.referenceCount == 0) {
                bgfx::destroy(it->second.handle);
                m_totalUniforms--;
                m_uniformCache.erase(it);
            }
            break;
        }
    }
}

void UniformManager::setUniform(bgfx::UniformHandle handle, const void* data, uint32_t size)
{
    if (handle.idx == bgfx::kInvalidHandle || !data || size == 0) {
        return;
    }
    
    bgfx::setUniform(handle, data, size);
}

void UniformManager::setUniform(bgfx::UniformHandle handle, float value)
{
    if (handle.idx == bgfx::kInvalidHandle) {
        return;
    }
    
    bgfx::setUniform(handle, &value, sizeof(float));
}

void UniformManager::setUniform(bgfx::UniformHandle handle, float x, float y)
{
    if (handle.idx == bgfx::kInvalidHandle) {
        return;
    }
    
    float vec2[2] = { x, y };
    bgfx::setUniform(handle, vec2, sizeof(float) * 2);
}

void UniformManager::setUniform(bgfx::UniformHandle handle, float x, float y, float z)
{
    if (handle.idx == bgfx::kInvalidHandle) {
        return;
    }
    
    float vec3[3] = { x, y, z };
    bgfx::setUniform(handle, vec3, sizeof(float) * 3);
}

void UniformManager::setUniform(bgfx::UniformHandle handle, float x, float y, float z, float w)
{
    if (handle.idx == bgfx::kInvalidHandle) {
        return;
    }
    
    float vec4[4] = { x, y, z, w };
    bgfx::setUniform(handle, vec4, sizeof(float) * 4);
}

void UniformManager::setUniform(bgfx::UniformHandle handle, const float* matrix)
{
    if (handle.idx == bgfx::kInvalidHandle || !matrix) {
        return;
    }
    
    bgfx::setUniform(handle, matrix, sizeof(float) * 9);
}

void UniformManager::setUniform(bgfx::UniformHandle handle, const float* matrix, bool transpose)
{
    if (handle.idx == bgfx::kInvalidHandle || !matrix) {
        return;
    }
    
    if (transpose) {
        // Transpose the matrix
        float transposed[16];
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                transposed[i * 4 + j] = matrix[j * 4 + i];
            }
        }
        bgfx::setUniform(handle, transposed, sizeof(float) * 16);
    } else {
        bgfx::setUniform(handle, matrix, sizeof(float) * 16);
    }
}

void UniformManager::clearAllUniforms()
{
    for (auto& pair : m_uniformCache) {
        bgfx::destroy(pair.second.handle);
    }
    m_uniformCache.clear();
    
    m_totalUniforms = 0;
}

std::string UniformManager::getStatistics() const
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
        "UniformManager: %u uniforms",
        m_totalUniforms);
    return std::string(buffer);
}

bgfx::UniformHandle UniformManager::getViewProjUniform() const
{
    return m_viewProjUniform;
}

bgfx::UniformHandle UniformManager::getViewUniform() const
{
    return m_viewUniform;
}

bgfx::UniformHandle UniformManager::getProjUniform() const
{
    return m_projUniform;
}

bgfx::UniformHandle UniformManager::getCameraPosUniform() const
{
    return m_cameraPosUniform;
}

bgfx::UniformHandle UniformManager::getTimeUniform() const
{
    return m_timeUniform;
}

bgfx::UniformHandle UniformManager::getLightDirUniform() const
{
    return m_lightDirUniform;
}

bgfx::UniformHandle UniformManager::getLightColorUniform() const
{
    return m_lightColorUniform;
}

bgfx::UniformHandle UniformManager::getFogColorUniform() const
{
    return m_fogColorUniform;
}

bgfx::UniformHandle UniformManager::getFogDensityUniform() const
{
    return m_fogDensityUniform;
}

void UniformManager::createCommonUniforms()
{
    s_logger.info("Creating common uniforms...");
    
    m_viewProjUniform = getUniform("u_viewProj", bgfx::UniformType::Mat4, 1);
    m_viewUniform = getUniform("u_view", bgfx::UniformType::Mat4, 1);
    m_projUniform = getUniform("u_proj", bgfx::UniformType::Mat4, 1);
    m_cameraPosUniform = getUniform("u_cameraPos", bgfx::UniformType::Vec4, 1);
    m_timeUniform = getUniform("u_time", bgfx::UniformType::Vec4, 1);
    m_lightDirUniform = getUniform("u_lightDir", bgfx::UniformType::Vec4, 1);
    m_lightColorUniform = getUniform("u_lightColor", bgfx::UniformType::Vec4, 1);
    m_fogColorUniform = getUniform("u_fogColor", bgfx::UniformType::Vec4, 1);
    m_fogDensityUniform = getUniform("u_fogDensity", bgfx::UniformType::Vec4, 1);
    
    s_logger.info("Common uniforms created");
}

void UniformManager::destroyCommonUniforms()
{
    s_logger.info("Destroying common uniforms...");
    
    if (m_viewProjUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_viewProjUniform);
    if (m_viewUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_viewUniform);
    if (m_projUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_projUniform);
    if (m_cameraPosUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_cameraPosUniform);
    if (m_timeUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_timeUniform);
    if (m_lightDirUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_lightDirUniform);
    if (m_lightColorUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_lightColorUniform);
    if (m_fogColorUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_fogColorUniform);
    if (m_fogDensityUniform.idx != bgfx::kInvalidHandle) bgfx::destroy(m_fogDensityUniform);
    
    m_viewProjUniform.idx = bgfx::kInvalidHandle;
    m_viewUniform.idx = bgfx::kInvalidHandle;
    m_projUniform.idx = bgfx::kInvalidHandle;
    m_cameraPosUniform.idx = bgfx::kInvalidHandle;
    m_timeUniform.idx = bgfx::kInvalidHandle;
    m_lightDirUniform.idx = bgfx::kInvalidHandle;
    m_lightColorUniform.idx = bgfx::kInvalidHandle;
    m_fogColorUniform.idx = bgfx::kInvalidHandle;
    m_fogDensityUniform.idx = bgfx::kInvalidHandle;
    
    s_logger.info("Common uniforms destroyed");
}

} // namespace renderbridge
