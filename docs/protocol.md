# Bridge Protocol Specification

This document provides a detailed specification of the RenderBridge protocol used for communication between the Minecraft client mod and the external renderer.

## Table of Contents
1. [Overview](#overview)
2. [Transport Layer](#transport-layer)
3. [Message Format](#message-format)
4. [Message Types](#message-types)
5. [Handshake Protocol](#handshake-protocol)
6. [Heartbeat Protocol](#heartbeat-protocol)
7. [Frame Synchronization](#frame-synchronization)
8. [Error Handling](#error-handling)
9. [Version Compatibility](#version-compatibility)

---

## Overview

The RenderBridge protocol is a binary protocol designed for low-latency, high-throughput communication between the Minecraft client mod (Process A) and the external renderer (Process B).

### Design Principles

- **Binary**: All data is transmitted in binary format for efficiency
- **Big-Endian**: Network byte order (Big-Endian) is used for all multi-byte values
- **Versioned**: Protocol version is included in every message for compatibility
- **Extensible**: New message types can be added without breaking existing implementations
- **Reliable**: Message sequencing and acknowledgment ensure reliable delivery

### Protocol Stack

```
┌─────────────────────────────────────────────────────────────┐
│                    RenderBridge Protocol Stack                 │
├─────────────────────────────────────────────────────────────┤
│  Application Layer  │  Message Layer  │  Transport Layer       │
│────────────────────│────────────────│───────────────────────│
│                     │                │                           │
│  Frame Data        │  Message Types │  Shared Memory         │
│  Camera Data       │  Serialization  │  Ring Buffer            │
│  Chunk Data        │  Versioning     │  Memory-Mapped Files    │
│  Entity Data       │                │                           │
│  Control Messages  │                │  OR                    │
│                     │                │                           │
│                     │                │  Local Socket           │
│                     │                │  TCP/IP (localhost)      │
│                     │                │  Message Framing         │
└────────────────────┴────────────────┴───────────────────────┘
```

---

## Transport Layer

### Shared Memory Transport

**Primary Transport Method**

- **Mechanism**: Memory-mapped files with ring buffer
- **Latency**: ~0.1ms
- **Throughput**: Very high (limited by memory bandwidth)
- **Platform Support**: Windows, Linux, macOS (via Java NIO)

#### Shared Memory Layout

```
┌─────────────────────────────────────────────────────────────┐
│                    Shared Memory File                          │
├─────────────────────────────────────────────────────────────┤
│                                                                  │
│  Header (16 bytes)                                              │
│  ┌─────────────────────────────────────────────────────────┐│
│  │ writePos: uint32 (4 bytes)   │ readPos: uint32 (4 bytes)   ││
│  │ version: uint8 (1 byte)     │ flags: uint8 (1 byte)       ││
│  │ padding: uint16 (2 bytes)                                  ││
│  └─────────────────────────────────────────────────────────┘│
│                                                                  │
│  Data Buffer (16MB - 16 bytes)                                 │
│  ┌─────────────────────────────────────────────────────────┐│
│  │ [Message 1 Size][Message 1 Data][Message 2 Size]...      ││
│  │ ...[Message N Data][Free Space]                          ││
│  └─────────────────────────────────────────────────────────┘│
│                                                                  │
└─────────────────────────────────────────────────────────────┘
```

#### Ring Buffer Operation

- **Write Position**: Points to the next byte to write
- **Read Position**: Points to the next byte to read
- **Wrap-Around**: Positions wrap around at buffer end
- **Message Framing**: Each message is prefixed with its size (4 bytes)

### Socket Transport

**Fallback Transport Method**

- **Mechanism**: TCP/IP on localhost
- **Latency**: ~0.5ms
- **Throughput**: High
- **Platform Support**: All platforms with Java support

#### Socket Message Framing

Each message is prefixed with its total size:

```
┌─────────────────────────────────────────────────────────────┐
│  Message Size (4 bytes, uint32, Big-Endian)                    │
├─────────────────────────────────────────────────────────────┤
│  Message Data (variable length)                               │
└─────────────────────────────────────────────────────────────┘
```

#### Default Socket Configuration

- **Host**: 127.0.0.1
- **Port**: 25565
- **TCP NoDelay**: Enabled
- **KeepAlive**: Enabled
- **Timeout**: 1000ms

---

## Message Format

### Message Header

All messages start with a 16-byte header:

```
Offset  Bytes  Typ          Feld               Beschreibung
──────  ─────  ───────────  ─────────────────  ─────────────────────────
0       4      uint32       messageId          Message type identifier
4       4      uint32       payloadSize        Size of the payload in bytes
8       1      uint8        version            Protocol version (currently 1)
9       1      uint8        flags             Message flags
10      2      padding      -                  Reserved for future use
12      2      uint16       checksum           CRC16 checksum (optional)
14      4      uint32       sequence          Sequence number

Total: 16 bytes
```

### Message Flags

| Flag | Value | Description |
|------|-------|-------------|
| NONE | 0x00 | No flags |
| COMPRESSED | 0x01 | Payload is compressed |
| ENCRYPTED | 0x02 | Payload is encrypted |
| REQUIRES_ACK | 0x04 | Message requires acknowledgment |

### Message Types

Message types are 16-bit unsigned integers organized into categories:

| Range | Category | Description |
|-------|----------|-------------|
| 0x0000-0x0FFF | Control Messages | Protocol control and management |
| 0x0100-0x01FF | Frame Messages | Frame synchronization |
| 0x0200-0x02FF | Data Messages | Scene data |
| 0x0300-0xFFFF | Reserved | For future use |

---

## Message Types

### Control Messages (0x0000-0x0FFF)

#### MSG_HANDSHAKE (0x0001)

Used for protocol version negotiation and capability exchange during connection establishment.

**Payload:**
```
Offset  Bytes  Typ          Feld               Beschreibung
──────  ─────  ───────────  ─────────────────  ─────────────────────────
0       1      uint8        protocolVersion    Protocol version
1       4      uint32       capabilities       Bitmask of capabilities
5       4      uint32       clientType         0=Mod, 1=Renderer
9       4      uint32       clientVersion      Version of the client
13      1      uint8        clientNameLength   Length of client name
14      var    string       clientName         Client name (UTF-8)
```

**Capabilities Bitmask:**
- BIT_0 (0x01): Shared Memory Support
- BIT_1 (0x02): Socket Support
- BIT_2 (0x04): Compression Support
- BIT_3 (0x08): Encryption Support

#### MSG_HEARTBEAT (0x0002)

Keep-alive message sent periodically to ensure the connection is still active.

**Payload:**
```
Offset  Bytes  Typ          Feld               Beschreibung
──────  ─────  ───────────  ─────────────────  ─────────────────────────
0       4      uint32       sequenceNumber     Sequence number
4       8      uint64       lastFrameProcessed  Last processed frame number
12      4      float        avgProcessingTimeMs Average processing time in ms
```

**Timing:**
- **Interval**: 1 second
- **Timeout**: 3 seconds (if no heartbeat received, connection is considered dead)

#### MSG_ACK (0x0003)

Acknowledgment message for confirming receipt of a message.

**Payload:**
```
Offset  Bytes  Typ          Feld               Beschreibung
──────  ─────  ───────────  ─────────────────  ─────────────────────────
0       4      uint32       acknowledgedSequence  Sequence number being acknowledged
4       4      uint32       acknowledgedMessageId Message ID being acknowledged
```

#### MSG_ERROR (0x0004)

Error message for reporting protocol or connection errors.

**Payload:**
```
Offset  Bytes  Typ          Feld               Beschreibung
──────  ─────  ───────────  ─────────────────  ─────────────────────────
0       4      uint32       errorCode          Error code
4       1      uint8        errorMessageLength Length of error message
5       var    string       errorMessage       Error message (UTF-8)
```

**Error Codes:**
- 0: Unknown error
- 1: Protocol version mismatch
- 2: Invalid message type or format
- 3: Checksum mismatch
- 4: Connection timeout
- 5: Transport layer error
- 6: Unsupported feature
- 7: Resource exhausted

#### MSG_CONTROL (0x0005)

Control message for sending commands between client and renderer.

**Payload:**
```
Offset  Bytes  Typ          Feld               Beschreibung
──────  ─────  ───────────  ─────────────────  ─────────────────────────
0       1      uint8        command            Control command
1       4      uint32       parameter1         First parameter
5       4      uint32       parameter2         Second parameter
9       4      float        parameterFloat     Float parameter
```

**Control Commands:**
- 0: PAUSE
- 1: RESUME
- 2: RESET
- 3: SET_QUALITY
- 4: SET_RENDER_MODE
- 5: SET_FOV
- 6: REQUEST_SYNC
- 7: PING
- 8: PONG

#### MSG_ECHO (0x0006)

Echo message for roundtrip testing and latency measurement.

**Payload:**
```
Offset  Bytes  Typ          Feld               Beschreibung
──────  ─────  ───────────  ─────────────────  ─────────────────────────
0       8      uint64       timestamp          Timestamp in nanoseconds
8       4      uint32       sequence           Sequence number
12      1      uint8        dataLength         Length of echo data
13      var    bytes        data               Echo data
```

### Frame Messages (0x0100-0x01FF)

#### MSG_FRAME_START (0x0100)

Indicates the start of a new frame.

**Payload:**
```
Offset  Bytes  Typ          Feld               Beschreibung
──────  ─────  ───────────  ─────────────────  ─────────────────────────
0       8      uint64       frameNumber        Frame number
8       8      uint64       timestamp          Timestamp in nanoseconds
16      4      uint32       flags              Frame flags
```

#### MSG_FRAME_END (0x0101)

Indicates the end of a frame.

**Payload:**
```
Offset  Bytes  Typ          Feld               Beschreibung
──────  ─────  ───────────  ─────────────────  ─────────────────────────
0       8      uint64       frameNumber        Frame number
8       8      uint64       timestamp          Timestamp in nanoseconds
16      4      uint32       messageCount       Number of messages in this frame
20      8      uint64       processingTimeNanos Processing time in nanoseconds
```

### Data Messages (0x0200-0x02FF)

#### MSG_CAMERA (0x0200)

Contains camera data for rendering.

**Payload:**
```
Offset  Bytes  Typ          Feld               Beschreibung
──────  ─────  ───────────  ─────────────────  ─────────────────────────
0       8      uint64       frameNumber        Frame number
8       8      uint64       timestamp          Timestamp in nanoseconds
16      12     float[3]     position           Camera position (x, y, z)
28      12     float[3]     rotation           Camera rotation in radians (pitch, yaw, roll)
40      4      float        fov                Field of view in degrees
44      4      float        nearPlane          Near clipping plane
48      4      float        farPlane           Far clipping plane
52      64     float[16]    viewMatrix         View matrix (4x4, column-major)
116     64     float[16]    projectionMatrix  Projection matrix (4x4, column-major)
180     16     uint16[4]    viewport           Viewport (x, y, width, height)
196     4      uint32       flags              Camera flags

Total: 200 bytes
```

**Camera Flags:**
- 0x01: Third person view
- 0x02: Inverse view
- 0x04: Underwater
- 0x08: In portal

#### MSG_CHUNK (0x0201)

Contains chunk data for rendering.

**Payload:**
```
Offset  Bytes  Typ          Feld               Beschreibung
──────  ─────  ───────────  ─────────────────  ─────────────────────────
0       4      int32        chunkX            Chunk X position
4       1      int8         chunkY            Chunk Y position
5       4      int32        chunkZ            Chunk Z position
9       2      uint16       sectionMask       Bitmask of present sections
11      1      uint8        compression        Compression type (0=None, 1=Zstd)
12      1      uint8        flags             Chunk flags
13      2      uint16       blockDataSize      Size of block data
15      var    bytes        blockData          Block data
var     2      uint16       lightDataSize      Size of light data
var+2   var    bytes        lightData          Light data
var     2      uint16       biomeDataSize      Size of biome data
var+2   var    bytes        biomeData          Biome data
```

**Block Data Format:**
- Each block: 3 bytes = [blockId: uint16][blockMeta: uint8]
- Total per section (16x16x16): 12,288 bytes

**Light Data Format:**
- Each light: 2 bytes = [skyLight: uint8][blockLight: uint8]
- Total per section: 8,192 bytes

**Biome Data Format:**
- Each biome: 1 byte
- Total per chunk (16x16): 256 bytes

#### MSG_ENTITY (0x0202)

Contains entity data for rendering.

**Payload:**
```
Offset  Bytes  Typ          Feld               Beschreibung
──────  ─────  ───────────  ─────────────────  ─────────────────────────
0       4      uint32       entityId          Entity ID
4       2      uint16       entityType        Entity type ID
6       2      uint16       modelId           Model ID
8       12     float[3]     position           Position (x, y, z)
20      12     float[3]     rotation           Rotation in radians (pitch, yaw, roll)
32      12     float[3]     velocity           Velocity (x, y, z)
44      2      uint16       metadataSize      Size of metadata
46      var    bytes        metadata           Entity metadata
```

#### MSG_PARTICLE (0x0203)

Contains particle data for rendering.

**Payload:**
```
Offset  Bytes  Typ          Feld               Beschreibung
──────  ─────  ───────────  ─────────────────  ─────────────────────────
0       2      uint16       particleType       Particle type ID
2       2      uint16       particleCount      Number of particles
4       var    float[]      positionsX         X positions (float[particleCount])
var     var    float[]      positionsY         Y positions (float[particleCount])
var     var    float[]      positionsZ         Z positions (float[particleCount])
var     var    float[]      velocitiesX        X velocities (float[particleCount])
var     var    float[]      velocitiesY        Y velocities (float[particleCount])
var     var    float[]      velocitiesZ        Z velocities (float[particleCount])
var     var    float[]      lifetimes          Lifetimes (float[particleCount])
var     var    float[]      scales             Scales (float[particleCount])
```

**Particle Types:**
- 0: EXPLOSION_NORMAL
- 1: EXPLOSION_LARGE
- 2: EXPLOSION_HUGE
- 3: FIREWORKS_SPARK
- 4: WATER_BUBBLE
- 5: WATER_SPLASH
- 6: WATER_WAKE
- 7: WATER_DROP
- 8: BLOCK_DUST
- 9: BLOCK_CRACK
- 10: BLOCK_BREAK
- 11: SMOKE_NORMAL
- 12: SMOKE_LARGE
- ...

#### MSG_WEATHER (0x0204)

Contains weather data for rendering.

**Payload:**
```
Offset  Bytes  Typ          Feld               Beschreibung
──────  ─────  ───────────  ─────────────────  ─────────────────────────
0       1      uint8        weatherType        Weather type (0=Clear, 1=Rain, 2=Snow, 3=Thunderstorm)
1       4      float        intensity          Weather intensity (0.0 to 1.0)
5       4      float        transitionProgress Transition progress (0.0 to 1.0)
9       4      uint32       flags             Weather flags
13      8      uint64       durationTicks      Duration in ticks
```

**Weather Flags:**
- 0x01: Raining
- 0x02: Thundering
- 0x04: Snowing

#### MSG_GUI (0x0205)

Contains GUI/HUD data for rendering.

**Payload:**
```
Offset  Bytes  Typ          Feld               Beschreibung
──────  ─────  ───────────  ─────────────────  ─────────────────────────
0       1      uint8        elementType        GUI element type
1       4      float        value              Current value
5       4      float        maxValue          Maximum value
9       1      uint8        flags             Element flags
10      1      uint8        textLength        Length of text
11      var    string       text               Text (UTF-8)
```

**GUI Element Types:**
- 0: HEALTH
- 1: ARMOR
- 2: HUNGER
- 3: EXPERIENCE
- 4: AIR
- 5: HOTBAR
- 6: CROSSHAIR
- 7: SCOREBOARD
- 8: CHAT
- 9: DEBUG

---

## Handshake Protocol

### Connection Establishment

1. **Client (Mod) → Server (Renderer)**: Send MSG_HANDSHAKE
   - Protocol version
   - Capabilities
   - Client type (0 = Mod)
   - Client version
   - Client name

2. **Server → Client**: Send MSG_HANDSHAKE (response)
   - Protocol version (must match or be lower)
   - Capabilities
   - Client type (1 = Renderer)
   - Client version
   - Client name

3. **If protocol version mismatch**: Send MSG_ERROR and disconnect

4. **If capabilities don't match**: Use highest common capability

### Handshake Example

```
Client: MSG_HANDSHAKE {
    protocolVersion: 1
    capabilities: CAPABILITY_SHARED_MEMORY | CAPABILITY_SOCKET
    clientType: 0 (Mod)
    clientVersion: 0x00010000
    clientName: "RenderBridgeMod"
}

Server: MSG_HANDSHAKE {
    protocolVersion: 1
    capabilities: CAPABILITY_SHARED_MEMORY | CAPABILITY_SOCKET
    clientType: 1 (Renderer)
    clientVersion: 0x00010000
    clientName: "RenderBridgeRenderer"
}
```

---

## Heartbeat Protocol

### Heartbeat Exchange

- **Interval**: 1 second
- **Timeout**: 3 seconds

```
Client → Server: MSG_HEARTBEAT {
    sequenceNumber: 1
    lastFrameProcessed: 100
    avgProcessingTimeMs: 16.67
}

Server → Client: MSG_HEARTBEAT {
    sequenceNumber: 1
    lastFrameProcessed: 100
    avgProcessingTimeMs: 1.0
}
```

### Connection Monitoring

- If no heartbeat received within 3 seconds, connection is considered dead
- Connection state is updated to ERROR
- Reconnection is attempted automatically

---

## Frame Synchronization

### Frame Sequence

1. **Frame Start**: MSG_FRAME_START indicates the beginning of a new frame
2. **Data Messages**: Multiple MSG_CAMERA, MSG_CHUNK, MSG_ENTITY, etc.
3. **Frame End**: MSG_FRAME_END indicates the end of the frame

### Frame Example

```
MSG_FRAME_START {
    frameNumber: 100
    timestamp: 1234567890
    flags: 0
}

MSG_CAMERA {
    frameNumber: 100
    position: (10, 5, 20)
    rotation: (0, 90, 0)
    fov: 70
    viewMatrix: [...]
    projectionMatrix: [...]
    viewport: (0, 0, 800, 600)
}

MSG_CHUNK {
    chunkX: 0
    chunkY: 0
    chunkZ: 0
    sectionMask: 0xFFFF
    blockData: [...]
    lightData: [...]
}

MSG_ENTITY {
    entityId: 1
    entityType: 0 (Player)
    position: (10, 5, 20)
    rotation: (0, 90, 0)
    velocity: (0, 0, 0)
}

MSG_FRAME_END {
    frameNumber: 100
    timestamp: 1234567891
    messageCount: 3
    processingTimeNanos: 16000000 (16ms)
}
```

---

## Error Handling

### Error Reporting

When an error occurs, the affected party sends a MSG_ERROR message:

```
MSG_ERROR {
    errorCode: 1 (Protocol version mismatch)
    errorMessage: "Protocol version mismatch: client=1, server=2"
}
```

### Error Recovery

- **Transient Errors**: Retry with exponential backoff (max 3 attempts)
- **Protocol Errors**: Reconnect with fresh handshake
- **Version Errors**: Disconnect and notify user
- **Fatal Errors**: Disconnect and require manual restart

---

## Version Compatibility

### Version Negotiation

1. Client sends its protocol version in MSG_HANDSHAKE
2. Server compares with its protocol version
3. If versions match, connection proceeds
4. If versions don't match:
   - If server version > client version: Server sends MSG_ERROR
   - If client version > server version: Client receives MSG_ERROR

### Backward Compatibility

- Protocol version 1 is the baseline
- Future versions must maintain backward compatibility for critical messages
- New message types can be added without breaking existing implementations
- Existing message types should not change their payload format

### Version History

| Version | Description | Date |
|---------|-------------|------|
| 1 | Initial version | 2024-XX-XX |

---

## Performance Considerations

### Latenz Budget

| Component | Budget | Notes |
|-----------|--------|-------|
| Data Extraction | 2ms | Client-side |
| Serialization | 1ms | Both sides |
| Transport | 0.5ms | Shared Memory |
| Deserialization | 1ms | Renderer-side |
| Rendering | 10ms | Renderer-side |
| **Total** | **15.5ms** | For 60 FPS |

### Optimization Techniques

1. **Double Buffering**: Client writes to buffer A while renderer reads from buffer B
2. **Zero-Copy**: Memory-mapped files allow direct access without copying
3. **Message Batching**: Multiple messages can be sent in a single frame
4. **Compression**: Optional compression for large chunk data
5. **LOD (Level of Detail)**: Reduce detail for distant objects
6. **Culling**: Don't send data for objects not visible to the camera

---

## Security Considerations

### Transport Security

- **Shared Memory**: File permissions set to user-only
- **Socket**: Only binds to localhost (127.0.0.1)
- **No Remote Connections**: Protocol is designed for local use only

### Data Validation

- All incoming messages are validated for:
  - Message type validity
  - Payload size correctness
  - Version compatibility
  - Sequence number continuity

### Error Handling

- Malformed messages are rejected
- Invalid data is logged and ignored
- Connection is terminated on protocol violations

---

## Implementation Notes

### Java Implementation

```java
// Sending a message
Message message = new CameraMessage();
// ... populate message ...
ByteBuffer serialized = message.serialize();
transportChannel.send(serialized);

// Receiving a message
ByteBuffer received = transportChannel.receive();
Message message = Message.deserialize(received);
if (message.getMessageId() == MessageType.CAMERA) {
    CameraMessage camera = (CameraMessage) message;
    // ... process camera ...
}
```

### Endianness

All multi-byte values are stored in Big-Endian (Network Byte Order):

```java
ByteBuffer buffer = ByteBuffer.allocate(1024);
buffer.order(ByteOrder.BIG_ENDIAN);
```

### Serialization

```java
// Writing to buffer
buffer.putInt(value);
buffer.putFloat(value);
buffer.putShort(value);

// Reading from buffer
int value = buffer.getInt();
float value = buffer.getFloat();
short value = buffer.getShort();
```

---

## Appendix

### Message Size Summary

| Message Type | Header Size | Payload Size | Total Size |
|--------------|-------------|--------------|------------|
| MSG_HANDSHAKE | 16 | 72-136 | 88-152 |
| MSG_HEARTBEAT | 16 | 16 | 32 |
| MSG_ACK | 16 | 8 | 24 |
| MSG_ERROR | 16 | 5-260 | 21-276 |
| MSG_CONTROL | 16 | 13 | 29 |
| MSG_ECHO | 16 | 13-268 | 29-284 |
| MSG_FRAME_START | 16 | 20 | 36 |
| MSG_FRAME_END | 16 | 32 | 48 |
| MSG_CAMERA | 16 | 184 | 200 |
| MSG_CHUNK | 16 | 20-131072 | 36-131088 |
| MSG_ENTITY | 16 | 54-4100 | 70-4116 |
| MSG_PARTICLE | 16 | 14-16402 | 30-16418 |
| MSG_WEATHER | 16 | 21 | 37 |
| MSG_GUI | 16 | 12-267 | 28-283 |

### Glossary

| Term | Definition |
|------|------------|
| Big-Endian | Byte order where most significant byte comes first |
| CRC | Cyclic Redundancy Check - error detection algorithm |
| Frame | A complete set of scene data for one render cycle |
| LOD | Level of Detail - reducing complexity based on distance |
| MSB | Most Significant Byte |
| NIO | New I/O - Java's non-blocking I/O API |
| Payload | The data portion of a message (excluding header) |
| Sequence Number | A unique number identifying the order of messages |
