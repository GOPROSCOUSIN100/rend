# Architektur-Dokumentation

Dieses Dokument beschreibt die detaillierte Architektur des **rend** Projekts für die Auslagerung des Minecraft 1.8.9 Renderers.

## Inhaltsverzeichnis
1. [Komponenten-Übersicht](#komponenten-übersicht)
2. [Datenfluss pro Frame](#datenfluss-pro-frame)
3. [Protokoll-Entwurf](#protokoll-entwurf)
4. [Fehlerbehandlung](#fehlerbehandlung)
5. [Latenz-Budget](#latenz-budget)
6. [Sicherheitsaspekte](#sicherheitsaspekte)

---

## Komponenten-Übersicht

### 1. Client-Mod (Prozess A)

**Verantwortlichkeit**: Hooken der Minecraft Render-Pipeline und Extrahieren von Frame-Daten

#### Subkomponenten:

```
┌─────────────────────────────────────────────────────────────┐
│                     Client Mod (Prozess A)                      │
├─────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌─────────────────┐    ┌─────────────────┐    ┌─────────┐ │
│  │  Render Hooks    │    │  Data Extractors │    │  Bridge  │ │
│  │                 │    │                 │    │  Client  │ │
│  │ • EntityRenderer │───▶│ • CameraData    │───▶│ • Sender │ │
│  │ • RenderGlobal   │    │ • ChunkData     │    │ • Queue  │ │
│  │ • Framebuffer    │    │ • EntityData    │    │ • Sync   │ │
│  │ • GUI            │    │ • ParticleData  │    │         │ │
│  └─────────────────┘    │ • WeatherData   │    └─────────┘ │
│                         └─────────────────┘                 │
│                                                                  │
│  ┌─────────────────┐                                         │
│  │  Fallback        │                                         │
│  │  Renderer        │                                         │
│  │                 │                                         │
│  │ • Overlay       │◀─────────────────────────────────────┘
│  │ • Stats Display  │
│  └─────────────────┘
└─────────────────────────────────────────────────────────────┘
```

#### Render Hooks

| Hook-Punkt | Klasse | Methode | Zweck |
|------------|--------|---------|-------|
| Kamera | `EntityRenderer` | `updateCameraAndRender` | Kamera-Matrizen extrahieren |
| Welt | `RenderGlobal` | `renderEntities` | Entity-Meshes extrahieren |
| Welt | `RenderGlobal` | `renderBlockLayer` | Chunk/Block-Meshes extrahieren |
| Partikel | `EffectRenderer` | `updateEffects` | Partikel-Daten extrahieren |
| GUI | `GuiIngame` | `renderGameOverlay` | GUI-Status extrahieren |

#### Daten-Extraktoren

Jeder Extractor ist für einen spezifischen Datentyp verantwortlich:

- **CameraExtractor**: Position, Rotation, FOV, View-Matrix, Projection-Matrix
- **ChunkExtractor**: Geladene Chunks, Block-IDs, Block-States, Licht-Daten
- **EntityExtractor**: Entity-Positionen, Rotationen, Modelle, Texturen
- **ParticleExtractor**: Partikel-Positionen, Typen, Lebensdauer
- **WeatherExtractor**: Regen/Schnee-Status, Intensität

#### Bridge Client

- **Sender**: Serialisiert Frame-Daten und sendet über Transport
- **Queue**: Ring-Buffer für Frame-Daten (Doppelte Pufferung)
- **Sync**: Synchronisation mit Renderer (Frame-Nummern)

### 2. Bridge-Protokoll (Transportschicht)

**Verantwortlichkeit**: Effiziente, versionierte Kommunikation zwischen Client und Renderer

#### Schichten-Architektur

```
┌─────────────────────────────────────────────────────────────┐
│                    Bridge Protocol Stack                       │
├─────────────────────────────────────────────────────────────┤
│  Application Layer  │  Message Layer  │  Transport Layer       │
│────────────────────│────────────────│───────────────────────│
│                     │                │                           │
│  FrameData         │  MessageHeader │  SharedMemoryChannel    │
│  CameraData        │  MessageType   │  SocketChannel          │
│  ChunkData         │  Payload       │  RingBuffer             │
│  EntityData        │  Checksum      │  MemoryMappedFile       │
│  ControlMessages   │                │  LocalSocket            │
└────────────────────┴────────────────┴───────────────────────┘
```

#### Transport-Implementierungen

| Transport | Typ | Latenz | Durchsatz | Portabilität | Komplexität |
|-----------|-----|---------|-----------|--------------|-------------|
| Shared Memory | Memory-Mapped File | ~0.1ms | Sehr hoch | Mittel | Hoch |
| Local Socket | TCP/IP (localhost) | ~0.5ms | Hoch | Hoch | Niedrig |
| Pipe | Named Pipe | ~0.2ms | Mittel | Niedrig | Mittel |

**Entscheidung**: Shared Memory als Primär-Transport, Local Socket als Fallback

- **Begründung**:
  - Shared Memory bietet die beste Performance (geringste Latenz)
  - Keine Serialisierung/Deserialisierung auf Netzwerk-Ebene nötig
  - Memory-Mapped Files sind plattformübergreifend (Java NIO)
  - Local Socket als Fallback für Debugging und plattformspezifische Issues

### 3. Externer Renderer (Prozess B)

**Verantwortlichkeit**: Empfangen und Rendern der Szene

#### Subkomponenten:

```
┌─────────────────────────────────────────────────────────────┐
│                  External Renderer (Prozess B)                 │
├─────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌─────────────────┐    ┌─────────────────┐    ┌─────────┐ │
│  │  Bridge Client   │    │   Scene Graph    │    │  GL      │ │
│  │                  │    │                 │    │  Renderer│ │
│  │ • Receiver       │───▶│ • CameraNode    │───▶│ • Shaders│ │
│  │ • Deserializer   │    │ • ChunkNode     │    │ • VBOs   │ │
│  │ • Queue          │    │ • EntityNode    │    │ • Textures││
│  └─────────────────┘    │ • ParticleNode  │    │ • FBOs   │ │
│                         └─────────────────┘    └─────────┘ │
│                                                                  │
│  ┌─────────────────┐                                         │
│  │  Window & Input   │                                         │
│  │                  │                                         │
│  │ • LWJGL Context  │                                         │
│  │ • GLFW Window    │                                         │
│  │ • Input Handling │                                         │
│  └─────────────────┘                                         │
└─────────────────────────────────────────────────────────────┘
```

#### GL Renderer

- **Shaders**: GLSL 1.20 (OpenGL 2.1 Kompatibilität für LWJGL 2)
- **VBOs**: Vertex Buffer Objects für Meshes
- **Textures**: Dynamisches Laden aus Ressourcenpaketen
- **FBOs**: Frame Buffer Objects für Post-Processing

---

## Datenfluss pro Frame

### Zeitliche Ablauf (Timing Diagram)

```
Client Mod              Bridge              Renderer
   │                      │                    │
   ├─ Frame Start ────────┼───────────────────▶│
   │                      │                    │
   │  Hook Render Pipeline │                    │
   │  Extract Camera Data  │                    │
   │  Extract Chunk Data   │                    │
   │  Extract Entity Data  │                    │
   │                      │                    │
   ├─ Serialize Frame ────┼───────────────────▶│
   │                      │ Deserialize Frame   │
   │                      │                    │
   │  Send via Transport  │                    │
   │◀─────────────────────┼─ ACK               │
   │                      │                    │
   │  Update Fallback     │                    │
   │  (if no ACK)         │                    │
   │                      │                    │
   │                      │◀─ Render Frame ───┤
   │                      │                    │
   │  Next Frame          │                    │
   │                      │                    │
```

### Frame-Datenstruktur

Jeder Frame besteht aus mehreren Message-Typen:

```
Frame (Logische Einheit)
├── FrameHeader
│   ├── frameNumber: uint32
│   ├── timestamp: uint64
│   ├── messageCount: uint16
│   └── flags: uint8
│
├── CameraMessage
│   ├── position: float[3] (x, y, z)
│   ├── rotation: float[3] (pitch, yaw, roll)
│   ├── fov: float
│   ├── viewMatrix: float[16]
│   ├── projectionMatrix: float[16]
│   └── viewport: uint16[4] (x, y, width, height)
│
├── ChunkMessage (pro geladenem Chunk)
│   ├── chunkX: int32
│   ├── chunkY: int8
│   ├── chunkZ: int32
│   ├── blockData: byte[] (komprimiert)
│   ├── lightData: byte[] (komprimiert)
│   └── flags: uint8
│
├── EntityMessage (pro Entity)
│   ├── entityId: uint32
│   ├── entityType: uint16
│   ├── position: float[3]
│   ├── rotation: float[3]
│   ├── velocity: float[3]
│   ├── modelId: uint16
│   └── metadata: byte[]
│
├── ParticleMessage (pro Partikel-System)
│   ├── particleType: uint16
│   ├── count: uint16
│   ├── positions: float[]
│   ├── velocities: float[]
│   └── lifetimes: float[]
│
└── WeatherMessage
    ├── weatherType: uint8 (0=clear, 1=rain, 2=snow)
    ├── intensity: float
    └── flags: uint8
```

### Byte-Layout Beispiel: CameraMessage

```
Offset  Bytes  Typ          Feld               Beschreibung
──────  ─────  ───────────  ─────────────────  ─────────────────────────
0       4      uint32       messageId          Message-Typ Identifier
4       4      uint32       messageSize        Größe der Message
8       1      uint8        version            Protokoll-Version
9       3      padding      -                  Reserviert
12      8      uint64       timestamp          Zeitstempel (Nanos)
20      4      uint32       frameNumber        Frame-Nummer
24      12     float[3]     position           Kamera-Position (x,y,z)
36      12     float[3]     rotation           Rotation (pitch,yaw,roll)
48      4      float        fov                Field of View
52      64     float[16]    viewMatrix         View-Matrix (4x4)
116     64     float[16]    projectionMatrix  Projection-Matrix (4x4)
180     8      uint16[4]    viewport           Viewport (x,y,w,h)

Gesamt: 188 Bytes
```

---

## Protokoll-Entwurf

### Message-Header (Alle Messages)

```
0       4      uint32       messageId     Message-Typ (siehe unten)
4       4      uint32       payloadSize   Größe des Payloads
8       1      uint8        version       Protokoll-Version (aktuell: 1)
9       1      uint8        flags        Message-Flags
10      2      padding      -             Reserviert

Gesamt: 16 Bytes Header
```

### Message-Typen

| ID | Name | Beschreibung | Priorität |
|----|------|--------------|-----------|
| 0x0001 | `MSG_HANDSHAKE` | Handshake und Version-Aushandlung | Hoch |
| 0x0002 | `MSG_HEARTBEAT` | Keep-Alive Signal | Niedrig |
| 0x0003 | `MSG_ACK` | Bestätigung für empfangene Message | Hoch |
| 0x0100 | `MSG_FRAME_START` | Beginn eines neuen Frames | Hoch |
| 0x0101 | `MSG_FRAME_END` | Ende eines Frames | Hoch |
| 0x0200 | `MSG_CAMERA` | Kamera-Daten | Hoch |
| 0x0201 | `MSG_CHUNK` | Chunk-Daten | Mittel |
| 0x0202 | `MSG_ENTITY` | Entity-Daten | Mittel |
| 0x0203 | `MSG_PARTICLE` | Partikel-Daten | Niedrig |
| 0x0204 | `MSG_WEATHER` | Wetter-Daten | Niedrig |
| 0x0205 | `MSG_GUI` | GUI-Status | Niedrig |
| 0x0300 | `MSG_CONTROL` | Steuerungs-Nachrichten | Hoch |
| 0xFFFF | `MSG_ERROR` | Fehler-Meldung | Hoch |

### Handshake-Protokoll

```
Client → Server:
  MSG_HANDSHAKE {
    version: uint8 = 1
    capabilities: uint32 (Bitmask)
      BIT_0: Shared Memory Support
      BIT_1: Socket Support
      BIT_2: Compression Support
      BIT_3: Encryption Support
    clientName: string (max 64 bytes)
  }

Server → Client:
  MSG_HANDSHAKE {
    version: uint8 (akzeptierte Version)
    selectedTransport: uint8
      0 = Shared Memory
      1 = Local Socket
    serverName: string (max 64 bytes)
  }

Falls Version nicht unterstützt:
  MSG_ERROR {
    errorCode: uint32
    errorMessage: string
  }
```

### Heartbeat-Mechanismus

- **Intervall**: 1 Sekunde
- **Timeout**: 3 Sekunden ohne Heartbeat = Verbindung als tot betrachten
- **Payload**: 
  ```
  0       4      uint32       sequenceNumber
  4       8      uint64       lastFrameProcessed
  12      4      float        avgProcessingTimeMs
  ```

### Fehlerbehandlung im Protokoll

- **Checksum**: CRC32 für jede Message (optional, konfigurierbar)
- **Sequence Numbers**: Jede Message hat eine Sequenz-Nummer
- **ACK**: Bestätigung für kritische Messages (Frame-Start/End)
- **Retry**: Automatischer Retry nach Timeout (max 3 Versuche)

### Binäre Serialisierung

- **Endianness**: Big-Endian (Netzwerk-Byte-Order)
- **Strings**: UTF-8, Length-Prefixed (uint16 für Länge)
- **Arrays**: Length-Prefixed (uint16 für Länge)
- **Floats**: IEEE 754 Single-Precision
- **Integers**: Signed/Unsigned wie angegeben

---

## Fehlerbehandlung

### Fehler-Kategorien

| Kategorie | Beschreibung | Behandlung |
|-----------|--------------|------------|
| **Transient** | Temporäre Probleme (Timeout, Netzwerk) | Retry mit Backoff |
| **Protocol** | Protokoll-Verletzungen | Verbindung zurücksetzen |
| **Version** | Inkompatible Versionen | Handshake fehlgeschlagen |
| **Resource** | Speicher/CPU Überlastung | Graceful Degradation |
| **Fatal** | Unerholbare Fehler | Prozess neustarten |

### Fehler-Propagation

```
Client Mod Fehler:
  ├─ Transient → Retry (max 3x)
  ├─ Protocol → Reconnect
  ├─ Version → Benutzer informieren
  └─ Fatal → Fallback-Rendering aktivieren

Renderer Fehler:
  ├─ Transient → Retry (max 3x)
  ├─ Protocol → Reconnect
  ├─ Version → Beenden mit Fehlermeldung
  └─ Fatal → Sauber beenden
```

### Fallback-Strategien

1. **Keine Verbindung**: Zeige Statistiken und Status an
2. **Teilweise Daten**: Rendere was verfügbar ist
3. **Verzögerte Daten**: Zeige letzten bekannten Frame
4. **Performance-Probleme**: Reduziere Qualitätsstufen

---

## Latenz-Budget

### Ziel: < 16.67ms pro Frame (60 FPS)

| Komponente | Budget | Optimierung |
|------------|--------|-------------|
| Daten-Extraktion | 2ms | Effiziente Hooks, Caching |
| Serialisierung | 1ms | Binäres Protokoll, Zero-Copy |
| Transport | 0.5ms | Shared Memory |
| Deserialisierung | 1ms | Binäres Protokoll, Zero-Copy |
| Rendering | 10ms | Optimierte Shaders, LOD |
| Overhead | 1.17ms | Puffer |
| **Gesamt** | **15.67ms** | |

### Messung

- **Frame-Time**: Zeit von Frame-Start bis Frame-End
- **Bridge-Latenz**: Zeit von Send bis ACK
- **Processing-Time**: Zeit von Receive bis Render-Complete
- **End-to-End**: Frame-Time + Bridge-Latenz + Processing-Time

### Optimierungs-Maßnahmen

1. **Doppelte Pufferung**: Client schreibt in Puffer A, Renderer liest aus Puffer B
2. **Zero-Copy**: Memory-Mapped Files direkt in OpenGL-Texturen
3. **Kompression**: Optional für große Chunk-Daten (Zstd)
4. **LOD (Level of Detail)**: Entfernte Chunks mit weniger Detail
5. **Culling**: Nicht sichtbare Objekte nicht senden

---

## Sicherheitsaspekte

### Anti-Cheat-Kompatibilität

**Risiko**: Der Mod kann auf Servern als Cheat erkannt werden, weil:
- Er greift auf interne Minecraft-Daten zu
- Er modifiziert die Render-Pipeline
- Er kommuniziert mit einem externen Prozess

**Mitigation**:
- Keine Spiel-Logik ändern (nur Rendering)
- Keine Input-Simulation
- Keine Server-Kommunikation
- Klare Dokumentation für Server-Admins

### Datenschutz

- **Keine Benutzerdaten**: Keine Minecraft-Konten, Sessions, etc.
- **Lokale Kommunikation**: Nur localhost, keine Remote-Verbindungen
- **Kein Tracking**: Keine Telemetrie oder Analytics

### Speicher-Sicherheit

- **Memory-Mapped Files**: Berechtigungen auf User-only setzen
- **Socket-Bindings**: Nur auf localhost binden
- **Input-Validation**: Alle empfangenen Daten validieren

---

## Performance-Metriken

### Gemessene Werte

| Metrik | Ziel | Warnung | Kritisch |
|--------|------|---------|----------|
| Frame-Time | < 16.67ms | < 20ms | > 33ms |
| Bridge-Latenz | < 1ms | < 2ms | > 5ms |
| Memory-Usage | < 512MB | < 1GB | > 1.5GB |
| CPU-Usage | < 50% | < 70% | > 90% |
| FPS | > 55 | > 45 | < 30 |

### Monitoring

- **Logs**: Frame-Zeiten, Latenzen, Fehler
- **Stats-Overlay**: Echtzeit-Anzeige im Fallback
- **Performance-Tests**: Automatisierte Benchmarks

---

## Anhang

### Glossar

| Begriff | Definition |
|---------|------------|
| **Frame** | Ein komplett gerendertes Bild |
| **Chunk** | 16x16x256 Block-Volumen in Minecraft |
| **Entity** | Dynamisches Objekt (Spieler, Mobs, Items) |
| **LOD** | Level of Detail - Detailgrad basierend auf Entfernung |
| **FBO** | Frame Buffer Object - Offscreen-Rendering |
| **VBO** | Vertex Buffer Object - GPU-Speicher für Geometrie |
| **NIO** | New I/O - Java Non-blocking I/O API |

### Referenzen

- [Minecraft 1.8.9 Forge Documentation](https://mcforge.readthedocs.io/)
- [LWJGL 2 Documentation](https://legacy.lwjgl.org/)
- [Java NIO Documentation](https://docs.oracle.com/javase/8/docs/api/java/nio/package-summary.html)
- [OpenGL 2.1 Specification](https://www.khronos.org/opengl/)

### Revision History

| Version | Datum | Autor | Änderungen |
|---------|-------|-------|-----------|
| 1.0 | 2024-XX-XX | Vibe Code | Initialer Entwurf |
