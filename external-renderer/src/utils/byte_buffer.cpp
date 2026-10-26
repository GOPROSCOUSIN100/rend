#include "byte_buffer.hpp"

#include <cstring>
#include <algorithm>
#include <stdexcept>

namespace renderbridge {

// Helper function to swap bytes (for big-endian conversion on little-endian systems)
static uint16_t swapBytes(uint16_t value) {
    return (value >> 8) | (value << 8);
}

static uint32_t swapBytes(uint32_t value) {
    return ((value >> 24) & 0x000000FF) |
           ((value >> 8) & 0x0000FF00) |
           ((value << 8) & 0x00FF0000) |
           ((value << 24) & 0xFF000000);
}

static uint64_t swapBytes(uint64_t value) {
    return ((value >> 56) & 0x00000000000000FF) |
           ((value >> 40) & 0x000000000000FF00) |
           ((value >> 24) & 0x0000000000FF0000) |
           ((value >> 8) & 0x00000000FF000000) |
           ((value << 8) & 0x000000FF00000000) |
           ((value << 24) & 0x0000FF0000000000) |
           ((value << 40) & 0x00FF000000000000) |
           ((value << 56) & 0xFF00000000000000);
}

// Check if system is little-endian
static bool isLittleEndian() {
    uint16_t test = 0x0001;
    return *(reinterpret_cast<uint8_t*>(&test)) == 0x01;
}

// Big-endian to host conversion (no-op on big-endian systems)
static uint16_t beToHost(uint16_t value) {
    return isLittleEndian() ? swapBytes(value) : value;
}

static uint32_t beToHost(uint32_t value) {
    return isLittleEndian() ? swapBytes(value) : value;
}

static uint64_t beToHost(uint64_t value) {
    return isLittleEndian() ? swapBytes(value) : value;
}

// Host to big-endian conversion
static uint16_t hostToBe(uint16_t value) {
    return beToHost(value);
}

static uint32_t hostToBe(uint32_t value) {
    return beToHost(value);
}

static uint64_t hostToBe(uint64_t value) {
    return beToHost(value);
}

// Reinterpret float bits as uint32_t and vice versa
static uint32_t floatToBits(float value) {
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(float));
    return bits;
}

static float bitsToFloat(uint32_t bits) {
    float value;
    std::memcpy(&value, &bits, sizeof(float));
    return value;
}

// Reinterpret double bits as uint64_t and vice versa
static uint64_t doubleToBits(double value) {
    uint64_t bits;
    std::memcpy(&bits, &value, sizeof(double));
    return bits;
}

static double bitsToDouble(uint64_t bits) {
    double value;
    std::memcpy(&value, &bits, sizeof(double));
    return value;
}

ByteBuffer::ByteBuffer(uint32_t capacity)
    : m_buffer(new uint8_t[capacity]),
      m_capacity(capacity),
      m_position(0),
      m_limit(capacity),
      m_ownsBuffer(true) {
}

ByteBuffer::ByteBuffer(const uint8_t* data, uint32_t size, bool copy)
    : m_buffer(nullptr),
      m_capacity(0),
      m_position(0),
      m_limit(0),
      m_ownsBuffer(false) {
    if (copy) {
        m_buffer = new uint8_t[size];
        m_capacity = size;
        m_limit = size;
        std::memcpy(m_buffer, data, size);
        m_ownsBuffer = true;
    } else {
        m_buffer = const_cast<uint8_t*>(data);
        m_capacity = size;
        m_limit = size;
        m_ownsBuffer = false;
    }
}

ByteBuffer::ByteBuffer(const std::vector<uint8_t>& data)
    : m_buffer(new uint8_t[data.size()]),
      m_capacity(static_cast<uint32_t>(data.size())),
      m_position(0),
      m_limit(static_cast<uint32_t>(data.size())),
      m_ownsBuffer(true) {
    std::memcpy(m_buffer, data.data(), data.size());
}

ByteBuffer::~ByteBuffer() {
    if (m_ownsBuffer && m_buffer) {
        delete[] m_buffer;
    }
}

uint32_t ByteBuffer::capacity() const {
    return m_capacity;
}

uint32_t ByteBuffer::position() const {
    return m_position;
}

uint32_t ByteBuffer::limit() const {
    return m_limit;
}

uint32_t ByteBuffer::remaining() const {
    return m_limit - m_position;
}

