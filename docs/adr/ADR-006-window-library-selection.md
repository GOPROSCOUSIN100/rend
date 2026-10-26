# ADR-006: Window Library Selection for bgfx Renderer

## Status
✅ Akzeptiert

## Kontext

Für die Implementierung des externen bgfx-basierten Renderers (Prozess B) wird eine Fenster-Bibliothek benötigt, um:
- Ein Render-Fenster zu erstellen und zu verwalten
- Eingabegeräte (Tastatur, Maus) zu handhaben
- OpenGL/Vulkan/Metal/D3D-Kontexte zu erstellen (je nach Backend)
- Plattformunabhängigkeit zu gewährleisten

bgfx selbst ist eine Rendering-Abstraktionsschicht und bietet **keine** Fensterverwaltung. Daher muss eine externe Bibliothek gewählt werden.

### Anforderungen an die Fenster-Bibliothek:

1. **Cross-Platform**: Windows, Linux, macOS
2. **Backend-Kompatibilität**: Funktioniert mit allen bgfx-Backends (D3D11/12, OpenGL, Vulkan, Metal)
3. **Einfache Integration**: Gute CMake-Unterstützung
4. **Lizenz**: Kompatibel mit MIT-Lizenz (keine GPL)
5. **Stabilität**: Ausgereift und gut getestet
6. **Community**: Aktive Entwicklung und Support
7. **Minimaler Overhead**: Leichtgewichtig, keine unnötigen Abhängigkeiten

## Optionen

### 1. SDL2 (Simple DirectMedia Layer)

**Beschreibung**: SDL2 ist eine weit verbreitete Multimedia-Bibliothek, die Fensterverwaltung, Eingabe, Audio und mehr bietet.

**Vorteile**:
- Sehr gut dokumentiert
- Große Community und viele Beispiele
- Unterstützt alle Plattformen (Windows, Linux, macOS, iOS, Android)
- Funktioniert mit OpenGL, Vulkan, Metal, D3D11/12
- Einfache API für Fenster und Eingabe
- Gute CMake-Integration (FindSDL2.cmake)
- Viele bgfx-Beispiele verwenden SDL2

**Nachteile**:
- Größere Bibliothek mit vielen Features, die wir nicht brauchen (Audio, Joystick, etc.)
- Etwas schwergewichtiger als GLFW
- SDL2 hat eigene Event-Schleife, die integriert werden muss

**Lizenz**: zlib/Libpng License (sehr permissiv, MIT-kompatibel)

### 2. GLFW (Graphics Library Framework)

**Beschreibung**: GLFW ist eine spezifisch für OpenGL entwickelte Fenster-Bibliothek, die sich auf Grafik-Anwendungen konzentriert.

**Vorteile**:
- Leichtgewichtig und fokussiert auf Fenster/Input für Grafik-Anwendungen
- Einfache, saubere API
- Gute Dokumentation und Tutorials
- Unterstützt alle Plattformen
- Funktioniert mit OpenGL, Vulkan, Metal (über MoltenVK)
- Sehr gute CMake-Unterstützung (FindGLFW.cmake)
- Wird von vielen modernen Grafik-Anwendungen verwendet

**Nachteile**:
- Ursprünglich für OpenGL entwickelt (Name), aber unterstützt jetzt auch andere APIs
- Kein direkter D3D11/12 Support (aber bgfx kann damit arbeiten)
- Etwas kleinere Community als SDL2

**Lizenz**: zlib/Libpng License (sehr permissiv, MIT-kompatibel)

### 3. Native Plattform-APIs

**Beschreibung**: Direkte Verwendung von Win32, X11/Wayland, Cocoa

**Vorteile**:
- Maximale Kontrolle
- Keine externen Abhängigkeiten
- Minimaler Overhead

**Nachteile**:
- Plattformspezifischer Code nötig
- Komplexe Build-Konfiguration
- Wartungsaufwand für mehrere Plattformen
- Nicht empfehlenswert für dieses Projekt

### 4. Andere Bibliotheken (sfml, Qt, etc.)

**Beschreibung**: Andere Fenster-Bibliotheken

