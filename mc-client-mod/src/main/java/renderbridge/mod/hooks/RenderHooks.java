package renderbridge.mod.hooks;

import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.GuiIngame;
import net.minecraft.client.renderer.EntityRenderer;
import net.minecraft.client.renderer.RenderGlobal;
import net.minecraft.client.renderer.texture.TextureManager;
import net.minecraft.world.World;
import net.minecraftforge.client.event.RenderGameOverlayEvent;
import net.minecraftforge.client.event.RenderWorldEvent;
import net.minecraftforge.client.event.RenderWorldLastEvent;
import net.minecraftforge.event.entity.EntityJoinWorldEvent;
import net.minecraftforge.event.world.ChunkEvent;
import net.minecraftforge.event.world.WorldEvent;
import net.minecraftforge.fml.common.eventhandler.EventPriority;
import net.minecraftforge.fml.common.eventhandler.SubscribeEvent;
import net.minecraftforge.fml.common.gameevent.TickEvent;
import net.minecraftforge.fml.relauncher.Side;
import net.minecraftforge.fml.relauncher.SideOnly;

import renderbridge.mod.RenderBridgeMod;
import renderbridge.mod.bridge.BridgeManager;
import renderbridge.mod.config.ModConfig;
import renderbridge.mod.extractors.CameraExtractor;
import renderbridge.mod.extractors.ChunkExtractor;
import renderbridge.mod.extractors.EntityExtractor;
import renderbridge.mod.extractors.ParticleExtractor;
import renderbridge.mod.extractors.WeatherExtractor;

/**
 * Forge event hooks for the render pipeline.
 * Uses Forge's event system to hook into various rendering stages.
 */
@SideOnly(Side.CLIENT)
public class RenderHooks {

    private final BridgeManager bridgeManager;
    private final ModConfig config;
    
    // Extractors
    private CameraExtractor cameraExtractor;
    private ChunkExtractor chunkExtractor;
    private EntityExtractor entityExtractor;
    private ParticleExtractor particleExtractor;
    private WeatherExtractor weatherExtractor;
    
    // State
    private int tickCounter = 0;
    private boolean initialized = false;

    /**
     * Create new render hooks.
     *
     * @param bridgeManager the bridge manager
     */
    public RenderHooks(BridgeManager bridgeManager) {
        this.bridgeManager = bridgeManager;
        this.config = RenderBridgeMod.getInstance().getConfig();
    }

    /**
     * Initialize extractors when the world loads.
     */
    private void initializeExtractors() {
        if (initialized) {
            return;
        }
        
        Minecraft mc = Minecraft.getMinecraft();
        if (mc == null || mc.theWorld == null) {
            return;
        }
        
        try {
            EntityRenderer entityRenderer = mc.entityRenderer;
            RenderGlobal renderGlobal = mc.renderGlobal;
            TextureManager textureManager = mc.renderEngine;
            World world = mc.theWorld;
            
            cameraExtractor = new CameraExtractor(entityRenderer);
            chunkExtractor = new ChunkExtractor(renderGlobal, config.getMaxChunkDistance());
            entityExtractor = new EntityExtractor(renderGlobal);
            particleExtractor = new ParticleExtractor(mc.effectRenderer);
            weatherExtractor = new WeatherExtractor(world);
            
            initialized = true;
            RenderBridgeMod.getLogger().info("Render hooks initialized");
        } catch (Exception e) {
            RenderBridgeMod.getLogger().error("Failed to initialize render hooks: " + e.getMessage());
        }
    }

    /**
     * Client tick event - called every client tick.
     */
    @SubscribeEvent
    public void onClientTick(TickEvent.ClientTickEvent event) {
        if (event.phase != TickEvent.Phase.END || !config.isEnabled()) {
            return;
        }
        
        // Initialize extractors on first tick
        if (!initialized) {
            initializeExtractors();
        }
        
        // Update bridge manager
        if (bridgeManager != null) {
            bridgeManager.update();
        }
        
        // Increment tick counter
        tickCounter++;
    }

    /**
     * Render world last event - called after all world rendering is done.
     * This is the main hook for extracting render data.
     */
    @SubscribeEvent
    public void onRenderWorldLast(RenderWorldLastEvent event) {
        if (!config.isEnabled() || bridgeManager == null || !bridgeManager.isConnected()) {
            return;
        }
        
        // Check if we should extract data based on update interval
        if (tickCounter % config.getUpdateInterval() != 0) {
            return;
        }
        
        try {
            // Extract camera data
            if (config.isEnableCameraExtraction() && cameraExtractor != null) {
                cameraExtractor.update(event.partialTicks);
            }
            
            // Extract chunk data
            if (config.isEnableChunkExtraction() && chunkExtractor != null) {
                chunkExtractor.update();
            }
            
            // Extract entity data
            if (config.isEnableEntityExtraction() && entityExtractor != null) {
                entityExtractor.update(event.partialTicks);
            }
            
            // Extract particle data
            if (config.isEnableParticleExtraction() && particleExtractor != null) {
                particleExtractor.update(event.partialTicks);
            }
            
            // Extract weather data
            if (config.isEnableWeatherExtraction() && weatherExtractor != null) {
                weatherExtractor.update();
            }
        } catch (Exception e) {
            RenderBridgeMod.getLogger().error("Error in render hooks: " + e.getMessage());
        }
    }

