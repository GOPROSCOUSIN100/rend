#include "input_handler.hpp"

#include <cstring>
#include <algorithm>

namespace renderbridge {

InputHandler::InputHandler()
    : m_mouseX(0.0),
      m_mouseY(0.0),
      m_mousePrevX(0.0),
      m_mousePrevY(0.0),
      m_mouseDeltaX(0.0),
      m_mouseDeltaY(0.0),
      m_mouseScrollX(0.0),
      m_mouseScrollY(0.0),
      m_shiftPressed(false),
      m_controlPressed(false),
      m_altPressed(false),
      m_superPressed(false)
{
    // Initialize key states
    for (int i = 0; i < MAX_KEYS; ++i) {
        m_keyStates[i] = KeyState::RELEASED;
        m_keyJustPressed[i] = false;
        m_keyJustReleased[i] = false;
    }
    
    // Initialize mouse button states
    for (int i = 0; i < MAX_MOUSE_BUTTONS; ++i) {
        m_mouseButtonStates[i] = MouseButtonState::RELEASED;
        m_mouseButtonJustPressed[i] = false;
        m_mouseButtonJustReleased[i] = false;
    }
}

InputHandler::~InputHandler()
{
    shutdown();
}

void InputHandler::initialize()
{
    clearAllInput();
}

void InputHandler::shutdown()
{
    clearAllInput();
}

void InputHandler::update(float deltaTime)
{
    // Reset just pressed/released flags
    resetInputState();
    
    // Reset mouse delta
    m_mouseDeltaX = 0.0;
    m_mouseDeltaY = 0.0;
    
    // Reset mouse scroll delta
    m_mouseScrollX = 0.0;
    m_mouseScrollY = 0.0;
    
    // Update previous mouse position
    m_mousePrevX = m_mouseX;
    m_mousePrevY = m_mouseY;
}

void InputHandler::onKey(int key, int scancode, int action, int mods)
{
    if (key < 0 || key >= MAX_KEYS) {
        return;
    }
    
    // Update modifier keys
    m_shiftPressed = (mods & 0x0001) != 0; // GLFW_MOD_SHIFT
    m_controlPressed = (mods & 0x0002) != 0; // GLFW_MOD_CONTROL
    m_altPressed = (mods & 0x0004) != 0; // GLFW_MOD_ALT
    m_superPressed = (mods & 0x0008) != 0; // GLFW_MOD_SUPER
    
    // Update key state
    switch (action) {
        case 0: // GLFW_RELEASE
            if (m_keyStates[key] == KeyState::PRESSED || m_keyStates[key] == KeyState::REPEAT) {
                m_keyJustReleased[key] = true;
            }
            m_keyStates[key] = KeyState::RELEASED;
            break;
        case 1: // GLFW_PRESS
            if (m_keyStates[key] == KeyState::RELEASED) {
                m_keyJustPressed[key] = true;
            }
            m_keyStates[key] = KeyState::PRESSED;
            break;
        case 2: // GLFW_REPEAT
            m_keyStates[key] = KeyState::REPEAT;
            break;
    }
}

void InputHandler::onMouseButton(int button, int action, int mods)
{
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) {
        return;
    }
    
    // Update modifier keys
    m_shiftPressed = (mods & 0x0001) != 0;
    m_controlPressed = (mods & 0x0002) != 0;
    m_altPressed = (mods & 0x0004) != 0;
    m_superPressed = (mods & 0x0008) != 0;
    
    // Update mouse button state
    switch (action) {
        case 0: // GLFW_RELEASE
            if (m_mouseButtonStates[button] == MouseButtonState::PRESSED) {
                m_mouseButtonJustReleased[button] = true;
            }
            m_mouseButtonStates[button] = MouseButtonState::RELEASED;
            break;
        case 1: // GLFW_PRESS
            if (m_mouseButtonStates[button] == MouseButtonState::RELEASED) {
                m_mouseButtonJustPressed[button] = true;
            }
            m_mouseButtonStates[button] = MouseButtonState::PRESSED;
            break;
    }
}

void InputHandler::onMouseMove(double x, double y)
{
    // Calculate delta
    m_mouseDeltaX = x - m_mouseX;
    m_mouseDeltaY = y - m_mouseY;
    
    // Update position
    m_mouseX = x;
    m_mouseY = y;
}

void InputHandler::onMouseScroll(double xOffset, double yOffset)
{
    m_mouseScrollX = xOffset;
    m_mouseScrollY = yOffset;
}

// Key state
bool InputHandler::isKeyPressed(int key) const
{
    if (key < 0 || key >= MAX_KEYS) {
        return false;
    }
    return m_keyStates[key] == KeyState::PRESSED || m_keyStates[key] == KeyState::REPEAT;
}

bool InputHandler::isKeyJustPressed(int key) const
{
    if (key < 0 || key >= MAX_KEYS) {
        return false;
    }
    return m_keyJustPressed[key];
}

bool InputHandler::isKeyJustReleased(int key) const
{
    if (key < 0 || key >= MAX_KEYS) {
        return false;
    }
    return m_keyJustReleased[key];
}

