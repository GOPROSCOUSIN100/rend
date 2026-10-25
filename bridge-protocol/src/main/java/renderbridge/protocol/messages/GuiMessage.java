package renderbridge.protocol.messages;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageType;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

/**
 * Message containing GUI/HUD data for rendering.
 * Includes health, armor, hunger, experience, and other HUD elements.
 */
public class GuiMessage extends Message {

    // Maximum text length
    private static final int MAX_TEXT_LENGTH = 255;

    // GUI element types
    public static final int ELEMENT_HEALTH = 0;
    public static final int ELEMENT_ARMOR = 1;
    public static final int ELEMENT_HUNGER = 2;
    public static final int ELEMENT_EXPERIENCE = 3;
    public static final int ELEMENT_AIR = 4;
    public static final int ELEMENT_HOTBAR = 5;
    public static final int ELEMENT_CROSSHAIR = 6;
    public static final int ELEMENT_SCOREBOARD = 7;
    public static final int ELEMENT_CHAT = 8;
    public static final int ELEMENT_DEBUG = 9;

    private byte elementType;
    private float value;
    private float maxValue;
    private byte flags;
    
    // Text data for elements like scoreboard, chat, etc.
    private String text;

    /**
     * Create a new GUI message.
     */
    public GuiMessage() {
        super(MessageType.GUI);
    }

    /**
     * Create a GUI message with the specified version.
     *
     * @param version the protocol version
     */
    public GuiMessage(int version) {
        super(MessageType.GUI);
    }

    /**
     * Create a GUI message from a header and payload buffer.
     *
     * @param version the protocol version
     * @param payload the payload buffer
     */
    public GuiMessage(int version, ByteBuffer payload) {
        super(MessageType.GUI);
        readPayload(payload);
    }

    @Override
    protected void readPayload(ByteBuffer buffer) {
        elementType = buffer.get();
        value = buffer.getFloat();
        maxValue = buffer.getFloat();
        flags = buffer.get();
        
        // Read text (length-prefixed string)
        byte textLength = buffer.get();
        byte[] textBytes = new byte[textLength];
        buffer.get(textBytes);
        text = new String(textBytes, StandardCharsets.UTF_8);
    }

    @Override
    protected void writePayload(ByteBuffer buffer) {
        buffer.put(elementType);
        buffer.putFloat(value);
        buffer.putFloat(maxValue);
        buffer.put(flags);
        
        // Write text (length-prefixed)
        byte[] textBytes = text != null ? text.getBytes(StandardCharsets.UTF_8) : new byte[0];
        byte textLength = (byte) Math.min(textBytes.length, MAX_TEXT_LENGTH);
        buffer.put(textLength);
        buffer.put(textBytes, 0, textLength);
    }

    @Override
    public int getPayloadSize() {
        // elementType(1) + value(4) + maxValue(4) + flags(1) + textLength(1) + text.length
        int textLength = text != null ? Math.min(text.length(), MAX_TEXT_LENGTH) : 0;
        return 1 + 4 + 4 + 1 + 1 + textLength;
    }

    // Getters and setters

    public byte getElementType() {
        return elementType;
    }

    public void setElementType(byte elementType) {
        this.elementType = elementType;
    }

    public float getValue() {
        return value;
    }

    public void setValue(float value) {
        this.value = value;
    }

    public float getMaxValue() {
        return maxValue;
    }

    public void setMaxValue(float maxValue) {
        this.maxValue = maxValue;
    }

    public byte getFlags() {
        return flags;
    }

    public void setFlags(byte flags) {
        this.flags = flags;
    }

    public String getText() {
        return text;
    }

    public void setText(String text) {
        this.text = text;
    }

    // Convenience methods for checking element types

    public boolean isHealth() {
        return elementType == ELEMENT_HEALTH;
    }

    public boolean isArmor() {
        return elementType == ELEMENT_ARMOR;
    }

    public boolean isHunger() {
        return elementType == ELEMENT_HUNGER;
    }

    public boolean isExperience() {
        return elementType == ELEMENT_EXPERIENCE;
    }

    public boolean isAir() {
        return elementType == ELEMENT_AIR;
    }

    public boolean isHotbar() {
        return elementType == ELEMENT_HOTBAR;
    }

    public boolean isCrosshair() {
        return elementType == ELEMENT_CROSSHAIR;
    }

    public boolean isScoreboard() {
        return elementType == ELEMENT_SCOREBOARD;
    }

    public boolean isChat() {
        return elementType == ELEMENT_CHAT;
    }

    public boolean isDebug() {
        return elementType == ELEMENT_DEBUG;
    }

    /**
     * Get the percentage value (value / maxValue * 100).
     *
     * @return the percentage value
     */
    public float getPercentage() {
        return maxValue > 0 ? (value / maxValue) * 100 : 0;
    }

    @Override
    public String toString() {
        return String.format(
                "GuiMessage{elementType=%d, value=%.2f, maxValue=%.2f, flags=0x%02X, text='%s'}",
                elementType, value, maxValue, flags, text
        );
    }
}
