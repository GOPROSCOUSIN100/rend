# ADR-004: Build-System Auswahl

## Status
✅ Akzeptiert

## Kontext

Für das Projekt benötigen wir Build-Systeme für:
1. **Bridge-Protokoll**: Standard Java-Bibliothek
2. **Client-Mod**: Forge-Mod für Minecraft 1.8.9
3. **Externer Renderer**: Standard Java-Anwendung mit LWJGL 2

Anforderungen:
- Java 8 Kompatibilität
- Forge 1.8.9 Unterstützung
- LWJGL 2 Integration
- Multi-Modul Build
- Einfache Abhängigkeitsverwaltung

Mögliche Optionen:

| System | Typ | Forge-Support | LWJGL-Support | Komplexität |
|--------|-----|---------------|----------------|-------------|
| ForgeGradle | Gradle | ✅ Ja | ✅ Ja | Hoch |
| RetroFuturaGradle | Gradle | ✅ Ja | ✅ Ja | Mittel |
| Vanilla Gradle | Gradle | ❌ Nein | ✅ Ja | Niedrig |
| Maven | Maven | ❌ Nein | ✅ Ja | Mittel |

## Entscheidung

**RetroFuturaGradle für Client-Mod, Standard Gradle für Bridge und Renderer**

### Begründung:

1. **ForgeGradle 2.1 Probleme**: 
   - Veraltet und hat bekannte Issues mit neueren Gradle-Versionen
   - Komplexe Konfiguration
   - RetroFuturaGradle ist ein moderner Fork mit Bugfixes

2. **RetroFuturaGradle Vorteile**:
   - Aktive Wartung (für Legacy Forge-Versionen)
   - Einfacheres Setup
   - Bessere Performance
   - Unterstützung für MCP-Mappings

3. **Standard Gradle für andere Module**:
   - Einfacher und leichter
   - Keine Forge-spezifischen Anforderungen
   - Bessere Kontrolle

### Build-Struktur

```
rend/
├── settings.gradle                    # Root settings
├── build.gradle                      # Root build (common config)
│
├── bridge-protocol/
│   ├── build.gradle                  # Standard Gradle
│   └── gradle.properties
│
├── mc-client-mod/
│   ├── build.gradle                  # RetroFuturaGradle
│   └── gradle.properties
│
└── external-renderer/
    ├── build.gradle                  # Standard Gradle
    └── gradle.properties
```

### Root build.gradle

```gradle
// Common configuration for all subprojects
allprojects {
    apply plugin: 'java'
    
    sourceCompatibility = JavaVersion.VERSION_1_8
    targetCompatibility = JavaVersion.VERSION_1_8
    
    repositories {
        mavenCentral()
        maven { url 'https://jitpack.io' }
    }
}

// Dependency versions
ext {
    lwjglVersion = '2.9.4'
    junitVersion = '4.13.2'
    slf4jVersion = '1.7.36'
}
```

### Bridge-Protokoll build.gradle

```gradle
plugins {
    id 'java'
}

dependencies {
    // No external dependencies for core protocol
    testImplementation "junit:junit:${junitVersion}"
}
```

### Client-Mod build.gradle

```gradle
buildscript {
    repositories {
        maven { url 'https://files.minecraftforge.net/maven' }
        jcenter()
        mavenCentral()
    }
    dependencies {
        classpath 'com.retrofutura.gradle:retrofuturagradle:1.0.0'
    }
}

apply plugin: 'com.retrofutura.mcp'
apply plugin: 'java'

mcp {
    mappings 'stable_22' // MCP 9.10 mappings for 1.8.9
    forge {
        version '1.8.9-11.15.1.2318-1.8.9'
        mappings 'stable_22'
    }
}

dependencies {
    compileOnly 'net.minecraftforge:forge:1.8.9-11.15.1.2318-1.8.9'
    
    // Bridge protocol dependency
    implementation project(':bridge-protocol')
    
    // Mixin for deep hooks
    implementation 'org.spongepowered:mixin:0.8.5'
}
```

### Externer Renderer build.gradle

```gradle
plugins {
    id 'java'
    id 'application'
}

application {
    mainClass = 'renderbridge.renderer.Main'
}

dependencies {
    // LWJGL 2
    implementation "org.lwjgl.lwjgl:lwjgl:2.9.4"
    implementation "org.lwjgl.lwjgl:lwjgl_util:2.9.4"
    
    // Bridge protocol dependency
    implementation project(':bridge-protocol')
    
    // Logging
    implementation "org.slf4j:slf4j-api:${slf4jVersion}"
    implementation "org.slf4j:slf4j-simple:${slf4jVersion}"
}
```

## Alternativen

### Alternative 1: ForgeGradle für Client-Mod
- **Vorteile**: Offiziell von Forge unterstützt
- **Nachteile**: Veraltet, bekannte Bugs, komplexe Konfiguration
- **Verworfen**: RetroFuturaGradle ist besser gewartet

### Alternative 2: Maven für alle Module
- **Vorteile**: Standardisiert, gute IDE-Integration
- **Nachteile**: Weniger flexibel, Forge-Support schwierig
- **Verworfen**: Gradle ist besser für Multi-Modul Builds

### Alternative 3: Ein einziges Gradle-Projekt
- **Vorteile**: Einfacheres Dependency-Management
- **Nachteile**: Unterschiedliche Build-Anforderungen (Forge vs Standard)
- **Verworfen**: Getrennte Builds sind klarer

## Konsequenzen

### Positive Konsequenzen:
- **Forge-Kompatibilität**: RetroFuturaGradle funktioniert gut mit 1.8.9
- **Flexibilität**: Getrennte Build-Konfigurationen für verschiedene Anforderungen
- **Wartbarkeit**: Klare Trennung der Module
- **Performance**: Optimierte Builds für jeden Modul-Typ

### Negative Konsequenzen:
- **Komplexität**: Unterschiedliche Build-Systeme in einem Projekt
- **Lernkurve**: RetroFuturaGradle ist weniger bekannt
- **Wartung**: Mehrere Build-Skripte zu pflegen

### Technische Anforderungen:
- Gradle 6.x+ (für RetroFuturaGradle)
- Java 8 JDK
- Internet-Verbindung für Abhängigkeiten

## Links
- [RetroFuturaGradle GitHub](https://github.com/RetroFutura/RetroFuturaGradle)
- [ForgeGradle Documentation](https://mcforge.readthedocs.io/en/latest/gettingstarted/)
- [Gradle Documentation](https://docs.gradle.org/6.9/userguide/userguide.html)
- [LWJGL 2 Maven Repository](https://legacy.lwjgl.org/)
