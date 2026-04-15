# mailfilter-analyze.py

Analytisches Python-Werkzeug für `mailfilter-sqlite` zur Auswertung gespeicherter Mail-Header, zur zeitbezogenen Sichtung realer Nachrichtenmengen und zur experimentellen Entwicklung zusätzlicher Heuristiken für Auth-, DKIM-, Bulk- und Phishing-Kontext.

Das Script ist bewusst als leichtgewichtiges Console-Werkzeug gehalten:
- keine externen Frameworks
- keine Web-Abhängigkeiten
- keine zusätzliche Server-Komponente
- nur Python, SQLite und das bestehende `mailfilter-sqlite`-Datenmodell

Damit eignet sich `mailfilter-analyze.py` gut für:
- schnelle Shell-Analysen
- Cronjobs
- zeitbezogene Statistikläufe
- experimentelle Heuristikentwicklung
- spätere halbautomatische Regel- oder Policy-Verdichtung

---

## Zweck

`mailfilter-analyze.py` dient dazu, aus real in der SQLite-Datenbank vorhandenen Nachrichten und Headern verwertbare Analyse-Signale abzuleiten.

Im Mittelpunkt stehen derzeit vor allem diese Aufgaben:

1. Zeitbezogene Sichtung realer Mailmengen über `date_hdr` oder `created_at`
2. Statistik über Decisions, Scores und häufige Betreffzeilen
3. technische Sichtung von `Authentication-Results`, `ARC-Authentication-Results` und `DKIM-Signature`
4. erste heuristische Einordnung in:
   - eher legitime Auth-Fälle
   - legitime Bulk-/Newsletter-Kontexte
   - technisch saubere, aber kontextuell verdächtige Fälle
5. Vorbereitung einer späteren komprimierten Advisory-/Policy-Schicht

Das Script ist ausdrücklich kein vollautomatischer Klassifikator, sondern ein Arbeitswerkzeug für Administratoren und für die Weiterentwicklung des Projekts.

---

## Einordnung im Projekt

`mailfilter-analyze.py` ist kein Ersatz für `mailfilter.cgi`, sondern dessen analytische Ergänzung.

### `extras/python/mailfilter-analyze.py`

Das Python-Script ist die eher analytische, experimentelle und generatornahe Schicht.

Es eignet sich gut für:
- Batch-Auswertungen
- wiederholbare Shell-Analysen
- Cronjobs
- Heuristik-Experimente
- Verdichtung größerer Mengen
- spätere Policy- bzw. Advisory-Generierung

### `extras/cgi/mailfilter.cgi`

Das CGI-Script ist die interaktive Web-Oberfläche für den manuellen Betrieb.

Es eignet sich gut für:
- schnelle Sichtung einzelner Nachrichten
- Prüfung konkreter Header
- manuelle Regelbildung
- direkte operative Nacharbeit auf dem System

Beide Werkzeuge ergänzen sich bewusst:

- `mailfilter-analyze.py` für Statistik, Heuristik und spätere Automatisierung
- `mailfilter.cgi` für interaktive Sichtung, Detailprüfung und manuelle Regelbildung

---

## Datenbasis

`mailfilter-analyze.py` arbeitet direkt auf der von `mailfilter-sqlite` gefüllten SQLite-Datenbank, typischerweise:

```text
/var/spool/filter/mailheader.sqlite3
```

Verwendet werden insbesondere diese Tabellen:

### `messages`

Enthält die pro Nachricht aggregierten Metadaten, z. B.:

- `msg_log_id`
- `message_id`
- `from_addr`
- `to_addr`
- `subject`
- `normal_subject`
- `date_hdr`
- `msg_size`
- `decision`
- `final_score`
- `created_at`

### `header_entries`

Enthält die zerlegten Header pro Nachricht in Reihenfolge, z. B.:

- `msg_log_id`
- `ordinal`
- `tag`
- `body`
- `created_at`

### `rule_hits`

