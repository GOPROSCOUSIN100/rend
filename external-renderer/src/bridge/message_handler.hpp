#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace renderbridge {

// Forward declarations
class BridgeConnection;

/**
 * @enum MessageHandlerState
 * @brief States for the message handler
 */
enum class MessageHandlerState {
    DISCONNECTED,
    CONNECTING,
    HANDSHAKE_REQUIRED,
    CONNECTED,
    ERROR
};

/**
 * @class MessageHandler
 * @brief Handles message processing and protocol logic for the bridge
 */
class MessageHandler {
public:
    /**
     * @brief Message callback type
     */
    using MessageCallback = std::function<void(uint16_t messageType, const uint8_t* payload, uint32_t payloadSize)>;
    
    /**
     * @brief State change callback type
     */
    using StateChangeCallback = std::function<void(MessageHandlerState oldState, MessageHandlerState newState)>;
    
    /**
     * @brief Constructor
     * @param connection Pointer to the bridge connection
     */
    MessageHandler(std::shared_ptr<BridgeConnection> connection);
    
    /**
     * @brief Destructor
     */
    ~MessageHandler();
    
    /**
     * @brief Initialize the message handler
     * @param clientType Type of client (0 = mod, 1 = renderer)
     * @param clientName Name of this client
     * @param clientVersion Version of this client
     * @param capabilities Bitmask of supported capabilities
     */
    void initialize(uint8_t clientType, const std::string& clientName, uint32_t clientVersion, uint32_t capabilities);
    
    /**
     * @brief Process incoming messages
     * @return Number of messages processed
     */
    uint32_t processMessages();
    
    /**
     * @brief Send a message
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
     * @param sequenceNumber Sequence number
     * @param lastFrameProcessed Last frame processed
     * @param avgProcessingTimeMs Average processing time
     * @return True if heartbeat was sent successfully
     */
    bool sendHeartbeat(uint32_t sequenceNumber, int64_t lastFrameProcessed, float avgProcessingTimeMs);
    
    /**
     * @brief Send an acknowledgment
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
     * @brief Register a message callback
     * @param callback The callback function
     */
    void registerMessageCallback(MessageCallback callback);
    
    /**
     * @brief Register a state change callback
     * @param callback The callback function
     */
    void registerStateChangeCallback(StateChangeCallback callback);
    
    /**
     * @brief Get the current state
     * @return Current state
     */
    MessageHandlerState getState() const;
    
    /**
     * @brief Check if handshake is complete
     * @return True if handshake is complete
     */
    bool isHandshakeComplete() const;
    
    /**
     * @brief Get the negotiated protocol version
     * @return Protocol version
     */
    uint8_t getProtocolVersion() const;
    
    /**
     * @brief Get the negotiated capabilities
     * @return Bitmask of capabilities
     */
    uint32_t getCapabilities() const;
    
    /**
     * @brief Check if a capability is supported
     * @param capability The capability to check
     * @return True if the capability is supported
     */
    bool hasCapability(uint32_t capability) const;
    
    /**
     * @brief Get the remote client type
     * @return Remote client type (0 = mod, 1 = renderer)
     */
    uint8_t getRemoteClientType() const;
    
    /**
     * @brief Get the remote client name
     * @return Remote client name
     */
    std::string getRemoteClientName() const;
    
    /**
     * @brief Get the remote client version
     * @return Remote client version
     */
    uint32_t getRemoteClientVersion() const;
    
    /**
     * @brief Get the current sequence number
     * @return Current sequence number
     */
    uint32_t getCurrentSequence() const;
    
    /**
     * @brief Increment the sequence number
     */
    void incrementSequence();
    
    /**
     * @brief Get the last received frame number
     * @return Last frame number
     */
    int64_t getLastFrameNumber() const;
    
    /**
     * @brief Set the last received frame number
     * @param frameNumber Frame number
     */
    void setLastFrameNumber(int64_t frameNumber);
    
    /**
     * @brief Get statistics
     * @return Statistics string
     */
    std::string getStatistics() const;

private:
    std::shared_ptr<BridgeConnection> m_connection;
    