    /**
     * Render game overlay event - called when GUI is rendered.
     * Used for fallback rendering when bridge is not connected.
     */
    @SubscribeEvent
    public void onRenderGameOverlay(RenderGameOverlayEvent event) {
        if (!config.isEnabled() || !config.isShowFallbackOverlay()) {
            return;
        }
        
        // Only render overlay in specific phases
        if (event.type != RenderGameOverlayEvent.ElementType.ALL) {
            return;
        }
        
        // Render fallback overlay if bridge is not connected
        if (bridgeManager != null && !bridgeManager.isConnected()) {
            renderFallbackOverlay(event);
        }
        
        // Render stats overlay if enabled
        if (config.isShowStats() && bridgeManager != null) {
            renderStatsOverlay(event);
        }
    }

    /**
     * Render fallback overlay when bridge is not connected.
     *
     * @param event the render event
     */
    private void renderFallbackOverlay(RenderGameOverlayEvent event) {
        Minecraft mc = Minecraft.getMinecraft();
        if (mc == null || mc.gameSettings == null) {
            return;
        }
        
        // Draw a semi-transparent overlay
        int width = event.resolution.getScaledWidth();
        int height = event.resolution.getScaledHeight();
        
        // Draw background
        // This would normally use Gui.drawRect, but we'll use a simpler approach
        // For now, just log that we would draw an overlay
        RenderBridgeMod.getLogger().debug("Rendering fallback overlay");
    }

    /**
     * Render statistics overlay.
     *
     * @param event the render event
     */
    private void renderStatsOverlay(RenderGameOverlayEvent event) {
        Minecraft mc = Minecraft.getMinecraft();
        if (mc == null || mc.gameSettings == null) {
            return;
        }
        
        // For now, just log stats
        if (bridgeManager != null) {
            RenderBridgeMod.getLogger().debug("Stats: " + bridgeManager.toString());
        }
    }

    /**
     * World load event - reinitialize extractors when world changes.
     */
    @SubscribeEvent
    public void onWorldLoad(WorldEvent.Load event) {
        if (!event.world.isRemote) {
            return;
        }
        
        initialized = false;
        initializeExtractors();
    }

    /**
     * World unload event - clean up when world unloads.
     */
    @SubscribeEvent
    public void onWorldUnload(WorldEvent.Unload event) {
        initialized = false;
    }

    /**
     * Chunk load event - notify chunk extractor.
     */
    @SubscribeEvent
    public void onChunkLoad(ChunkEvent.Load event) {
        if (!config.isEnabled() || chunkExtractor == null) {
            return;
        }
        
        chunkExtractor.onChunkLoaded(event.getChunk());
    }

    /**
     * Chunk unload event - notify chunk extractor.
     */
    @SubscribeEvent
    public void onChunkUnload(ChunkEvent.Unload event) {
        if (!config.isEnabled() || chunkExtractor == null) {
            return;
        }
        
        chunkExtractor.onChunkUnloaded(event.getChunk());
    }

    /**
     * Entity join world event - notify entity extractor.
     */
    @SubscribeEvent
    public void onEntityJoinWorld(EntityJoinWorldEvent event) {
        if (!config.isEnabled() || entityExtractor == null) {
            return;
        }
        
        entityExtractor.onEntityAdded(event.entity);
    }

    /**
     * Get the camera extractor.
     *
     * @return the camera extractor
     */
    public CameraExtractor getCameraExtractor() {
        return cameraExtractor;
    }

    /**
     * Get the chunk extractor.
     *
     * @return the chunk extractor
     */
    public ChunkExtractor getChunkExtractor() {
        return chunkExtractor;
    }

    /**
     * Get the entity extractor.
     *
     * @return the entity extractor
     */
    public EntityExtractor getEntityExtractor() {
        return entityExtractor;
    }

    /**
     * Get the particle extractor.
     *
     * @return the particle extractor
     */
    public ParticleExtractor getParticleExtractor() {
        return particleExtractor;
    }

    /**
     * Get the weather extractor.
     *
     * @return the weather extractor
     */
    public WeatherExtractor getWeatherExtractor() {
        return weatherExtractor;
    }

    /**
     * Check if hooks are initialized.
     *
     * @return true if initialized
     */
    public boolean isInitialized() {
        return initialized;
    }

    /**
     * Reset initialization flag.
     */
    public void resetInitialization() {
        initialized = false;
    }
}
