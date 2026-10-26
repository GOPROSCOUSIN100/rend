#pragma once

#include <cstdint>

namespace renderbridge {

/**
 * @class Weather
 * @brief Represents weather conditions in the scene
 * 
 * This class stores weather data extracted from Minecraft and
 * provides methods for working with weather in the renderer.
 */
class Weather {
public:
    // Weather types
    enum Type : uint8_t {
        CLEAR = 0,
        RAIN = 1,
        THUNDERSTORM = 2,
        SNOW = 3
    };

public:
    /**
     * @brief Constructor
     */
    Weather();
    
    /**
     * @brief Destructor
     */
    ~Weather();
    
    /**
     * @brief Parse weather data from a message payload
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     * @return True if parsing succeeded
     */
    bool parseFromPayload(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Serialize weather data to a buffer
     * @param buffer Output buffer
     * @param bufferSize Size of the buffer
     * @return Number of bytes written
     */
    uint32_t serializeToBuffer(uint8_t* buffer, uint32_t bufferSize) const;
    
    // Type accessors
    Type getType() const;
    void setType(Type type);
    
    // Strength accessors (0.0 to 1.0)
    float getStrength() const;
    void setStrength(float strength);
    
    // Time accessors (in ticks)
    int64_t getTime() const;
    void setTime(int64_t time);
    
    // Rain gradient accessors (0.0 to 1.0)
    float getRainGradient() const;
    void setRainGradient(float gradient);
    
    // Thunder gradient accessors (0.0 to 1.0)
    float getThunderGradient() const;
    void setThunderGradient(float gradient);
    
    // Fog density accessors
    float getFogDensity() const;
    void setFogDensity(float density);
    
    // Fog color accessors (RGB, 0.0 to 1.0)
    float getFogColorR() const;
    float getFogColorG() const;
    float getFogColorB() const;
    void setFogColor(float r, float g, float b);
    
    // Wind direction accessors (in degrees)
    float getWindDirection() const;
    void setWindDirection(float direction);
    
    // Wind strength accessors
    float getWindStrength() const;
    void setWindStrength(float strength);
    
    // Temperature accessors
    float getTemperature() const;
    void setTemperature(float temperature);
    
    // Wetness accessors (0.0 to 1.0)
    float getWetness() const;
    void setWetness(float wetness);
    
    // Check if weather has changed
    /**
     * @brief Check if the weather has changed since the last update
     * @return True if the weather has changed
     */
    bool hasChanged() const;
    
    /**
     * @brief Mark the weather as unchanged
     */
    void markUnchanged();
    
    /**
     * @brief Update the weather from another weather
     * @param other Other weather
     */
    void updateFrom(const Weather& other);
    
    /**
     * @brief Get weather uniforms for shader
     * @param output Output array (16 floats for uniform buffer)
     */
    void getUniforms(float* output) const;

private:
    Type m_type;
    float m_strength;
    int64_t m_time;
    float m_rainGradient;
    float m_thunderGradient;
    float m_fogDensity;
    float m_fogColorR;
    float m_fogColorG;
    float m_fogColorB;
    float m_windDirection;
    float m_windStrength;
    float m_temperature;
    float m_wetness;
    
    // Dirty flag for change detection
    mutable bool m_dirty;
    
    // Previous values for change detection
    mutable Type m_previousType;
    mutable float m_previousStrength;
    mutable float m_previousRainGradient;
    mutable float m_previousThunderGradient;
    mutable float m_previousFogDensity;
    mutable float m_previousFogColorR;
    mutable float m_previousFogColorG;
    mutable float m_previousFogColorB;
};

} // namespace renderbridge