InputHandler::KeyState InputHandler::getKeyState(int key) const
{
    if (key < 0 || key >= MAX_KEYS) {
        return KeyState::RELEASED;
    }
    return m_keyStates[key];
}

// Mouse button state
bool InputHandler::isMouseButtonPressed(int button) const
{
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) {
        return false;
    }
    return m_mouseButtonStates[button] == MouseButtonState::PRESSED;
}

bool InputHandler::isMouseButtonJustPressed(int button) const
{
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) {
        return false;
    }
    return m_mouseButtonJustPressed[button];
}

bool InputHandler::isMouseButtonJustReleased(int button) const
{
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) {
        return false;
    }
    return m_mouseButtonJustReleased[button];
}

InputHandler::MouseButtonState InputHandler::getMouseButtonState(int button) const
{
    if (button < 0 || button >= MAX_MOUSE_BUTTONS) {
        return MouseButtonState::RELEASED;
    }
    return m_mouseButtonStates[button];
}

// Mouse position
void InputHandler::getMousePosition(double& x, double& y) const
{
    x = m_mouseX;
    y = m_mouseY;
}

void InputHandler::getMouseDelta(double& x, double& y) const
{
    x = m_mouseDeltaX;
    y = m_mouseDeltaY;
}

void InputHandler::getMouseScrollDelta(double& x, double& y) const
{
    x = m_mouseScrollX;
    y = m_mouseScrollY;
}

// Modifier keys
bool InputHandler::isShiftPressed() const
{
    return m_shiftPressed;
}

bool InputHandler::isControlPressed() const
{
    return m_controlPressed;
}

bool InputHandler::isAltPressed() const
{
    return m_altPressed;
}

bool InputHandler::isSuperPressed() const
{
    return m_superPressed;
}

// Action mappings
void InputHandler::registerAction(const std::string& name, int key)
{
    m_actionMappings[name] = key;
}

bool InputHandler::isActionPressed(const std::string& name) const
{
    auto it = m_actionMappings.find(name);
    if (it == m_actionMappings.end()) {
        return false;
    }
    return isKeyPressed(it->second);
}

bool InputHandler::isActionJustPressed(const std::string& name) const
{
    auto it = m_actionMappings.find(name);
    if (it == m_actionMappings.end()) {
        return false;
    }
    return isKeyJustPressed(it->second);
}

bool InputHandler::isActionJustReleased(const std::string& name) const
{
    auto it = m_actionMappings.find(name);
    if (it == m_actionMappings.end()) {
        return false;
    }
    return isKeyJustReleased(it->second);
}

// Axis mappings
void InputHandler::registerAxis(const std::string& name, int positiveKey, int negativeKey)
{
    AxisMapping mapping;
    mapping.positiveKey = positiveKey;
    mapping.negativeKey = negativeKey;
    m_axisMappings[name] = mapping;
}

float InputHandler::getAxisValue(const std::string& name) const
{
    auto it = m_axisMappings.find(name);
    if (it == m_axisMappings.end()) {
        return 0.0f;
    }
    
    const AxisMapping& mapping = it->second;
    float positive = isKeyPressed(mapping.positiveKey) ? 1.0f : 0.0f;
    float negative = isKeyPressed(mapping.negativeKey) ? 1.0f : 0.0f;
    
    return positive - negative;
}

// Mouse axis
void InputHandler::registerMouseAxis(const std::string& name, int mouseAxis)
{
    m_mouseAxisMappings[name] = mouseAxis;
}

float InputHandler::getMouseAxisValue(const std::string& name) const
{
    auto it = m_mouseAxisMappings.find(name);
    if (it == m_mouseAxisMappings.end()) {
        return 0.0f;
    }
    
    if (it->second == 0) {
        return static_cast<float>(m_mouseDeltaX);
    } else {
        return static_cast<float>(m_mouseDeltaY);
    }
}

// Reset input state
void InputHandler::resetInputState()
{
    for (int i = 0; i < MAX_KEYS; ++i) {
        m_keyJustPressed[i] = false;
        m_keyJustReleased[i] = false;
    }
    
    for (int i = 0; i < MAX_MOUSE_BUTTONS; ++i) {
        m_mouseButtonJustPressed[i] = false;
        m_mouseButtonJustReleased[i] = false;
    }
}

// Clear all input
void InputHandler::clearAllInput()
{
    resetInputState();
    
    for (int i = 0; i < MAX_KEYS; ++i) {
        m_keyStates[i] = KeyState::RELEASED;
    }
    
    for (int i = 0; i < MAX_MOUSE_BUTTONS; ++i) {
        m_mouseButtonStates[i] = MouseButtonState::RELEASED;
    }
    
    m_mouseX = 0.0;
    m_mouseY = 0.0;
    m_mousePrevX = 0.0;
    m_mousePrevY = 0.0;
    m_mouseDeltaX = 0.0;
    m_mouseDeltaY = 0.0;
    m_mouseScrollX = 0.0;
    m_mouseScrollY = 0.0;
    
    m_shiftPressed = false;
    m_controlPressed = false;
    m_altPressed = false;
    m_superPressed = false;
}

} // namespace renderbridge
