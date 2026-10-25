# Contributing Guide

Dieses Dokument beschreibt den Arbeitsablauf für die Entwicklung dieses Projekts. Da die Entwicklung primär durch eine KI (Vibe Code) erfolgt, dient diese Datei als Referenz für die Entscheidungsfindung und Qualitätskontrolle.

## Arbeitsprinzipien

### 1. Iterative Entwicklung
- Arbeit wird in kleine, überprüfbare Schritte unterteilt
- Jede Phase wird vollständig abgeschlossen, bevor die nächste beginnt
- Nach jeder Phase: lauffähiger Build, Tests, Dokumentation

### 2. Qualitätsstandards
- **Keine Kommentare im Code** (außer wo absolut notwendig für JavaDoc)
- **Kleine Commits** mit klaren, deskriptiven Messages
- **Jede Entscheidung wird dokumentiert** (ADRs für Architektur, Code-Dokumentation)
- **Tests vor Implementation** (Test-Driven Development wo sinnvoll)

### 3. Branch-Strategie

```
main (protected)
  │
  ├── vibe/phase-1-setup
  ├── vibe/phase-2-protocol
  ├── vibe/phase-3-client-mod
  ├── vibe/phase-4-renderer
  ├── vibe/phase-5-expansion
  └── vibe/phase-6-stabilization
```

- Jede Phase bekommt einen eigenen Feature-Branch
- Commits werden regelmäßig in `main` gemergt
- Pull Requests werden für jede Phase erstellt

## Entwicklungs-Workflow

### 1. Issue-Erstellung

Bevor mit der Implementierung begonnen wird, wird für jede Phase ein GitHub Issue erstellt:

- **Titel**: `[Phase X] <Beschreibung>`
- **Labels**: `phase-X`, `enhancement`, `documentation` etc.
- **Description**: 
  - Ziele der Phase
  - Akzeptanzkriterien
  - Abhängigkeiten
  - Geschätzter Aufwand

### 2. Branch erstellen

```bash
# Von main aus
git checkout main
git pull origin main

# neuen Branch für die Phase erstellen
git checkout -b vibe/phase-<n>-<slug>
```

### 3. Implementierung

- **Kleine Commits**: Jede logische Änderung = ein Commit
- **Commit Messages**: 
  - Präfix mit Typ: `feat:`, `fix:`, `docs:`, `refactor:`, `test:`
  - Beschreibend: "feat: add camera data extraction from EntityRenderer"
  - Nicht: "fixed bug" oder "changed stuff"

### 4. Testing

- **Unit-Tests**: Für alle nicht-trivialen Funktionen
- **Integration-Tests**: Für Modul-Interaktionen
- **Manuelle Tests**: Dokumentiert in Test-Skripten

### 5. Dokumentation

- **Code**: Selbst-dokumentierend durch klare Naming-Konventionen
- **ADRs**: Für jede architektonische Entscheidung
- **Protokoll-Doku**: Immer aktuell halten
- **README**: Roadmap und Status aktualisieren

### 6. Pull Request

- **Titel**: Klare Beschreibung der Änderungen
- **Description**: 
  - Was wurde geändert
  - Warum wurde es geändert
  - Wie wurde es getestet
  - Screenshots/Logs bei UI-Änderungen
- **Linked Issues**: Referenz auf das entsprechende Issue

### 7. Review und Merge

- **Selbst-Review**: Vor dem Push nochmal den Code durchgehen
- **CI-Checks**: Alle Tests müssen durchlaufen
- **Merge**: Erst nach erfolgreicher Überprüfung

## Code-Conventions

### Java

- **Java 8**: Keine modernen Java-Features (Streams API ist erlaubt)
- **Naming**: 
  - Klassen: `PascalCase`
  - Methoden: `camelCase`
  - Variablen: `camelCase`
  - Konstanten: `UPPER_SNAKE_CASE`
- **Package-Naming**: `renderbridge.<modul>.<submodul>`
- **Import-Order**: 
  1. Java Standard Library
  2. Third-Party Libraries
  3. Projekt-interne Imports

### Gradle

