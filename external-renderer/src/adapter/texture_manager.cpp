#include "texture_manager.hpp"
#include "utils/logger.hpp"

#include <bgfx/bgfx.h>

#include <cstring>
#include <algorithm>

namespace renderbridge {

// Static logger
static Logger s_logger("TextureManager");

TextureManager::TextureManager()
    : m_whiteTexture(bgfx::kInvalidHandle),
      m_blackTexture(bgfx::kInvalidHandle),
      m_missingTexture(bgfx::kInvalidHandle),
      m_defaultNormalMap(bgfx::kInvalidHandle),
      m_defaultSpecularMap(bgfx::kInvalidHandle),
      m_totalTextures(0),
      m_totalTextureMemory(0)
{
}

TextureManager::~TextureManager()
{
    shutdown();
}

bool TextureManager::initialize()
{
    s_logger.info("Initializing texture manager...");
    
    createDefaultTextures();
    
    s_logger.info("Texture manager initialized successfully");
    return true;
}

void TextureManager::shutdown()
{
    s_logger.info("Shutting down texture manager...");
    
    clearAllTextures();
    destroyDefaultTextures();
    
    s_logger.info("Texture manager shut down");
}

bgfx::TextureHandle TextureManager::loadTexture(const std::string& path)
{
    // Check if texture is already loaded
    auto it = m_textureCache.find(path);
    if (it != m_textureCache.end()) {
        it->second.referenceCount++;
        s_logger.debug("Texture cache hit: %s", path.c_str());
        return it->second.handle;
    }
    
    // Load image file
    uint32_t width, height;
    uint8_t channels;
    std::vector<uint8_t> data;
    
    if (!loadImageFile(path, width, height, channels, data)) {
        s_logger.warn("Failed to load texture: %s", path.c_str());
        return m_missingTexture;
    }
    
    // Create texture
    bgfx::TextureFormat::Enum format = getTextureFormat(channels);
    bgfx::TextureHandle handle = bgfx::createTexture(
        bgfx::TextureInfo::create(width, height, 1, false, false, 1, format)
    );
    
    if (handle.idx == bgfx::kInvalidHandle) {
        s_logger.error("Failed to create texture: %s", path.c_str());
        return m_missingTexture;
    }
    
    // Update texture data
    bgfx::updateTexture(handle, 0, 0, 0, 0, width, height, data.data());
    
    // Store in cache
    TextureInfo info;
    info.handle = handle;
    info.path = path;
    info.width = width;
    info.height = height;
    info.referenceCount = 1;
    m_textureCache[path] = info;
    
    // Update statistics
    m_totalTextures++;
    m_totalTextureMemory += calculateTextureMemory(width, height, format);
    
    s_logger.info("Loaded texture: %s (%ux%u, %d channels)", path.c_str(), width, height, channels);
    
    return handle;
}

bgfx::TextureHandle TextureManager::loadTextureFromMemory(const uint8_t* data, uint32_t width, uint32_t height, uint8_t channels)
{
    if (!data || width == 0 || height == 0 || channels < 3 || channels > 4) {
        s_logger.error("Invalid parameters for loadTextureFromMemory");
        return m_missingTexture;
    }
    
    // Create texture
    bgfx::TextureFormat::Enum format = getTextureFormat(channels);
    bgfx::TextureHandle handle = bgfx::createTexture(
        bgfx::TextureInfo::create(width, height, 1, false, false, 1, format)
    );
    
    if (handle.idx == bgfx::kInvalidHandle) {
        s_logger.error("Failed to create texture from memory");
        return m_missingTexture;
    }
    
    // Update texture data
    bgfx::updateTexture(handle, 0, 0, 0, 0, width, height, data);
    
    s_logger.info("Created texture from memory: %ux%u, %d channels", width, height, channels);
    
    return handle;
}

bgfx::TextureHandle TextureManager::createSolidColorTexture(float r, float g, float b, float a)
{
    // Clamp values
    r = std::max(0.0f, std::min(1.0f, r));
    g = std::max(0.0f, std::min(1.0f, g));
    b = std::max(0.0f, std::min(1.0f, b));
    a = std::max(0.0f, std::min(1.0f, a));
    
    // Create a 1x1 texture
    uint32_t color = (
        (static_cast<uint8_t>(a * 255) << 24) |
        (static_cast<uint8_t>(b * 255) << 16) |
        (static_cast<uint8_t>(g * 255) << 8) |
        (static_cast<uint8_t>(r * 255) << 0)
    );
    
    bgfx::TextureHandle handle = bgfx::createTexture(
        bgfx::TextureInfo::create(1, 1, 1, false, false, 1, bgfx::TextureFormat::BGRA8)
    );
    
    if (handle.idx != bgfx::kInvalidHandle) {
        bgfx::updateTexture(handle, 0, 0, 0, 0, 1, 1, &color);
    }
    
    return handle;
}

bgfx::TextureHandle TextureManager::createTextureArray(const std::vector<bgfx::TextureHandle>& textures)
{
    if (textures.empty()) {
        return bgfx::TextureHandle();
    }
    
    // Get dimensions from first texture
    uint32_t width = 0;
    uint32_t height = 0;
    bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Count;
    
    // In a real implementation, we would query the texture info
    // For now, assume all textures are the same size
    
    // Create texture array
    bgfx::TextureHandle handle = bgfx::createTexture(
        bgfx::TextureInfo::create(width, height, 1, false, true, static_cast<uint16_t>(textures.size()), format)
    );
    
    return handle;
}

bgfx::TextureHandle TextureManager::getTexture(const std::string& name) const
{
    auto it = m_textureCache.find(name);
    if (it != m_textureCache.end()) {
        return it->second.handle;
    }
    return bgfx::TextureHandle();
}

void TextureManager::releaseTexture(const std::string& name)
{
    auto it = m_textureCache.find(name);
    if (it != m_textureCache.end()) {
        it->second.referenceCount--;
        
        if (it->second.referenceCount == 0) {
            bgfx::destroy(it->second.handle);
            m_totalTextures--;
            m_totalTextureMemory -= calculateTextureMemory(
                it->second.width, it->second.height, 
                getTextureFormat(4) // Assume RGBA
            );
            m_textureCache.erase(it);
        }
    }
}

void TextureManager::releaseTexture(bgfx::TextureHandle handle)
{
    if (handle.idx == bgfx::kInvalidHandle) {
        return;
    }
    
    // Find and release the texture
    for (auto it = m_textureCache.begin(); it != m_textureCache.end(); ++it) {
        if (it->second.handle.idx == handle.idx) {
            it->second.referenceCount--;
            
            if (it->second.referenceCount == 0) {
                bgfx::destroy(it->second.handle);
                m_totalTextures--;
                m_totalTextureMemory -= calculateTextureMemory(
                    it->second.width, it->second.height,
                    getTextureFormat(4)
                );
                m_textureCache.erase(it);
            }
            break;
        }
    }
}

bgfx::TextureHandle TextureManager::getWhiteTexture() const
{
    return m_whiteTexture;
}

bgfx::TextureHandle TextureManager::getBlackTexture() const
{
    return m_blackTexture;
}

bgfx::TextureHandle TextureManager::getMissingTexture() const
{
    return m_missingTexture;
}

bgfx::TextureHandle TextureManager::getDefaultNormalMap() const
{
    return m_defaultNormalMap;
}

bgfx::TextureHandle TextureManager::getDefaultSpecularMap() const
{
    return m_defaultSpecularMap;
}

void TextureManager::clearAllTextures()
{
    for (auto& pair : m_textureCache) {
        bgfx::destroy(pair.second.handle);
    }
    m_textureCache.clear();
    
    m_totalTextures = 0;
    m_totalTextureMemory = 0;
}

std::string TextureManager::getStatistics() const
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
        "TextureManager: %u textures, %.2f MB memory",
        m_totalTextures, m_totalTextureMemory / (1024.0f * 1024.0f));
    return std::string(buffer);
}

