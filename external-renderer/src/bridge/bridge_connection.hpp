#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace renderbridge {

/**
 * @enum TransportType
 * @brief Transport types for bridge communication
 */
enum class TransportType {
    SHARED_MEMORY,
    LOCAL_SOCKET
};

/**
 * @class BridgeConnection
 * @brief Abstract base class for bridge connections
 * 
 * This class provides an abstract interface for different transport
 * implementations (shared memory, socket, etc.)
 */
class BridgeConnection {
public:
    /**
     * @brief Constructor
     */
    BridgeConnection();
    
    /**
     * @brief Virtual destructor
     */
    virtual ~BridgeConnection();
    
    /**
     * @brief Connect to the other end
     * @return True if connection succeeded
     */
    virtual bool connect() = 0;
    
    /**
     * @brief Disconnect
     */
    virtual void disconnect() = 0;
    
    /**
     * @brief Check if connected
     * @return True if connected
     */
    virtual bool isConnected() const = 0;
    
    /**
     * @brief Send a message
     * @param messageType Type of the message
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     * @return True if message was sent successfully
     */
    virtual bool sendMessage(uint16_t messageType, const uint8_t* payload, uint32_t payloadSize) = 0;
    
    /**
     * @brief Receive messages
     * @param timeoutMs Timeout in milliseconds
     * @return Vector of received messages (message type, payload, payload size)
     */
    virtual std::vector<std::vector<uint8_t>> receiveMessages(uint32_t timeoutMs) = 0;
    
    /**
     * @brief Check if data is available
     * @return True if data is available to read
     */
    virtual bool isDataAvailable() const = 0;
    
    /**
     * @brief Get the transport type
     * @return Transport type
     */
    virtual TransportType getTransportType() const = 0;
    
    /**
     * @brief Get connection statistics
     * @return Statistics string
     */
    virtual std::string getStatistics() const = 0;
    
    /**
     * @brief Get the last error message
     * @return Error message
     */
    std::string getLastError() const;

protected:
    std::string m_lastError;
    
    /**
     * @brief Set the last error message
     * @param error Error message
     */
    void setLastError(const std::string& error);
};

/**
 * @class SharedMemoryConnection
 * @brief Bridge connection using shared memory (memory-mapped file)
 */
class SharedMemoryConnection : public BridgeConnection {
public:
    /**
     * @brief Constructor
     * @param name Name of the shared memory segment
     * @param size Size of the shared memory segment
     */
    SharedMemoryConnection(const std::string& name = "renderbridge_shm", uint32_t size = 16 * 1024 * 1024);
    
    /**
     * @brief Destructor
     */
    ~SharedMemoryConnection() override;
    
    /**
     * @brief Connect to shared memory
     * @return True if connection succeeded
     */
    bool connect() override;
    
    /**
     * @brief Disconnect
     */
    void disconnect() override;
    
    /**
     * @brief Check if connected
     * @return True if connected
     */
    bool isConnected() const override;
    
    /**
     * @brief Send a message
     * @param messageType Type of the message
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     * @return True if message was sent successfully
     */
    bool sendMessage(uint16_t messageType, const uint8_t* payload, uint32_t payloadSize) override;
    
    /**
     * @brief Receive messages
     * @param timeoutMs Timeout in milliseconds
     * @return Vector of received messages
     */
    std::vector<std::vector<uint8_t>> receiveMessages(uint32_t timeoutMs) override;
    
    /**
     * @brief Check if data is available
     * @return True if data is available to read
     */
    bool isDataAvailable() const override;
    
    /**
     * @brief Get the transport type
     * @return Transport type
     */
    TransportType getTransportType() const override;
    
    /**
     * @brief Get connection statistics
     * @return Statistics string
     */
    std::string getStatistics() const override;

private:
    std::string m_name;
    uint32_t m_size;
    
    // Platform-specific implementation
#ifdef _WIN32
    void* m_sharedMemory;
    void* m_mappedAddress;
#elif defined(__linux__) || defined(__APPLE__)
    int m_sharedMemoryFd;
    void* m_mappedAddress;
#endif
    
    bool m_connected;
    
    /**
     * @brief Create shared memory segment
     * @return True if creation succeeded
     */
    bool createSharedMemory();
    
    /**
     * @brief Open existing shared memory segment
     * @return True if opening succeeded
     */
    bool openSharedMemory();
    
    /**
     * @brief Map shared memory into address space
     * @return True if mapping succeeded
     */
    bool mapSharedMemory();
    
    /**
     * @brief Unmap shared memory
     */
    void unmapSharedMemory();
};

/**
 * @class SocketConnection
 * @brief Bridge connection using local TCP socket
 */
class SocketConnection : public BridgeConnection {
public:
    /**
     * @brief Constructor
     * @param host Hostname or IP address
     * @param port Port number
     */
    SocketConnection(const std::string& host = "127.0.0.1", uint16_t port = 25565);
    
    /**
     * @brief Destructor
     */
    ~SocketConnection() override;
    
    /**
     * @brief Connect to socket
     * @return True if connection succeeded
     */
    bool connect() override;
    
    /**
     * @brief Disconnect
     */
    void disconnect() override;
    
    /**
     * @brief Check if connected
     * @return True if connected
     */
    bool isConnected() const override;
    
    /**
     * @brief Send a message
     * @param messageType Type of the message
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     * @return True if message was sent successfully
     */
    bool sendMessage(uint16_t messageType, const uint8_t* payload, uint32_t payloadSize) override;
    
    /**
     * @brief Receive messages
     * @param timeoutMs Timeout in milliseconds
     * @return Vector of received messages
     */
    std::vector<std::vector<uint8_t>> receiveMessages(uint32_t timeoutMs) override;
    
    /**
     * @brief Check if data is available
     * @return True if data is available to read
     */
    bool isDataAvailable() const override;
    
    /**
     * @brief Get the transport type
     * @return Transport type
     */
    TransportType getTransportType() const override;
    
    /**
     * @brief Get connection statistics
     * @return Statistics string
     */
    std::string getStatistics() const override;
    
    /**
     * @brief Set socket options
     * @param noDelay Enable/disable Nagle's algorithm
     * @param keepAlive Enable/disable keep-alive
     */
    void setSocketOptions(bool noDelay = true, bool keepAlive = true);

private:
    std::string m_host;
    uint16_t m_port;
    
    // Platform-specific socket handle
#ifdef _WIN32
    void* m_socket;
#elif defined(__linux__) || defined(__APPLE__)
    int m_socket;
#endif
    
    bool m_connected;
    bool m_noDelay;
    bool m_keepAlive;
    
    /**
     * @brief Create socket
     * @return True if socket creation succeeded
     */
    bool createSocket();
    
    /**
     * @brief Connect to remote host
     * @return True if connection succeeded
     */
    bool connectToHost();
    
    /**
     * @brief Close socket
     */
    void closeSocket();
    
    /**
     * @brief Receive raw data from socket
     * @param buffer Buffer to receive into
     * @param size Size of the buffer
     * @param timeoutMs Timeout in milliseconds
     * @return Number of bytes received, or -1 on error
     */
    int receiveData(uint8_t* buffer, uint32_t size, uint32_t timeoutMs);
};

/**
 * @brief Create a bridge connection based on transport type
 * @param transportType Transport type
 * @param host Hostname (for socket)
 * @param port Port (for socket)
 * @return Unique pointer to the bridge connection
 */
std::unique_ptr<BridgeConnection> createBridgeConnection(
    TransportType transportType,
    const std::string& host = "127.0.0.1",
    uint16_t port = 25565
);

} // namespace renderbridge
