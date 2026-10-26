#include "bridge_connection.hpp"

#include "utils/byte_buffer.hpp"
#include "utils/logger.hpp"

#include <cstring>
#include <thread>
#include <chrono>

// Platform-specific includes
#ifdef _WIN32
    #include <windows.h>
    #include <fileapi.h>
    #include <handleapi.h>
    #include <winbase.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#elif defined(__linux__) || defined(__APPLE__)
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <cerrno>
#endif

namespace renderbridge {

// Message header size (16 bytes)
static constexpr uint32_t MESSAGE_HEADER_SIZE = 16;

// ============================================================================
// BridgeConnection
// ============================================================================

BridgeConnection::BridgeConnection() : m_lastError("No error") {}

BridgeConnection::~BridgeConnection() {}

std::string BridgeConnection::getLastError() const {
    return m_lastError;
}

void BridgeConnection::setLastError(const std::string& error) {
    m_lastError = error;
}

// ============================================================================
// SharedMemoryConnection
// ============================================================================

SharedMemoryConnection::SharedMemoryConnection(const std::string& name, uint32_t size)
    : m_name(name),
      m_size(size),
      m_connected(false),
#ifdef _WIN32
      m_sharedMemory(nullptr),
      m_mappedAddress(nullptr),
#elif defined(__linux__) || defined(__APPLE__)
      m_sharedMemoryFd(-1),
      m_mappedAddress(nullptr),
#endif
      m_lastError("No error") {
}

SharedMemoryConnection::~SharedMemoryConnection() {
    disconnect();
}

bool SharedMemoryConnection::connect() {
    // Try to open existing shared memory first
    if (openSharedMemory()) {
        if (mapSharedMemory()) {
            m_connected = true;
            RB_LOG_INFO_FMT("SharedMemoryConnection: Connected to existing shared memory '{}'", m_name);
            return true;
        }
    }
    
    // If opening failed, try to create new
    if (createSharedMemory()) {
        if (mapSharedMemory()) {
            m_connected = true;
            RB_LOG_INFO_FMT("SharedMemoryConnection: Created new shared memory '{}'", m_name);
            return true;
        }
    }
    
    m_connected = false;
    RB_LOG_ERROR_FMT("SharedMemoryConnection: Failed to connect to shared memory '{}': {}", m_name, m_lastError);
    return false;
}

void SharedMemoryConnection::disconnect() {
    if (m_connected) {
        unmapSharedMemory();
        
#ifdef _WIN32
        if (m_sharedMemory) {
            CloseHandle(m_sharedMemory);
            m_sharedMemory = nullptr;
        }
#elif defined(__linux__) || defined(__APPLE__)
        if (m_sharedMemoryFd >= 0) {
            close(m_sharedMemoryFd);
            m_sharedMemoryFd = -1;
            
            // Remove the shared memory file
            shm_unlink(m_name.c_str());
        }
#endif
        
        m_connected = false;
        RB_LOG_INFO_FMT("SharedMemoryConnection: Disconnected from shared memory '{}'", m_name);
    }
}

bool SharedMemoryConnection::isConnected() const {
    return m_connected;
}

bool SharedMemoryConnection::sendMessage(uint16_t messageType, const uint8_t* payload, uint32_t payloadSize) {
    if (!m_connected || !m_mappedAddress) {
        setLastError("Not connected");
        return false;
    }
    
    // Total message size: header (16) + payload
    uint32_t totalSize = MESSAGE_HEADER_SIZE + payloadSize;
    
    // Check if we have enough space
    // For now, we assume the shared memory is large enough
    // In a real implementation, we would use a ring buffer
    
    // Write message to shared memory
    uint8_t* sharedMemory = static_cast<uint8_t*>(m_mappedAddress);
    
    // Write header
    sharedMemory[0] = static_cast<uint8_t>((messageType >> 8) & 0xFF);
    sharedMemory[1] = static_cast<uint8_t>(messageType & 0xFF);
    sharedMemory[2] = static_cast<uint8_t>((payloadSize >> 24) & 0xFF);
    sharedMemory[3] = static_cast<uint8_t>((payloadSize >> 16) & 0xFF);
    sharedMemory[4] = static_cast<uint8_t>((payloadSize >> 8) & 0xFF);
    sharedMemory[5] = static_cast<uint8_t>(payloadSize & 0xFF);
    sharedMemory[6] = 1; // version
    sharedMemory[7] = 0; // flags
    sharedMemory[8] = 0; // checksum (high)
    sharedMemory[9] = 0; // checksum (low)
    sharedMemory[10] = 0; // sequence (high high)
    sharedMemory[11] = 0; // sequence (high low)
    sharedMemory[12] = 0; // sequence (low high)
    sharedMemory[13] = 0; // sequence (low low)
    
    // Write payload
    if (payload && payloadSize > 0) {
        std::memcpy(sharedMemory + MESSAGE_HEADER_SIZE, payload, payloadSize);
    }
    
    return true;
}

std::vector<std::vector<uint8_t>> SharedMemoryConnection::receiveMessages(uint32_t timeoutMs) {
    std::vector<std::vector<uint8_t>> messages;
    
    if (!m_connected || !m_mappedAddress) {
        return messages;
    }
    
    // For now, just read the first message
    // In a real implementation, we would use a ring buffer with proper synchronization
    
    uint8_t* sharedMemory = static_cast<uint8_t*>(m_mappedAddress);
    
    // Read header
    uint16_t messageType = static_cast<uint16_t>(sharedMemory[0] << 8 | sharedMemory[1]);
    uint32_t payloadSize = static_cast<uint32_t>(sharedMemory[2] << 24 | 
                                                   sharedMemory[3] << 16 | 
                                                   sharedMemory[4] << 8 | 
                                                   sharedMemory[5]);
    
    if (payloadSize > 0) {
        // Read payload
        std::vector<uint8_t> message;
        message.resize(MESSAGE_HEADER_SIZE + payloadSize);
        std::memcpy(message.data(), sharedMemory, MESSAGE_HEADER_SIZE + payloadSize);
        messages.push_back(message);
    }
    
    return messages;
}

bool SharedMemoryConnection::isDataAvailable() const {
    // For now, always return true if connected
    // In a real implementation, we would check if there's data in the ring buffer
    return m_connected && m_mappedAddress != nullptr;
}

TransportType SharedMemoryConnection::getTransportType() const {
    return TransportType::SHARED_MEMORY;
}

std::string SharedMemoryConnection::getStatistics() const {
    return "SharedMemory: " + m_name + " (" + std::to_string(m_size) + " bytes)";
}

bool SharedMemoryConnection::createSharedMemory() {
#ifdef _WIN32
    // Windows implementation
    m_sharedMemory = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        nullptr,
        PAGE_READWRITE,
        0,
        m_size,
        m_name.c_str()
    );
    