    MessageCallback m_messageCallback;
    StateChangeCallback m_stateChangeCallback;
    
    // Local client info
    uint8_t m_localClientType;
    std::string m_localClientName;
    uint32_t m_localClientVersion;
    uint32_t m_localCapabilities;
    
    // Remote client info
    uint8_t m_remoteClientType;
    std::string m_remoteClientName;
    uint32_t m_remoteClientVersion;
    uint32_t m_remoteCapabilities;
    
    // Protocol state
    MessageHandlerState m_state;
    uint8_t m_protocolVersion;
    uint32_t m_currentSequence;
    int64_t m_lastFrameNumber;
    
    // Statistics
    uint32_t m_messagesSent;
    uint32_t m_messagesReceived;
    uint32_t m_errors;
    
    /**
     * @brief Handle a handshake message
     * @param payload Pointer to the payload
     * @param payloadSize Size of the payload
     */
    void handleHandshake(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Handle a heartbeat message
     * @param payload Pointer to the payload
     * @param payloadSize Size of the payload
     */
    void handleHeartbeat(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Handle an acknowledgment message
     * @param payload Pointer to the payload
     * @param payloadSize Size of the payload
     */
    void handleAcknowledgment(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Handle an error message
     * @param payload Pointer to the payload
     * @param payloadSize Size of the payload
     */
    void handleError(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Handle a frame start message
     * @param payload Pointer to the payload
     * @param payloadSize Size of the payload
     */
    void handleFrameStart(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Handle a frame end message
     * @param payload Pointer to the payload
     * @param payloadSize Size of the payload
     */
    void handleFrameEnd(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Handle a camera message
     * @param payload Pointer to the payload
     * @param payloadSize Size of the payload
     */
    void handleCamera(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Handle a chunk message
     * @param payload Pointer to the payload
     * @param payloadSize Size of the payload
     */
    void handleChunk(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Handle an entity message
     * @param payload Pointer to the payload
     * @param payloadSize Size of the payload
     */
    void handleEntity(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Handle a particle message
     * @param payload Pointer to the payload
     * @param payloadSize Size of the payload
     */
    void handleParticle(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Handle a weather message
     * @param payload Pointer to the payload
     * @param payloadSize Size of the payload
     */
    void handleWeather(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Handle a GUI message
     * @param payload Pointer to the payload
     * @param payloadSize Size of the payload
     */
    void handleGui(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Handle an echo message
     * @param payload Pointer to the payload
     * @param payloadSize Size of the payload
     */
    void handleEcho(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Handle a control message
     * @param payload Pointer to the payload
     * @param payloadSize Size of the payload
     */
    void handleControl(const uint8_t* payload, uint32_t payloadSize);
    
    /**
     * @brief Change state and notify callback
     * @param newState New state
     */
    void changeState(MessageHandlerState newState);
    
    /**
     * @brief Parse a handshake payload
     * @param payload Pointer to the payload
     * @param payloadSize Size of the payload
     * @param protocolVersion Output: protocol version
     * @param capabilities Output: capabilities
     * @param clientType Output: client type
     * @param clientVersion Output: client version
     * @param clientName Output: client name
     * @return True if parsing succeeded
     */
    static bool parseHandshakePayload(
        const uint8_t* payload, uint32_t payloadSize,
        uint8_t& protocolVersion, uint32_t& capabilities,
        uint8_t& clientType, uint32_t& clientVersion,
        std::string& clientName
    );
    
    /**
     * @brief Build a handshake payload
     * @param protocolVersion Protocol version
     * @param capabilities Capabilities
     * @param clientType Client type
     * @param clientVersion Client version
     * @param clientName Client name
     * @param output Output buffer
     * @return Size of the payload
     */
    static uint32_t buildHandshakePayload(
        uint8_t protocolVersion, uint32_t capabilities,
        uint8_t clientType, uint32_t clientVersion,
        const std::string& clientName, std::vector<uint8_t>& output
    );
};

} // namespace renderbridge