**Nachteile**:
- Zu schwergewichtig (Qt)
- Nicht fokussiert auf Grafik (sfml)
- Komplexere Integration
- Lizenzprobleme (Qt hat GPL/kommerzielle Dual-Lizenz)

## Entscheidung

**GLFW als Fenster-Bibliothek für den bgfx-basierten Renderer**

### Begründung

#### Warum GLFW?

1. **Fokus auf Grafik-Anwendungen**:
   - GLFW wurde speziell für Grafik-Anwendungen entwickelt
   - Kein Ballast wie Audio oder Joystick-Handling
   - Perfekt für unseren Use-Case (nur Fenster + Input + Grafik-Kontext)

2. **Leichtgewichtig**:
   - Deutlich kleiner als SDL2
   - Weniger Abhängigkeiten
   - Einfacheres Linken und Deployment

3. **bgfx-Kompatibilität**:
   - GLFW funktioniert problemlos mit bgfx
   - Viele bgfx-Beispiele und Projekte verwenden GLFW
   - Einfache Integration des bgfx-Render-Kontexts

4. **Moderne API**:
   - Saubere, moderne C-API
   - Einfache Handhabung von Fenster, Kontext und Input
   - Gute Fehlerbehandlung

5. **CMake-Integration**:
   - FindGLFW.cmake ist weit verbreitet
   - Einfache Integration in unser CMake-Build-System
   - Gute Unterstützung für Paketmanager (vcpkg, conan)

6. **Lizenz**:
   - zlib/Libpng License ist sehr permissiv
   - Volle Kompatibilität mit MIT-Lizenz
   - Keine Probleme mit bgfx (BSD-2-Clause)

7. **Plattformunterstützung**:
   - Windows: Win32 API
   - Linux: X11, Wayland (experimentell)
   - macOS: Cocoa, Metal-Unterstützung
   - Alle Plattformen, die wir unterstützen wollen

#### Vergleichstabelle

| Kriterium | SDL2 | GLFW | Native |
|----------|------|------|--------|
| Gewicht | ❌ Schwer | ✅ Leicht | ✅ Leicht |
| Fokus | ⚠️ Multimedia | ✅ Grafik | ✅ Grafik |
| API | ✅ Gut | ✅ Sehr Gut | ❌ Plattformspezifisch |
| CMake | ✅ Gut | ✅ Sehr Gut | ❌ Komplex |
| bgfx Beispiele | ✅ Viele | ✅ Viele | ❌ Kaum |
| Lizenz | ✅ zlib | ✅ zlib | ✅ Keine |
| Plattformen | ✅ Alle | ✅ Alle | ❌ Plattformspezifisch |
| Community | ✅ Groß | ✅ Groß | ❌ Keine |
| Wartung | ✅ Einfach | ✅ Einfach | ❌ Komplex |

## Konsequenzen

### Positive Konsequenzen

- **Einfache Integration**: GLFW + bgfx arbeiten perfekt zusammen
- **Leichtgewichtig**: Kleine Binärdateien, wenige Abhängigkeiten
- **Moderne API**: Angenehme Entwicklungserfahrung
- **Gute Dokumentation**: Viele Tutorials und Beispiele verfügbar
- **Plattformunabhängig**: Funktioniert auf allen Zielplattformen

### Negative Konsequenzen

- **Kein direkter D3D11/12 Support**: Aber bgfx handhabt das intern, GLFW muss nur den Window Handle bereitstellen
- **Etwas kleinere Community als SDL2**: Aber immer noch sehr aktiv

### Technische Anforderungen

- GLFW 3.3 oder neuer (für beste Vulkan/Metal-Unterstützung)
- CMake 3.15+
- Plattformspezifische Abhängigkeiten werden von GLFW selbst gehandhabt

## Implementierungs-Strategie

### Projekt-Struktur

```
external-renderer-cpp/
├── CMakeLists.txt
├── src/
│   ├── main.cpp              # Haupt-Entry-Point
│   ├── window.hpp            # Fenster-Verwaltung mit GLFW
│   ├── window.cpp
│   ├── input_handler.hpp     # Eingabe-Verarbeitung
│   ├── input_handler.cpp
│   └── ...
├── cmake/
│   └── FindGLFW.cmake        # Falls nicht vom System bereitgestellt
└── 3rdparty/
    └── bgfx/                 # Git Submodule
```