void ByteBuffer::position(uint32_t pos) {
    checkBounds(pos, 0);
    m_position = pos;
}

void ByteBuffer::limit(uint32_t limit) {
    checkBounds(limit, 0);
    m_limit = limit;
}

void ByteBuffer::clear() {
    m_position = 0;
    m_limit = m_capacity;
}

void ByteBuffer::flip() {
    m_limit = m_position;
    m_position = 0;
}

void ByteBuffer::compact() {
    if (m_position > 0) {
        uint32_t remaining = m_limit - m_position;
        if (remaining > 0) {
            std::memmove(m_buffer, m_buffer + m_position, remaining);
        }
        m_position = 0;
        m_limit = m_capacity;
    }
}

void ByteBuffer::ensureCapacity(uint32_t required) {
    if (required > m_capacity) {
        uint32_t newCapacity = m_capacity;
        while (newCapacity < required) {
            newCapacity *= 2;
        }
        resize(newCapacity);
    }
}

const uint8_t* ByteBuffer::data() const {
    return m_buffer;
}

uint8_t* ByteBuffer::data() {
    return m_buffer;
}

uint32_t ByteBuffer::size() const {
    return m_limit;
}

uint8_t ByteBuffer::get() {
    checkBounds(m_position, 1);
    return m_buffer[m_position++];
}

uint8_t ByteBuffer::get(uint32_t pos) const {
    checkBounds(pos, 1);
    return m_buffer[pos];
}

void ByteBuffer::put(uint8_t value) {
    ensureCapacity(m_position + 1);
    m_buffer[m_position++] = value;
}

void ByteBuffer::put(uint32_t pos, uint8_t value) {
    checkBounds(pos, 1);
    ensureCapacity(pos + 1);
    m_buffer[pos] = value;
}

int16_t ByteBuffer::getShort() {
    checkBounds(m_position, 2);
    uint16_t value = static_cast<uint16_t>(m_buffer[m_position]) << 8 |
                   static_cast<uint16_t>(m_buffer[m_position + 1]);
    m_position += 2;
    return static_cast<int16_t>(value);
}

int16_t ByteBuffer::getShort(uint32_t pos) const {
    checkBounds(pos, 2);
    uint16_t value = static_cast<uint16_t>(m_buffer[pos]) << 8 |
                   static_cast<uint16_t>(m_buffer[pos + 1]);
    return static_cast<int16_t>(value);
}

void ByteBuffer::putShort(int16_t value) {
    ensureCapacity(m_position + 2);
    m_buffer[m_position++] = static_cast<uint8_t>((value >> 8) & 0xFF);
    m_buffer[m_position++] = static_cast<uint8_t>(value & 0xFF);
}

void ByteBuffer::putShort(uint32_t pos, int16_t value) {
    checkBounds(pos, 2);
    ensureCapacity(pos + 2);
    m_buffer[pos] = static_cast<uint8_t>((value >> 8) & 0xFF);
    m_buffer[pos + 1] = static_cast<uint8_t>(value & 0xFF);
}

uint16_t ByteBuffer::getUnsignedShort() {
    checkBounds(m_position, 2);
    uint16_t value = static_cast<uint16_t>(m_buffer[m_position]) << 8 |
                   static_cast<uint16_t>(m_buffer[m_position + 1]);
    m_position += 2;
    return value;
}

uint16_t ByteBuffer::getUnsignedShort(uint32_t pos) const {
    checkBounds(pos, 2);
    uint16_t value = static_cast<uint16_t>(m_buffer[pos]) << 8 |
                   static_cast<uint16_t>(m_buffer[pos + 1]);
    return value;
}

void ByteBuffer::putUnsignedShort(uint16_t value) {
    ensureCapacity(m_position + 2);
    m_buffer[m_position++] = static_cast<uint8_t>((value >> 8) & 0xFF);
    m_buffer[m_position++] = static_cast<uint8_t>(value & 0xFF);
}

void ByteBuffer::putUnsignedShort(uint32_t pos, uint16_t value) {
    checkBounds(pos, 2);
    ensureCapacity(pos + 2);
    m_buffer[pos] = static_cast<uint8_t>((value >> 8) & 0xFF);
    m_buffer[pos + 1] = static_cast<uint8_t>(value & 0xFF);
}

