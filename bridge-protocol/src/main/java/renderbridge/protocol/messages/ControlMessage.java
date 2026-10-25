package renderbridge.protocol.messages;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageType;

import java.nio.ByteBuffer;

/**
 * Control message for sending control commands between client and renderer.
 * Used for synchronization, configuration, and other control purposes.
 */
public class ControlMessage extends Message {

    // Control command types
    public static final int CMD_PAUSE = 0;
    public static final int CMD_RESUME = 1;
    public static final int CMD_RESET = 2;
    public static final int CMD_SET_QUALITY = 3;
    public static final int CMD_SET_RENDER_MODE = 4;
    public static final int CMD_SET_FOV = 5;
    public static final int CMD_REQUEST_SYNC = 6;
    public static final int CMD_PING = 7;
    public static final int CMD_PONG = 8;

    // Quality levels
    public static final int QUALITY_LOW = 0;
    public static final int QUALITY_MEDIUM = 1;
    public static final int QUALITY_HIGH = 2;
    public static final int QUALITY_ULTRA = 3;

    // Render modes
    public static final int RENDER_MODE_NORMAL = 0;
    public static final int RENDER_MODE_WIREFRAME = 1;
    public static final int RENDER_MODE_POINTS = 2;
    public static final int RENDER_MODE_MINIMAL = 3;

    private byte command;
    private int parameter1;
    private int parameter2;
    private float parameterFloat;

    /**
     * Create a new control message.
     */
    public ControlMessage() {
        super(MessageType.CONTROL);
    }

    /**
     * Create a control message with the specified version.
     *
     * @param version the protocol version
     */
    public ControlMessage(int version) {
        super(MessageType.CONTROL);
    }

    /**
     * Create a control message from a header and payload buffer.
     *
     * @param version the protocol version
     * @param payload the payload buffer
     */
    public ControlMessage(int version, ByteBuffer payload) {
        super(MessageType.CONTROL);
        readPayload(payload);
    }

    /**
     * Create a control message with a command.
     *
     * @param command the control command
     */
    public ControlMessage(byte command) {
        super(MessageType.CONTROL);
        this.command = command;
    }

    @Override
    protected void readPayload(ByteBuffer buffer) {
        command = buffer.get();
        parameter1 = buffer.getInt();
        parameter2 = buffer.getInt();
        parameterFloat = buffer.getFloat();
    }

    @Override
    protected void writePayload(ByteBuffer buffer) {
        buffer.put(command);
        buffer.putInt(parameter1);
        buffer.putInt(parameter2);
        buffer.putFloat(parameterFloat);
    }

    @Override
    public int getPayloadSize() {
        // command(1) + parameter1(4) + parameter2(4) + parameterFloat(4)
        return 1 + 4 + 4 + 4;
    }

    // Getters and setters

    public byte getCommand() {
        return command;
    }

    public void setCommand(byte command) {
        this.command = command;
    }

    public int getParameter1() {
        return parameter1;
    }

    public void setParameter1(int parameter1) {
        this.parameter1 = parameter1;
    }

    public int getParameter2() {
        return parameter2;
    }

    public void setParameter2(int parameter2) {
        this.parameter2 = parameter2;
    }

    public float getParameterFloat() {
        return parameterFloat;
    }

    public void setParameterFloat(float parameterFloat) {
        this.parameterFloat = parameterFloat;
    }

    // Convenience methods for checking commands

    public boolean isPause() {
        return command == CMD_PAUSE;
    }

    public boolean isResume() {
        return command == CMD_RESUME;
    }

    public boolean isReset() {
        return command == CMD_RESET;
    }

    public boolean isSetQuality() {
        return command == CMD_SET_QUALITY;
    }

    public boolean isSetRenderMode() {
        return command == CMD_SET_RENDER_MODE;
    }

    public boolean isSetFov() {
        return command == CMD_SET_FOV;
    }

    public boolean isRequestSync() {
        return command == CMD_REQUEST_SYNC;
    }

    public boolean isPing() {
        return command == CMD_PING;
    }

    public boolean isPong() {
        return command == CMD_PONG;
    }

    @Override
    public String toString() {
        return String.format(
                "ControlMessage{command=%d, parameter1=%d, parameter2=%d, parameterFloat=%.2f}",
                command, parameter1, parameter2, parameterFloat
        );
    }
}