### CMake-Integration

```cmake
# In external-renderer-cpp/CMakeLists.txt

# GLFW finden
find_package(glfw3 3.3 REQUIRED)

# bgfx als Submodule
add_subdirectory(3rdparty/bgfx)

# Renderer-Target
add_executable(renderer
    src/main.cpp
    src/window.cpp
    src/input_handler.cpp
    # ... andere Quellen
)

target_link_libraries(renderer PRIVATE
    glfw
    bgfx
    # ... andere Bibliotheken
)

# Plattformspezifische Definitionen
if(WIN32)
    target_compile_definitions(renderer PRIVATE GLFW_EXPOSE_NATIVE_WIN32)
elseif(APPLE)
    target_compile_definitions(renderer PRIVATE GLFW_EXPOSE_NATIVE_COCOA)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(COREVIDEO_FRAMEWORK CoreVideo)
    target_link_libraries(renderer PRIVATE ${COCOA_FRAMEWORK} ${IOKIT_FRAMEWORK} ${COREVIDEO_FRAMEWORK})
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(renderer PRIVATE GLFW_EXPOSE_NATIVE_X11)
    find_package(X11 REQUIRED)
    target_link_libraries(renderer PRIVATE ${X11_LIBRARIES})
endif()
```

### Grundlegende Fenster-Verwaltung

```cpp
// window.hpp
#pragma once

#include <GLFW/glfw3.h>
#include <string>

namespace renderbridge {

class Window {
public:
    Window(const std::string& title, int width, int height);
    ~Window();

    void create();
    void destroy();
    void swapBuffers();
    bool shouldClose() const;
    void pollEvents();

    GLFWwindow* getHandle() const { return m_handle; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    // Callbacks
    void setKeyCallback(GLFWkeyfun callback);
    void setMouseButtonCallback(GLFWmousebuttonfun callback);
    void setCursorPosCallback(GLFWcursorposfun callback);
    void setScrollCallback(GLFWscrollfun callback);
    void setFramebufferSizeCallback(GLFWframebuffersizefun callback);

private:
    GLFWwindow* m_handle;
    std::string m_title;
    int m_width;
    int m_height;
};

} // namespace renderbridge
```

```cpp
// window.cpp
#include "window.hpp"
#include <stdexcept>

namespace renderbridge {

Window::Window(const std::string& title, int width, int height)
    : m_handle(nullptr), m_title(title), m_width(width), m_height(height) {}

Window::~Window() {
    destroy();
}

void Window::create() {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // GLFW Konfiguration
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // bgfx erstellt seinen eigenen Kontext
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Fenster zunächst unsichtbar

    m_handle = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_handle) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    // Fenster zentrieren
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos(m_handle, (mode->width - m_width) / 2, (mode->height - m_height) / 2);

    // Fenster sichtbar machen
    glfwShowWindow(m_handle);
}

void Window::destroy() {
    if (m_handle) {
        glfwDestroyWindow(m_handle);
        m_handle = nullptr;
    }
    glfwTerminate();
}

void Window::swapBuffers() {
    if (m_handle) {
        glfwSwapBuffers(m_handle);
    }
}

bool Window::shouldClose() const {
    return m_handle ? glfwWindowShouldClose(m_handle) : true;
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::setKeyCallback(GLFWkeyfun callback) {
    if (m_handle) {
        glfwSetKeyCallback(m_handle, callback);
    }
}

void Window::setMouseButtonCallback(GLFWmousebuttonfun callback) {
    if (m_handle) {
        glfwSetMouseButtonCallback(m_handle, callback);
    }
}

void Window::setCursorPosCallback(GLFWcursorposfun callback) {
    if (m_handle) {
        glfwSetCursorPosCallback(m_handle, callback);
    }
}

void Window::setScrollCallback(GLFWscrollfun callback) {
    if (m_handle) {
        glfwSetScrollCallback(m_handle, callback);
    }
}

void Window::setFramebufferSizeCallback(GLFWframebuffersizefun callback) {
    if (m_handle) {
        glfwSetFramebufferSizeCallback(m_handle, callback);
    }
}

} // namespace renderbridge
```

### bgfx + GLFW Integration

