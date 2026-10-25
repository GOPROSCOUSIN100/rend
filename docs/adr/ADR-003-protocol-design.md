# ADR-003: Binäres Protokoll-Design

## Status
✅ Akzeptiert

## Kontext

Das Bridge-Protokoll muss folgende Anforderungen erfüllen:

- **Effizienz**: Geringer Overhead für hohe Frame-Raten
- **Versionierbar**: Unterstützung für zukünftige Versionen
- **Plattformunabhängig**: Gleiche Darstellung auf allen Systemen
- **Fehlertolerant**: Erkennung von korrupten Daten
- **Erweiterbar**: Einfaches Hinzufügen neuer Message-Typen

Design-Entscheidungen:
1. Binär vs Text (JSON/XML/Protobuf)
2. Endianness (Little-Endian vs Big-Endian)
3. Message-Struktur (Header + Payload)
4. Serialisierungs-Methode

## Entscheidung

**Binäres Protokoll mit Big-Endian, Fixed-Header + Variable Payload**

### Protokoll-Struktur

```
┌─────────────────────────────────────────────────────────────┐
│                    Message Format                              │
├─────────────────────────────────────────────────────────────┤
│  Header (16 Bytes)  │  Payload (variabel)                      │
│────────────────────│───────────────────────────────────────│
│                     │                                           │
│  0-3: messageId     │  Message-spezifische Daten              │
│  4-7: payloadSize   │  (siehe Message-Typ Definitionen)        │
│  8:   version       │                                           │
│  9:   flags         │                                           │
│  10-11: checksum    │  (optional, CRC32)                       │
│  12-15: sequence    │                                           │
└────────────────────┴───────────────────────────────────────┘
```

### Message-Typen Hierarchie

```
Message (Base)
├── Control Messages (0x0000-0x0FFF)
│   ├── MSG_HANDSHAKE (0x0001)
│   ├── MSG_HEARTBEAT (0x0002)
│   ├── MSG_ACK (0x0003)
│   ├── MSG_ERROR (0x0004)
│   └── MSG_CONTROL (0x0005)
│
├── Frame Messages (0x0100-0x01FF)
│   ├── MSG_FRAME_START (0x0100)
│   └── MSG_FRAME_END (0x0101)
│
├── Data Messages (0x0200-0x02FF)
│   ├── MSG_CAMERA (0x0200)
│   ├── MSG_CHUNK (0x0201)
│   ├── MSG_ENTITY (0x0202)
│   ├── MSG_PARTICLE (0x0203)
│   ├── MSG_WEATHER (0x0204)
│   └── MSG_GUI (0x0205)
│
└── Reserved (0x0300-0xFFFF)
```

### Message-Definitionen

#### MSG_HANDSHAKE (0x0001)

```
Header (16 Bytes)
├── messageId: 0x0001
├── payloadSize: 72 + clientName.length
├── version: 1
├── flags: 0
└── sequence: 0

Payload:
├── protocolVersion: uint8 (1)
├── capabilities: uint32 (Bitmask)
│   ├── BIT_0: Shared Memory Support
│   ├── BIT_1: Socket Support
│   ├── BIT_2: Compression Support
│   └── BIT_3: Encryption Support
├── clientType: uint8 (0=Mod, 1=Renderer)
├── clientVersion: uint32 (Semantic Version: 0xMMmmpp00)
├── clientName: string (UTF-8, length-prefixed with uint8)
└── padding: variable (to align to 8 bytes)
```

#### MSG_CAMERA (0x0200)

```
Header (16 Bytes)
├── messageId: 0x0200
├── payloadSize: 172
├── version: 1
├── flags: 0
└── sequence: incrementing

Payload:
├── frameNumber: uint32
├── timestamp: uint64 (nanoseconds)
├── position: float[3] (x, y, z)
├── rotation: float[3] (pitch, yaw, roll in radians)
├── fov: float (in degrees)
├── viewMatrix: float[16] (column-major)
├── projectionMatrix: float[16] (column-major)
├── viewport: uint16[4] (x, y, width, height)
└── flags: uint8 (bit flags for camera state)
```

#### MSG_CHUNK (0x0201)

```
Header (16 Bytes)
├── messageId: 0x0201
├── payloadSize: 20 + blockData.length + lightData.length
├── version: 1
├── flags: compressionFlag (0=None, 1=Zstd)
└── sequence: incrementing

Payload:
├── chunkX: int32
├── chunkY: int8
├── chunkZ: int32
├── sectionMask: uint16 (bitmask of which 16-block sections are present)
├── blockData: byte[] (compressed if flag set)
│   Format: [blockId: uint16][blockMeta: uint8] per block
│   Total: 16*16*16 * 3 bytes = 12KB uncompressed per section
├── lightData: byte[] (compressed if flag set)
│   Format: [skyLight: uint8][blockLight: uint8] per block
│   Total: 16*16*16 * 2 bytes = 8KB uncompressed per section
└── metadata: byte[] (optional, for biome data, etc.)
```

#### MSG_ENTITY (0x0202)

```
Header (16 Bytes)
├── messageId: 0x0202
├── payloadSize: 44 + metadata.length
├── version: 1
├── flags: 0
└── sequence: incrementing

Payload:
├── entityId: uint32
├── entityType: uint16 (Minecraft entity type ID)
├── position: float[3] (x, y, z)
├── rotation: float[3] (pitch, yaw, roll in radians)
├── velocity: float[3] (x, y, z)
├── modelId: uint16 (reference to model in resource pack)
├── metadata: byte[] (NBT-like data, length-prefixed)
└── flags: uint8 (bit flags for entity state)
```

