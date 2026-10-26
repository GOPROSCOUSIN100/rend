#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <functional>
#include <vector>

namespace renderbridge {

// Forward declarations
class BridgeConnection;
class MessageHandler;

/**
 * @class RendererBridgeClient
 * @brief C++ bridge client for the external renderer (Process B)
 * 
 * This class manages the connection to the Minecraft mod (Process A) and
 * handles incoming messages from the bridge protocol.
 */
class RendererBridgeClient {
public:
    /**
     * @brief Message callback type
     * @param messageType The type of the message
     * @param payload Pointer to the message payload
     * @param payloadSize Size of the payload in bytes
     */
    using MessageCallback = std::function<void(uint16_t messageType, const uint8_t* payload, uint32_t payloadSize)>;
    
    /**
     * @brief Connection status callback type
     * @param connected True if connected, false if disconnected
     * @param errorMessage Error message if disconnected, empty otherwise
     */
    using ConnectionStatusCallback = std::function<void(bool connected, const std::string& errorMessage)>;
    
    /**
     * @brief Constructor
     */
    RendererBridgeClient();
    
    /**
     * @brief Destructor
     */
    ~RendererBridgeClient();
    
    /**
     * @brief Initialize the bridge client
     * @param clientName Name of this client (for handshake)
     * @param clientVersion Version of this client
     * @return True if initialization succeeded
     */
    bool initialize(const std::string& clientName, uint32_t clientVersion);
    
    /**
     * @brief Connect to the mod via shared memory
     * @return True if connection succeeded
     */
    bool connectSharedMemory();
    
    /**
     * @brief Connect to the mod via local socket
     * @param host Hostname or IP address (default: localhost)
     * @param port Port number (default: 25565)
     * @return True if connection succeeded
     */
    bool connectSocket(const std::string& host = "127.0.0.1", uint16_t port = 25565);
    
    /**
     * @brief Disconnect from the mod
     */
    void disconnect();
    
    /**
     * @brief Check if connected
     * @return True if connected
     */
    bool isConnected() const;
    
    /**
     * @brief Poll for incoming messages
     * @return Number of messages processed
     */
    uint32_t pollMessages();
    
    /**
     * @brief Send a message to the mod
     * @param messageType Type of the message
     * @param payload Pointer to the payload data
     * @param payloadSize Size of the payload
     * @return True if message was sent successfully
     */
    bool sendMessage(uint16_t messageType, const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Send a handshake message
     * @return True if handshake was sent successfully
     */
    bool sendHandshake();
    
    /**
     * @brief Send a heartbeat message
     * @param sequenceNumber Sequence number for the heartbeat
     * @param lastFrameProcessed Last frame that was processed
     * @param avgProcessingTimeMs Average processing time in milliseconds
     * @return True if heartbeat was sent successfully
     */
    bool sendHeartbeat(uint32_t sequenceNumber, int64_t lastFrameProcessed, float avgProcessingTimeMs);
    
    /**
     * @brief Send an acknowledgment message
     * @param sequenceNumber Sequence number to acknowledge
     * @return True if acknowledgment was sent successfully
     */
    bool sendAcknowledgment(uint32_t sequenceNumber);
    
    /**
     * @brief Send an error message
     * @param errorCode Error code
     * @param errorMessage Error message string
     * @return True if error message was sent successfully
     */
    bool sendError(uint32_t errorCode, const std::string& errorMessage);
    
    /**
     * @brief Register a callback for incoming messages
     * @param callback The callback function
     */
    void registerMessageCallback(MessageCallback callback);
    
    /**
     * @brief Register a callback for connection status changes
     * @param callback The callback function
     */
    void registerConnectionStatusCallback(ConnectionStatusCallback callback);
    
    /**
     * @brief Get the protocol version
     * @return Protocol version
     */
    uint8_t getProtocolVersion() const;
    
    /**
     * @brief Get the negotiated capabilities
     * @return Bitmask of capabilities
     */
    uint32_t getCapabilities() const;
    
    /**
     * @brief Check if a specific capability is supported
     * @param capability The capability to check
     * @return True if the capability is supported
     */
    bool hasCapability(uint32_t capability) const;
    
    /**
     * @brief Get the current sequence number
     * @return Current sequence number
     */
    uint32_t getCurrentSequence() const;
    
    /**
     * @brief Get the last received frame number
     * @return Last frame number
     */
    int64_t getLastFrameNumber() const;
    
    /**
     * @brief Set the transport preference
     * @param preferSharedMemory True to prefer shared memory, false for socket
     */
    void setPreferSharedMemory(bool preferSharedMemory);
    
    /**
     * @brief Check if shared memory is available
     * @return True if shared memory transport is available
     */
    bool isSharedMemoryAvailable() const;
    
    /**
     * @brief Check if socket transport is available
     * @return True if socket transport is available
     */
    bool isSocketAvailable() const;
    
    /**
     * @brief Get the current transport type
     * @return "shared_memory" or "socket"
     */
    std::string getCurrentTransport() const;

private:
    std::unique_ptr<BridgeConnection> m_connection;
    std::unique_ptr<MessageHandler> m_messageHandler;
    
    MessageCallback m_messageCallback;
    ConnectionStatusCallback m_connectionStatusCallback;
    
    std::string m_clientName;
    uint32_t m_clientVersion;
    uint8_t m_protocolVersion;
    uint32_t m_capabilities;
    uint32_t m_currentSequence;
    int64_t m_lastFrameNumber;
    bool m_preferSharedMemory;
    bool m_connected;
    std::string m_currentTransport;
    
    /**
     * @brief Handle incoming message
     * @param messageType Type of the message
     * @param payload Pointer to the payload
     * @param payloadSize Size of the payload
     */
    void handleMessage(uint16_t messageType, const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Handle connection status change
     * @param connected True if connected
     * @param errorMessage Error message if disconnected
     */
    void handleConnectionStatus(bool connected, const std::string& errorMessage);
};

} // namespace renderbridge
