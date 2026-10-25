package renderbridge.protocol.messages;

import renderbridge.protocol.Message;
import renderbridge.protocol.MessageType;

import java.nio.ByteBuffer;

/**
 * Message containing weather data for rendering.
 * Includes weather type, intensity, and other weather-related information.
 */
public class WeatherMessage extends Message {

    // Weather types
    public static final int WEATHER_CLEAR = 0;
    public static final int WEATHER_RAIN = 1;
    public static final int WEATHER_SNOW = 2;
    public static final int WEATHER_THUNDERSTORM = 3;

    // Weather flags
    public static final int FLAG_RAINING = 0x00000001;
    public static final int FLAG_THUNDERING = 0x00000002;
    public static final int FLAG_SNOWING = 0x00000004;

    private byte weatherType;
    private float intensity;
    private float transitionProgress;
    private int flags;
    private long durationTicks;

    /**
     * Create a new weather message.
     */
    public WeatherMessage() {
        super(MessageType.WEATHER);
    }

    /**
     * Create a weather message with the specified version.
     *
     * @param version the protocol version
     */
    public WeatherMessage(int version) {
        super(MessageType.WEATHER);
    }

    /**
     * Create a weather message from a header and payload buffer.
     *
     * @param version the protocol version
     * @param payload the payload buffer
     */
    public WeatherMessage(int version, ByteBuffer payload) {
        super(MessageType.WEATHER);
        readPayload(payload);
    }

    @Override
    protected void readPayload(ByteBuffer buffer) {
        weatherType = buffer.get();
        intensity = buffer.getFloat();
        transitionProgress = buffer.getFloat();
        flags = buffer.getInt();
        durationTicks = buffer.getLong();
    }

    @Override
    protected void writePayload(ByteBuffer buffer) {
        buffer.put(weatherType);
        buffer.putFloat(intensity);
        buffer.putFloat(transitionProgress);
        buffer.putInt(flags);
        buffer.putLong(durationTicks);
    }

    @Override
    public int getPayloadSize() {
        // weatherType(1) + intensity(4) + transitionProgress(4) + flags(4) + durationTicks(8)
        return 1 + 4 + 4 + 4 + 8;
    }

    // Getters and setters

    public byte getWeatherType() {
        return weatherType;
    }

    public void setWeatherType(byte weatherType) {
        this.weatherType = weatherType;
    }

    public float getIntensity() {
        return intensity;
    }

    public void setIntensity(float intensity) {
        this.intensity = intensity;
    }

    public float getTransitionProgress() {
        return transitionProgress;
    }

    public void setTransitionProgress(float transitionProgress) {
        this.transitionProgress = transitionProgress;
    }

    public int getFlags() {
        return flags;
    }

    public void setFlags(int flags) {
        this.flags = flags;
    }

    public void addFlag(int flag) {
        this.flags |= flag;
    }

    public boolean hasFlag(int flag) {
        return (this.flags & flag) != 0;
    }

    public long getDurationTicks() {
        return durationTicks;
    }

    public void setDurationTicks(long durationTicks) {
        this.durationTicks = durationTicks;
    }

    // Convenience methods for checking specific flags

    public boolean isRaining() {
        return hasFlag(FLAG_RAINING);
    }

    public boolean isThundering() {
        return hasFlag(FLAG_THUNDERING);
    }

    public boolean isSnowing() {
        return hasFlag(FLAG_SNOWING);
    }

    // Convenience methods for weather type

    public boolean isClear() {
        return weatherType == WEATHER_CLEAR;
    }

    public boolean isRain() {
        return weatherType == WEATHER_RAIN || weatherType == WEATHER_THUNDERSTORM;
    }

    public boolean isSnow() {
        return weatherType == WEATHER_SNOW;
    }

    public boolean isThunderstorm() {
        return weatherType == WEATHER_THUNDERSTORM;
    }

    @Override
    public String toString() {
        return String.format(
                "WeatherMessage{weatherType=%d, intensity=%.2f, transitionProgress=%.2f, flags=0x%08X, durationTicks=%d}",
                weatherType, intensity, transitionProgress, flags, durationTicks
        );
    }
}