void TextureManager::createDefaultTextures()
{
    s_logger.info("Creating default textures...");
    
    // White texture
    m_whiteTexture = createSolidColorTexture(1.0f, 1.0f, 1.0f, 1.0f);
    
    // Black texture
    m_blackTexture = createSolidColorTexture(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Missing texture (magenta)
    m_missingTexture = createSolidColorTexture(1.0f, 0.0f, 1.0f, 1.0f);
    
    // Default normal map (flat normal: 0,0,1 encoded as 0.5,0.5,1.0)
    // For now, use a solid blue texture as placeholder
    m_defaultNormalMap = createSolidColorTexture(0.5f, 0.5f, 1.0f, 1.0f);
    
    // Default specular map (low specular)
    m_defaultSpecularMap = createSolidColorTexture(0.1f, 0.1f, 0.1f, 1.0f);
    
    s_logger.info("Default textures created");
}

void TextureManager::destroyDefaultTextures()
{
    s_logger.info("Destroying default textures...");
    
    if (m_whiteTexture.idx != bgfx::kInvalidHandle) bgfx::destroy(m_whiteTexture);
    if (m_blackTexture.idx != bgfx::kInvalidHandle) bgfx::destroy(m_blackTexture);
    if (m_missingTexture.idx != bgfx::kInvalidHandle) bgfx::destroy(m_missingTexture);
    if (m_defaultNormalMap.idx != bgfx::kInvalidHandle) bgfx::destroy(m_defaultNormalMap);
    if (m_defaultSpecularMap.idx != bgfx::kInvalidHandle) bgfx::destroy(m_defaultSpecularMap);
    
    m_whiteTexture.idx = bgfx::kInvalidHandle;
    m_blackTexture.idx = bgfx::kInvalidHandle;
    m_missingTexture.idx = bgfx::kInvalidHandle;
    m_defaultNormalMap.idx = bgfx::kInvalidHandle;
    m_defaultSpecularMap.idx = bgfx::kInvalidHandle;
    
    s_logger.info("Default textures destroyed");
}

bool TextureManager::loadImageFile(const std::string& path, uint32_t& width, uint32_t& height, uint8_t& channels, std::vector<uint8_t>& data)
{
    // In a real implementation, this would use stb_image or similar
    // For now, return false to indicate failure
    
    s_logger.error("Image loading not implemented yet. Use stb_image or similar.");
    return false;
}

bgfx::TextureFormat::Enum TextureManager::getTextureFormat(uint8_t channels) const
{
    switch (channels) {
        case 1:
            return bgfx::TextureFormat::R8;
        case 2:
            return bgfx::TextureFormat::RG8;
        case 3:
            return bgfx::TextureFormat::RGB8;
        case 4:
            return bgfx::TextureFormat::RGBA8;
        default:
            return bgfx::TextureFormat::RGBA8;
    }
}

uint32_t TextureManager::calculateTextureMemory(uint32_t width, uint32_t height, bgfx::TextureFormat::Enum format) const
{
    // Calculate memory size in bytes
    uint32_t bytesPerPixel = 0;
    
    switch (format) {
        case bgfx::TextureFormat::R8:
            bytesPerPixel = 1;
            break;
        case bgfx::TextureFormat::RG8:
            bytesPerPixel = 2;
            break;
        case bgfx::TextureFormat::RGB8:
            bytesPerPixel = 3;
            break;
        case bgfx::TextureFormat::RGBA8:
            bytesPerPixel = 4;
            break;
        case bgfx::TextureFormat::RGB10A2:
            bytesPerPixel = 4;
            break;
        case bgfx::TextureFormat::RGBA16:
            bytesPerPixel = 8;
            break;
        case bgfx::TextureFormat::RGB16F:
        case bgfx::TextureFormat::RGBA16F:
            bytesPerPixel = 8;
            break;
        case bgfx::TextureFormat::RGB32F:
        case bgfx::TextureFormat::RGBA32F:
            bytesPerPixel = 16;
            break;
        default:
            bytesPerPixel = 4;
            break;
    }
    
    return width * height * bytesPerPixel;
}

} // namespace renderbridge