Kann später zusätzlich für vertiefte Auswertungen herangezogen werden, z. B. für:

- Trefferphasen
- Ausdrucksverhalten
- Score-Herkunft
- spätere Korrelationen

Aktuell nutzt `mailfilter-analyze.py` vor allem `messages` und gezielt ausgewählte Header aus `header_entries`.

---

## Zeitlogik

Das Script kann wahlweise mit zwei Zeitbezügen arbeiten:

### `date_hdr`

Das echte Datum aus dem Mail-Header.

Das ist insbesondere dann sinnvoll, wenn:
- alte Mails nachträglich importiert werden
- POP3-Konten initial vollständig abgerufen werden
- reale Mailzeit wichtiger ist als der reine Erfassungszeitpunkt

### `created_at`

Der tatsächliche Erfassungszeitpunkt in der SQLite-Datenbank.

Das ist insbesondere dann sinnvoll, wenn:
- Abrufwellen oder Folgeimporte betrachtet werden sollen
- zeitliche Wiederholung derselben Mail relevant ist
- reine Importdynamik sichtbar gemacht werden soll

---

## Aktueller Funktionsstand

### 1. Statistikmodus

Das Script kann auf der Shell zeitbezogene Statistiken erzeugen, u. a. über:

- Anzahl gefilterter Mails
- Decision-Verteilung
- durchschnittliche Scores
- häufige Subjects
- Beispiel-`msg_log_id`

### 2. Zeitfilterung

Unterstützt werden aktuell insbesondere:

- `--days`
- `--latest`
- `--time-field date_hdr`
- `--time-field created_at`

Damit lassen sich sowohl echte Mailzeiträume als auch reine Erfassungszeiträume untersuchen.

### 3. Zeit-Diagnose

Optional kann eine kleine technische Zeitdiagnose ausgegeben werden, z. B.:

- parsebare Datumswerte
- unparsebare Datumswerte
- Zukunftsausreißer (`future_skew`)

Das hilft, die Datenqualität der Zeitbasis schnell einzuschätzen.

### 4. Auth-/DKIM-Analyse

Das Script wertet aktuell insbesondere aus:

- `Authentication-Results`
- `ARC-Authentication-Results`
- `DKIM-Signature`
- `List-Unsubscribe`
- `List-Unsubscribe-Post`
- `List-Id`
- teilweise `Reply-To`

Dabei werden derzeit u. a. folgende Signalklassen abgeleitet:

- technische Pass-Signale
- Domain-Alignment
- Bulk-/Listen-Kontext
- Plattform-/Infra-Hinweise
- alarmistische Subject-Muster
- erste Marker wie:
  - `!!! suspicious`
  - `bulk/auth-ok`
  - `auth-strong`
  - `auth-neutral`

### 5. Console-taugliche Darstellung

Die Ausgabe ist bewusst für Shell und SSH-Betrieb gedacht:

- feste Textausgabe
- schnelle Sichtung über `msg_log_id`
- direkte Nutzbarkeit auf kleinen Systemen
- gut geeignet für Administratoren ohne Zusatz-Frontend

---

## Aktuelle Heuristik-Idee

Die Auth-Analyse versucht derzeit nicht nur technische `pass`-Signale zu erkennen, sondern diese vorsichtig mit Kontext zu kombinieren.

Beispiele für derzeit relevante Unterscheidungen:

### Eher legitim

- DKIM passt zur sichtbaren Absenderdomain
- First-Party- oder klar vertrauenswürdige Domain
- kein Bulk-/Listen-Kontext
- konsistentes Sicherheits-/Account-Thema

### Eher legitimer Bulk-/Newsletter-Fall

- `List-*`-Header vorhanden
- Marketing-/Newsletter-Kontext
- technisch konsistenter Versand
- keine alarmistische Account-/Security-Semantik

### Eher verdächtig

- alarmistisches Subject
- Bulk-/Campaign-Kontext
- Plattform-/Infra-DKIM statt eigentlicher sichtbarer Absenderidentität
- DKIM- oder Domain-Mismatch
- technisch saubere Auth-Signale, aber semantisch unplausibler Versandkontext