    if (!m_sharedMemory) {
        DWORD error = GetLastError();
        setLastError("CreateFileMapping failed with error " + std::to_string(error));
        RB_LOG_ERROR_FMT("SharedMemoryConnection: CreateFileMapping failed: {}", error);
        return false;
    }
    
    return true;
#elif defined(__linux__) || defined(__APPLE__)
    // Linux/macOS implementation
    m_sharedMemoryFd = shm_open(m_name.c_str(), O_CREAT | O_RDWR, 0666);
    
    if (m_sharedMemoryFd < 0) {
        setLastError("shm_open failed: " + std::string(strerror(errno)));
        RB_LOG_ERROR_FMT("SharedMemoryConnection: shm_open failed: {}", strerror(errno));
        return false;
    }
    
    // Set size
    if (ftruncate(m_sharedMemoryFd, m_size) < 0) {
        setLastError("ftruncate failed: " + std::string(strerror(errno)));
        RB_LOG_ERROR_FMT("SharedMemoryConnection: ftruncate failed: {}", strerror(errno));
        close(m_sharedMemoryFd);
        m_sharedMemoryFd = -1;
        return false;
    }
    
    return true;
#else
    setLastError("Shared memory not supported on this platform");
    return false;
#endif
}

bool SharedMemoryConnection::openSharedMemory() {
#ifdef _WIN32
    // Windows implementation
    m_sharedMemory = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        m_name.c_str()
    );
    
    if (!m_sharedMemory) {
        DWORD error = GetLastError();
        setLastError("OpenFileMapping failed with error " + std::to_string(error));
        RB_LOG_ERROR_FMT("SharedMemoryConnection: OpenFileMapping failed: {}", error);
        return false;
    }
    
    return true;
