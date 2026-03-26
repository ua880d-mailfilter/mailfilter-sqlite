# USECASES.md

## Überblick

Dieses Dokument beschreibt typische Einsatzszenarien für das mailfilter-sqlite System.

Das System kann flexibel in unterschiedlichen Rollen eingesetzt werden.

---

## 1. Leichter Mail-Vorfilter (Home / IPFire)

### Szenario

Kleines System (z. B. IPFire) mit:

- fetchmail
- postfix (oder vergleichbarer MTA)
- SpamAssassin

### Ziel

Entlastung nachgelagerter Systeme durch frühe Filterung.

### Funktionsweise

- mailfilter verarbeitet zuerst Header
- offensichtlicher Spam wird früh verworfen
- verbleibende Mails werden weitergeleitet

### Vorteile

- geringere CPU-Last
- weniger Spam
- schnellere Verarbeitung

---

## 2. Analysemodus (passiv)

### Szenario

Keine direkte Filterung, nur Datensammlung.

### Ziel

Verstehen des Mailverkehrs vor (automatischer) Aktivierung von Regeln.

### Funktionsweise

- Header werden in SQLite gespeichert
- rulegen.pl analysiert die Daten
- Regeln werden erzeugt, aber nicht angewendet (durch einhängen per INCLUDE in mailfilterrc aber möglich)

### Vorteile

- kein Risiko
- volle Transparenz
- sichere Einlernphase

---

## 3. Schrittweise Regelaktivierung

### Szenario

Regeln werden langsam eingeführt.

### Ziel

Vermeidung von False Positives.

### Funktionsweise

- zunächst nur SCORE-Regeln
- Verhalten beobachten
- später gezielt DENY-Regeln aktivieren

### Vorteile

- kontrollierter Rollout
- sichere Optimierung
- hohe Zuverlässigkeit

---

## 4. Kampagnen-Erkennung

### Szenario

Wiederkehrende Spam- oder Phishing-Kampagnen erkennen.

### Ziel

Muster über mehrere Mails hinweg identifizieren.

### Funktionsweise

- Gruppierung nach Domain, Subject, Infrastruktur
- Analyse von Häufigkeit und Zeitverlauf
- Generierung kampagnenbasierter Regeln

### Vorteile

- frühe Erkennung
- effiziente Regelbildung
- hohe Trefferquote

---

## 5. Sicherheitsanalyse / Mail-Telemetrie

### Szenario

Nutzung der SQLite-Datenbank als Datenquelle.

### Ziel

Weitergehende Analysen oder Integration in andere Tools.

### Funktionsweise

- direkte SQL-Abfragen
- Trend- und Musteranalyse
- Export in externe Systeme

### Vorteile

- strukturierte Datenbasis
- flexible Auswertung
- Grundlage für Erweiterungen

---

## 6. Mail-Gateway mit höherem Volumen

### Szenario

Einsatz in größeren Umgebungen.

### Ziel

Entlastung zentraler Filtersysteme.

### Funktionsweise

- Vorfilterung am Eingang
- frühzeitiges Verwerfen von Spam
- Weiterleitung relevanter Mails

### Vorteile

- bessere Skalierbarkeit
- geringere Systemlast
- höhere Effizienz

---

## Zusammenfassung

Das System kann eingesetzt werden als:

- Vorfilter
- Analyse-Engine
- Regelgenerator
- Datenquelle für Auswertungen

Es eignet sich sowohl für kleine als auch größere Umgebungen und bleibt dabei transparent und kontrollierbar.
