# ADR-002: Transportmedium für Bridge-Kommunikation

## Status
✅ Akzeptiert

## Kontext

Die Bridge zwischen Client-Mod (Prozess A) und Externem Renderer (Prozess B) benötigt ein effizientes Transportmedium. Die Anforderungen sind:

- **Geringe Latenz**: < 1ms für 60 FPS
- **Hoher Durchsatz**: Mehrere MB/s für Frame-Daten
- **Zuverlässigkeit**: Kein Datenverlust
- **Portabilität**: Funktioniert auf Windows, Linux, macOS
- **Einfachheit**: Einfache Implementierung und Debugging

Mögliche Transportmedien:

| Medium | Typ | Latenz | Durchsatz | Portabilität | Komplexität |
|--------|-----|---------|-----------|--------------|-------------|
| Shared Memory (Memory-Mapped File) | IPC | ~0.1ms | Sehr hoch | Mittel | Hoch |
| Local Socket (TCP/IP) | Netzwerk | ~0.5ms | Hoch | Hoch | Niedrig |
| Named Pipe | IPC | ~0.2ms | Mittel | Niedrig | Mittel |
| Unix Domain Socket | IPC | ~0.2ms | Hoch | Niedrig | Niedrig |
| Java RMI | Java-spezifisch | ~1ms | Mittel | Hoch | Hoch |

## Entscheidung

**Shared Memory (Memory-Mapped File) als Primär-Transport, Local Socket als Fallback**

### Implementierungs-Strategie:

```java
// Transport Interface
public interface TransportChannel {
    void connect() throws IOException;
    void disconnect();
    void send(ByteBuffer data) throws IOException;
    ByteBuffer receive() throws IOException;
    boolean isConnected();
    TransportType getType();
}

// Shared Memory Implementierung
public class SharedMemoryChannel implements TransportChannel {
    private FileChannel fileChannel;
    private MappedByteBuffer mappedBuffer;
    private RingBuffer ringBuffer;
    // ...
}

// Socket Fallback
public class LocalSocketChannel implements TransportChannel {
    private Socket socket;
    private DataInputStream input;
    private DataOutputStream output;
    // ...
}

// Factory für Transport-Auswahl
public class TransportFactory {
    public static TransportChannel createPrimary() {
        try {
            return new SharedMemoryChannel();
        } catch (Exception e) {
            return new LocalSocketChannel();
        }
    }
}
```

### Ring-Buffer Design für Shared Memory:

```
┌─────────────────────────────────────────────────────────────┐
│                    Shared Memory Layout                        │
├─────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────────┬──────────────┬──────────────────────────┐ │
│  │   Header     │   Buffer A    │         Buffer B           │ │
│  │              │              │                          │ │
│  │  writePos:4  │  [FrameData]  │      [FrameData]           │ │
│  │  readPos:4   │              │                          │ │
│  │  version:1   │              │                          │ │
│  │  flags:1    │              │                          │ │
│  │  padding:2  │              │                          │ │
│  └──────────────┴──────────────┴──────────────────────────┘ │
│                                                                  │
│  Buffer Größe: 16MB (konfigurierbar)                            │
│  Frame Daten: ~1-2MB pro Frame (komprimiert)                   │
│                                                                  │
└─────────────────────────────────────────────────────────────┘
```

## Alternativen

### Alternative 1: Nur Local Socket
- **Vorteile**: Einfacher zu implementieren, bessere Portabilität
- **Nachteile**: Höhere Latenz, Serialisierung/Deserialisierung nötig
- **Verworfen**: Performance-Anforderungen nicht optimal erfüllbar

### Alternative 2: Nur Shared Memory
- **Vorteile**: Beste Performance
- **Nachteile**: Plattform-spezifische Probleme (Windows vs Unix)
- **Verworfen**: Fallback für Debugging und plattformspezifische Issues nötig

### Alternative 3: Unix Domain Socket + Named Pipe
- **Vorteile**: Gute Performance auf Unix
- **Nachteile**: Keine Windows-Unterstützung für Unix Domain Sockets
- **Verworfen**: Portabilität nicht gegeben

## Konsequenzen

### Positive Konsequenzen:
- **Performance**: Shared Memory bietet die beste Performance
- **Zuverlässigkeit**: Memory-Mapped Files sind zuverlässig
- **Flexibilität**: Fallback auf Socket für Debugging
- **Portabilität**: Java NIO funktioniert plattformübergreifend

### Negative Konsequenzen:
- **Komplexität**: Ring-Buffer Implementierung für Shared Memory
- **Speicher-Verwaltung**: Memory-Mapped Files müssen korrekt verwaltet werden
- **Synchronisation**: Thread-Safety muss gewährleistet sein

### Technische Anforderungen:
- Java NIO (Standard in Java 8)
- FileChannel und MappedByteBuffer
- AtomicInteger für Position-Pointer
- Volatile für Synchronisation

## Links
- [Java NIO Documentation](https://docs.oracle.com/javase/8/docs/api/java/nio/package-summary.html)
- [Memory-Mapped Files Tutorial](https://docs.oracle.com/javase/8/docs/technotes/guides/io/mmap.html)
- [Ring Buffer Pattern](https://en.wikipedia.org/wiki/Circular_buffer)
