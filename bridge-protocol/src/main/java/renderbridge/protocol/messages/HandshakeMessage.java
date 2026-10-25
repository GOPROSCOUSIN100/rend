package renderbridge.protocol.messages;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageHeader;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

/**
 * Handshake message for protocol version negotiation and capability exchange.
 * Used during initial connection setup.
 */
public class HandshakeMessage extends Message {

    // Capability flags
    public static final int CAPABILITY_SHARED_MEMORY = 0x00000001;
    public static final int CAPABILITY_SOCKET = 0x00000002;
    public static final int CAPABILITY_COMPRESSION = 0x00000004;
    public static final int CAPABILITY_ENCRYPTION = 0x00000008;

    // Client types
    public static final int CLIENT_TYPE_MOD = 0;
    public static final int CLIENT_TYPE_RENDERER = 1;

    // Maximum client name length
    private static final int MAX_CLIENT_NAME_LENGTH = 64;

    private byte protocolVersion;
    private int capabilities;
    private int clientType;
    private int clientVersion;
    private String clientName;

    /**
     * Create a new handshake message.
     */
    public HandshakeMessage() {
        super(MessageType.HANDSHAKE);
    }

    /**
     * Create a handshake message with the specified version.
     *
     * @param version the protocol version
     */
    public HandshakeMessage(int version) {
        super(MessageType.HANDSHAKE);
        this.protocolVersion = (byte) version;
    }

    /**
     * Create a handshake message from a header and payload buffer.
     *
     * @param version the protocol version
     * @param payload the payload buffer
     */
    public HandshakeMessage(int version, ByteBuffer payload) {
        super(MessageType.HANDSHAKE);
        this.protocolVersion = (byte) version;
        readPayload(payload);
    }

    @Override
    protected void readPayload(ByteBuffer buffer) {
        protocolVersion = buffer.get();
        capabilities = buffer.getInt();
        clientType = buffer.getInt();
        clientVersion = buffer.getInt();
        
        // Read client name (length-prefixed string)
        byte nameLength = buffer.get();
        byte[] nameBytes = new byte[nameLength];
        buffer.get(nameBytes);
        clientName = new String(nameBytes, StandardCharsets.UTF_8);
    }

    @Override
    protected void writePayload(ByteBuffer buffer) {
        buffer.put(protocolVersion);
        buffer.putInt(capabilities);
        buffer.putInt(clientType);
        buffer.putInt(clientVersion);
        
        // Write client name (length-prefixed)
        byte[] nameBytes = clientName != null ? clientName.getBytes(StandardCharsets.UTF_8) : new byte[0];
        byte nameLength = (byte) Math.min(nameBytes.length, MAX_CLIENT_NAME_LENGTH);
        buffer.put(nameLength);
        buffer.put(nameBytes, 0, nameLength);
    }

    @Override
    public int getPayloadSize() {
        // protocolVersion(1) + capabilities(4) + clientType(4) + clientVersion(4) + nameLength(1) + name(up to 64)
        int nameLength = clientName != null ? Math.min(clientName.length(), MAX_CLIENT_NAME_LENGTH) : 0;
        return 1 + 4 + 4 + 4 + 1 + nameLength;
    }

    // Getters and setters

    public byte getProtocolVersion() {
        return protocolVersion;
    }

    public void setProtocolVersion(byte protocolVersion) {
        this.protocolVersion = protocolVersion;
    }

    public int getCapabilities() {
        return capabilities;
    }

    public void setCapabilities(int capabilities) {
        this.capabilities = capabilities;
    }

    public void addCapability(int capability) {
        this.capabilities |= capability;
    }

    public boolean hasCapability(int capability) {
        return (this.capabilities & capability) != 0;
    }

    public int getClientType() {
        return clientType;
    }

    public void setClientType(int clientType) {
        this.clientType = clientType;
    }

    public int getClientVersion() {
        return clientVersion;
    }

    public void setClientVersion(int clientVersion) {
        this.clientVersion = clientVersion;
    }

    public String getClientName() {
        return clientName;
    }

    public void setClientName(String clientName) {
        this.clientName = clientName;
    }

    /**
     * Check if this handshake is from a mod client.
     *
     * @return true if the client is a mod
     */
    public boolean isModClient() {
        return clientType == CLIENT_TYPE_MOD;
    }

    /**
     * Check if this handshake is from a renderer client.
     *
     * @return true if the client is a renderer
     */
    public boolean isRendererClient() {
        return clientType == CLIENT_TYPE_RENDERER;
    }

    @Override
    public String toString() {
        return String.format(
                "HandshakeMessage{protocolVersion=%d, capabilities=0x%08X, clientType=%d, clientVersion=0x%08X, clientName='%s'}",
                protocolVersion, capabilities, clientType, clientVersion, clientName
        );
    }
}