#elif defined(__linux__) || defined(__APPLE__)
    // Linux/macOS implementation
    m_sharedMemoryFd = shm_open(m_name.c_str(), O_RDWR, 0666);
    
    if (m_sharedMemoryFd < 0) {
        setLastError("shm_open failed: " + std::string(strerror(errno)));
        RB_LOG_ERROR_FMT("SharedMemoryConnection: shm_open failed: {}", strerror(errno));
        return false;
    }
    
    return true;
#else
    setLastError("Shared memory not supported on this platform");
    return false;
#endif
}

bool SharedMemoryConnection::mapSharedMemory() {
#ifdef _WIN32
    // Windows implementation
    m_mappedAddress = MapViewOfFile(
        m_sharedMemory,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        m_size
    );
    
    if (!m_mappedAddress) {
        DWORD error = GetLastError();
        setLastError("MapViewOfFile failed with error " + std::to_string(error));
        RB_LOG_ERROR_FMT("SharedMemoryConnection: MapViewOfFile failed: {}", error);
        return false;
    }
    
    return true;
#elif defined(__linux__) || defined(__APPLE__)
    // Linux/macOS implementation
    m_mappedAddress = mmap(
        nullptr,
        m_size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        m_sharedMemoryFd,
        0
    );
    
    if (m_mappedAddress == MAP_FAILED) {
        setLastError("mmap failed: " + std::string(strerror(errno)));
        RB_LOG_ERROR_FMT("SharedMemoryConnection: mmap failed: {}", strerror(errno));
        return false;
    }
    
    return true;
#else
    setLastError("Shared memory mapping not supported on this platform");
    return false;
#endif
}

void SharedMemoryConnection::unmapSharedMemory() {
#ifdef _WIN32
    if (m_mappedAddress) {
        UnmapViewOfFile(m_mappedAddress);
        m_mappedAddress = nullptr;
    }
#elif defined(__linux__) || defined(__APPLE__)
    if (m_mappedAddress) {
        munmap(m_mappedAddress, m_size);
        m_mappedAddress = nullptr;
    }
#endif
}

// ============================================================================
// SocketConnection
// ============================================================================

SocketConnection::SocketConnection(const std::string& host, uint16_t port)
    : m_host(host),
      m_port(port),
      m_connected(false),
      m_noDelay(true),
      m_keepAlive(true),
      m_lastError("No error") {
#ifdef _WIN32
    m_socket = INVALID_SOCKET;
#else
    m_socket = -1;
#endif
}

SocketConnection::~SocketConnection() {
    disconnect();
}

bool SocketConnection::connect() {
    if (!createSocket()) {
        return false;
    }
    
    if (!connectToHost()) {
        return false;
    }
    
    setSocketOptions(m_noDelay, m_keepAlive);
    m_connected = true;
    RB_LOG_INFO_FMT("SocketConnection: Connected to {}:{}", m_host, m_port);
    return true;
}

void SocketConnection::disconnect() {
    if (m_connected) {
        closeSocket();
        m_connected = false;
        RB_LOG_INFO_FMT("SocketConnection: Disconnected from {}:{}", m_host, m_port);
    }
}

bool SocketConnection::isConnected() const {
    return m_connected;
}