int32_t ByteBuffer::getInt() {
    checkBounds(m_position, 4);
    uint32_t value = static_cast<uint32_t>(m_buffer[m_position]) << 24 |
                   static_cast<uint32_t>(m_buffer[m_position + 1]) << 16 |
                   static_cast<uint32_t>(m_buffer[m_position + 2]) << 8 |
                   static_cast<uint32_t>(m_buffer[m_position + 3]);
    m_position += 4;
    return static_cast<int32_t>(value);
}

int32_t ByteBuffer::getInt(uint32_t pos) const {
    checkBounds(pos, 4);
    uint32_t value = static_cast<uint32_t>(m_buffer[pos]) << 24 |
                   static_cast<uint32_t>(m_buffer[pos + 1]) << 16 |
                   static_cast<uint32_t>(m_buffer[pos + 2]) << 8 |
                   static_cast<uint32_t>(m_buffer[pos + 3]);
    return static_cast<int32_t>(value);
}

void ByteBuffer::putInt(int32_t value) {
    ensureCapacity(m_position + 4);
    m_buffer[m_position++] = static_cast<uint8_t>((value >> 24) & 0xFF);
    m_buffer[m_position++] = static_cast<uint8_t>((value >> 16) & 0xFF);
    m_buffer[m_position++] = static_cast<uint8_t>((value >> 8) & 0xFF);
    m_buffer[m_position++] = static_cast<uint8_t>(value & 0xFF);
}

void ByteBuffer::putInt(uint32_t pos, int32_t value) {
    checkBounds(pos, 4);
    ensureCapacity(pos + 4);
    m_buffer[pos] = static_cast<uint8_t>((value >> 24) & 0xFF);
    m_buffer[pos + 1] = static_cast<uint8_t>((value >> 16) & 0xFF);
    m_buffer[pos + 2] = static_cast<uint8_t>((value >> 8) & 0xFF);
    m_buffer[pos + 3] = static_cast<uint8_t>(value & 0xFF);
}

uint32_t ByteBuffer::getUnsignedInt() {
    checkBounds(m_position, 4);
    uint32_t value = static_cast<uint32_t>(m_buffer[m_position]) << 24 |
                   static_cast<uint32_t>(m_buffer[m_position + 1]) << 16 |
                   static_cast<uint32_t>(m_buffer[m_position + 2]) << 8 |
                   static_cast<uint32_t>(m_buffer[m_position + 3]);
    m_position += 4;
    return value;
}

uint32_t ByteBuffer::getUnsignedInt(uint32_t pos) const {
    checkBounds(pos, 4);
    uint32_t value = static_cast<uint32_t>(m_buffer[pos]) << 24 |
                   static_cast<uint32_t>(m_buffer[pos + 1]) << 16 |
                   static_cast<uint32_t>(m_buffer[pos + 2]) << 8 |
                   static_cast<uint32_t>(m_buffer[pos + 3]);
    return value;
}

void ByteBuffer::putUnsignedInt(uint32_t value) {
    ensureCapacity(m_position + 4);
    m_buffer[m_position++] = static_cast<uint8_t>((value >> 24) & 0xFF);
    m_buffer[m_position++] = static_cast<uint8_t>((value >> 16) & 0xFF);
    m_buffer[m_position++] = static_cast<uint8_t>((value >> 8) & 0xFF);
    m_buffer[m_position++] = static_cast<uint8_t>(value & 0xFF);
}

void ByteBuffer::putUnsignedInt(uint32_t pos, uint32_t value) {
    checkBounds(pos, 4);
    ensureCapacity(pos + 4);
    m_buffer[pos] = static_cast<uint8_t>((value >> 24) & 0xFF);
    m_buffer[pos + 1] = static_cast<uint8_t>((value >> 16) & 0xFF);
    m_buffer[pos + 2] = static_cast<uint8_t>((value >> 8) & 0xFF);
    m_buffer[pos + 3] = static_cast<uint8_t>(value & 0xFF);
}