- **Build-Skripte**: Konsistente Struktur across alle Module
- **Abhängigkeiten**: Versionen zentral in `gradle.properties` oder `buildSrc`
- **Tasks**: Klare Benennung und Dokumentation

### Git

- **Commit Messages**: 50-72 Zeichen Titel, dann Leerzeile, dann Körper
- **Line Endings**: LF (Unix-style)
- **Whitespace**: Keine trailing whitespaces

## Phase-spezifische Anweisungen

### Phase 1: Setup
- Verzeichnisstruktur nach Plan erstellen
- Alle Dokumentationsdateien anlegen
- Build-Systeme minimal konfigurieren
- ADRs für alle grundlegenden Entscheidungen erstellen

### Phase 2: Protokoll
- Protokoll-Design in `docs/architecture.md` finalisieren
- Message-Typen als Java-Klassen definieren
- Serialisierung mit ByteBuffer implementieren
- Transport-Schicht abstrakt halten (Interface + Implementierungen)
- Unit-Tests für alle Codecs

### Phase 3: Client-Mod
- ForgeGradle oder RetroFuturaGradle verwenden
- MCP-Mappings für 1.8.9 nutzen
- Keine Reflection wenn vermeidbar (Bytecode-Agent bevorzugt)
- Fallback-Rendering implementieren

### Phase 4: Externer Renderer
- LWJGL 2 verwenden (Kompatibilität mit 1.8.9)
- Eigene Fenster-Verwaltung
- Kamera-Steuerung aus übertragenen Daten
- Einfache Voxel-Darstellung für den Anfang

### Phase 5: Ausbau
- Texturen über Ressourcenpaket-Pfad laden
- Keine Mojang-Assets im Repository
- Performance-Metriken sammeln

### Phase 6: Stabilisierung
- Fehlerbehandlung für alle Edge-Cases
- Reconnect-Logik mit Exponential Backoff
- Protokoll-Version in jeder Message
- Release-Tag mit Changelog

## Fehlerbehandlung

### In der Entwicklung

1. **Problem identifizieren**: Genaues Verständnis des Issues
2. **Reproduzieren**: Minimales Testbeispiel erstellen
3. **Analysieren**: Root-Cause finden
4. **Lösen**: Minimale Änderung implementieren
5. **Testen**: Fix verifizieren

### Im Code

- **Exceptions**: Spezifische Exception-Typen verwenden
- **Logging**: SLF4J oder java.util.logging
- **Validation**: Früh scheitern mit klaren Fehlermeldungen
- **Recovery**: Graceful Degradation wo möglich

## Dokumentation-Standards

### ADRs (Architecture Decision Records)

Jede architektonische Entscheidung wird in `docs/adr/` dokumentiert:

```markdown
# ADR-001: <Titel>

## Status
✅ Akzeptiert | ❌ Verworfen | ⏳ In Diskussion

## Kontext
[Problemstellung und Hintergrund]

## Entscheidung
[Getroffene Entscheidung]

## Alternativen
- [Alternative 1]
- [Alternative 2]

## Konsequenzen
[Positive und negative Auswirkungen]

## Links
- [Relevante Issues]
- [Externe Referenzen]
```

### Protokoll-Dokumentation

- Binäre Layouts in ASCII-Tabellen
- Endianness immer angeben
- Byte-Offsets für jedes Feld
- Beispiel-Payloads

## Tooling

### Empfohlene IDE
- IntelliJ IDEA (Community Edition)
- Eclipse mit Buildship

### Plugins
- Checkstyle
- PMD
- SonarLint

### Command-Line Tools
- `./gradlew` für alle Build-Aufgaben
- `git` für Versionskontrolle
- `gh` CLI für GitHub-Interaktion

## Kommunikation

Da die Entwicklung durch eine KI erfolgt:
- **Issues**: Für Tracking und Dokumentation
- **Commits**: Als Fortschrittsprotokoll
- **Pull Requests**: Für Review und Merge
- **ADRs**: Für Entscheidungsdokumentation

## Lizenz für Beiträge

Alle Beiträge werden unter der MIT-Lizenz bereitgestellt. Durch das Einreichen von Code bestätigst du, dass du die Rechte daran besitzt und diese unter MIT lizenzieren darfst.