bool SocketConnection::sendMessage(uint16_t messageType, const uint8_t* payload, uint32_t payloadSize) {
    if (!m_connected) {
        setLastError("Not connected");
        return false;
    }
    
    // Create message buffer
    std::vector<uint8_t> messageBuffer(MESSAGE_HEADER_SIZE + payloadSize);
    
    // Write header (big-endian)
    messageBuffer[0] = static_cast<uint8_t>((messageType >> 8) & 0xFF);
    messageBuffer[1] = static_cast<uint8_t>(messageType & 0xFF);
    messageBuffer[2] = static_cast<uint8_t>((payloadSize >> 24) & 0xFF);
    messageBuffer[3] = static_cast<uint8_t>((payloadSize >> 16) & 0xFF);
    messageBuffer[4] = static_cast<uint8_t>((payloadSize >> 8) & 0xFF);
    messageBuffer[5] = static_cast<uint8_t>(payloadSize & 0xFF);
    messageBuffer[6] = 1; // version
    messageBuffer[7] = 0; // flags
    messageBuffer[8] = 0; // checksum (high)
    messageBuffer[9] = 0; // checksum (low)
    messageBuffer[10] = 0; // sequence (high high)
    messageBuffer[11] = 0; // sequence (high low)
    messageBuffer[12] = 0; // sequence (low high)
    messageBuffer[13] = 0; // sequence (low low)
    
    // Write payload
    if (payload && payloadSize > 0) {
        std::memcpy(messageBuffer.data() + MESSAGE_HEADER_SIZE, payload, payloadSize);
    }
    
    // Send message
    ssize_t bytesSent = send(
        m_socket,
        reinterpret_cast<const char*>(messageBuffer.data()),
        static_cast<int>(messageBuffer.size()),
        0
    );
    
    if (bytesSent < 0) {
#ifdef _WIN32
        int error = WSAGetLastError();
        setLastError("send failed with error " + std::to_string(error));
        RB_LOG_ERROR_FMT("SocketConnection: send failed: {}", error);
#else
        setLastError("send failed: " + std::string(strerror(errno)));
        RB_LOG_ERROR_FMT("SocketConnection: send failed: {}", strerror(errno));
#endif
        return false;
    }
    
    if (static_cast<uint32_t>(bytesSent) != messageBuffer.size()) {
        setLastError("Partial send: expected " + std::to_string(messageBuffer.size()) + 
                     " bytes, sent " + std::to_string(bytesSent) + " bytes");
        RB_LOG_ERROR_FMT("SocketConnection: Partial send: {}/{} bytes", bytesSent, messageBuffer.size());
        return false;
    }
    
    return true;
}

std::vector<std::vector<uint8_t>> SocketConnection::receiveMessages(uint32_t timeoutMs) {
    std::vector<std::vector<uint8_t>> messages;
    
    if (!m_connected) {
        return messages;
    }
    
    // Read message header first
    std::vector<uint8_t> headerBuffer(MESSAGE_HEADER_SIZE);
    
    int bytesReceived = receiveData(headerBuffer.data(), MESSAGE_HEADER_SIZE, timeoutMs);
    
    if (bytesReceived <= 0) {
        // Timeout or error
        return messages;
    }
    
    if (static_cast<uint32_t>(bytesReceived) < MESSAGE_HEADER_SIZE) {
        setLastError("Incomplete header: received " + std::to_string(bytesReceived) + " bytes");
        RB_LOG_ERROR_FMT("SocketConnection: Incomplete header: {}/{} bytes", bytesReceived, MESSAGE_HEADER_SIZE);
        return messages;
    }
    
    // Parse header
    uint16_t messageType = static_cast<uint16_t>(headerBuffer[0] << 8 | headerBuffer[1]);
    uint32_t payloadSize = static_cast<uint32_t>(headerBuffer[2] << 24 | 
                                                   headerBuffer[3] << 16 | 
                                                   headerBuffer[4] << 8 | 
                                                   headerBuffer[5]);
    
    // Read payload
    if (payloadSize > 0) {
        std::vector<uint8_t> message(MESSAGE_HEADER_SIZE + payloadSize);
        std::memcpy(message.data(), headerBuffer.data(), MESSAGE_HEADER_SIZE);
        
        int payloadBytesReceived = receiveData(
            message.data() + MESSAGE_HEADER_SIZE,
            payloadSize,
            timeoutMs
        );
        
        if (payloadBytesReceived <= 0) {
            return messages;
        }
        
        if (static_cast<uint32_t>(payloadBytesReceived) < payloadSize) {
            setLastError("Incomplete payload: received " + std::to_string(payloadBytesReceived) + 
                         "/" + std::to_string(payloadSize) + " bytes");
            RB_LOG_ERROR_FMT("SocketConnection: Incomplete payload: {}/{} bytes", 
                            payloadBytesReceived, payloadSize);
            return messages;
        }
        
        messages.push_back(message);
    } else {
        // Message with no payload
        std::vector<uint8_t> message(MESSAGE_HEADER_SIZE);
        std::memcpy(message.data(), headerBuffer.data(), MESSAGE_HEADER_SIZE);
        messages.push_back(message);
    }
    
    return messages;
}