int64_t ByteBuffer::getLong() {
    checkBounds(m_position, 8);
    uint64_t value = static_cast<uint64_t>(m_buffer[m_position]) << 56 |
                   static_cast<uint64_t>(m_buffer[m_position + 1]) << 48 |
                   static_cast<uint64_t>(m_buffer[m_position + 2]) << 40 |
                   static_cast<uint64_t>(m_buffer[m_position + 3]) << 32 |
                   static_cast<uint64_t>(m_buffer[m_position + 4]) << 24 |
                   static_cast<uint64_t>(m_buffer[m_position + 5]) << 16 |
                   static_cast<uint64_t>(m_buffer[m_position + 6]) << 8 |
                   static_cast<uint64_t>(m_buffer[m_position + 7]);
    m_position += 8;
    return static_cast<int64_t>(value);
}

int64_t ByteBuffer::getLong(uint32_t pos) const {
    checkBounds(pos, 8);
    uint64_t value = static_cast<uint64_t>(m_buffer[pos]) << 56 |
                   static_cast<uint64_t>(m_buffer[pos + 1]) << 48 |
                   static_cast<uint64_t>(m_buffer[pos + 2]) << 40 |
                   static_cast<uint64_t>(m_buffer[pos + 3]) << 32 |
                   static_cast<uint64_t>(m_buffer[pos + 4]) << 24 |
                   static_cast<uint64_t>(m_buffer[pos + 5]) << 16 |
                   static_cast<uint64_t>(m_buffer[pos + 6]) << 8 |
                   static_cast<uint64_t>(m_buffer[pos + 7]);
    return static_cast<int64_t>(value);
}

void ByteBuffer::putLong(int64_t value) {
    ensureCapacity(m_position + 8);
    m_buffer[m_position++] = static_cast<uint8_t>((value >> 56) & 0xFF);
    m_buffer[m_position++] = static_cast<uint8_t>((value >> 48) & 0xFF);
    m_buffer[m_position++] = static_cast<uint8_t>((value >> 40) & 0xFF);
    m_buffer[m_position++] = static_cast<uint8_t>((value >> 32) & 0xFF);
    m_buffer[m_position++] = static_cast<uint8_t>((value >> 24) & 0xFF);
    m_buffer[m_position++] = static_cast<uint8_t>((value >> 16) & 0xFF);
    m_buffer[m_position++] = static_cast<uint8_t>((value >> 8) & 0xFF);
    m_buffer[m_position++] = static_cast<uint8_t>(value & 0xFF);
}

void ByteBuffer::putLong(uint32_t pos, int64_t value) {
    checkBounds(pos, 8);
    ensureCapacity(pos + 8);
    m_buffer[pos] = static_cast<uint8_t>((value >> 56) & 0xFF);
    m_buffer[pos + 1] = static_cast<uint8_t>((value >> 48) & 0xFF);
    m_buffer[pos + 2] = static_cast<uint8_t>((value >> 40) & 0xFF);
    m_buffer[pos + 3] = static_cast<uint8_t>((value >> 32) & 0xFF);
    m_buffer[pos + 4] = static_cast<uint8_t>((value >> 24) & 0xFF);
    m_buffer[pos + 5] = static_cast<uint8_t>((value >> 16) & 0xFF);
    m_buffer[pos + 6] = static_cast<uint8_t>((value >> 8) & 0xFF);
    m_buffer[pos + 7] = static_cast<uint8_t>(value & 0xFF);
}

float ByteBuffer::getFloat() {
    uint32_t bits = getUnsignedInt();
    return bitsToFloat(bits);
}

float ByteBuffer::getFloat(uint32_t pos) const {
    uint32_t bits = getUnsignedInt(pos);
    return bitsToFloat(bits);
}

void ByteBuffer::putFloat(float value) {
    uint32_t bits = floatToBits(value);
    putUnsignedInt(bits);
}

void ByteBuffer::putFloat(uint32_t pos, float value) {
    uint32_t bits = floatToBits(value);
    putUnsignedInt(pos, bits);
}

double ByteBuffer::getDouble() {
    uint64_t bits = static_cast<uint64_t>(getLong());
    return bitsToDouble(bits);
}

double ByteBuffer::getDouble(uint32_t pos) const {
    uint64_t bits = static_cast<uint64_t>(getLong(pos));
    return bitsToDouble(bits);
}

void ByteBuffer::putDouble(double value) {
    uint64_t bits = doubleToBits(value);
    putLong(static_cast<int64_t>(bits));
}

void ByteBuffer::putDouble(uint32_t pos, double value) {
    uint64_t bits = doubleToBits(value);
    putLong(pos, static_cast<int64_t>(bits));
}

