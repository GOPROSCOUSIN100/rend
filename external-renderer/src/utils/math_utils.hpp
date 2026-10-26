#pragma once

#include <cstdint>
#include <cmath>

namespace renderbridge {

/**
 * @class MathUtils
 * @brief Collection of math utilities
 */
class MathUtils {
public:
    // Constants
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float TWO_PI = 6.28318530717958647692f;
    static constexpr float HALF_PI = 1.57079632679489661923f;
    static constexpr float DEG_TO_RAD = PI / 180.0f;
    static constexpr float RAD_TO_DEG = 180.0f / PI;
    static constexpr float EPSILON = 1e-6f;
    
    // ========================================================================
    // Basic math functions
    // ========================================================================
    
    /**
     * @brief Clamp a value between min and max
     * @tparam T Value type
     * @param value Value to clamp
     * @param min Minimum value
     * @param max Maximum value
     * @return Clamped value
     */
    template<typename T>
    static T clamp(T value, T min, T max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
    
    /**
     * @brief Linear interpolation
     * @tparam T Value type
     * @param a Start value
     * @param b End value
     * @param t Interpolation factor (0.0 to 1.0)
     * @return Interpolated value
     */
    template<typename T>
    static T lerp(T a, T b, float t) {
        return a + (b - a) * t;
    }
    
    /**
     * @brief Inverse linear interpolation
     * @param a Start value
     * @param b End value
     * @param value Value to find interpolation factor for
     * @return Interpolation factor (0.0 to 1.0)
     */
    template<typename T>
    static float inverseLerp(T a, T b, T value) {
        if (a == b) return 0.0f;
        return static_cast<float>(value - a) / static_cast<float>(b - a);
    }
    
    /**
     * @brief Remap a value from one range to another
     * @tparam T Value type
     * @param value Value to remap
     * @param fromMin From range minimum
     * @param fromMax From range maximum
     * @param toMin To range minimum
     * @param toMax To range maximum
     * @return Remapped value
     */
    template<typename T>
    static T remap(T value, T fromMin, T fromMax, T toMin, T toMax) {
        float t = inverseLerp(fromMin, fromMax, value);
        return lerp(toMin, toMax, t);
    }
    
    // ========================================================================
    // Trigonometry
    // ========================================================================
    
    /**
     * @brief Convert degrees to radians
     * @param degrees Angle in degrees
     * @return Angle in radians
     */
    static float degToRad(float degrees) {
        return degrees * DEG_TO_RAD;
    }
    
    /**
     * @brief Convert radians to degrees
     * @param radians Angle in radians
     * @return Angle in degrees
     */
    static float radToDeg(float radians) {
        return radians * RAD_TO_DEG;
    }
    
    /**
     * @brief Calculate sine of an angle in degrees
     * @param degrees Angle in degrees
     * @return Sine of the angle
     */
    static float sinDeg(float degrees) {
        return std::sin(degToRad(degrees));
    }
    
    /**
     * @brief Calculate cosine of an angle in degrees
     * @param degrees Angle in degrees
     * @return Cosine of the angle
     */
    static float cosDeg(float degrees) {
        return std::cos(degToRad(degrees));
    }
    
    /**
     * @brief Calculate tangent of an angle in degrees
     * @param degrees Angle in degrees
     * @return Tangent of the angle
     */
    static float tanDeg(float degrees) {
        return std::tan(degToRad(degrees));
    }
    
    // ========================================================================
    // Vector math
    // ========================================================================
    
    /**
     * @brief Calculate dot product of two 3D vectors
     * @param x1 X component of first vector
     * @param y1 Y component of first vector
     * @param z1 Z component of first vector
     * @param x2 X component of second vector
     * @param y2 Y component of second vector
     * @param z2 Z component of second vector
     * @return Dot product
     */
    static float dot(float x1, float y1, float z1, float x2, float y2, float z2) {
        return x1 * x2 + y1 * y2 + z1 * z2;
    }
    
    /**
     * @brief Calculate cross product of two 3D vectors
     * @param x1 X component of first vector
     * @param y1 Y component of first vector
     * @param z1 Z component of first vector
     * @param x2 X component of second vector
     * @param y2 Y component of second vector
     * @param z2 Z component of second vector
     * @param outX Output: X component of result
     * @param outY Output: Y component of result
     * @param outZ Output: Z component of result
     */
    static void cross(float x1, float y1, float z1, float x2, float y2, float z2, float& outX, float& outY, float& outZ) {
        outX = y1 * z2 - z1 * y2;
        outY = z1 * x2 - x1 * z2;
        outZ = x1 * y2 - y1 * x2;
    }
    
    /**
     * @brief Calculate vector length
     * @param x X component
     * @param y Y component
     * @param z Z component
     * @return Vector length
     */
    static float length(float x, float y, float z) {
        return std::sqrt(x * x + y * y + z * z);
    }
    
    /**
     * @brief Calculate squared vector length (faster, no sqrt)
     * @param x X component
     * @param y Y component
     * @param z Z component
     * @return Squared vector length
     */
    static float lengthSquared(float x, float y, float z) {
        return x * x + y * y + z * z;
    }
    
    /**
     * @brief Normalize a 3D vector
     * @param x X component
     * @param y Y component
     * @param z Z component
     * @param outX Output: X component of normalized vector
     * @param outY Output: Y component of normalized vector
     * @param outZ Output: Z component of normalized vector
     */
    static void normalize(float x, float y, float z, float& outX, float& outY, float& outZ) {
        float len = length(x, y, z);
        if (len > EPSILON) {
            float invLen = 1.0f / len;
            outX = x * invLen;
            outY = y * invLen;
            outZ = z * invLen;
        } else {
            outX = outY = outZ = 0.0f;
        }
    }
    
    /**
     * @brief Calculate distance between two points
     * @param x1 X component of first point
     * @param y1 Y component of first point
     * @param z1 Z component of first point
     * @param x2 X component of second point
     * @param y2 Y component of second point
     * @param z2 Z component of second point
     * @return Distance between points
     */
    static float distance(float x1, float y1, float z1, float x2, float y2, float z2) {
        float dx = x2 - x1;
        float dy = y2 - y1;
        float dz = z2 - z1;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
    
    /**
     * @brief Calculate squared distance between two points (faster, no sqrt)
     * @param x1 X component of first point
     * @param y1 Y component of first point
     * @param z1 Z component of first point
     * @param x2 X component of second point
     * @param y2 Y component of second point
     * @param z2 Z component of second point
     * @return Squared distance between points
     */
    static float distanceSquared(float x1, float y1, float z1, float x2, float y2, float z2) {
        float dx = x2 - x1;
        float dy = y2 - y1;
        float dz = z2 - z1;
        return dx * dx + dy * dy + dz * dz;
    }
    
    // ========================================================================
    // Matrix math
    // ========================================================================
    
    /**
     * @brief Create identity matrix
     * @param matrix Output: 4x4 matrix
     */
    static void identity(float matrix[16]) {
        matrix[0] = 1.0f; matrix[1] = 0.0f; matrix[2] = 0.0f; matrix[3] = 0.0f;
        matrix[4] = 0.0f; matrix[5] = 1.0f; matrix[6] = 0.0f; matrix[7] = 0.0f;
        matrix[8] = 0.0f; matrix[9] = 0.0f; matrix[10] = 1.0f; matrix[11] = 0.0f;
        matrix[12] = 0.0f; matrix[13] = 0.0f; matrix[14] = 0.0f; matrix[15] = 1.0f;
    }
    
    /**
     * @brief Create translation matrix
     * @param x X translation
     * @param y Y translation
     * @param z Z translation
     * @param matrix Output: 4x4 matrix
     */
    static void translation(float x, float y, float z, float matrix[16]) {
        identity(matrix);
        matrix[12] = x;
        matrix[13] = y;
        matrix[14] = z;
    }
    
    /**
     * @brief Create scaling matrix
     * @param x X scale
     * @param y Y scale
     * @param z Z scale
     * @param matrix Output: 4x4 matrix
     */
    static void scale(float x, float y, float z, float matrix[16]) {
        identity(matrix);
        matrix[0] = x;
        matrix[5] = y;
        matrix[10] = z;
    }
    
    /**
     * @brief Create rotation matrix around X axis
     * @param angle Angle in degrees
     * @param matrix Output: 4x4 matrix
     */
    static void rotationX(float angle, float matrix[16]) {
        float rad = degToRad(angle);
        float c = std::cos(rad);
        float s = std::sin(rad);
        
        matrix[0] = 1.0f; matrix[1] = 0.0f; matrix[2] = 0.0f; matrix[3] = 0.0f;
        matrix[4] = 0.0f; matrix[5] = c;    matrix[6] = -s;   matrix[7] = 0.0f;
        matrix[8] = 0.0f; matrix[9] = s;    matrix[10] = c;    matrix[11] = 0.0f;
        matrix[12] = 0.0f; matrix[13] = 0.0f; matrix[14] = 0.0f; matrix[15] = 1.0f;
    }
    
    /**
     * @brief Create rotation matrix around Y axis
     * @param angle Angle in degrees
     * @param matrix Output: 4x4 matrix
     */
    static void rotationY(float angle, float matrix[16]) {
        float rad = degToRad(angle);
        float c = std::cos(rad);
        float s = std::sin(rad);
        
        matrix[0] = c;    matrix[1] = 0.0f; matrix[2] = s;    matrix[3] = 0.0f;
        matrix[4] = 0.0f; matrix[5] = 1.0f; matrix[6] = 0.0f; matrix[7] = 0.0f;
        matrix[8] = -s;   matrix[9] = 0.0f; matrix[10] = c;    matrix[11] = 0.0f;
        matrix[12] = 0.0f; matrix[13] = 0.0f; matrix[14] = 0.0f; matrix[15] = 1.0f;
    }
    
    /**
     * @brief Create rotation matrix around Z axis
     * @param angle Angle in degrees
     * @param matrix Output: 4x4 matrix
     */
    static void rotationZ(float angle, float matrix[16]) {
        float rad = degToRad(angle);
        float c = std::cos(rad);
        float s = std::sin(rad);
        
        matrix[0] = c;    matrix[1] = -s;   matrix[2] = 0.0f; matrix[3] = 0.0f;
        matrix[4] = s;    matrix[5] = c;    matrix[6] = 0.0f; matrix[7] = 0.0f;
        matrix[8] = 0.0f; matrix[9] = 0.0f; matrix[10] = 1.0f; matrix[11] = 0.0f;
        matrix[12] = 0.0f; matrix[13] = 0.0f; matrix[14] = 0.0f; matrix[15] = 1.0f;
    }
    
    /**
     * @brief Create rotation matrix from Euler angles (XYZ order)
     * @param pitch Pitch in degrees
     * @param yaw Yaw in degrees
     * @param roll Roll in degrees
     * @param matrix Output: 4x4 matrix
     */
    static void rotationXYZ(float pitch, float yaw, float roll, float matrix[16]) {
        float rotX[16], rotY[16], rotZ[16], temp[16];
        
        rotationX(pitch, rotX);
        rotationY(yaw, rotY);
        rotationZ(roll, rotZ);
        
        // Multiply matrices: Z * Y * X
        multiply(rotY, rotX, temp);
        multiply(rotZ, temp, matrix);
    }
    
    /**
     * @brief Multiply two 4x4 matrices
     * @param a First matrix
     * @param b Second matrix
     * @param result Output: Result matrix
     */
    static void multiply(const float a[16], const float b[16], float result[16]) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result[i * 4 + j] = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    result[i * 4 + j] += a[i * 4 + k] * b[k * 4 + j];
                }
            }
        }
    }
    
    /**
     * @brief Create perspective projection matrix
     * @param fov Field of view in degrees
     * @param aspect Aspect ratio (width / height)
     * @param near Near plane distance
     * @param far Far plane distance
     * @param matrix Output: 4x4 matrix
     */
    static void perspective(float fov, float aspect, float near, float far, float matrix[16]) {
        float rad = degToRad(fov);
        float tanHalfFov = std::tan(rad * 0.5f);
        
        matrix[0] = 1.0f / (aspect * tanHalfFov);
        matrix[1] = 0.0f;
        matrix[2] = 0.0f;
        matrix[3] = 0.0f;
        
        matrix[4] = 0.0f;
        matrix[5] = 1.0f / tanHalfFov;
        matrix[6] = 0.0f;
        matrix[7] = 0.0f;
        
        matrix[8] = 0.0f;
        matrix[9] = 0.0f;
        matrix[10] = -(far + near) / (far - near);
        matrix[11] = -1.0f;
        
        matrix[12] = 0.0f;
        matrix[13] = 0.0f;
        matrix[14] = -(2.0f * far * near) / (far - near);
        matrix[15] = 0.0f;
    }
    
    /**
     * @brief Create orthographic projection matrix
     * @param left Left plane
     * @param right Right plane
     * @param bottom Bottom plane
     * @param top Top plane
     * @param near Near plane distance
     * @param far Far plane distance
     * @param matrix Output: 4x4 matrix
     */
    static void orthographic(float left, float right, float bottom, float top, float near, float far, float matrix[16]) {
        matrix[0] = 2.0f / (right - left);
        matrix[1] = 0.0f;
        matrix[2] = 0.0f;
        matrix[3] = 0.0f;
        
        matrix[4] = 0.0f;
        matrix[5] = 2.0f / (top - bottom);
        matrix[6] = 0.0f;
        matrix[7] = 0.0f;
        
        matrix[8] = 0.0f;
        matrix[9] = 0.0f;
        matrix[10] = -2.0f / (far - near);
        matrix[11] = 0.0f;
        
        matrix[12] = -(right + left) / (right - left);
        matrix[13] = -(top + bottom) / (top - bottom);
        matrix[14] = -(far + near) / (far - near);
        matrix[15] = 1.0f;
    }
    
    /**
     * @brief Create view matrix (look-at)
     * @param eyeX Eye position X
     * @param eyeY Eye position Y
     * @param eyeZ Eye position Z
     * @param centerX Target position X
     * @param centerY Target position Y
     * @param centerZ Target position Z
     * @param upX Up vector X
     * @param upY Up vector Y
     * @param upZ Up vector Z
     * @param matrix Output: 4x4 matrix
     */
    static void lookAt(float eyeX, float eyeY, float eyeZ,
                      float centerX, float centerY, float centerZ,
                      float upX, float upY, float upZ,
                      float matrix[16]) {
        float zx = centerX - eyeX;
        float zy = centerY - eyeY;
        float zz = centerZ - eyeZ;
        
        // Normalize Z axis
        float len = length(zx, zy, zz);
        if (len > EPSILON) {
            zx /= len;
            zy /= len;
            zz /= len;
        }
        
        // Calculate X axis (cross product of up and Z)
        float xx, xy, xz;
        cross(upX, upY, upZ, zx, zy, zz, xx, xy, xz);
        
        // Normalize X axis
        len = length(xx, xy, xz);
        if (len > EPSILON) {
            xx /= len;
            xy /= len;
            xz /= len;
        }
        
        // Calculate Y axis (cross product of Z and X)
        float yx, yy, yz;
        cross(zx, zy, zz, xx, xy, xz, yx, yy, yz);
        
        // Create matrix
        matrix[0] = xx; matrix[1] = yx; matrix[2] = zx; matrix[3] = 0.0f;
        matrix[4] = xy; matrix[5] = yy; matrix[6] = zy; matrix[7] = 0.0f;
        matrix[8] = xz; matrix[9] = yz; matrix[10] = zz; matrix[11] = 0.0f;
        matrix[12] = -dot(xx, xy, xz, eyeX, eyeY, eyeZ);
        matrix[13] = -dot(yx, yy, yz, eyeX, eyeY, eyeZ);
        matrix[14] = -dot(zx, zy, zz, eyeX, eyeY, eyeZ);
        matrix[15] = 1.0f;
    }
    
    // ========================================================================
    // Random number generation
    // ========================================================================
    
    /**
     * @brief Seed the random number generator
     * @param seed Seed value
     */
    static void seed(uint32_t seed) {
        s_randomSeed = seed;
    }
    
    /**
     * @brief Generate a random float between 0.0 and 1.0
     * @return Random float
     */
    static float randomFloat() {
        s_randomSeed = (s_randomSeed * 1664525 + 1013904223) & 0xFFFFFFFF;
        return static_cast<float>(s_randomSeed) / 4294967296.0f;
    }
    
    /**
     * @brief Generate a random float in a range
     * @param min Minimum value
     * @param max Maximum value
     * @return Random float in range
     */
    static float randomFloat(float min, float max) {
        return min + (max - min) * randomFloat();
    }
    
    /**
     * @brief Generate a random integer
     * @param min Minimum value
     * @param max Maximum value
     * @return Random integer in range
     */
    static int randomInt(int min, int max) {
        return min + static_cast<int>(randomFloat() * (max - min + 1));
    }
    
    // ========================================================================
    // Color utilities
    // ========================================================================
    
    /**
     * @brief Convert RGB to HSL
     * @param r Red (0.0 to 1.0)
     * @param g Green (0.0 to 1.0)
     * @param b Blue (0.0 to 1.0)
     * @param h Output: Hue (0.0 to 1.0)
     * @param s Output: Saturation (0.0 to 1.0)
     * @param l Output: Lightness (0.0 to 1.0)
     */
    static void rgbToHsl(float r, float g, float b, float& h, float& s, float& l) {
        float max = std::max(std::max(r, g), b);
        float min = std::min(std::min(r, g), b);
        float delta = max - min;
        
        l = (max + min) * 0.5f;
        
        if (delta < EPSILON) {
            h = 0.0f;
            s = 0.0f;
        } else {
            s = delta / (1.0f - std::abs(2.0f * l - 1.0f));
            
            if (max == r) {
                h = (g - b) / delta + (g < b ? 6.0f : 0.0f);
            } else if (max == g) {
                h = (b - r) / delta + 2.0f;
            } else {
                h = (r - g) / delta + 4.0f;
            }
            
            h /= 6.0f;
        }
    }
    
    /**
     * @brief Convert HSL to RGB
     * @param h Hue (0.0 to 1.0)
     * @param s Saturation (0.0 to 1.0)
     * @param l Lightness (0.0 to 1.0)
     * @param r Output: Red (0.0 to 1.0)
     * @param g Output: Green (0.0 to 1.0)
     * @param b Output: Blue (0.0 to 1.0)
     */
    static void hslToRgb(float h, float s, float l, float& r, float& g, float& b) {
        if (s < EPSILON) {
            r = g = b = l;
        } else {
            float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
            float p = 2.0f * l - q;
            
            h *= 6.0f;
            float t = h - static_cast<int>(h);
            
            float rgb[3];
            rgb[0] = h < 1.0f ? q : (h < 3.0f ? p + (q - p) * (2.0f / 3.0f - t) * 6.0f : (h < 4.0f ? p : p + (q - p) * (4.0f / 3.0f - t) * 6.0f));
            rgb[1] = h < 2.0f ? p + (q - p) * (2.0f / 3.0f - t) * 6.0f : (h < 4.0f ? q : p + (q - p) * (4.0f / 3.0f - t) * 6.0f);
            rgb[2] = h < 3.0f ? p : (h < 5.0f ? p + (q - p) * (4.0f / 3.0f - t) * 6.0f : q);
            
            r = rgb[(static_cast<int>(h) + 2) % 3];
            g = rgb[(static_cast<int>(h) + 1) % 3];
            b = rgb[static_cast<int>(h) % 3];
        }
    }

private:
    static uint32_t s_randomSeed;
};

// Initialize static member
uint32_t MathUtils::s_randomSeed = 42;

} // namespace renderbridge
