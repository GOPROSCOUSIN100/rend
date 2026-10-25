# ADR-001: Hook-Methode für Render-Pipeline

## Status
✅ Akzeptiert

## Kontext

Für das Hooken der Minecraft 1.8.9 Render-Pipeline gibt es mehrere Ansätze:

1. **Event-Bus (Forge Events)**: Nutzung der bestehenden Forge-Event-Systeme
2. **Reflection**: Dynamisches Aufrufen privater Methoden via Reflection
3. **Bytecode-Agent (CoreMod)**: Modifikation des Bytecodes zur Laufzeit
4. **Mixin**: Nutzung des Mixin-Frameworks für Bytecode-Transformation

Minecraft 1.8.9 mit Forge bietet verschiedene Hook-Punkte:
- `EntityRenderer.updateCameraAndRender(float partialTicks)` - Haupt-Render-Methode
- `RenderGlobal.renderEntities(EntityRenderer, float, int)` - Entity-Rendering
- `RenderGlobal.renderBlockLayer(BlockLayer, float, int)` - Block-Rendering

## Entscheidung

**Nutzung von Forge Events + Mixin für tiefe Hooks**

### Primär: Forge Events
- `RenderGameOverlayEvent` für GUI-Overlay
- `RenderWorldEvent` für Welt-Rendering
- `RenderEntityEvent` für Entity-Rendering

### Sekundär: Mixin für tiefe Integration
Für Methoden, die nicht über Events zugänglich sind:
- `EntityRenderer.updateCameraAndRender`
- `RenderGlobal.renderChunkLayer`

### Implementierungs-Strategie:

```java
// 1. Event-Handler registrieren
@SubscribeEvent
public void onRenderWorld(RenderWorldLastEvent event) {
    // Chunk/Entity-Daten extrahieren
}

// 2. Mixin für EntityRenderer
@Mixin(EntityRenderer.class)
public abstract class MixinEntityRenderer {
    @Inject(method = "updateCameraAndRender", at = @At("HEAD"))
    private void onUpdateCameraAndRender(float partialTicks, callbackInfo ci) {
        // Kamera-Daten extrahieren
    }
}
```

## Alternativen

### Alternative 1: Reine Reflection
- **Vorteile**: Einfach zu implementieren, keine Build-Komplexität
- **Nachteile**: Langsam, fragil (bricht bei Forge-Updates), schlechte Performance
- **Verworfen**: Performance-Anforderungen nicht erfüllbar

### Alternative 2: Reiner Bytecode-Agent (CoreMod)
- **Vorteile**: Maximale Kontrolle, beste Performance
- **Nachteile**: Sehr komplex, schwer zu debuggen, Forge-spezifische Integration
- **Verworfen**: Zu komplex für den Anfang, Mixin bietet ähnliche Vorteile

### Alternative 3: Nur Forge Events
- **Vorteile**: Einfach, Forge-kompatibel
- **Nachteile**: Nicht alle Hook-Punkte verfügbar, eingeschränkte Kontrolle
- **Verworfen**: Nicht ausreichend für tiefgreifende Änderungen

## Konsequenzen

### Positive Konsequenzen:
- **Forge-kompatibel**: Nutzt bestehende Forge-Infrastruktur
- **Performance**: Mixin bietet gute Performance (nahe an nativ)
- **Flexibilität**: Kombination aus Events und Mixin für maximale Abdeckung
- **Wartbarkeit**: Klare Trennung zwischen Event-Handlern und Mixins

### Negative Konsequenzen:
- **Build-Komplexität**: Mixin erfordert zusätzliche Build-Konfiguration
- **Forge-Abhängigkeit**: Events sind Forge-spezifisch
- **Lernkurve**: Mixin erfordert Verständnis von Bytecode-Transformation

### Technische Anforderungen:
- ForgeGradle oder RetroFuturaGradle für Mixin-Unterstützung
- MCP-Mappings für 1.8.9
- Mixin-Bibliothek als Abhängigkeit

## Links
- [Forge Documentation - Events](https://mcforge.readthedocs.io/en/1.8.9/events/)
- [Mixin Tutorials](https://fabricmc.net/wiki/start)
- [RetroFuturaGradle](https://github.com/RetroFutura/RetroFuturaGradle)
