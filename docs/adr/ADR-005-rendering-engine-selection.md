# ADR-005: Rendering Engine Selection - bgfx

## Status
✅ Akzeptiert

## Kontext

Für die Implementierung des externen Renderers (Prozess B) gab es mehrere Optionen:

1. **LWJGL 2 (Java)**: Ursprünglich geplant, aber eingeschränkte Möglichkeiten
2. **LWJGL 3 (Java)**: Moderne API, aber immer noch Java-basiert
3. **OpenGL direkt (C/C++)**: Volle Kontrolle, aber plattformspezifisch
4. **Vulkan direkt (C/C++)**: Beste Performance, aber komplex
5. **bgfx (C++)**: Cross-Platform Rendering-Abstraktion mit Multi-Backend-Support
6. **Andere Engines**: Unity, Unreal, Godot - zu schwer für dieses Projekt

### Anforderungen an die Rendering-Engine:

- **Cross-Platform**: Windows, Linux, macOS
- **Multi-Backend**: Unterstützung für D3D11/12, OpenGL, Vulkan, Metal
- **Moderne Features**: Compute Shaders, Multi-Render-Targets, etc.
- **Einfache Integration**: Gute Dokumentation und Community
- **Lizenz**: Kompatibel mit MIT-Lizenz (keine GPL)
- **Performance**: Geringer Overhead
- **Shader-Pipeline**: Integrierte Shader-Kompilierung

## Entscheidung

