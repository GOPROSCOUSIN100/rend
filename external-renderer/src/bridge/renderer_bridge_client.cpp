#include "renderer_bridge_client.hpp"
#include "bridge_connection.hpp"
#include "message_handler.hpp"
#include "utils/logger.hpp"

#include <algorithm>

namespace renderbridge {

RendererBridgeClient::RendererBridgeClient()
    : m_clientName("RenderBridge-Renderer"),
      m_clientVersion(0x00020000),
      m_protocolVersion(1),
      m_capabilities(0),
      m_currentSequence(0),
      m_lastFrameNumber(-1),
      m_preferSharedMemory(true),
      m_connected(false),
      m_currentTransport("none") {
}

RendererBridgeClient::~RendererBridgeClient() {
    disconnect();
}

bool RendererBridgeClient::initialize(const std::string& clientName, uint32_t clientVersion) {
    m_clientName = clientName;
    m_clientVersion = clientVersion;
    
    // Set default capabilities
    m_capabilities = 0x00000001 | 0x00000002; // SHARED_MEMORY | SOCKET
    
    RB_LOG_INFO_FMT("RendererBridgeClient: Initialized as '{}' version 0x{:08X}", 
                    m_clientName, m_clientVersion);
    
    return true;
}

bool RendererBridgeClient::connectSharedMemory() {
    RB_LOG_INFO("RendererBridgeClient: Connecting via shared memory...");
    
    // Create shared memory connection
    m_connection = std::make_unique<SharedMemoryConnection>();
    m_currentTransport = "shared_memory";
    
    if (m_connection->connect()) {
        // Create message handler
        m_messageHandler = std::make_unique<MessageHandler>(m_connection);
        m_messageHandler->initialize(
            1, // CLIENT_TYPE_RENDERER
            m_clientName,
            m_clientVersion,
            m_capabilities
        );
        
        // Set up callbacks
        m_messageHandler->registerMessageCallback([this](uint16_t messageType, const uint8_t* payload, uint32_t payloadSize) {
            handleMessage(messageType, payload, payloadSize);
        });
        
        m_messageHandler->registerStateChangeCallback([this](MessageHandlerState oldState, MessageHandlerState newState) {
            if (newState == MessageHandlerState::CONNECTED) {
                handleConnectionStatus(true, "");
            } else if (newState == MessageHandlerState::DISCONNECTED || newState == MessageHandlerState::ERROR) {
                handleConnectionStatus(false, "State changed to " + std::to_string(static_cast<int>(newState)));
            }
        });
        
        m_connected = true;
        RB_LOG_INFO("RendererBridgeClient: Connected via shared memory");
        return true;
    }
    
    m_connected = false;
    RB_LOG_ERROR_FMT("RendererBridgeClient: Failed to connect via shared memory: {}", 
                     m_connection->getLastError());
    return false;
}

bool RendererBridgeClient::connectSocket(const std::string& host, uint16_t port) {
    RB_LOG_INFO_FMT("RendererBridgeClient: Connecting via socket to {}:{)...", host, port);
    
    // Create socket connection
    m_connection = std::make_unique<SocketConnection>(host, port);
    m_currentTransport = "socket";
    
    if (m_connection->connect()) {
        // Create message handler
        m_messageHandler = std::make_unique<MessageHandler>(m_connection);
        m_messageHandler->initialize(
            1, // CLIENT_TYPE_RENDERER
            m_clientName,
            m_clientVersion,
            m_capabilities
        );
        
        // Set up callbacks
        m_messageHandler->registerMessageCallback([this](uint16_t messageType, const uint8_t* payload, uint32_t payloadSize) {
            handleMessage(messageType, payload, payloadSize);
        });
        
        m_messageHandler->registerStateChangeCallback([this](MessageHandlerState oldState, MessageHandlerState newState) {
            if (newState == MessageHandlerState::CONNECTED) {
                handleConnectionStatus(true, "");
            } else if (newState == MessageHandlerState::DISCONNECTED || newState == MessageHandlerState::ERROR) {
                handleConnectionStatus(false, "State changed to " + std::to_string(static_cast<int>(newState)));
            }
        });
        
        m_connected = true;
        RB_LOG_INFO("RendererBridgeClient: Connected via socket");
        return true;
    }
    
    m_connected = false;
    RB_LOG_ERROR_FMT("RendererBridgeClient: Failed to connect via socket: {}", 
                     m_connection->getLastError());
    return false;
}

void RendererBridgeClient::disconnect() {
    if (m_connected) {
        if (m_messageHandler) {
            m_messageHandler.reset();
        }
        if (m_connection) {
            m_connection->disconnect();
            m_connection.reset();
        }
        m_connected = false;
        m_currentTransport = "none";
        RB_LOG_INFO("RendererBridgeClient: Disconnected");
    }
}

bool RendererBridgeClient::isConnected() const {
    return m_connected && m_connection && m_connection->isConnected();
}

uint32_t RendererBridgeClient::pollMessages() {
    if (!m_connected || !m_messageHandler) {
        return 0;
    }
    
    return m_messageHandler->processMessages();
}

bool RendererBridgeClient::sendMessage(uint16_t messageType, const uint8_t* payload, uint32_t payloadSize) {
    if (!m_connected || !m_messageHandler) {
        return false;
    }
    
    return m_messageHandler->sendMessage(messageType, payload, payloadSize);
}

bool RendererBridgeClient::sendHandshake() {
    if (!m_connected || !m_messageHandler) {
        return false;
    }
    
    return m_messageHandler->sendHandshake();
}

bool RendererBridgeClient::sendHeartbeat(uint32_t sequenceNumber, int64_t lastFrameProcessed, float avgProcessingTimeMs) {
    if (!m_connected || !m_messageHandler) {
        return false;
    }
    
    return m_messageHandler->sendHeartbeat(sequenceNumber, lastFrameProcessed, avgProcessingTimeMs);
}

bool RendererBridgeClient::sendAcknowledgment(uint32_t sequenceNumber) {
    if (!m_connected || !m_messageHandler) {
        return false;
    }
    
    return m_messageHandler->sendAcknowledgment(sequenceNumber);
}

bool RendererBridgeClient::sendError(uint32_t errorCode, const std::string& errorMessage) {
    if (!m_connected || !m_messageHandler) {
        return false;
    }
    
    return m_messageHandler->sendError(errorCode, errorMessage);
}

void RendererBridgeClient::registerMessageCallback(MessageCallback callback) {
    m_messageCallback = callback;
}

void RendererBridgeClient::registerConnectionStatusCallback(ConnectionStatusCallback callback) {
    m_connectionStatusCallback = callback;
}

uint8_t RendererBridgeClient::getProtocolVersion() const {
    if (m_messageHandler) {
        return m_messageHandler->getProtocolVersion();
    }
    return m_protocolVersion;
}

uint32_t RendererBridgeClient::getCapabilities() const {
    if (m_messageHandler) {
        return m_messageHandler->getCapabilities();
    }
    return m_capabilities;
}

bool RendererBridgeClient::hasCapability(uint32_t capability) const {
    if (m_messageHandler) {
        return m_messageHandler->hasCapability(capability);
    }
    return (m_capabilities & capability) != 0;
}

uint32_t RendererBridgeClient::getCurrentSequence() const {
    if (m_messageHandler) {
        return m_messageHandler->getCurrentSequence();
    }
    return m_currentSequence;
}

int64_t RendererBridgeClient::getLastFrameNumber() const {
    if (m_messageHandler) {
        return m_messageHandler->getLastFrameNumber();
    }
    return m_lastFrameNumber;
}

void RendererBridgeClient::setPreferSharedMemory(bool preferSharedMemory) {
    m_preferSharedMemory = preferSharedMemory;
}

bool RendererBridgeClient::isSharedMemoryAvailable() const {
    // For now, assume shared memory is available
    // In a real implementation, we would check platform support
    return true;
}

bool RendererBridgeClient::isSocketAvailable() const {
    // For now, assume socket is available
    return true;
}

std::string RendererBridgeClient::getCurrentTransport() const {
    return m_currentTransport;
}

void RendererBridgeClient::handleMessage(uint16_t messageType, const uint8_t* payload, uint32_t payloadSize) {
    // Forward to user callback
    if (m_messageCallback) {
        m_messageCallback(messageType, payload, payloadSize);
    }
}

void RendererBridgeClient::handleConnectionStatus(bool connected, const std::string& errorMessage) {
    m_connected = connected;
    
    // Forward to user callback
    if (m_connectionStatusCallback) {
        m_connectionStatusCallback(connected, errorMessage);
    }
    
    if (connected) {
        RB_LOG_INFO("RendererBridgeClient: Connection established");
    } else {
        RB_LOG_WARN_FMT("RendererBridgeClient: Connection lost: {}", errorMessage);
    }
}

} // namespace renderbridge