void ByteBuffer::getBytes(uint8_t* dst, uint32_t length) {
    checkBounds(m_position, length);
    std::memcpy(dst, m_buffer + m_position, length);
    m_position += length;
}

void ByteBuffer::putBytes(const uint8_t* src, uint32_t length) {
    ensureCapacity(m_position + length);
    std::memcpy(m_buffer + m_position, src, length);
    m_position += length;
}

std::string ByteBuffer::getString() {
    uint8_t length = get();
    std::string str;
    if (length > 0) {
        str.resize(length);
        getBytes(reinterpret_cast<uint8_t*>(&str[0]), length);
    }
    return str;
}

void ByteBuffer::putString(const std::string& str) {
    uint8_t length = static_cast<uint8_t>(std::min(str.length(), static_cast<size_t>(255)));
    put(length);
    putBytes(reinterpret_cast<const uint8_t*>(str.data()), length);
}

std::string ByteBuffer::getString(uint32_t maxLength) {
    uint8_t length = get();
    length = static_cast<uint8_t>(std::min(static_cast<uint32_t>(length), maxLength));
    std::string str;
    if (length > 0) {
        str.resize(length);
        getBytes(reinterpret_cast<uint8_t*>(&str[0]), length);
    }
    return str;
}

void ByteBuffer::putString(const std::string& str, uint32_t maxLength) {
    uint8_t length = static_cast<uint8_t>(std::min(str.length(), maxLength));
    put(length);
    putBytes(reinterpret_cast<const uint8_t*>(str.data()), length);
}

void ByteBuffer::skip(uint32_t n) {
    checkBounds(m_position, n);
    m_position += n;
}

std::vector<uint8_t> ByteBuffer::toVector() const {
    return std::vector<uint8_t>(m_buffer, m_buffer + m_limit);
}

ByteBuffer ByteBuffer::slice(uint32_t start, uint32_t length) const {
    checkBounds(start, length);
    return ByteBuffer(m_buffer + start, length, false);
}

void ByteBuffer::resize(uint32_t newCapacity) {
    uint8_t* newBuffer = new uint8_t[newCapacity];
    uint32_t copySize = std::min(m_limit, newCapacity);
    if (copySize > 0) {
        std::memcpy(newBuffer, m_buffer, copySize);
    }
    if (m_ownsBuffer && m_buffer) {
        delete[] m_buffer;
    }
    m_buffer = newBuffer;
    m_capacity = newCapacity;
    m_ownsBuffer = true;
}

void ByteBuffer::checkBounds(uint32_t pos, uint32_t size) const {
    if (pos + size > m_limit) {
        throw std::out_of_range("ByteBuffer: out of bounds");
    }
}

// Helper functions
ByteBuffer createMessageBuffer(uint16_t messageType, const uint8_t* payload, uint32_t payloadSize) {
    // Header: messageType(2) + payloadSize(4) + version(1) + flags(1) + checksum(2) + sequence(4) = 14 bytes
    // But our protocol uses 16-byte header
    ByteBuffer buffer(16 + payloadSize);
    
    // Write header
    buffer.putUnsignedShort(messageType);
    buffer.putUnsignedInt(payloadSize);
    buffer.put(static_cast<uint8_t>(1)); // version
    buffer.put(static_cast<uint8_t>(0)); // flags
    buffer.putShort(0); // checksum
    buffer.putUnsignedInt(0); // sequence
    
    // Write payload
    if (payload && payloadSize > 0) {
        buffer.putBytes(payload, payloadSize);
    }
    
    buffer.flip();
    return buffer;
}

bool parseMessageBuffer(ByteBuffer& buffer, uint16_t& messageType, std::vector<uint8_t>& payload) {
    if (buffer.remaining() < 16) {
        return false; // Not enough data for header
    }
    
    // Read header
    messageType = buffer.getUnsignedShort();
    uint32_t payloadSize = buffer.getUnsignedInt();
    uint8_t version = buffer.get();
    uint8_t flags = buffer.get();
    uint16_t checksum = buffer.getShort();
    uint32_t sequence = buffer.getUnsignedInt();
    
    // Check if we have enough data for payload
    if (buffer.remaining() < payloadSize) {
        return false;
    }
    
    // Read payload
    payload.resize(payloadSize);
    buffer.getBytes(payload.data(), payloadSize);
    
    return true;
}

} // namespace renderbridge
