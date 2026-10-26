#include "weather.hpp"

#include <cstring>
#include <algorithm>
#include <cmath>

namespace renderbridge {

Weather::Weather()
    : m_type(CLEAR),
      m_strength(0.0f),
      m_time(0),
      m_rainGradient(0.0f),
      m_thunderGradient(0.0f),
      m_fogDensity(0.0f),
      m_fogColorR(0.53f), m_fogColorG(0.81f), m_fogColorB(1.0f),
      m_windDirection(0.0f),
      m_windStrength(0.0f),
      m_temperature(0.5f),
      m_wetness(0.0f),
      m_dirty(true),
      m_previousType(CLEAR),
      m_previousStrength(0.0f),
      m_previousRainGradient(0.0f),
      m_previousThunderGradient(0.0f),
      m_previousFogDensity(0.0f),
      m_previousFogColorR(0.53f), m_previousFogColorG(0.81f), m_previousFogColorB(1.0f)
{
}

Weather::~Weather()
{
}

// Parse weather data from a message payload
// Payload format:
// type(uint8), strength(float), time(int64),
// rainGradient(float), thunderGradient(float),
// fogDensity(float), fogColorR(float), fogColorG(float), fogColorB(float),
// windDirection(float), windStrength(float),
// temperature(float), wetness(float)
bool Weather::parseFromPayload(const uint8_t* payload, uint32_t payloadSize)
{
    if (payload == nullptr || payloadSize < 53) {
        // Minimum size: 1 + 4 + 8 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 = 53 bytes
        return false;
    }

    const uint8_t* ptr = payload;
    uint32_t offset = 0;

    // Read type (uint8)
    m_previousType = m_type;
    m_type = static_cast<Type>(*reinterpret_cast<const uint8_t*>(ptr + offset));
    offset += 1;

    // Read strength (float)
    m_previousStrength = m_strength;
    m_strength = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    // Read time (int64)
    m_time = *reinterpret_cast<const int64_t*>(ptr + offset);
    offset += 8;

    // Read rain gradient (float)
    m_previousRainGradient = m_rainGradient;
    m_rainGradient = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    // Read thunder gradient (float)
    m_previousThunderGradient = m_thunderGradient;
    m_thunderGradient = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    // Read fog density (float)
    m_previousFogDensity = m_fogDensity;
    m_fogDensity = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    // Read fog color (3 floats)
    m_previousFogColorR = m_fogColorR;
    m_previousFogColorG = m_fogColorG;
    m_previousFogColorB = m_fogColorB;
    m_fogColorR = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_fogColorG = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;
    m_fogColorB = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    // Read wind direction (float)
    m_windDirection = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    // Read wind strength (float)
    m_windStrength = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    // Read temperature (float)
    m_temperature = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    // Read wetness (float)
    m_wetness = *reinterpret_cast<const float*>(ptr + offset);
    offset += 4;

    m_dirty = true;
    return true;
}

// Serialize weather data to a buffer
uint32_t Weather::serializeToBuffer(uint8_t* buffer, uint32_t bufferSize) const
{
    if (buffer == nullptr || bufferSize < 53) {
        return 0;
    }

    uint8_t* ptr = buffer;
    uint32_t offset = 0;

    // Write type (uint8)
    *reinterpret_cast<uint8_t*>(ptr + offset) = static_cast<uint8_t>(m_type);
    offset += 1;

    // Write strength (float)
    *reinterpret_cast<float*>(ptr + offset) = m_strength;
    offset += 4;

    // Write time (int64)
    *reinterpret_cast<int64_t*>(ptr + offset) = m_time;
    offset += 8;

    // Write rain gradient (float)
    *reinterpret_cast<float*>(ptr + offset) = m_rainGradient;
    offset += 4;

    // Write thunder gradient (float)
    *reinterpret_cast<float*>(ptr + offset) = m_thunderGradient;
    offset += 4;

    // Write fog density (float)
    *reinterpret_cast<float*>(ptr + offset) = m_fogDensity;
    offset += 4;

    // Write fog color (3 floats)
    *reinterpret_cast<float*>(ptr + offset) = m_fogColorR;
    offset += 4;
    *reinterpret_cast<float*>(ptr + offset) = m_fogColorG;
    offset += 4;
    *reinterpret_cast<float*>(ptr + offset) = m_fogColorB;
    offset += 4;

    // Write wind direction (float)
    *reinterpret_cast<float*>(ptr + offset) = m_windDirection;
    offset += 4;

    // Write wind strength (float)
    *reinterpret_cast<float*>(ptr + offset) = m_windStrength;
    offset += 4;

    // Write temperature (float)
    *reinterpret_cast<float*>(ptr + offset) = m_temperature;
    offset += 4;

    // Write wetness (float)
    *reinterpret_cast<float*>(ptr + offset) = m_wetness;
    offset += 4;

    return offset;
}

// Type accessors
Weather::Type Weather::getType() const {
    return m_type;
}

void Weather::setType(Type type) {
    if (m_type != type) {
        m_type = type;
        m_dirty = true;
    }
}

// Strength accessors
float Weather::getStrength() const {
    return m_strength;
}

void Weather::setStrength(float strength) {
    if (m_strength != strength) {
        m_strength = strength;
        m_dirty = true;
    }
}

// Time accessors
int64_t Weather::getTime() const {
    return m_time;
}

void Weather::setTime(int64_t time) {
    if (m_time != time) {
        m_time = time;
        m_dirty = true;
    }
}

// Rain gradient accessors
float Weather::getRainGradient() const {
    return m_rainGradient;
}

void Weather::setRainGradient(float gradient) {
    if (m_rainGradient != gradient) {
        m_rainGradient = gradient;
        m_dirty = true;
    }
}

// Thunder gradient accessors
float Weather::getThunderGradient() const {
    return m_thunderGradient;
}

void Weather::setThunderGradient(float gradient) {
    if (m_thunderGradient != gradient) {
        m_thunderGradient = gradient;
        m_dirty = true;
    }
}

// Fog density accessors
float Weather::getFogDensity() const {
    return m_fogDensity;
}

void Weather::setFogDensity(float density) {
    if (m_fogDensity != density) {
        m_fogDensity = density;
        m_dirty = true;
    }
}

// Fog color accessors
float Weather::getFogColorR() const {
    return m_fogColorR;
}

float Weather::getFogColorG() const {
    return m_fogColorG;
}

float Weather::getFogColorB() const {
    return m_fogColorB;
}

void Weather::setFogColor(float r, float g, float b) {
    if (m_fogColorR != r || m_fogColorG != g || m_fogColorB != b) {
        m_fogColorR = r;
        m_fogColorG = g;
        m_fogColorB = b;
        m_dirty = true;
    }
}

// Wind direction accessors
float Weather::getWindDirection() const {
    return m_windDirection;
}

void Weather::setWindDirection(float direction) {
    if (m_windDirection != direction) {
        m_windDirection = direction;
        m_dirty = true;
    }
}

// Wind strength accessors
float Weather::getWindStrength() const {
    return m_windStrength;
}

void Weather::setWindStrength(float strength) {
    if (m_windStrength != strength) {
        m_windStrength = strength;
        m_dirty = true;
    }
}

// Temperature accessors
float Weather::getTemperature() const {
    return m_temperature;
}

void Weather::setTemperature(float temperature) {
    if (m_temperature != temperature) {
        m_temperature = temperature;
        m_dirty = true;
    }
}

// Wetness accessors
float Weather::getWetness() const {
    return m_wetness;
}

void Weather::setWetness(float wetness) {
    if (m_wetness != wetness) {
        m_wetness = wetness;
        m_dirty = true;
    }
}

// Change detection
bool Weather::hasChanged() const {
    return m_dirty;
}

void Weather::markUnchanged() {
    m_dirty = false;
    m_previousType = m_type;
    m_previousStrength = m_strength;
    m_previousRainGradient = m_rainGradient;
    m_previousThunderGradient = m_thunderGradient;
    m_previousFogDensity = m_fogDensity;
    m_previousFogColorR = m_fogColorR;
    m_previousFogColorG = m_fogColorG;
    m_previousFogColorB = m_fogColorB;
}

// Update from another weather
void Weather::updateFrom(const Weather& other) {
    m_type = other.m_type;
    m_strength = other.m_strength;
    m_time = other.m_time;
    m_rainGradient = other.m_rainGradient;
    m_thunderGradient = other.m_thunderGradient;
    m_fogDensity = other.m_fogDensity;
    m_fogColorR = other.m_fogColorR;
    m_fogColorG = other.m_fogColorG;
    m_fogColorB = other.m_fogColorB;
    m_windDirection = other.m_windDirection;
    m_windStrength = other.m_windStrength;
    m_temperature = other.m_temperature;
    m_wetness = other.m_wetness;
    m_dirty = true;
}

// Get weather uniforms for shader
// Layout: [type, strength, rainGradient, thunderGradient, fogDensity, fogColorR, fogColorG, fogColorB,
//         windDirection, windStrength, temperature, wetness, padding, padding, padding, padding]
void Weather::getUniforms(float* output) const {
    if (output == nullptr) return;
    
    output[0] = static_cast<float>(m_type);
    output[1] = m_strength;
    output[2] = m_rainGradient;
    output[3] = m_thunderGradient;
    output[4] = m_fogDensity;
    output[5] = m_fogColorR;
    output[6] = m_fogColorG;
    output[7] = m_fogColorB;
    output[8] = m_windDirection;
    output[9] = m_windStrength;
    output[10] = m_temperature;
    output[11] = m_wetness;
    output[12] = 0.0f;
    output[13] = 0.0f;
    output[14] = 0.0f;
    output[15] = 0.0f;
}

} // namespace renderbridge