```cpp
// main.cpp - Grundlegende bgfx + GLFW Integration
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include "window.hpp"

// Plattformspezifische GLFW <-> bgfx Integration
#if defined(_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
#elif defined(__APPLE__)
    #define GLFW_EXPOSE_NATIVE_COCOA
    #include <GLFW/glfw3native.h>
#elif defined(__linux__)
    #define GLFW_EXPOSE_NATIVE_X11
    #include <GLFW/glfw3native.h>
#endif

namespace renderbridge {

class RendererApp {
public:
    RendererApp() : m_window("RenderBridge - bgfx Renderer", 1280, 720) {}

    void run() {
        // Fenster erstellen
        m_window.create();

        // bgfx initialisieren
        initBgfx();

        // Hauptschleife
        while (!m_window.shouldClose()) {
            m_window.pollEvents();

            // bgfx Frame
            bgfx::frame();

            // Buffer swap
            m_window.swapBuffers();
        }

        // Cleanup
        bgfx::shutdown();
        m_window.destroy();
    }

private:
    void initBgfx() {
        // Plattformspezifische Daten für bgfx
        bgfx::PlatformData pd = {};

#if defined(_WIN32)
        pd.nwh = glfwGetWin32Window(m_window.getHandle());
        pd.ndt = nullptr;
#elif defined(__APPLE__)
        pd.nwh = glfwGetCocoaWindow(m_window.getHandle());
        pd.ndt = nullptr;
#elif defined(__linux__)
        pd.nwh = (void*)(uintptr_t)glfwGetX11Window(m_window.getHandle());
        pd.ndt = glfwGetX11Display();
#endif

        pd.context = nullptr;
        pd.backBuffer = nullptr;
        pd.backBufferDS = nullptr;

        // bgfx Initialisierung
        bgfx::Init init;
        init.type = bgfx::RendererType::Count; // Automatische Auswahl
        init.vendorId = BGFX_PCI_ID_SOFTWARE_RASTERIZER; // Fallback
        init.deviceId = 0;
        init.debug = true;
        init.profile = true;
        init.platformData = pd;

        if (!bgfx::init(init)) {
            throw std::runtime_error("Failed to initialize bgfx");
        }

        // View erstellen
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
        bgfx::setViewRect(0, 0, 0, m_window.getWidth(), m_window.getHeight());
    }

    Window m_window;
};

} // namespace renderbridge

int main(int argc, char* argv[]) {
    renderbridge::RendererApp app;
    app.run();
    return 0;
}
```

## Lizenz-Pfad

GLFW verwendet die **zlib/Libpng License**, die vollständig kompatibel mit unserer MIT-Lizenz ist:

```
Copyright (c) 2002-2006 Marcus Geelnard
Copyright (c) 2006-2016 Camilla Berglund

This software is provided 'as-is', without any express or implied warranty. In no
 event will the authors be held liable for any damages arising from the use of
 this software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to
the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim
   that you wrote the original software. If you use this software in a product,
   an acknowledgment in the product documentation would be appreciated but is
   not required.

2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
```

Diese Lizenz:
- Erlaubt kommerzielle Nutzung
- Erlaubt Modifikationen
- Erlaubt Weiterverbreitung
- Erfordert nur, dass der Ursprung nicht falsch dargestellt wird
- Ist vollständig kompatibel mit MIT und BSD-2-Clause

## Links

- [GLFW Website](https://www.glfw.org/)
- [GLFW GitHub](https://github.com/glfw/glfw)
- [GLFW Documentation](https://www.glfw.org/documentation.html)
- [GLFW CMake](https://github.com/glfw/glfw/tree/master/CMake)
- [bgfx + GLFW Beispiele](https://github.com/bkaradzic/bgfx/tree/master/examples)
- [zlib/Libpng License](https://opensource.org/licenses/Zlib)

## Alternative: SDL2

Falls es Probleme mit GLFW gibt oder spezifische SDL2-Features benötigt werden (z.B. Audio in der Zukunft), kann einfach zu SDL2 gewechselt werden. Die Architektur bleibt gleich, nur die Fenster-Verwaltungsklasse muss angepasst werden.

Die Entscheidung für GLFW ist jedoch gut begründet und sollte für die meisten Use-Cases ausreichen.
