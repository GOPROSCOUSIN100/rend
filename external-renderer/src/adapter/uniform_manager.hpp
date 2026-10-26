#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// bgfx includes
#include <bgfx/bgfx.h>

namespace renderbridge {

/**
 * @class UniformManager
 * @brief Manages bgfx uniforms for the renderer
 */
class UniformManager {
public:
    /**
     * @brief Constructor
     */
    UniformManager();
    
    /**
     * @brief Destructor
     */
    ~UniformManager();
    
    /**
     * @brief Initialize the uniform manager
     * @return True if initialization succeeded
     */
    bool initialize();
    
    /**
     * @brief Shutdown the uniform manager
     */
    void shutdown();
    
    /**
     * @brief Get or create a uniform
     * @param name Uniform name
     * @param type Uniform type
     * @param num Number of elements (for arrays)
     * @return Uniform handle
     */
    bgfx::UniformHandle getUniform(const std::string& name, bgfx::UniformType::Enum type, uint16_t num = 1);
    
    /**
     * @brief Release a uniform
     * @param name Uniform name
     */
    void releaseUniform(const std::string& name);
    
    /**
     * @brief Release a uniform by handle
     * @param handle Uniform handle
     */
    void releaseUniform(bgfx::UniformHandle handle);
    
    /**
     * @brief Set a uniform value
     * @param handle Uniform handle
     * @param data Data to set
     * @param size Size of data in bytes
     */
    void setUniform(bgfx::UniformHandle handle, const void* data, uint32_t size);
    
    /**
     * @brief Set a uniform value (convenience for float)
     * @param handle Uniform handle
     * @param value Value to set
     */
    void setUniform(bgfx::UniformHandle handle, float value);
    
    /**
     * @brief Set a uniform value (convenience for vec2)
     * @param handle Uniform handle
     * @param x X component
     * @param y Y component
     */
    void setUniform(bgfx::UniformHandle handle, float x, float y);
    
    /**
     * @brief Set a uniform value (convenience for vec3)
     * @param handle Uniform handle
     * @param x X component
     * @param y Y component
     * @param z Z component
     */
    void setUniform(bgfx::UniformHandle handle, float x, float y, float z);
    
    /**
     * @brief Set a uniform value (convenience for vec4)
     * @param handle Uniform handle
     * @param x X component
     * @param y Y component
     * @param z Z component
     * @param w W component
     */
    void setUniform(bgfx::UniformHandle handle, float x, float y, float z, float w);
    
    /**
     * @brief Set a uniform value (convenience for mat3)
     * @param handle Uniform handle
     * @param matrix Matrix data (9 floats)
     */
    void setUniform(bgfx::UniformHandle handle, const float* matrix);
    
    /**
     * @brief Set a uniform value (convenience for mat4)
     * @param handle Uniform handle
     * @param matrix Matrix data (16 floats)
     */
    void setUniform(bgfx::UniformHandle handle, const float* matrix, bool transpose);
    
    /**
     * @brief Clear all uniforms
     */
    void clearAllUniforms();
    
    /**
     * @brief Get statistics
     * @return Statistics string
     */
    std::string getStatistics() const;
    
    // ========================================================================
    // Common uniform accessors
    // ========================================================================
    
    /**
     * @brief Get the view-projection matrix uniform
     * @return Uniform handle
     */
    bgfx::UniformHandle getViewProjUniform() const;
    
    /**
     * @brief Get the view matrix uniform
     * @return Uniform handle
     */
    bgfx::UniformHandle getViewUniform() const;
    
    /**
     * @brief Get the projection matrix uniform
     * @return Uniform handle
     */
    bgfx::UniformHandle getProjUniform() const;
    
    /**
     * @brief Get the camera position uniform
     * @return Uniform handle
     */
    bgfx::UniformHandle getCameraPosUniform() const;
    
    /**
     * @brief Get the time uniform
     * @return Uniform handle
     */
    bgfx::UniformHandle getTimeUniform() const;
    
    /**
     * @brief Get the light direction uniform
     * @return Uniform handle
     */
    bgfx::UniformHandle getLightDirUniform() const;
    
    /**
     * @brief Get the light color uniform
     * @return Uniform handle
     */
    bgfx::UniformHandle getLightColorUniform() const;
    
    /**
     * @brief Get the fog color uniform
     * @return Uniform handle
     */
    bgfx::UniformHandle getFogColorUniform() const;
    
    /**
     * @brief Get the fog density uniform
     * @return Uniform handle
     */
    bgfx::UniformHandle getFogDensityUniform() const;

private:
    // Uniform cache
    struct UniformInfo {
        bgfx::UniformHandle handle;
        std::string name;
        bgfx::UniformType::Enum type;
        uint16_t num;
        uint32_t referenceCount;
    };
    
    std::unordered_map<std::string, UniformInfo> m_uniformCache;
    
    // Common uniforms (cached for quick access)
    bgfx::UniformHandle m_viewProjUniform;
    bgfx::UniformHandle m_viewUniform;
    bgfx::UniformHandle m_projUniform;
    bgfx::UniformHandle m_cameraPosUniform;
    bgfx::UniformHandle m_timeUniform;
    bgfx::UniformHandle m_lightDirUniform;
    bgfx::UniformHandle m_lightColorUniform;
    bgfx::UniformHandle m_fogColorUniform;
    bgfx::UniformHandle m_fogDensityUniform;
    
    // Statistics
    uint32_t m_totalUniforms;
    
    /**
     * @brief Create common uniforms
     */
    void createCommonUniforms();
    
    /**
     * @brief Destroy common uniforms
     */
    void destroyCommonUniforms();
};

} // namespace renderbridge
