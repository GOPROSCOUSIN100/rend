#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

// bgfx includes
#include <bgfx/bgfx.h>

namespace renderbridge {

/**
 * @class TextureManager
 * @brief Manages textures for the renderer
 */
class TextureManager {
public:
    /**
     * @brief Constructor
     */
    TextureManager();
    
    /**
     * @brief Destructor
     */
    ~TextureManager();
    
    /**
     * @brief Initialize the texture manager
     * @return True if initialization succeeded
     */
    bool initialize();
    
    /**
     * @brief Shutdown the texture manager
     */
    void shutdown();
    
    /**
     * @brief Load a texture from a file
     * @param path File path
     * @return Texture handle
     */
    bgfx::TextureHandle loadTexture(const std::string& path);
    
    /**
     * @brief Load a texture from memory
     * @param data Image data
     * @param width Width
     * @param height Height
     * @param channels Number of channels (3 or 4)
     * @return Texture handle
     */
    bgfx::TextureHandle loadTextureFromMemory(const uint8_t* data, uint32_t width, uint32_t height, uint8_t channels);
    
    /**
     * @brief Create a solid color texture
     * @param r Red component (0.0 to 1.0)
     * @param g Green component (0.0 to 1.0)
     * @param b Blue component (0.0 to 1.0)
     * @param a Alpha component (0.0 to 1.0)
     * @return Texture handle
     */
    bgfx::TextureHandle createSolidColorTexture(float r, float g, float b, float a);
    
    /**
     * @brief Create a texture array
     * @param textures Vector of texture handles
     * @return Texture array handle
     */
    bgfx::TextureHandle createTextureArray(const std::vector<bgfx::TextureHandle>& textures);
    
    /**
     * @brief Get a texture by name
     * @param name Texture name
     * @return Texture handle, or invalid handle if not found
     */
    bgfx::TextureHandle getTexture(const std::string& name) const;
    
    /**
     * @brief Release a texture
     * @param name Texture name
     */
    void releaseTexture(const std::string& name);
    
    /**
     * @brief Release a texture by handle
     * @param handle Texture handle
     */
    void releaseTexture(bgfx::TextureHandle handle);
    
    /**
     * @brief Get the white texture
     * @return White texture handle
     */
    bgfx::TextureHandle getWhiteTexture() const;
    
    /**
     * @brief Get the black texture
     * @return Black texture handle
     */
    bgfx::TextureHandle getBlackTexture() const;
    
    /**
     * @brief Get the missing texture (magenta)
     * @return Missing texture handle
     */
    bgfx::TextureHandle getMissingTexture() const;
    
    /**
     * @brief Get the default normal map
     * @return Default normal map handle
     */
    bgfx::TextureHandle getDefaultNormalMap() const;
    
    /**
     * @brief Get the default specular map
     * @return Default specular map handle
     */
    bgfx::TextureHandle getDefaultSpecularMap() const;
    
    /**
     * @brief Clear all textures
     */
    void clearAllTextures();
    
    /**
     * @brief Get statistics
     * @return Statistics string
     */
    std::string getStatistics() const;

private:
    // Texture cache
    struct TextureInfo {
        bgfx::TextureHandle handle;
        std::string path;
        uint32_t width;
        uint32_t height;
        uint32_t referenceCount;
    };
    
    std::unordered_map<std::string, TextureInfo> m_textureCache;
    
    // Default textures
    bgfx::TextureHandle m_whiteTexture;
    bgfx::TextureHandle m_blackTexture;
    bgfx::TextureHandle m_missingTexture;
    bgfx::TextureHandle m_defaultNormalMap;
    bgfx::TextureHandle m_defaultSpecularMap;
    
    // Statistics
    uint32_t m_totalTextures;
    uint32_t m_totalTextureMemory;
    
    /**
     * @brief Create default textures
     */
    void createDefaultTextures();
    
    /**
     * @brief Destroy default textures
     */
    void destroyDefaultTextures();
    
    /**
     * @brief Load an image file
     * @param path File path
     * @param width Output: width
     * @param height Output: height
     * @param channels Output: number of channels
     * @param data Output: image data
     * @return True if loading succeeded
     */
    bool loadImageFile(const std::string& path, uint32_t& width, uint32_t& height, uint8_t& channels, std::vector<uint8_t>& data);
    
    /**
     * @brief Determine bgfx texture format from number of channels
     * @param channels Number of channels
     * @return bgfx texture format
     */
    bgfx::TextureFormat::Enum getTextureFormat(uint8_t channels) const;
    
    /**
     * @brief Calculate texture memory size
     * @param width Width
     * @param height Height
     * @param format Texture format
     * @return Memory size in bytes
     */
    uint32_t calculateTextureMemory(uint32_t width, uint32_t height, bgfx::TextureFormat::Enum format) const;
};

} // namespace renderbridge
