# SQLITE_INTEGRATION.md

## Überblick

Die SQLite-Integration erweitert das bestehende Logging-System von mailfilter
um eine strukturierte Datenbank zur Analyse und Regelgenerierung.

---

## Bestehendes Logging

Mailfilter kann Header optional in eine Textdatei schreiben:

    SHOW_HEADERS = "/var/spool/filter/mailheader.log"

Dieses Logging dient hauptsächlich der Analyse und Fehlersuche.

---

## SQLite-Logging

Das SQLite-Backend speichert die gleichen Informationen strukturiert:

    LOG_HEADERS_SQLITE3 = "/var/spool/filter/mailheader.log.sqlite3"

Beide Logging-Varianten können unabhängig voneinander aktiviert werden.

---

## Designansatz

- Das SQLite-Logging ist an denselben Stellen integriert, an denen Header
  verarbeitet und bewertet werden.

- Es wird keine separate Parsing-Logik verwendet.

- Das ursprüngliche Filterverhalten bleibt unverändert.

- Es wurden lediglich zusätzliche Logging-Hooks ergänzt.

---

## Implementierungsdetails

- Initialisierung erfolgt beim Programmstart (wenn aktiviert)
- Logging erfolgt während der Header-Verarbeitung und Regelbewertung
- Datenbank wird beim Programmende sauber geschlossen

Die Implementierung ist im Modul `dblog.cc` gekapselt.

---

## Gespeicherte Daten

Die Datenbank enthält strukturierte Informationen:

### messages
- Nachrichten-ID
- Entscheidung (pass / deny / score-deny)
- finaler Score

### header_entries
- einzelne Headerfelder (Name/Wert)
- Verknüpfung zur Nachricht

### rule_hits
- Bewertungsphase
- Ausdruck
- Trefferstatus
- Score-Beitrag

---

```mermaid
erDiagram
    MESSAGES {
        string msg_log_id PK "Primärschlüssel"
        string message_id "Original Message-ID"
        string from_addr "Absender"
        string to_addr "Empfänger"
        string subject "Original-Betreff"
        string normal_subject "Normalisierter Betreff (für Clustering)"
        string date_hdr "Date-Header (für Temporal Relevance)"
        integer msg_size "Nachrichtengröße"
        string decision "PASS / DENY / SCORE"
        integer final_score "Gesamt-Score"
        datetime created_at
    }

    HEADER_ENTRIES {
        int id PK
        string msg_log_id FK "Verweis auf messages"
        int ordinal "Reihenfolge (z.B. Received)"
        string tag "Header-Name (Received, From, Subject...)"
        string body "Header-Wert"
        datetime created_at
    }

    RULE_HITS {
        int id PK
        string msg_log_id FK "Verweis auf messages"
        string phase "Verarbeitungsphase"
        string expression "Regel-Ausdruck"
        boolean is_negative "Negativ-Regel?"
        boolean matched "Getroffen?"
        string header_tag
        string header_body
        boolean normalized_subject
        integer score_delta "Score-Änderung"
        datetime created_at
    }

    MESSAGES ||--o{ HEADER_ENTRIES : "hat viele Header"
    MESSAGES ||--o{ RULE_HITS : "hat viele Regel-Treffer"

    classDef messagesClass fill:#e3f2fd,stroke:#1976d2,stroke-width:3px,color:#000;
    classDef headersClass fill:#f1f8e9,stroke:#388e3c,stroke-width:3px,color:#000;
    classDef rulehitsClass fill:#fff3e0,stroke:#f57c00,stroke-width:3px,color:#000;

    class MESSAGES messagesClass
    class HEADER_ENTRIES headersClass
    class RULE_HITS rulehitsClass
```

---

## Datenumfang

- Es werden ausschließlich Header gespeichert
- Kein Mail-Body wird erfasst
- Die Daten spiegeln den internen Entscheidungsprozess wider

---

## Vorteile

- Strukturierte und durchsuchbare Daten
- Direkte Nutzung für Regelgenerierung
- Keine Beeinflussung des bestehenden Verhaltens
- Vollständig deterministisch

---

## Zusammenfassung

Die SQLite-Integration ist eine nicht-invasive Erweiterung des bestehenden
Logging-Systems. Sie ermöglicht tiefe Einblicke in den Entscheidungsprozess,
ohne das Kernverhalten von mailfilter zu verändern.
