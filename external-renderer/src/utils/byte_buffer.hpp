#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

namespace renderbridge {

/**
 * @class ByteBuffer
 * @brief A simple byte buffer for reading and writing binary data
 * 
 * This class provides methods for reading and writing primitive types
 * in big-endian format (network byte order), matching the Java
 * ByteBuffer behavior used in the bridge protocol.
 */
class ByteBuffer {
public:
    /**
     * @brief Constructor
     * @param capacity Initial capacity
     */
    explicit ByteBuffer(uint32_t capacity = 1024);
    
    /**
     * @brief Constructor with existing data
     * @param data Pointer to data
     * @param size Size of data
     * @param copy If true, copy the data; otherwise, take ownership
     */
    ByteBuffer(const uint8_t* data, uint32_t size, bool copy = true);
    
    /**
     * @brief Constructor with vector
     * @param data Vector of bytes
     */
    explicit ByteBuffer(const std::vector<uint8_t>& data);
    
    /**
     * @brief Destructor
     */
    ~ByteBuffer();
    
    /**
     * @brief Get the capacity of the buffer
     * @return Capacity in bytes
     */
    uint32_t capacity() const;
    
    /**
     * @brief Get the current position
     * @return Current position
     */
    uint32_t position() const;
    
    /**
     * @brief Get the limit
     * @return Limit
     */
    uint32_t limit() const;
    
    /**
     * @brief Get the remaining bytes
     * @return Number of remaining bytes
     */
    uint32_t remaining() const;
    
    /**
     * @brief Set the position
     * @param pos New position
     */
    void position(uint32_t pos);
    
    /**
     * @brief Set the limit
     * @param limit New limit
     */
    void limit(uint32_t limit);
    
    /**
     * @brief Clear the buffer
     */
    void clear();
    
    /**
     * @brief Flip the buffer (set limit to position, then position to 0)
     */
    void flip();
    
    /**
     * @brief Compact the buffer
     */
    void compact();
    
    /**
     * @brief Ensure capacity
     * @param required Required capacity
     */
    void ensureCapacity(uint32_t required);
    
    /**
     * @brief Get the underlying data
     * @return Pointer to the data
     */
    const uint8_t* data() const;
    
    /**
     * @brief Get the underlying data (non-const)
     * @return Pointer to the data
     */
    uint8_t* data();
    
    /**
     * @brief Get the size of the data
     * @return Size of the data
     */
    uint32_t size() const;
    
    /**
     * @brief Get a byte at the current position
     * @return The byte value
     */
    uint8_t get();
    
    /**
     * @brief Get a byte at a specific position
     * @param pos Position
     * @return The byte value
     */
    uint8_t get(uint32_t pos) const;
    
    /**
     * @brief Put a byte at the current position
     * @param value The byte value
     */
    void put(uint8_t value);
    
    /**
     * @brief Put a byte at a specific position
     * @param pos Position
     * @param value The byte value
     */
    void put(uint32_t pos, uint8_t value);
    
    /**
     * @brief Get a short (16-bit) at the current position (big-endian)
     * @return The short value
     */
    int16_t getShort();
    
    /**
     * @brief Get a short at a specific position (big-endian)
     * @param pos Position
     * @return The short value
     */
    int16_t getShort(uint32_t pos) const;
    
    /**
     * @brief Put a short (16-bit) at the current position (big-endian)
     * @param value The short value
     */
    void putShort(int16_t value);
    
    /**
     * @brief Put a short at a specific position (big-endian)
     * @param pos Position
     * @param value The short value
     */
    void putShort(uint32_t pos, int16_t value);
    
    /**
     * @brief Get an unsigned short (16-bit) at the current position (big-endian)
     * @return The unsigned short value
     */
    uint16_t getUnsignedShort();
    
    /**
     * @brief Get an unsigned short at a specific position (big-endian)
     * @param pos Position
     * @return The unsigned short value
     */
    uint16_t getUnsignedShort(uint32_t pos) const;
    
    /**
     * @brief Put an unsigned short (16-bit) at the current position (big-endian)
     * @param value The unsigned short value
     */
    void putUnsignedShort(uint16_t value);
    
    /**
     * @brief Put an unsigned short at a specific position (big-endian)
     * @param pos Position
     * @param value The unsigned short value
     */
    void putUnsignedShort(uint32_t pos, uint16_t value);
    
    /**
     * @brief Get an int (32-bit) at the current position (big-endian)
     * @return The int value
     */
    int32_t getInt();
    
    /**
     * @brief Get an int at a specific position (big-endian)
     * @param pos Position
     * @return The int value
     */
    int32_t getInt(uint32_t pos) const;
    
    /**
     * @brief Put an int (32-bit) at the current position (big-endian)
     * @param value The int value
     */
    void putInt(int32_t value);
    
    /**
     * @brief Put an int at a specific position (big-endian)
     * @param pos Position
     * @param value The int value
     */
    void putInt(uint32_t pos, int32_t value);
    
    /**
     * @brief Get an unsigned int (32-bit) at the current position (big-endian)
     * @return The unsigned int value
     */
    uint32_t getUnsignedInt();
    
