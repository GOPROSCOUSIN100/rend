package renderbridge.mod.extractors;

import net.minecraft.world.World;
import net.minecraft.world.storage.WorldInfo;

import renderbridge.protocol.messages.WeatherMessage;

/**
 * Extracts weather data from the World.
 * Converts Minecraft weather data to WeatherMessage for the bridge protocol.
 */
public class WeatherExtractor {

    private final World world;
    
    // Cached values
    private float lastIntensity = 0;
    private float lastTransitionProgress = 0;
    private long lastUpdateTime = 0;

    /**
     * Create a new weather extractor.
     *
     * @param world the World instance
     */
    public WeatherExtractor(World world) {
        this.world = world;
    }

    /**
     * Update the weather extractor.
     */
    public void update() {
        if (world == null) {
            return;
        }
        
        long currentTime = System.currentTimeMillis();
        if (currentTime - lastUpdateTime < 100) { // Update every 100ms
            return;
        }
        
        lastUpdateTime = currentTime;
    }

    /**
     * Extract weather data and create a WeatherMessage.
     *
     * @return a WeatherMessage with the current weather data, or null if extraction fails
     */
    public WeatherMessage extract() {
        if (world == null) {
            return null;
        }
        
        try {
            WeatherMessage message = new WeatherMessage();
            
            // Extract weather type
            extractWeatherType(message);
            
            // Extract weather intensity
            extractWeatherIntensity(message);
            
            // Extract transition progress
            extractTransitionProgress(message);
            
            // Extract flags
            extractFlags(message);
            
            // Extract duration
            extractDuration(message);
            
            return message;
        } catch (Exception e) {
            System.err.println("Failed to extract weather data: " + e.getMessage());
            return null;
        }
    }

    /**
     * Extract weather type.
     *
     * @param message the message to populate
     */
    private void extractWeatherType(WeatherMessage message) {
        if (world == null) {
            return;
        }
        
        try {
            WorldInfo worldInfo = world.getWorldInfo();
            if (worldInfo == null) {
                return;
            }
            
            // Check for thunderstorm
            if (worldInfo.isThundering()) {
                message.setWeatherType(WeatherMessage.WEATHER_THUNDERSTORM);
                return;
            }
            
            // Check for rain
            if (worldInfo.isRaining()) {
                message.setWeatherType(WeatherMessage.WEATHER_RAIN);
                return;
            }
            
            // Check for snow (rain + cold biome)
            // In Minecraft 1.8.9, snow is determined by biome temperature
            // For simplicity, we'll just check if it's raining and assume snow in cold biomes
            // would be handled by the biome data
            
            // Default to clear
            message.setWeatherType(WeatherMessage.WEATHER_CLEAR);
        } catch (Exception e) {
            System.err.println("Failed to extract weather type: " + e.getMessage());
        }
    }

    /**
     * Extract weather intensity.
     *
     * @param message the message to populate
     */
    private void extractWeatherIntensity(WeatherMessage message) {
        if (world == null) {
            return;
        }
        
        try {
            WorldInfo worldInfo = world.getWorldInfo();
            if (worldInfo == null) {
                return;
            }
            
            // Get rain strength (0.0 to 1.0)
            float rainStrength = worldInfo.getRainStrength(lastPartialTicks);
            message.setIntensity(rainStrength);
            
            lastIntensity = rainStrength;
        } catch (Exception e) {
            System.err.println("Failed to extract weather intensity: " + e.getMessage());
        }
    }

    /**
     * Extract weather transition progress.
     *
     * @param message the message to populate
     */
    private void extractTransitionProgress(WeatherMessage message) {
        if (world == null) {
            return;
        }
        
        try {
            WorldInfo worldInfo = world.getWorldInfo();
            if (worldInfo == null) {
                return;
            }
            
            // Get thunder strength (0.0 to 1.0)
            float thunderStrength = worldInfo.getThunderStrength(lastPartialTicks);
            message.setTransitionProgress(thunderStrength);
            
            lastTransitionProgress = thunderStrength;
        } catch (Exception e) {
            System.err.println("Failed to extract transition progress: " + e.getMessage());
        }
    }

    /**
     * Extract weather flags.
     *
     * @param message the message to populate
     */
    private void extractFlags(WeatherMessage message) {
        if (world == null) {
            return;
        }
        
        try {
            WorldInfo worldInfo = world.getWorldInfo();
            if (worldInfo == null) {
                return;
            }
            
            int flags = 0;
            
            if (worldInfo.isRaining()) {
                flags |= WeatherMessage.FLAG_RAINING;
            }
            
            if (worldInfo.isThundering()) {
                flags |= WeatherMessage.FLAG_THUNDERING;
            }
            
            // Snow detection would require biome information
            // For now, we'll just use the rain flag
            
            message.setFlags(flags);
        } catch (Exception e) {
            System.err.println("Failed to extract weather flags: " + e.getMessage());
        }
    }

    /**
     * Extract weather duration.
     *
     * @param message the message to populate
     */
    private void extractDuration(WeatherMessage message) {
        if (world == null) {
            return;
        }
        
        try {
            WorldInfo worldInfo = world.getWorldInfo();
            if (worldInfo == null) {
                return;
            }
            
            // Get the remaining weather duration in ticks
            // In Minecraft 1.8.9, this is stored in the WorldInfo
            int duration = worldInfo.getCleanWeatherTime();
            message.setDurationTicks(duration);
        } catch (Exception e) {
            System.err.println("Failed to extract weather duration: " + e.getMessage());
        }
    }

    /**
     * Get the last partial ticks (from camera extractor).
     * This is a temporary solution - in a real implementation, we would
     * pass the partial ticks from the render event.
     *
     * @return the last partial ticks
     */
    private float getLastPartialTicks() {
        // For now, return 0
        return 0;
    }

    @Override
    public String toString() {
        return String.format(
                "WeatherExtractor{world=%s}",
                world != null
        );
    }
}
