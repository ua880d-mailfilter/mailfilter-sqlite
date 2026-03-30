# mailfilter-stats (Text & HTML)

Diese Werkzeuge dienen der Auswertung der SQLite-Datenbank von **mailfilter-sqlite**.

```bash
mailfilter-stats.sh
mailfilter-stats-html.sh
```

Die Scripts ermöglichen eine schnelle Analyse von:
- Header-Verteilungen
- häufigen Absendern
- Rule-Hits (deny / score)
- generierten Scores
- auffälligen Subjects

Diese Daten bilden die Grundlage für:
- Regeloptimierung
- Rulegen (mailfilter-rulegen.pl)
- Spam-/Anomalie-Analyse

---

## Enthaltene Tools

### 1. mailfilter-stats.sh
Klassische Textausgabe für Terminal / CLI.

**Eigenschaften:**
- schnell
- ideal für SSH / Konsole
- gut für Debugging
- grep-/pipe-freundlich

---

### 2. mailfilter-stats-html.sh
Erzeugt eine strukturierte HTML-Ausgabe.

**Eigenschaften:**
- übersichtliche Tabellen
- farblich strukturiert
- geeignet für Reports
- ideal für GitHub-Dokumentation
- kann in Webserver integriert werden (z. B. als CGI)

---

## Verwendung (Beispiele)

### Text-Version

```bash
./mailfilter-stats.sh --db /var/spool/filter/mailheader.log.sqlite3
```

### HTML-Version

```bash
./mailfilter-stats-html.sh \
  --db /var/spool/filter/mailheader.log.sqlite3 \
  --limit 20 > stats.html
 ``` 