    /**
     * @brief Get an unsigned int at a specific position (big-endian)
     * @param pos Position
     * @return The unsigned int value
     */
    uint32_t getUnsignedInt(uint32_t pos) const;
    
    /**
     * @brief Put an unsigned int (32-bit) at the current position (big-endian)
     * @param value The unsigned int value
     */
    void putUnsignedInt(uint32_t value);
    
    /**
     * @brief Put an unsigned int at a specific position (big-endian)
     * @param pos Position
     * @param value The unsigned int value
     */
    void putUnsignedInt(uint32_t pos, uint32_t value);
    
    /**
     * @brief Get a long (64-bit) at the current position (big-endian)
     * @return The long value
     */
    int64_t getLong();
    
    /**
     * @brief Get a long at a specific position (big-endian)
     * @param pos Position
     * @return The long value
     */
    int64_t getLong(uint32_t pos) const;
    
    /**
     * @brief Put a long (64-bit) at the current position (big-endian)
     * @param value The long value
     */
    void putLong(int64_t value);
    
    /**
     * @brief Put a long at a specific position (big-endian)
     * @param pos Position
     * @param value The long value
     */
    void putLong(uint32_t pos, int64_t value);
    
    /**
     * @brief Get a float (32-bit) at the current position (big-endian)
     * @return The float value
     */
    float getFloat();
    
    /**
     * @brief Get a float at a specific position (big-endian)
     * @param pos Position
     * @return The float value
     */
    float getFloat(uint32_t pos) const;
    
    /**
     * @brief Put a float (32-bit) at the current position (big-endian)
     * @param value The float value
     */
    void putFloat(float value);
    
    /**
     * @brief Put a float at a specific position (big-endian)
     * @param pos Position
     * @param value The float value
     */
    void putFloat(uint32_t pos, float value);
    
    /**
     * @brief Get a double (64-bit) at the current position (big-endian)
     * @return The double value
     */
    double getDouble();
    
    /**
     * @brief Get a double at a specific position (big-endian)
     * @param pos Position
     * @return The double value
     */
    double getDouble(uint32_t pos) const;
    
    /**
     * @brief Put a double (64-bit) at the current position (big-endian)
     * @param value The double value
     */
    void putDouble(double value);
    
    /**
     * @brief Put a double at a specific position (big-endian)
     * @param pos Position
     * @param value The double value
     */
    void putDouble(uint32_t pos, double value);
    
    /**
     * @brief Read bytes into a buffer
     * @param dst Destination buffer
     * @param length Number of bytes to read
     */
    void getBytes(uint8_t* dst, uint32_t length);
    
    /**
     * @brief Write bytes from a buffer
     * @param src Source buffer
     * @param length Number of bytes to write
     */
    void putBytes(const uint8_t* src, uint32_t length);
    
    /**
     * @brief Read a string (length-prefixed)
     * @return The string
     */
    std::string getString();
    
    /**
     * @brief Write a string (length-prefixed)
     * @param str The string to write
     */
    void putString(const std::string& str);
    
    /**
     * @brief Read a string with a maximum length
     * @param maxLength Maximum length
     * @return The string
     */
    std::string getString(uint32_t maxLength);
    
    /**
     * @brief Write a string with a maximum length
     * @param str The string to write
     * @param maxLength Maximum length
     */
    void putString(const std::string& str, uint32_t maxLength);
    
    /**
     * @brief Skip bytes
     * @param n Number of bytes to skip
     */
    void skip(uint32_t n);
    
    /**
     * @brief Convert to vector
     * @return Vector of bytes
     */
    std::vector<uint8_t> toVector() const;
    
    /**
     * @brief Create a slice of this buffer
     * @param start Start position
     * @param length Length of the slice
     * @return New ByteBuffer with the slice
     */
    ByteBuffer slice(uint32_t start, uint32_t length) const;

private:
    uint8_t* m_buffer;
    uint32_t m_capacity;
    uint32_t m_position;
    uint32_t m_limit;
    bool m_ownsBuffer;
    
    /**
     * @brief Resize the buffer
     * @param newCapacity New capacity
     */
    void resize(uint32_t newCapacity);
    
    /**
     * @brief Check bounds
     * @param pos Position
     * @param size Size
     */
    void checkBounds(uint32_t pos, uint32_t size) const;
};

/**
 * @brief Helper function to create a ByteBuffer from a message
 * @param messageType Message type
 * @param payload Payload data
 * @param payloadSize Payload size
 * @return ByteBuffer with header and payload
 */
ByteBuffer createMessageBuffer(uint16_t messageType, const uint8_t* payload, uint32_t payloadSize);

/**
 * @brief Helper function to parse a message from a ByteBuffer
 * @param buffer ByteBuffer to parse
 * @param messageType Output: message type
 * @param payload Output: payload data
 * @return True if parsing succeeded
 */
bool parseMessageBuffer(ByteBuffer& buffer, uint16_t& messageType, std::vector<uint8_t>& payload);

} // namespace renderbridge
