#include "message_handler.hpp"
#include "bridge_connection.hpp"
#include "utils/byte_buffer.hpp"
#include "utils/logger.hpp"

#include <algorithm>

namespace renderbridge {

// Message type constants (from Java protocol)
static constexpr uint16_t MSG_HANDSHAKE = 0x0001;
static constexpr uint16_t MSG_HEARTBEAT = 0x0002;
static constexpr uint16_t MSG_ACK = 0x0003;
static constexpr uint16_t MSG_ERROR = 0x0004;
static constexpr uint16_t MSG_CONTROL = 0x0005;
static constexpr uint16_t MSG_ECHO = 0x0006;
static constexpr uint16_t MSG_FRAME_START = 0x0100;
static constexpr uint16_t MSG_FRAME_END = 0x0101;
static constexpr uint16_t MSG_CAMERA = 0x0200;
static constexpr uint16_t MSG_CHUNK = 0x0201;
static constexpr uint16_t MSG_ENTITY = 0x0202;
static constexpr uint16_t MSG_PARTICLE = 0x0203;
static constexpr uint16_t MSG_WEATHER = 0x0204;
static constexpr uint16_t MSG_GUI = 0x0205;

// Capability constants
static constexpr uint32_t CAP_SHARED_MEMORY = 0x00000001;
static constexpr uint32_t CAP_SOCKET = 0x00000002;
static constexpr uint32_t CAP_COMPRESSION = 0x00000004;
static constexpr uint32_t CAP_ENCRYPTION = 0x00000008;

// Client types
static constexpr uint8_t CLIENT_TYPE_MOD = 0;
static constexpr uint8_t CLIENT_TYPE_RENDERER = 1;

MessageHandler::MessageHandler(std::shared_ptr<BridgeConnection> connection)
    : m_connection(connection),
      m_localClientType(CLIENT_TYPE_RENDERER),
      m_localClientVersion(0x00020000),
      m_localCapabilities(CAP_SHARED_MEMORY | CAP_SOCKET),
      m_remoteClientType(0),
      m_remoteClientVersion(0),
      m_remoteCapabilities(0),
      m_state(MessageHandlerState::DISCONNECTED),
      m_protocolVersion(1),
      m_currentSequence(0),
      m_lastFrameNumber(-1),
      m_messagesSent(0),
      m_messagesReceived(0),
      m_errors(0) {
}

MessageHandler::~MessageHandler() {
}

void MessageHandler::initialize(uint8_t clientType, const std::string& clientName, 
                              uint32_t clientVersion, uint32_t capabilities) {
    m_localClientType = clientType;
    m_localClientName = clientName;
    m_localClientVersion = clientVersion;
    m_localCapabilities = capabilities;
    
    // Default to renderer capabilities
    if (m_localCapabilities == 0) {
        m_localCapabilities = CAP_SHARED_MEMORY | CAP_SOCKET;
    }
    
    RB_LOG_INFO_FMT("MessageHandler: Initialized as client type {} ({}), version 0x{:08X}, capabilities 0x{:08X}",
                    m_localClientType, m_localClientName, m_localClientVersion, m_localCapabilities);
}

uint32_t MessageHandler::processMessages() {
    if (!m_connection || !m_connection->isConnected()) {
        if (m_state != MessageHandlerState::DISCONNECTED) {
            changeState(MessageHandlerState::DISCONNECTED);
        }
        return 0;
    }
    
    if (m_state == MessageHandlerState::DISCONNECTED) {
        changeState(MessageHandlerState::CONNECTING);
    }
    
    uint32_t messageCount = 0;
    
    // Receive messages
    auto messages = m_connection->receiveMessages(10); // 10ms timeout
    
    for (auto& message : messages) {
        if (message.size() < 16) { // Minimum header size
            RB_LOG_WARN_FMT("MessageHandler: Received message too small ({} bytes), skipping", message.size());
            continue;
        }
        
        // Parse message type
        uint16_t messageType = static_cast<uint16_t>(message[0] << 8 | message[1]);
        uint32_t payloadSize = static_cast<uint32_t>(message[2] << 24 | 
                                                       message[3] << 16 | 
                                                       message[4] << 8 | 
                                                       message[5]);
        
        // Validate payload size
        if (payloadSize + 16 > message.size()) {
            RB_LOG_WARN_FMT("MessageHandler: Message payload size mismatch: expected {}, got {} bytes",
                           payloadSize, message.size() - 16);
            continue;
        }
        
        // Get payload pointer
        const uint8_t* payload = message.data() + 16;
        
        // Process message based on type
        switch (messageType) {
            case MSG_HANDSHAKE:
                handleHandshake(payload, payloadSize);
                break;
            case MSG_HEARTBEAT:
                handleHeartbeat(payload, payloadSize);
                break;
            case MSG_ACK:
                handleAcknowledgment(payload, payloadSize);
                break;
            case MSG_ERROR:
                handleError(payload, payloadSize);
                break;
            case MSG_FRAME_START:
                handleFrameStart(payload, payloadSize);
                break;
            case MSG_FRAME_END:
                handleFrameEnd(payload, payloadSize);
                break;
            case MSG_CAMERA:
                handleCamera(payload, payloadSize);
                break;
            case MSG_CHUNK:
                handleChunk(payload, payloadSize);
                break;
            case MSG_ENTITY:
                handleEntity(payload, payloadSize);
                break;
            case MSG_PARTICLE:
                handleParticle(payload, payloadSize);
                break;
            case MSG_WEATHER:
                handleWeather(payload, payloadSize);
                break;
            case MSG_GUI:
                handleGui(payload, payloadSize);
                break;
            case MSG_ECHO:
                handleEcho(payload, payloadSize);
                break;
            case MSG_CONTROL:
                handleControl(payload, payloadSize);
                break;
            default:
                RB_LOG_WARN_FMT("MessageHandler: Unknown message type 0x{:04X}", messageType);
                break;
        }
        
        messageCount++;
        m_messagesReceived++;
    }
    
    // If we're connecting and handshake hasn't completed, send handshake
    if (m_state == MessageHandlerState::CONNECTING && !isHandshakeComplete()) {
        sendHandshake();
    }
    
    return messageCount;
}

bool MessageHandler::sendMessage(uint16_t messageType, const uint8_t* payload, uint32_t payloadSize) {
    if (!m_connection || !m_connection->isConnected()) {
        setLastError("Not connected");
        return false;
    }
    
    bool success = m_connection->sendMessage(messageType, payload, payloadSize);
    
    if (success) {
        m_messagesSent++;
    } else {
        m_errors++;
        RB_LOG_ERROR_FMT("MessageHandler: Failed to send message 0x{:04X}: {}", 
                         messageType, m_connection->getLastError());
    }
    
    return success;
}

bool MessageHandler::sendHandshake() {
    RB_LOG_INFO("MessageHandler: Sending handshake...");
    
    std::vector<uint8_t> payload;
    uint32_t size = buildHandshakePayload(
        m_protocolVersion,
        m_localCapabilities,
        m_localClientType,
        m_localClientVersion,
        m_localClientName,
        payload
    );
    
    bool success = sendMessage(MSG_HANDSHAKE, payload.data(), static_cast<uint32_t>(payload.size()));
    
    if (success) {
        RB_LOG_INFO("MessageHandler: Handshake sent successfully");
    } else {
        RB_LOG_ERROR("MessageHandler: Failed to send handshake");
    }
    
    return success;
}

bool MessageHandler::sendHeartbeat(uint32_t sequenceNumber, int64_t lastFrameProcessed, float avgProcessingTimeMs) {
    // Build heartbeat payload
    std::vector<uint8_t> payload(16); // 4 + 8 + 4 = 16 bytes
    
    // Write sequence number (4 bytes)
    payload[0] = static_cast<uint8_t>((sequenceNumber >> 24) & 0xFF);
    payload[1] = static_cast<uint8_t>((sequenceNumber >> 16) & 0xFF);
    payload[2] = static_cast<uint8_t>((sequenceNumber >> 8) & 0xFF);
    payload[3] = static_cast<uint8_t>(sequenceNumber & 0xFF);
    
    // Write last frame processed (8 bytes)
    uint64_t frameNumber = static_cast<uint64_t>(lastFrameProcessed);
    payload[4] = static_cast<uint8_t>((frameNumber >> 56) & 0xFF);
    payload[5] = static_cast<uint8_t>((frameNumber >> 48) & 0xFF);
    payload[6] = static_cast<uint8_t>((frameNumber >> 40) & 0xFF);
    payload[7] = static_cast<uint8_t>((frameNumber >> 32) & 0xFF);
    payload[8] = static_cast<uint8_t>((frameNumber >> 24) & 0xFF);
    payload[9] = static_cast<uint8_t>((frameNumber >> 16) & 0xFF);
    payload[10] = static_cast<uint8_t>((frameNumber >> 8) & 0xFF);
    payload[11] = static_cast<uint8_t>(frameNumber & 0xFF);
    
    // Write avg processing time (4 bytes)
    uint32_t timeBits;
    std::memcpy(&timeBits, &avgProcessingTimeMs, sizeof(float));
    payload[12] = static_cast<uint8_t>((timeBits >> 24) & 0xFF);
    payload[13] = static_cast<uint8_t>((timeBits >> 16) & 0xFF);
    payload[14] = static_cast<uint8_t>((timeBits >> 8) & 0xFF);
    payload[15] = static_cast<uint8_t>(timeBits & 0xFF);
    
    bool success = sendMessage(MSG_HEARTBEAT, payload.data(), static_cast<uint32_t>(payload.size()));
    
    if (success) {
        RB_LOG_DEBUG_FMT("MessageHandler: Heartbeat sent (seq={}, lastFrame={}, avgTime={:.2f}ms)",
                         sequenceNumber, lastFrameProcessed, avgProcessingTimeMs);
    }
    
    return success;
}

bool MessageHandler::sendAcknowledgment(uint32_t sequenceNumber) {
    // Build ack payload (just sequence number)
    std::vector<uint8_t> payload(4);
    payload[0] = static_cast<uint8_t>((sequenceNumber >> 24) & 0xFF);
    payload[1] = static_cast<uint8_t>((sequenceNumber >> 16) & 0xFF);
    payload[2] = static_cast<uint8_t>((sequenceNumber >> 8) & 0xFF);
    payload[3] = static_cast<uint8_t>(sequenceNumber & 0xFF);
    
    bool success = sendMessage(MSG_ACK, payload.data(), static_cast<uint32_t>(payload.size()));
    
    if (success) {
        RB_LOG_DEBUG_FMT("MessageHandler: ACK sent for sequence {}", sequenceNumber);
    }
    
    return success;
}

bool MessageHandler::sendError(uint32_t errorCode, const std::string& errorMessage) {
    // Build error payload
    std::vector<uint8_t> payload;
    
    // Write error code (4 bytes)
    payload.push_back(static_cast<uint8_t>((errorCode >> 24) & 0xFF));
    payload.push_back(static_cast<uint8_t>((errorCode >> 16) & 0xFF));
    payload.push_back(static_cast<uint8_t>((errorCode >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(errorCode & 0xFF));
    
    // Write error message (length-prefixed string)
    if (!errorMessage.empty()) {
        uint8_t msgLength = static_cast<uint8_t>(std::min(errorMessage.length(), static_cast<size_t>(255)));
        payload.push_back(msgLength);
        payload.insert(payload.end(), errorMessage.begin(), errorMessage.begin() + msgLength);
    } else {
        payload.push_back(0);
    }
    
    bool success = sendMessage(MSG_ERROR, payload.data(), static_cast<uint32_t>(payload.size()));
    
    if (success) {
        RB_LOG_WARN_FMT("MessageHandler: Error sent (code={}, message='{}')", errorCode, errorMessage);
    }
    
    return success;
}

void MessageHandler::registerMessageCallback(MessageCallback callback) {
    m_messageCallback = callback;
}

void MessageHandler::registerStateChangeCallback(StateChangeCallback callback) {
    m_stateChangeCallback = callback;
}

MessageHandlerState MessageHandler::getState() const {
    return m_state;
}

bool MessageHandler::isHandshakeComplete() const {
    return m_state == MessageHandlerState::CONNECTED;
}

uint8_t MessageHandler::getProtocolVersion() const {
    return m_protocolVersion;
}

uint32_t MessageHandler::getCapabilities() const {
    return m_remoteCapabilities;
}

bool MessageHandler::hasCapability(uint32_t capability) const {
    return (m_remoteCapabilities & capability) != 0;
}

uint8_t MessageHandler::getRemoteClientType() const {
    return m_remoteClientType;
}

std::string MessageHandler::getRemoteClientName() const {
    return m_remoteClientName;
}

uint32_t MessageHandler::getRemoteClientVersion() const {
    return m_remoteClientVersion;
}

uint32_t MessageHandler::getCurrentSequence() const {
    return m_currentSequence;
}

void MessageHandler::incrementSequence() {
    m_currentSequence++;
}

int64_t MessageHandler::getLastFrameNumber() const {
    return m_lastFrameNumber;
}

void MessageHandler::setLastFrameNumber(int64_t frameNumber) {
    m_lastFrameNumber = frameNumber;
}

std::string MessageHandler::getStatistics() const {
    return "Messages: sent=" + std::to_string(m_messagesSent) + 
           ", received=" + std::to_string(m_messagesReceived) + 
           ", errors=" + std::to_string(m_errors);
}

void MessageHandler::handleHandshake(const uint8_t* payload, uint32_t payloadSize) {
    uint8_t protocolVersion;
    uint32_t capabilities;
    uint8_t clientType;
    uint32_t clientVersion;
    std::string clientName;
    
    if (!parseHandshakePayload(payload, payloadSize, protocolVersion, capabilities, 
                              clientType, clientVersion, clientName)) {
        RB_LOG_ERROR("MessageHandler: Failed to parse handshake payload");
        sendError(1, "Invalid handshake payload");
        return;
    }
    
    // Store remote info
    m_remoteClientType = clientType;
    m_remoteClientVersion = clientVersion;
    m_remoteClientName = clientName;
    m_remoteCapabilities = capabilities;
    
    // Negotiate protocol version
    m_protocolVersion = std::min(protocolVersion, m_protocolVersion);
    
    RB_LOG_INFO_FMT("MessageHandler: Received handshake from {} (type={}, version=0x{:08X}, caps=0x{:08X})",
                    clientName, clientType, clientVersion, capabilities);
    RB_LOG_INFO_FMT("MessageHandler: Negotiated protocol version: {}", m_protocolVersion);
    
    // If this is the mod, send our handshake back
    if (m_localClientType == CLIENT_TYPE_RENDERER && clientType == CLIENT_TYPE_MOD) {
        sendHandshake();
    }
    
    // Transition to connected state
    changeState(MessageHandlerState::CONNECTED);
    
    // Send ACK
    sendAcknowledgment(m_currentSequence++);
}

void MessageHandler::handleHeartbeat(const uint8_t* payload, uint32_t payloadSize) {
    if (payloadSize < 16) {
        RB_LOG_WARN_FMT("MessageHandler: Invalid heartbeat payload size: {} bytes", payloadSize);
        return;
    }
    
    // Parse heartbeat
    uint32_t sequenceNumber = static_cast<uint32_t>(payload[0] << 24 | 
                                                     payload[1] << 16 | 
                                                     payload[2] << 8 | 
                                                     payload[3]);
    
    int64_t lastFrameProcessed = static_cast<int64_t>(
        static_cast<uint64_t>(payload[4]) << 56 |
        static_cast<uint64_t>(payload[5]) << 48 |
        static_cast<uint64_t>(payload[6]) << 40 |
        static_cast<uint64_t>(payload[7]) << 32 |
        static_cast<uint64_t>(payload[8]) << 24 |
        static_cast<uint64_t>(payload[9]) << 16 |
        static_cast<uint64_t>(payload[10]) << 8 |
        static_cast<uint64_t>(payload[11])
    );
    
    uint32_t timeBits = static_cast<uint32_t>(payload[12] << 24 | 
                                               payload[13] << 16 | 
                                               payload[14] << 8 | 
                                               payload[15]);
    float avgProcessingTimeMs;
    std::memcpy(&avgProcessingTimeMs, &timeBits, sizeof(float));
    
    // Update last frame number
    setLastFrameNumber(lastFrameProcessed);
    
    RB_LOG_DEBUG_FMT("MessageHandler: Heartbeat received (seq={}, lastFrame={}, avgTime={:.2f}ms)",
                     sequenceNumber, lastFrameProcessed, avgProcessingTimeMs);
    
    // Send ACK
    sendAcknowledgment(sequenceNumber);
}

void MessageHandler::handleAcknowledgment(const uint8_t* payload, uint32_t payloadSize) {
    if (payloadSize < 4) {
        RB_LOG_WARN_FMT("MessageHandler: Invalid ACK payload size: {} bytes", payloadSize);
        return;
    }
    
    uint32_t sequenceNumber = static_cast<uint32_t>(payload[0] << 24 | 
                                                     payload[1] << 16 | 
                                                     payload[2] << 8 | 
                                                     payload[3]);
    
    RB_LOG_DEBUG_FMT("MessageHandler: ACK received for sequence {}", sequenceNumber);
}

void MessageHandler::handleError(const uint8_t* payload, uint32_t payloadSize) {
    if (payloadSize < 4) {
        RB_LOG_WARN_FMT("MessageHandler: Invalid error payload size: {} bytes", payloadSize);
        return;
    }
    
    // Parse error code
    uint32_t errorCode = static_cast<uint32_t>(payload[0] << 24 | 
                                               payload[1] << 16 | 
                                               payload[2] << 8 | 
                                               payload[3]);
    
    // Parse error message
    std::string errorMessage;
    if (payloadSize > 4) {
        uint8_t msgLength = payload[4];
        if (msgLength > 0 && payloadSize >= 5 + msgLength) {
            errorMessage = std::string(reinterpret_cast<const char*>(payload + 5), msgLength);
        }
    }
    
    RB_LOG_WARN_FMT("MessageHandler: Error received (code={}, message='{}')", errorCode, errorMessage);
    
    // Disconnect on error
    changeState(MessageHandlerState::ERROR);
}

void MessageHandler::handleFrameStart(const uint8_t* payload, uint32_t payloadSize) {
    // Parse frame number and timestamp
    if (payloadSize >= 16) {
        int64_t frameNumber = static_cast<int64_t>(
            static_cast<uint64_t>(payload[0]) << 56 |
            static_cast<uint64_t>(payload[1]) << 48 |
            static_cast<uint64_t>(payload[2]) << 40 |
            static_cast<uint64_t>(payload[3]) << 32 |
            static_cast<uint64_t>(payload[4]) << 24 |
            static_cast<uint64_t>(payload[5]) << 16 |
            static_cast<uint64_t>(payload[6]) << 8 |
            static_cast<uint64_t>(payload[7])
        );
        
        int64_t timestamp = static_cast<int64_t>(
            static_cast<uint64_t>(payload[8]) << 56 |
            static_cast<uint64_t>(payload[9]) << 48 |
            static_cast<uint64_t>(payload[10]) << 40 |
            static_cast<uint64_t>(payload[11]) << 32 |
            static_cast<uint64_t>(payload[12]) << 24 |
            static_cast<uint64_t>(payload[13]) << 16 |
            static_cast<uint64_t>(payload[14]) << 8 |
            static_cast<uint64_t>(payload[15])
        );
        
        setLastFrameNumber(frameNumber);
        RB_LOG_DEBUG_FMT("MessageHandler: Frame start received (frame={}, timestamp={})", frameNumber, timestamp);
    }
    
    // Forward to callback
    if (m_messageCallback) {
        m_messageCallback(MSG_FRAME_START, payload, payloadSize);
    }
}

void MessageHandler::handleFrameEnd(const uint8_t* payload, uint32_t payloadSize) {
    RB_LOG_DEBUG("MessageHandler: Frame end received");
    
    // Forward to callback
    if (m_messageCallback) {
        m_messageCallback(MSG_FRAME_END, payload, payloadSize);
    }
}

void MessageHandler::handleCamera(const uint8_t* payload, uint32_t payloadSize) {
    RB_LOG_DEBUG_FMT("MessageHandler: Camera message received ({} bytes)", payloadSize);
    
    // Forward to callback
    if (m_messageCallback) {
        m_messageCallback(MSG_CAMERA, payload, payloadSize);
    }
}

void MessageHandler::handleChunk(const uint8_t* payload, uint32_t payloadSize) {
    RB_LOG_DEBUG_FMT("MessageHandler: Chunk message received ({} bytes)", payloadSize);
    
    // Forward to callback
    if (m_messageCallback) {
        m_messageCallback(MSG_CHUNK, payload, payloadSize);
    }
}

void MessageHandler::handleEntity(const uint8_t* payload, uint32_t payloadSize) {
    RB_LOG_DEBUG_FMT("MessageHandler: Entity message received ({} bytes)", payloadSize);
    
    // Forward to callback
    if (m_messageCallback) {
        m_messageCallback(MSG_ENTITY, payload, payloadSize);
    }
}

void MessageHandler::handleParticle(const uint8_t* payload, uint32_t payloadSize) {
    RB_LOG_DEBUG_FMT("MessageHandler: Particle message received ({} bytes)", payloadSize);
    
    // Forward to callback
    if (m_messageCallback) {
        m_messageCallback(MSG_PARTICLE, payload, payloadSize);
    }
}

void MessageHandler::handleWeather(const uint8_t* payload, uint32_t payloadSize) {
    RB_LOG_DEBUG_FMT("MessageHandler: Weather message received ({} bytes)", payloadSize);
    
    // Forward to callback
    if (m_messageCallback) {
        m_messageCallback(MSG_WEATHER, payload, payloadSize);
    }
}

void MessageHandler::handleGui(const uint8_t* payload, uint32_t payloadSize) {
    RB_LOG_DEBUG_FMT("MessageHandler: GUI message received ({} bytes)", payloadSize);
    
    // Forward to callback
    if (m_messageCallback) {
        m_messageCallback(MSG_GUI, payload, payloadSize);
    }
}

void MessageHandler::handleEcho(const uint8_t* payload, uint32_t payloadSize) {
    RB_LOG_DEBUG_FMT("MessageHandler: Echo message received ({} bytes)", payloadSize);
    
    // Send echo back
    sendMessage(MSG_ECHO, payload, payloadSize);
}

void MessageHandler::handleControl(const uint8_t* payload, uint32_t payloadSize) {
    RB_LOG_DEBUG_FMT("MessageHandler: Control message received ({} bytes)", payloadSize);
    
    // Forward to callback
    if (m_messageCallback) {
        m_messageCallback(MSG_CONTROL, payload, payloadSize);
    }
}

void MessageHandler::changeState(MessageHandlerState newState) {
    if (m_state == newState) {
        return;
    }
    
    MessageHandlerState oldState = m_state;
    m_state = newState;
    
    RB_LOG_INFO_FMT("MessageHandler: State changed from {} to {}", 
                    static_cast<int>(oldState), static_cast<int>(newState));
    
    if (m_stateChangeCallback) {
        m_stateChangeCallback(oldState, newState);
    }
}

bool MessageHandler::parseHandshakePayload(
    const uint8_t* payload, uint32_t payloadSize,
    uint8_t& protocolVersion, uint32_t& capabilities,
    uint8_t& clientType, uint32_t& clientVersion,
    std::string& clientName
) {
    if (payloadSize < 14) { // Minimum: 1 + 4 + 4 + 4 + 1 = 14 bytes
        return false;
    }
    
    uint32_t offset = 0;
    
    // Protocol version (1 byte)
    protocolVersion = payload[offset++];
    
    // Capabilities (4 bytes)
    capabilities = static_cast<uint32_t>(payload[offset] << 24 | 
                                         payload[offset + 1] << 16 | 
                                         payload[offset + 2] << 8 | 
                                         payload[offset + 3]);
    offset += 4;
    
    // Client type (4 bytes)
    clientType = payload[offset++];
    offset += 3; // Skip padding
    
    // Client version (4 bytes)
    clientVersion = static_cast<uint32_t>(payload[offset] << 24 | 
                                          payload[offset + 1] << 16 | 
                                          payload[offset + 2] << 8 | 
                                          payload[offset + 3]);
    offset += 4;
    
    // Client name (length-prefixed)
    if (offset < payloadSize) {
        uint8_t nameLength = payload[offset++];
        if (nameLength > 0 && offset + nameLength <= payloadSize) {
            clientName = std::string(reinterpret_cast<const char*>(payload + offset), nameLength);
            offset += nameLength;
        }
    }
    
    return true;
}

uint32_t MessageHandler::buildHandshakePayload(
    uint8_t protocolVersion, uint32_t capabilities,
    uint8_t clientType, uint32_t clientVersion,
    const std::string& clientName, std::vector<uint8_t>& output
) {
    output.clear();
    
    // Protocol version (1 byte)
    output.push_back(protocolVersion);
    
    // Capabilities (4 bytes)
    output.push_back(static_cast<uint8_t>((capabilities >> 24) & 0xFF));
    output.push_back(static_cast<uint8_t>((capabilities >> 16) & 0xFF));
    output.push_back(static_cast<uint8_t>((capabilities >> 8) & 0xFF));
    output.push_back(static_cast<uint8_t>(capabilities & 0xFF));
    
    // Client type (4 bytes)
    output.push_back(clientType);
    output.push_back(0); // padding
    output.push_back(0); // padding
    output.push_back(0); // padding
    
    // Client version (4 bytes)
    output.push_back(static_cast<uint8_t>((clientVersion >> 24) & 0xFF));
    output.push_back(static_cast<uint8_t>((clientVersion >> 16) & 0xFF));
    output.push_back(static_cast<uint8_t>((clientVersion >> 8) & 0xFF));
    output.push_back(static_cast<uint8_t>(clientVersion & 0xFF));
    
    // Client name (length-prefixed)
    uint8_t nameLength = static_cast<uint8_t>(std::min(clientName.length(), static_cast<size_t>(255)));
    output.push_back(nameLength);
    if (nameLength > 0) {
        output.insert(output.end(), clientName.begin(), clientName.begin() + nameLength);
    }
    
    return static_cast<uint32_t>(output.size());
}

} // namespace renderbridge