bool SocketConnection::isDataAvailable() const {
    if (!m_connected) {
        return false;
    }
    
    // Check if data is available to read
    // This is a simple implementation; in production, you might want to use select() or poll()
    
#ifdef _WIN32
    u_long available = 0;
    if (ioctlsocket(m_socket, FIONREAD, &available) == 0) {
        return available > 0;
    }
#else
    int available = 0;
    if (ioctl(m_socket, FIONREAD, &available) == 0) {
        return available > 0;
    }
#endif
    
    return false;
}

TransportType SocketConnection::getTransportType() const {
    return TransportType::LOCAL_SOCKET;
}

std::string SocketConnection::getStatistics() const {
    return "Socket: " + m_host + ":" + std::to_string(m_port);
}

void SocketConnection::setSocketOptions(bool noDelay, bool keepAlive) {
    m_noDelay = noDelay;
    m_keepAlive = keepAlive;
    
    if (!m_connected) {
        return;
    }
    
    // Set TCP_NODELAY
    int flag = noDelay ? 1 : 0;
    if (setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, 
                   reinterpret_cast<const char*>(&flag), sizeof(flag)) < 0) {
#ifdef _WIN32
        int error = WSAGetLastError();
        RB_LOG_ERROR_FMT("SocketConnection: setsockopt(TCP_NODELAY) failed: {}", error);
#else
        RB_LOG_ERROR_FMT("SocketConnection: setsockopt(TCP_NODELAY) failed: {}", strerror(errno));
#endif
    }
    
    // Set SO_KEEPALIVE
    flag = keepAlive ? 1 : 0;
    if (setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE,
                   reinterpret_cast<const char*>(&flag), sizeof(flag)) < 0) {
#ifdef _WIN32
        int error = WSAGetLastError();
        RB_LOG_ERROR_FMT("SocketConnection: setsockopt(SO_KEEPALIVE) failed: {}", error);
#else
        RB_LOG_ERROR_FMT("SocketConnection: setsockopt(SO_KEEPALIVE) failed: {}", strerror(errno));
#endif
    }
}

bool SocketConnection::createSocket() {
#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        setLastError("WSAStartup failed with error " + std::to_string(result));
        RB_LOG_ERROR_FMT("SocketConnection: WSAStartup failed: {}", result);
        return false;
    }
    
    // Create socket
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET) {
        int error = WSAGetLastError();
        setLastError("socket() failed with error " + std::to_string(error));
        RB_LOG_ERROR_FMT("SocketConnection: socket() failed: {}", error);
        WSACleanup();
        return false;
    }
#else
    // Create socket
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0) {
        setLastError("socket() failed: " + std::string(strerror(errno)));
        RB_LOG_ERROR_FMT("SocketConnection: socket() failed: {}", strerror(errno));
        return false;
    }
    
    // Set socket to non-blocking for timeout support
    int flags = fcntl(m_socket, F_GETFL, 0);
    if (fcntl(m_socket, F_SETFL, flags | O_NONBLOCK) < 0) {
        setLastError("fcntl(O_NONBLOCK) failed: " + std::string(strerror(errno)));
        RB_LOG_ERROR_FMT("SocketConnection: fcntl(O_NONBLOCK) failed: {}", strerror(errno));
        close(m_socket);
        m_socket = -1;
        return false;
    }
