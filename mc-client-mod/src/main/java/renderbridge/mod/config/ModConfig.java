package renderbridge.mod.config;

import net.minecraftforge.common.config.Configuration;

import java.io.File;

/**
 * Configuration for the RenderBridge mod.
 */
public class ModConfig {

    // Configuration categories
    private static final String CATEGORY_GENERAL = "general";
    private static final String CATEGORY_BRIDGE = "bridge";
    private static final String CATEGORY_RENDERING = "rendering";
    private static final String CATEGORY_PERFORMANCE = "performance";

    // Configuration values
    private boolean enabled = true;
    private boolean useSharedMemory = true;
    private boolean useSocket = true;
    private String socketHost = "127.0.0.1";
    private int socketPort = 25565;
    private int sharedMemorySize = 16;
    
    private boolean enableCameraExtraction = true;
    private boolean enableChunkExtraction = true;
    private boolean enableEntityExtraction = true;
    private boolean enableParticleExtraction = true;
    private boolean enableWeatherExtraction = true;
    
    private boolean showFallbackOverlay = true;
    private boolean showStats = true;
    
    private int maxChunkDistance = 8;
    private int updateInterval = 1;
    
    private Configuration config;

    /**
     * Create a new mod configuration.
     *
     * @param configFile the configuration file
     */
    public ModConfig(File configFile) {
        config = new Configuration(configFile);
    }

    /**
     * Load the configuration from file.
     */
    public void load() {
        config.load();
        
        // General settings
        enabled = config.getBoolean("enabled", CATEGORY_GENERAL, true, 
                "Enable or disable the mod");
        
        // Bridge settings
        useSharedMemory = config.getBoolean("useSharedMemory", CATEGORY_BRIDGE, true,
                "Use shared memory transport (best performance)");
        useSocket = config.getBoolean("useSocket", CATEGORY_BRIDGE, true,
                "Use socket transport (fallback)");
        socketHost = config.getString("socketHost", CATEGORY_BRIDGE, "127.0.0.1",
                "Host for socket transport");
        socketPort = config.getInt("socketPort", CATEGORY_BRIDGE, 25565, 1, 65535,
                "Port for socket transport");
        sharedMemorySize = config.getInt("sharedMemorySize", CATEGORY_BRIDGE, 16, 1, 256,
                "Size of shared memory buffer in MB");
        
        // Rendering settings
        enableCameraExtraction = config.getBoolean("enableCameraExtraction", CATEGORY_RENDERING, true,
                "Enable camera data extraction");
        enableChunkExtraction = config.getBoolean("enableChunkExtraction", CATEGORY_RENDERING, true,
                "Enable chunk data extraction");
        enableEntityExtraction = config.getBoolean("enableEntityExtraction", CATEGORY_RENDERING, true,
                "Enable entity data extraction");
        enableParticleExtraction = config.getBoolean("enableParticleExtraction", CATEGORY_RENDERING, true,
                "Enable particle data extraction");
        enableWeatherExtraction = config.getBoolean("enableWeatherExtraction", CATEGORY_RENDERING, true,
                "Enable weather data extraction");
        
        showFallbackOverlay = config.getBoolean("showFallbackOverlay", CATEGORY_RENDERING, true,
                "Show fallback overlay when bridge is not connected");
        showStats = config.getBoolean("showStats", CATEGORY_RENDERING, true,
                "Show statistics overlay");
        
        // Performance settings
        maxChunkDistance = config.getInt("maxChunkDistance", CATEGORY_PERFORMANCE, 8, 1, 32,
                "Maximum distance for chunk extraction");
        updateInterval = config.getInt("updateInterval", CATEGORY_PERFORMANCE, 1, 1, 10,
                "Number of ticks between updates (higher = less CPU usage, lower FPS)");
        
        // Save if there are changes
        if (config.hasChanged()) {
            config.save();
        }
    }