Wichtig:
Diese Heuristik ist bewusst noch experimentell und soll Administratoren unterstützen, nicht ersetzen.

---

## Typische Nutzung

Typische Aufrufe sind z. B.:

```bash
python3 /usr/bin/mailfilter-analyze.py --stats --days 10
```

```bash
python3 /usr/bin/mailfilter-analyze.py --stats --days 30 --latest 10 --time-field date_hdr --analyze-auth
```

```bash
python3 /usr/bin/mailfilter-analyze.py --stats --days 30 --latest 10 --time-field date_hdr --analyze-auth --verbose
```

Das ist besonders nützlich für:

- schnelle tägliche Sichtung
- Heuristiktests auf realen Mails
- Vergleich zwischen `date_hdr` und `created_at`
- Bewertung neuer Bulk-/Phishing-Muster
- Vorbereitung späterer Regel- oder Advisory-Ideen

---

## Fernziel

Das langfristige Ziel des Scripts ist nicht nur Statistik, sondern eine zusätzliche analytische Verdichtungsschicht für `mailfilter-sqlite`.

Geplant bzw. angedacht ist insbesondere:

1. weitere Schärfung der Auth-/Bulk-/Kontext-Heuristik
2. stabilere Trennung zwischen:
   - legitimen Account-/Transaktionsmails
   - legitimen Bulk-/Newsletter-Mails
   - technisch sauberen, aber kontextuell verdächtigen Mails
3. optionale Verdichtung der Ergebnisse in ein zusätzliches Policy-/Advisory-File
4. optionale Einbindung dieses komprimierten Policy-Files über bestehende Generator- und Wrapper-Strukturen
5. spätere Nutzung als experimentelle Vorstufe für halbautomatische Kampagnen- oder Kontextbewertung

Wichtig:
Dieses Fernziel soll die bestehende Perl-/Rulegen-Logik nicht ersetzen, sondern sinnvoll ergänzen.

---

## Designziel

`mailfilter-analyze.py` soll:

- nachvollziehbar statt magisch sein
- lokal und leichtgewichtig bleiben
- auf realen Headerdaten arbeiten
- Administratoren beim Denken unterstützen
- genug Flexibilität für Heuristik-Experimente bieten
- später komprimierbare Signale erzeugen, ohne den Kern des Projekts unnötig umzubauen

Gerade deshalb bleibt das Script bewusst nah an:
- realen Headern
- realen Entscheidungen
- einfachen Shell-Ausgaben
- kleinen, prüfbaren Heuristikschritten

---

## Grenzen

`mailfilter-analyze.py` ersetzt aktuell nicht:

- vollständige Kampagnenkorrelation
- belastbare Massenklassifikation
- forensische Mailanalyse
- endgültige Sicherheitsentscheidungen
- die manuelle Einordnung durch einen Administrator

Insbesondere bleiben weiterhin eher in anderen Werkzeugen oder späteren Ausbaustufen:

- tiefe Korrelationslogik über viele Mails
- stabile Kampagnenerkennung
- harte Policy-Automatisierung
- vollständige Deduplizierungslogik
- endgültige Scoring-Strategien für alle Kontexte

---

## Fazit

`mailfilter-analyze.py` hat sich von einem kleinen Hilfs- und Brainstorming-Script zu einem brauchbaren Console-Werkzeug für `mailfilter-sqlite` entwickelt.

Aktuell verbindet es bereits:

- zeitbezogene Mailanalyse
- Decision- und Subject-Statistik
- technische Auth-/DKIM-Sichtung
- erste Kontextmarker für verdächtige, legitime und Bulk-Fälle
- experimentelle Heuristikentwicklung auf realen Daten

Damit eignet sich das Script bereits jetzt gut für Administratoren, die auf der Shell reale Mailmengen sichten, technische Signale verdichten und spätere Automatisierung vorbereiten möchten.