#endif
    
    return true;
}

bool SocketConnection::connectToHost() {
    // Resolve hostname
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    struct addrinfo* result = nullptr;
    int error = getaddrinfo(m_host.c_str(), std::to_string(m_port).c_str(), &hints, &result);
    
    if (error != 0) {
        setLastError("getaddrinfo failed: " + std::string(gai_strerror(error)));
        RB_LOG_ERROR_FMT("SocketConnection: getaddrinfo failed: {}", gai_strerror(error));
        return false;
    }
    
    // Connect to first address
    struct addrinfo* ptr = result;
    bool connected = false;
    
    while (ptr != nullptr) {
        if (connect(m_socket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen)) == 0) {
            connected = true;
            break;
        }
        
        ptr = ptr->ai_next;
    }
    
    freeaddrinfo(result);
    
    if (!connected) {
#ifdef _WIN32
        int error = WSAGetLastError();
        setLastError("connect() failed with error " + std::to_string(error));
        RB_LOG_ERROR_FMT("SocketConnection: connect() failed: {}", error);
#else
        setLastError("connect() failed: " + std::string(strerror(errno)));
        RB_LOG_ERROR_FMT("SocketConnection: connect() failed: {}", strerror(errno));
#endif
        return false;
    }
    
    return true;
}

void SocketConnection::closeSocket() {
#ifdef _WIN32
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        WSACleanup();
    }
#else
    if (m_socket >= 0) {
        close(m_socket);
        m_socket = -1;
    }
#endif
}

int SocketConnection::receiveData(uint8_t* buffer, uint32_t size, uint32_t timeoutMs) {
    if (!m_connected) {
        return -1;
    }
    
    // Set timeout
#ifdef _WIN32
    DWORD timeout = timeoutMs;
    if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, 
                   reinterpret_cast<const char*>(&timeout), sizeof(timeout)) < 0) {
        int error = WSAGetLastError();
        RB_LOG_ERROR_FMT("SocketConnection: setsockopt(SO_RCVTIMEO) failed: {}", error);
    }
#else
    struct timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const char*>(&tv), sizeof(tv)) < 0) {
        RB_LOG_ERROR_FMT("SocketConnection: setsockopt(SO_RCVTIMEO) failed: {}", strerror(errno));
    }
#endif
    
    // Receive data
    ssize_t bytesReceived = recv(m_socket, reinterpret_cast<char*>(buffer), size, 0);
    
    if (bytesReceived < 0) {
#ifdef _WIN32
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK) {
            // Timeout
            return 0;
        }
        setLastError("recv failed with error " + std::to_string(error));
        RB_LOG_ERROR_FMT("SocketConnection: recv failed: {}", error);
#else
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            // Timeout
            return 0;
        }
        setLastError("recv failed: " + std::string(strerror(errno)));
        RB_LOG_ERROR_FMT("SocketConnection: recv failed: {}", strerror(errno));
#endif
        return -1;
    }
    
    if (bytesReceived == 0) {
        // Connection closed
        setLastError("Connection closed by peer");
        RB_LOG_INFO("SocketConnection: Connection closed by peer");
        return -1;
    }
    
    return static_cast<int>(bytesReceived);
}

// ============================================================================
// Factory function
// ============================================================================

std::unique_ptr<BridgeConnection> createBridgeConnection(
    TransportType transportType,
    const std::string& host,
    uint16_t port
) {
    switch (transportType) {
        case TransportType::SHARED_MEMORY:
            return std::make_unique<SharedMemoryConnection>();
        case TransportType::LOCAL_SOCKET:
            return std::make_unique<SocketConnection>(host, port);
        default:
            return nullptr;
    }
}

} // namespace renderbridge