**bgfx (https://github.com/bkaradzic/bgfx) als Rendering-Engine für den externen Renderer**

### Begründung:

#### Vorteile von bgfx:

1. **Cross-Platform mit Multi-Backend**:
   - Unterstützt D3D11, D3D12, OpenGL (2.1+, 3.x, 4.x), OpenGLES, Vulkan, Metal
   - Automatische Backend-Auswahl basierend auf Plattform
   - Graceful Fallback wenn ein Backend nicht verfügbar ist

2. **Einfaches API-Design**:
   - State-Sortierung automatisch
   - Vertex/Index-Buffer Management
   - Uniform-Handling
   - Texture-Management
   - Shader-Management

3. **Shader-Pipeline**:
   - Integrierter Shader-Compiler (shaderc)
   - Cross-Platform Shader (gleiche Shader für alle Backends)
   - Vertex/Fragment/Compute Shader Support
   - Preprocessors für verschiedene Backends

4. **Lizenz**:
   - BSD-2-Clause Lizenz - sehr permissiv
   - Kompatibel mit MIT-Lizenz
   - Keine Copyleft-Anforderungen
   - Kommerzielle Nutzung erlaubt

5. **Performance**:
   - Minimaler Overhead
   - Batch-Rendering
   - Efficient State Management

6. **Community & Support**:
   - Aktive Entwicklung
   - Gute Dokumentation
   - Viele Beispiele
   - Große Community

7. **Integration**:
   - CMake-Unterstützung
   - Git-Submodule möglich
   - Keine Abhängigkeiten auf System-Bibliotheken (außer Grafik-Treibern)

#### Vergleich mit Alternativen:

| Engine | Backends | Lizenz | API | Shader | Integration |
|--------|----------|--------|-----|--------|-------------|
| LWJGL 2 | OpenGL 2.1 | BSD | Java | GLSL | Einfach |
| LWJGL 3 | OpenGL 3+, Vulkan | BSD | Java | GLSL | Mittel |
| OpenGL | Plattformspezifisch | - | C/C++ | GLSL | Komplex |
| Vulkan | Vulkan | - | C/C++ | SPIR-V | Sehr komplex |
| **bgfx** | **D3D11/12, OpenGL, Vulkan, Metal** | **BSD-2** | **C++** | **Cross-Platform** | **CMake** |

### Nachteile von bgfx:

1. **C++ statt Java**: Erfordert separate Codebasis
2. **Build-Komplexität**: CMake + bgfx-Build
3. **Größere Binärdateien**: bgfx enthält alle Backends
4. **Lernkurve**: Neues API zu lernen

## Konsequenzen

### Positive Konsequenzen:

- **Plattformunabhängigkeit**: Funktioniert auf allen wichtigen Plattformen
- **Zukunftssicherheit**: Unterstützt moderne Grafik-APIs
- **Performance**: Optimiert für Spiele
- **Flexibilität**: Einfaches Wechseln zwischen Backends
- **Lizenzkompatibilität**: BSD-2 ist perfekt für unser Projekt

### Negative Konsequenzen:

- **Separate Codebasis**: Java-Mod und C++-Renderer müssen separat gebaut werden
- **Build-Komplexität**: CMake + Gradle in einem Projekt
- **Debugging**: C++-Debugging kann schwieriger sein

### Technische Anforderungen:

- C++17 Compiler (für bgfx)
- CMake 3.15+
- Git (für Submodule)
- Python (für Shader-Compiler)
- Grafik-Treiber für gewünschte Backends

## Implementierungs-Strategie

### Projekt-Struktur:

```
rend/
├── bridge-protocol/          # Java - Protokoll (unverändert)
├── mc-client-mod/            # Java - Minecraft Mod (unverändert)
├── external-renderer-cpp/    # NEU: C++ - bgfx-basierter Renderer
│   ├── CMakeLists.txt
│   ├── src/
│   │   ├── main.cpp
│   │   ├── bridge_client.hpp
│   │   ├── renderer.hpp
│   │   ├── scene/
│   │   │   ├── camera.hpp
│   │   │   ├── chunk.hpp
│   │   │   └── entity.hpp
│   │   ├── shaders/
│   │   │   ├── vs_voxel.sc
│   │   │   └── fs_voxel.sc
│   │   └── adapter/
│   │       └── protocol_adapter.hpp
│   ├── cmake/
│   │   └── Findbgfx.cmake
│   └── 3rdparty/
│       └── bgfx/           # Git Submodule
│
└── docs/
    └── THIRD-PARTY-NOTICES.md  # NEU: Lizenz-Informationen
```

### Bindungs-Strategie:

1. **bgfx als Git-Submodule**:
   ```bash
   git submodule add https://github.com/bkaradzic/bgfx external-renderer-cpp/3rdparty/bgfx
   git submodule update --init --recursive
   ```

2. **CMake Integration**:
   ```cmake
   # In external-renderer-cpp/CMakeLists.txt
   add_subdirectory(3rdparty/bgfx)
   target_link_libraries(renderer PRIVATE bgfx)
   ```

3. **Shader-Pipeline**:
   - Shader als `.sc` Dateien (bgfx Shader Language)
   - Build-Skript kompiliert Shader zu `.bin` Dateien
   - Separate `.bin` Dateien für jedes Backend

### Adapter-Layer:

```cpp
// protocol_adapter.hpp
namespace renderbridge {
    
    // Konvertiert Bridge-Protokoll Daten zu bgfx-Daten
    class ProtocolAdapter {
    public:
        // Konvertiert CameraMessage zu bgfx View
        static bgfx::ViewId createView(const CameraMessage& camera);
        
        // Konvertiert ChunkMessage zu bgfx Vertex/Index Buffers
        static MeshHandle createMesh(const ChunkMessage& chunk);
        
        // Konvertiert EntityMessage zu bgfx Transform
        static void updateEntity(EntityHandle entity, const EntityMessage& msg);
        
        // Konvertiert WeatherMessage zu bgfx Uniforms
        static void updateWeatherUniforms(const WeatherMessage& weather);
    };
}
```

### Fenster-Bibliothek:

bgfx benötigt eine Fenster-Bibliothek. Optionen:
- **SDL2**: Gut unterstützt, viele Beispiele
- **GLFW**: Leichtgewichtig, gut für OpenGL
- **Native**: Plattformspezifisch

→ Siehe **ADR-006** für die Auswahl.

## Lizenz-Pfad

### bgfx Lizenz (BSD-2-Clause):

```
Copyright (c) 2011-2024, Branimir Karadzic

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
```

### 3rdparty-Lizenzen:

bgfx bündelt mehrere 3rdparty-Bibliotheken im `3rdparty/` Ordner:
- **bx**: BSD-2-Clause (Branimir Karadzic)
- **bimg**: BSD-2-Clause (Branimir Karadzic)
- **astc**: Apache 2.0 (ARM Limited)
- **etc1**: Public Domain
- **etc2**: BSD-2-Clause
- **iqa**: MIT (Intel)
- **...**: Siehe bgfx 3rdparty/ Ordner

→ Alle müssen in **THIRD-PARTY-NOTICES.md** dokumentiert werden.

## Links

- [bgfx GitHub](https://github.com/bkaradzic/bgfx)
- [bgfx Documentation](https://bkaradzic.github.io/bgfx/)
- [bgfx Examples](https://github.com/bkaradzic/bgfx/tree/master/examples)
- [bgfx Shader Language](https://bkaradzic.github.io/bgfx/shader.html)
- [bgfx CMake Integration](https://github.com/bkaradzic/bgfx/wiki/CMake)
- [BSD-2-Clause License](https://opensource.org/licenses/BSD-2-Clause)
