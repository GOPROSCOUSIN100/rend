#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <unordered_map>
#include <functional>

namespace renderbridge {

/**
 * @class InputHandler
 * @brief Handles input state and provides input mapping
 */
class InputHandler {
public:
    // Key states
    enum class KeyState {
        RELEASED,
        PRESSED,
        REPEAT
    };
    
    // Mouse button states
    enum class MouseButtonState {
        RELEASED,
        PRESSED
    };

public:
    /**
     * @brief Constructor
     */
    InputHandler();
    
    /**
     * @brief Destructor
     */
    ~InputHandler();
    
    /**
     * @brief Initialize the input handler
     */
    void initialize();
    
    /**
     * @brief Shutdown the input handler
     */
    void shutdown();
    
    /**
     * @brief Update input state
     * @param deltaTime Delta time in seconds
     */
    void update(float deltaTime);
    
    /**
     * @brief Handle key event
     * @param key Key code
     * @param scancode Scancode
     * @param action Action
     * @param mods Modifiers
     */
    void onKey(int key, int scancode, int action, int mods);
    
    /**
     * @brief Handle mouse button event
     * @param button Button code
     * @param action Action
     * @param mods Modifiers
     */
    void onMouseButton(int button, int action, int mods);
    
    /**
     * @brief Handle mouse movement
     * @param x X position
     * @param y Y position
     */
    void onMouseMove(double x, double y);
    
    /**
     * @brief Handle mouse scroll
     * @param xOffset X offset
     * @param yOffset Y offset
     */
    void onMouseScroll(double xOffset, double yOffset);
    
    // Key state
    /**
     * @brief Check if a key is pressed
     * @param key Key code
     * @return True if the key is pressed
     */
    bool isKeyPressed(int key) const;
    
    /**
     * @brief Check if a key was just pressed
     * @param key Key code
     * @return True if the key was just pressed
     */
    bool isKeyJustPressed(int key) const;
    
    /**
     * @brief Check if a key was just released
     * @param key Key code
     * @return True if the key was just released
     */
    bool isKeyJustReleased(int key) const;
    
    /**
     * @brief Get key state
     * @param key Key code
     * @return Key state
     */
    KeyState getKeyState(int key) const;
    
    // Mouse button state
    /**
     * @brief Check if a mouse button is pressed
     * @param button Button code
     * @return True if the button is pressed
     */
    bool isMouseButtonPressed(int button) const;
    
    /**
     * @brief Check if a mouse button was just pressed
     * @param button Button code
     * @return True if the button was just pressed
     */
    bool isMouseButtonJustPressed(int button) const;
    
    /**
     * @brief Check if a mouse button was just released
     * @param button Button code
     * @return True if the button was just released
     */
    bool isMouseButtonJustReleased(int button) const;
    
    /**
     * @brief Get mouse button state
     * @param button Button code
     * @return Mouse button state
     */
    MouseButtonState getMouseButtonState(int button) const;
    
    // Mouse position
    /**
     * @brief Get mouse position
     * @param x Output: X position
     * @param y Output: Y position
     */
    void getMousePosition(double& x, double& y) const;
    
    /**
     * @brief Get mouse delta
     * @param x Output: X delta
     * @param y Output: Y delta
     */
    void getMouseDelta(double& x, double& y) const;
    
    /**
     * @brief Get mouse scroll delta
     * @param x Output: X scroll delta
     * @param y Output: Y scroll delta
     */
    void getMouseScrollDelta(double& x, double& y) const;
    
    // Modifier keys
    /**
     * @brief Check if shift is pressed
     * @return True if shift is pressed
     */
    bool isShiftPressed() const;
    
    /**
     * @brief Check if control is pressed
     * @return True if control is pressed
     */
    bool isControlPressed() const;
    
    /**
     * @brief Check if alt is pressed
     * @return True if alt is pressed
     */
    bool isAltPressed() const;
    
    /**
     * @brief Check if super (Windows/Command) is pressed
     * @return True if super is pressed
     */
    bool isSuperPressed() const;
    
    // Action mappings
    /**
     * @brief Register an action mapping
     * @param name Action name
     * @param key Key code
     */
    void registerAction(const std::string& name, int key);
    
    /**
     * @brief Check if an action is pressed
     * @param name Action name
     * @return True if the action is pressed
     */
    bool isActionPressed(const std::string& name) const;
    
    /**
     * @brief Check if an action was just pressed
     * @param name Action name
     * @return True if the action was just pressed
     */
    bool isActionJustPressed(const std::string& name) const;
    
    /**
     * @brief Check if an action was just released
     * @param name Action name
     * @return True if the action was just released
     */
    bool isActionJustReleased(const std::string& name) const;
    
    // Axis mappings
    /**
     * @brief Register an axis mapping
     * @param name Axis name
     * @param positiveKey Key for positive direction
     * @param negativeKey Key for negative direction
     */
    void registerAxis(const std::string& name, int positiveKey, int negativeKey);
    
    /**
     * @brief Get axis value
     * @param name Axis name
     * @return Axis value (-1.0 to 1.0)
     */
    float getAxisValue(const std::string& name) const;
    
    // Mouse axis
    /**
     * @brief Register a mouse axis mapping
     * @param name Axis name
     * @param mouseAxis 0 for X, 1 for Y
     */
    void registerMouseAxis(const std::string& name, int mouseAxis);
    
    /**
     * @brief Get mouse axis value
     * @param name Axis name
     * @return Axis value
     */
    float getMouseAxisValue(const std::string& name) const;
    
    // Reset input state
    /**
     * @brief Reset input state (clear just pressed/released flags)
     */
    void resetInputState();
    
    // Clear all input
    /**
     * @brief Clear all input state
     */
    void clearAllInput();

private:
    // Key states
    static constexpr int MAX_KEYS = 512;
    std::array<KeyState, MAX_KEYS> m_keyStates;
    std::array<bool, MAX_KEYS> m_keyJustPressed;
    std::array<bool, MAX_KEYS> m_keyJustReleased;
    
    // Mouse button states
    static constexpr int MAX_MOUSE_BUTTONS = 16;
    std::array<MouseButtonState, MAX_MOUSE_BUTTONS> m_mouseButtonStates;
    std::array<bool, MAX_MOUSE_BUTTONS> m_mouseButtonJustPressed;
    std::array<bool, MAX_MOUSE_BUTTONS> m_mouseButtonJustReleased;
    
    // Mouse position
    double m_mouseX;
    double m_mouseY;
    double m_mousePrevX;
    double m_mousePrevY;
    double m_mouseDeltaX;
    double m_mouseDeltaY;
    
    // Mouse scroll
    double m_mouseScrollX;
    double m_mouseScrollY;
    
    // Modifier keys
    bool m_shiftPressed;
    bool m_controlPressed;
    bool m_altPressed;
    bool m_superPressed;
    
    // Action mappings
    std::unordered_map<std::string, int> m_actionMappings;
    
    // Axis mappings
    struct AxisMapping {
        int positiveKey;
        int negativeKey;
    };
    std::unordered_map<std::string, AxisMapping> m_axisMappings;
    
    // Mouse axis mappings
    std::unordered_map<std::string, int> m_mouseAxisMappings;
};

} // namespace renderbridge
