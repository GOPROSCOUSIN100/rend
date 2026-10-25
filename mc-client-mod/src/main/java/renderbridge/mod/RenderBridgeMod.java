package renderbridge.mod;

import cpw.mods.fml.common.Mod;
import cpw.mods.fml.common.Mod.EventHandler;
import cpw.mods.fml.common.event.FMLInitializationEvent;
import cpw.mods.fml.common.event.FMLPreInitializationEvent;
import cpw.mods.fml.common.event.FMLPostInitializationEvent;
import cpw.mods.fml.common.eventhandler.SubscribeEvent;
import cpw.mods.fml.common.gameevent.TickEvent;
import cpw.mods.fml.relauncher.Side;
import cpw.mods.fml.relauncher.SideOnly;
import net.minecraftforge.common.MinecraftForge;
import net.minecraftforge.fml.common.FMLCommonHandler;
import net.minecraftforge.fml.common.eventhandler.EventPriority;
import net.minecraftforge.fml.relauncher.Side;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import renderbridge.mod.bridge.BridgeManager;
import renderbridge.mod.config.ModConfig;
import renderbridge.mod.hooks.RenderHooks;

/**
 * Main class for the RenderBridge mod.
 * This mod hooks into the Minecraft render pipeline and sends data
 * to an external renderer via the bridge protocol.
 */
@Mod(modid = RenderBridgeMod.MODID, name = RenderBridgeMod.NAME, version = RenderBridgeMod.VERSION, 
     clientSideOnly = true, useMetadata = true)
public class RenderBridgeMod {

    public static final String MODID = "renderbridge";
    public static final String NAME = "RenderBridge";
    public static final String VERSION = "0.1.0-SNAPSHOT";

    private static final Logger LOGGER = LogManager.getLogger(MODID);

    // Mod instances
    private static RenderBridgeMod instance;
    private BridgeManager bridgeManager;
    private ModConfig config;
    private RenderHooks renderHooks;

    /**
     * Pre-initialization event handler.
     */
    @EventHandler
    public void preInit(FMLPreInitializationEvent event) {
        LOGGER.info("Pre-initializing " + NAME + " v" + VERSION);
        
        instance = this;
        
        // Load configuration
        config = new ModConfig(event.getSuggestedConfigurationFile());
        config.load();
        
        // Check if mod is enabled
        if (!config.isEnabled()) {
            LOGGER.info("Mod is disabled in configuration");
            return;
        }
        
        // Initialize bridge manager
        bridgeManager = new BridgeManager(config);
        
        // Register events
        MinecraftForge.EVENT_BUS.register(this);
        FMLCommonHandler.instance().bus().register(this);
        
        LOGGER.info("Pre-initialization complete");
    }

    /**
     * Initialization event handler.
     */
    @EventHandler
    public void init(FMLInitializationEvent event) {
        LOGGER.info("Initializing " + NAME);
        
        if (!config.isEnabled()) {
            return;
        }
        
        // Initialize render hooks
        renderHooks = new RenderHooks(bridgeManager);
        
        // Register hooks
        MinecraftForge.EVENT_BUS.register(renderHooks);
        
        // Connect to bridge server
        bridgeManager.connect();
        
        LOGGER.info("Initialization complete");
    }

    /**
     * Post-initialization event handler.
     */
    @EventHandler
    public void postInit(FMLPostInitializationEvent event) {
        LOGGER.info("Post-initializing " + NAME);
        LOGGER.info("Post-initialization complete");
    }

    /**
     * Client tick event handler.
     */
    @SubscribeEvent
    @SideOnly(Side.CLIENT)
    public void onClientTick(TickEvent.ClientTickEvent event) {
        if (!config.isEnabled() || bridgeManager == null) {
            return;
        }
        
        // Update bridge manager
        bridgeManager.update();
    }

    /**
     * Get the mod instance.
     *
     * @return the mod instance
     */
    public static RenderBridgeMod getInstance() {
        return instance;
    }

    /**
     * Get the bridge manager.
     *
     * @return the bridge manager
     */
    public BridgeManager getBridgeManager() {
        return bridgeManager;
    }

    /**
     * Get the mod configuration.
     *
     * @return the mod configuration
     */
    public ModConfig getConfig() {
        return config;
    }

    /**
     * Get the render hooks.
     *
     * @return the render hooks
     */
    public RenderHooks getRenderHooks() {
        return renderHooks;
    }

    /**
     * Get the logger.
     *
     * @return the logger
     */
    public static Logger getLogger() {
        return LOGGER;
    }
}