### Serialisierungs-Regeln

1. **Endianness**: Big-Endian (Network Byte Order)
   - Rational: Plattformunabhängig, Standard für Netzwerk-Protokolle

2. **Primitive Typen**:
   - uint8: 1 Byte, unsigned
   - int8: 1 Byte, signed (two's complement)
   - uint16: 2 Bytes, unsigned
   - int16: 2 Bytes, signed
   - uint32: 4 Bytes, unsigned
   - int32: 4 Bytes, signed
   - float: 4 Bytes, IEEE 754 single-precision
   - double: 8 Bytes, IEEE 754 double-precision

3. **Strings**:
   - UTF-8 Kodierung
   - Length-Prefixed mit uint8 oder uint16 (je nach max Länge)
   - Null-Terminator nicht nötig

4. **Arrays**:
   - Length-Prefixed mit uint16
   - Elemente direkt hintereinander

5. **Padding**:
   - Für Alignment auf 4 oder 8 Bytes
   - Mit Null-Bytes füllen

### Versionierung

- **Protokoll-Version**: 1 Byte im Header
- **Message-Version**: 1 Byte in jedem Message-Payload
- **Version-Aushandlung**: Während Handshake

### Fehlererkennung

1. **Checksum**: Optional CRC32 im Header (flags basiert)
2. **Size Validation**: payloadSize muss mit Message-Typ übereinstimmen
3. **Version Check**: Version muss unterstützt werden
4. **Sequence Numbers**: Lücken oder Duplikate erkennen

## Alternativen

### Alternative 1: JSON
- **Vorteile**: Menschlich lesbar, einfach zu debuggen
- **Nachteile**: Sehr langsame Serialisierung/Deserialisierung, hoher Speicherbedarf
- **Verworfen**: Performance-Anforderungen nicht erfüllbar

### Alternative 2: Protocol Buffers
- **Vorteile**: Effizient, gut unterstützt
- **Nachteile**: Externe Abhängigkeit, Code-Generierung nötig
- **Verworfen**: Wir wollen keine externen Abhängigkeiten für das Kern-Protokoll

### Alternative 3: FlatBuffers
- **Vorteile**: Zero-Copy, sehr effizient
- **Nachteile**: Komplexe Schema-Definition, Code-Generierung
- **Verworfen**: Zu komplex für den Anfang, kann später evaluiert werden

### Alternative 4: Little-Endian
- **Vorteile**: Natürliche Byte-Order auf x86
- **Nachteile**: Nicht plattformunabhängig (ARM ist Big-Endian)
- **Verworfen**: Big-Endian ist Standard für Netzwerk-Protokolle

## Konsequenzen

### Positive Konsequenzen:
- **Performance**: Binäre Serialisierung ist schnell
- **Größe**: Kompakte Darstellung (kein Text-Overhead)
- **Versionierbar**: Klare Versionierung im Protokoll
- **Erweiterbar**: Neue Message-Typen können leicht hinzugefügt werden

### Negative Konsequenzen:
- **Debugging**: Binäre Daten sind schwerer zu debuggen
- **Komplexität**: Serialisierung/Deserialisierung muss manuell implementiert werden
- **Wartung**: Änderungen am Protokoll erfordern Versionierung

### Technische Anforderungen:
- ByteBuffer für Serialisierung/Deserialisierung
- CRC32 für Checksum (java.util.zip.CRC32)
- Endianness-Kontrolle (ByteBuffer.order(ByteOrder.BIG_ENDIAN))

## Implementierungs-Beispiel

```java
// Serialisierung
public class MessageSerializer {
    public static ByteBuffer serialize(Message message) {
        ByteBuffer buffer = ByteBuffer.allocate(message.getSize())
            .order(ByteOrder.BIG_ENDIAN);
        
        // Header
        buffer.putInt(message.getMessageId());
        buffer.putInt(message.getPayloadSize());
        buffer.put(message.getVersion());
        buffer.put(message.getFlags());
        buffer.putShort(message.getChecksum());
        buffer.putInt(message.getSequence());
        
        // Payload (message-spezifisch)
        message.writePayload(buffer);
        
        buffer.flip();
        return buffer;
    }
}

// Deserialisierung
public class MessageDeserializer {
    public static Message deserialize(ByteBuffer buffer) {
        buffer.order(ByteOrder.BIG_ENDIAN);
        
        // Header lesen
        int messageId = buffer.getInt();
        int payloadSize = buffer.getInt();
        byte version = buffer.get();
        byte flags = buffer.get();
        short checksum = buffer.getShort();
        int sequence = buffer.getInt();
        
        // Message-Typ bestimmen und Payload lesen
        Message message = MessageFactory.create(messageId, version);
        message.readPayload(buffer, payloadSize);
        
        return message;
    }
}
```

## Links
- [Java ByteBuffer Documentation](https://docs.oracle.com/javase/8/docs/api/java/nio/ByteBuffer.html)
- [Network Byte Order (RFC 1700)](https://tools.ietf.org/html/rfc1700)
- [CRC32 Algorithm](https://en.wikipedia.org/wiki/Cyclic_redundancy_check)