    /**
     * Save the configuration to file.
     */
    public void save() {
        // Update config values
        config.getCategory(CATEGORY_GENERAL).get("enabled").set(enabled);
        
        config.getCategory(CATEGORY_BRIDGE).get("useSharedMemory").set(useSharedMemory);
        config.getCategory(CATEGORY_BRIDGE).get("useSocket").set(useSocket);
        config.getCategory(CATEGORY_BRIDGE).get("socketHost").set(socketHost);
        config.getCategory(CATEGORY_BRIDGE).get("socketPort").set(socketPort);
        config.getCategory(CATEGORY_BRIDGE).get("sharedMemorySize").set(sharedMemorySize);
        
        config.getCategory(CATEGORY_RENDERING).get("enableCameraExtraction").set(enableCameraExtraction);
        config.getCategory(CATEGORY_RENDERING).get("enableChunkExtraction").set(enableChunkExtraction);
        config.getCategory(CATEGORY_RENDERING).get("enableEntityExtraction").set(enableEntityExtraction);
        config.getCategory(CATEGORY_RENDERING).get("enableParticleExtraction").set(enableParticleExtraction);
        config.getCategory(CATEGORY_RENDERING).get("enableWeatherExtraction").set(enableWeatherExtraction);
        config.getCategory(CATEGORY_RENDERING).get("showFallbackOverlay").set(showFallbackOverlay);
        config.getCategory(CATEGORY_RENDERING).get("showStats").set(showStats);
        
        config.getCategory(CATEGORY_PERFORMANCE).get("maxChunkDistance").set(maxChunkDistance);
        config.getCategory(CATEGORY_PERFORMANCE).get("updateInterval").set(updateInterval);
        
        config.save();
    }

    // Getters and setters

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
    }

    public boolean isUseSharedMemory() {
        return useSharedMemory;
    }

    public void setUseSharedMemory(boolean useSharedMemory) {
        this.useSharedMemory = useSharedMemory;
    }

    public boolean isUseSocket() {
        return useSocket;
    }

    public void setUseSocket(boolean useSocket) {
        this.useSocket = useSocket;
    }

    public String getSocketHost() {
        return socketHost;
    }

    public void setSocketHost(String socketHost) {
        this.socketHost = socketHost;
    }

    public int getSocketPort() {
        return socketPort;
    }

    public void setSocketPort(int socketPort) {
        this.socketPort = socketPort;
    }

    public int getSharedMemorySize() {
        return sharedMemorySize;
    }

    public void setSharedMemorySize(int sharedMemorySize) {
        this.sharedMemorySize = sharedMemorySize;
    }

    public boolean isEnableCameraExtraction() {
        return enableCameraExtraction;
    }

    public void setEnableCameraExtraction(boolean enableCameraExtraction) {
        this.enableCameraExtraction = enableCameraExtraction;
    }

    public boolean isEnableChunkExtraction() {
        return enableChunkExtraction;
    }

    public void setEnableChunkExtraction(boolean enableChunkExtraction) {
        this.enableChunkExtraction = enableChunkExtraction;
    }

    public boolean isEnableEntityExtraction() {
        return enableEntityExtraction;
    }

    public void setEnableEntityExtraction(boolean enableEntityExtraction) {
        this.enableEntityExtraction = enableEntityExtraction;
    }

    public boolean isEnableParticleExtraction() {
        return enableParticleExtraction;
    }

    public void setEnableParticleExtraction(boolean enableParticleExtraction) {
        this.enableParticleExtraction = enableParticleExtraction;
    }

    public boolean isEnableWeatherExtraction() {
        return enableWeatherExtraction;
    }

    public void setEnableWeatherExtraction(boolean enableWeatherExtraction) {
        this.enableWeatherExtraction = enableWeatherExtraction;
    }

    public boolean isShowFallbackOverlay() {
        return showFallbackOverlay;
    }

    public void setShowFallbackOverlay(boolean showFallbackOverlay) {
        this.showFallbackOverlay = showFallbackOverlay;
    }

    public boolean isShowStats() {
        return showStats;
    }

    public void setShowStats(boolean showStats) {
        this.showStats = showStats;
    }

    public int getMaxChunkDistance() {
        return maxChunkDistance;
    }

    public void setMaxChunkDistance(int maxChunkDistance) {
        this.maxChunkDistance = maxChunkDistance;
    }

    public int getUpdateInterval() {
        return updateInterval;
    }

    public void setUpdateInterval(int updateInterval) {
        this.updateInterval = updateInterval;
    }

    /**
     * Get the shared memory buffer size in bytes.
     *
     * @return the shared memory buffer size in bytes
     */
    public int getSharedMemoryBufferSize() {
        return sharedMemorySize * 1024 * 1024; // Convert MB to bytes
    }
}
