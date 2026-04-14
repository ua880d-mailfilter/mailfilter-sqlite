# mailfilter.cgi

Interaktive CGI-Oberfläche für `mailfilter-sqlite` zur Analyse gespeicherter Mail-Header, zur schnellen Ableitung von `mailfilter`-Regeln und zum direkten Schreiben zusätzlicher Regeln in eine separate Include-Datei.

Diese Komponente ist als leichtgewichtiges Web-Frontend für kleine bis mittlere Administrationsumgebungen gedacht, insbesondere für Systeme wie IPFire, auf denen Header bereits durch `mailfilter-sqlite` in einer SQLite-Datenbank protokolliert werden.

Die CGI-Oberfläche ist bewusst einfach gehalten:

- keine externen Frameworks
- keine JavaScript-Bibliotheken
- kein zusätzliches Backend
- nur Python, SQLite, HTML/CSS/JavaScript und das bestehende `mailfilter-sqlite`-Datenmodell

Damit eignet sich `mailfilter.cgi` gut für interaktive Sichtung, manuelle Regelbildung und schnelles operatives Nachsteuern direkt auf dem System.

---

## Zweck

`mailfilter.cgi` dient dazu, aus real in der SQLite-Datenbank vorhandenen Headern und Metadaten verwertbare `mailfilter`-Regeln abzuleiten.

Im Mittelpunkt stehen dabei drei praktische Aufgaben:

1. Sichtung auffälliger oder häufiger Betreffzeilen innerhalb eines konfigurierbaren Zeitraums
2. Analyse vollständiger Header einer konkreten Nachricht über `msg_log_id`
3. Interaktive Generierung und Speicherung zusätzlicher Regeln (`DENY`, `SCORE`, `ALLOW`)

Die erzeugten Regeln werden nicht in die Hauptkonfiguration geschrieben, sondern in eine separate Include-Datei ausgelagert. Das hält die eigentliche `mailfilterrc` sauber und erlaubt eine saubere Trennung zwischen:

- Basis-Konfiguration
- manuell gepflegten Regeln
- automatisch oder halbautomatisch erzeugten Regeln
- generator- bzw. CGI-spezifischen Regeldateien

---

## Einordnung im Projekt

`mailfilter.cgi` ist kein Ersatz für `mailfilter-analyze.py`, sondern dessen interaktive Ergänzung.

### `extras/python/mailfilter-analyze.py`

Das Python-Script ist das eher analytische bzw. generatororientierte Werkzeug. Es eignet sich gut für:

- Batch-Auswertungen
- Cronjobs
- schnelle Statistikläufe auf der Shell
- experimentelle Regel- und Heuristikentwicklung
- spätere Automatisierung

### `extras/cgi/mailfilter.cgi`

Das CGI-Script ist die Web-Oberfläche für den manuellen Betrieb. Es eignet sich gut für:

- schnelle Sichtung einzelner Nachrichten
- unmittelbare Regelbildung beim Durchsehen verdächtiger Betreffzeilen
- Prüfung konkreter Header (`From`, `Return-path`, `Subject`, `Authentication-Results`, `ARC-Authentication-Results`, `List-Unsubscribe`, `X-Mailer`, …)
- direktes Speichern neuer Regeln in eine zusätzliche Include-Datei

Beide Werkzeuge ergänzen sich sehr gut:

- `mailfilter-analyze.py` für Statistik, Brainstorming und Automatisierung
- `mailfilter.cgi` für interaktive Nacharbeit, Header-Sichtung und manuelle Präzisierung

---

## Datenbasis

`mailfilter.cgi` arbeitet direkt auf der von `mailfilter-sqlite` gefüllten SQLite-Datenbank, typischerweise:

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

Enthält die zerlegten Header pro Nachricht in Reihenfolge:

- `msg_log_id`
- `ordinal`
- `tag`
- `body`
- `created_at`

### `rule_hits`

Kann später zusätzlich für vertiefte Auswertungen herangezogen werden, z. B. für:

- Trefferphasen (`allow`, `deny`, `score`)
- Matching-Verhalten einzelner Ausdrücke
- Rückverfolgung von Entscheidungen

Aktuell nutzt die CGI-Oberfläche vor allem `messages` und `header_entries`.

---

## Zeitfilterung

Die Oberfläche filtert standardmäßig anhand von `date_hdr`, also des echten Datums aus dem Mail-Header.

Das ist bei `mailfilter-sqlite` bewusst sinnvoll, weil `created_at` bei vollständigen POP3-Abfragen bzw. beim erstmaligen Import vieler älterer Mails auf denselben Erfassungszeitpunkt fallen kann. Für Administratoren ist in solchen Fällen das tatsächliche Mail-Datum in `date_hdr` wesentlich aussagekräftiger.

---

## Hauptfunktionen der Oberfläche

### 1. Zeitraum- und Listenfilter

Im oberen Bereich der Oberfläche kann gesteuert werden:

- Zeitraum: z. B. `1`, `2`, `5`, `10`, `20`, `30`, `60`, `90`, `180`, `365` Tage
- maximale Anzahl der anzuzeigenden Top-Einträge, z. B. `5`, `10`, `15`, `20`, `30`, `50`

Die Ansicht bleibt auch nach dem Speichern neuer Regeln erhalten.

### 2. Decision-Verteilung

Die Oberfläche zeigt die aktuelle Verteilung der in `messages.decision` gespeicherten Entscheidungen für den gewählten Zeitraum, z. B.:

- `pass`
- `allow`
- `deny`

Das dient als schnelle Übersicht über das aktuelle Gesamtbild.

### 3. Top Subjects

Die Haupttabelle gruppiert Betreffzeilen und zeigt pro Eintrag u. a.:

- Häufigkeit
- durchschnittlichen Score
- gekürzten Subject-Text
- eine Beispiel-`msg_log_id`
- Aktionsbuttons

Damit können häufige oder auffällige Kampagnen schnell identifiziert werden.

### 4. Header-Detailansicht

Über den Button **`Header`** wird für eine konkrete `msg_log_id` ein separates Modal geöffnet. Dort werden alle in `header_entries` gespeicherten Header der Nachricht tabellarisch angezeigt.

Das ist besonders hilfreich, um:

- den realen `From`-Header zu sehen
- `Return-path`, `Reply-To`, `List-Unsubscribe` und `Received` zu prüfen
- technische Versandmerkmale zu erkennen
- Phishing-, Marketing- oder Listenmails manuell einzuordnen

### 5. Rule-Modal

Über Klick auf die Zeile oder über **`Rule`** wird ein eigenes Regel-Modal geöffnet.

Im Kopf des Modals werden angezeigt:

- `msg_log_id`
- `Mail-Datum` (formatiert aus `date_hdr`)
- `erfasst am` (aus `created_at`)
- `From`
- `Subject`

Format der Datumsanzeige:

```text
DD.MM.JJJJ - HH:MM:SS
```

So lassen sich reale Mailzeit und Datenbank-Erfassung sofort unterscheiden.

---

## Regelbildung im Modal

### Grundprinzip

Das Modal erzeugt interaktiv `mailfilter`-Regeln der Typen:

- `DENY`
- `SCORE`
- `ALLOW`

Für `SCORE` bleibt der Wert frei editierbar. Positive und negative Werte werden korrekt formatiert.

### Header-Feld-Auswahl

Das Feld **`Header-Feld für Regel`** wird dynamisch aus den tatsächlich vorhandenen Header-Tags der gewählten Mail aufgebaut.

Damit können Regeln nicht nur aus `Subject`, sondern auch aus anderen Headern erzeugt werden, z. B.:

- `From`
- `Return-path`
- `Reply-To`
- `Subject`
- `Authentication-Results`
- `ARC-Authentication-Results`
- `List-Unsubscribe`
- `X-Mailer`

---

## Subject-Strategie

Für `Subject` existieren zwei Vorschlagsmodi:

### Konservativ

Der konservative Vorschlag wählt aus dem Subject wenige starke Tokens aus und erzeugt eine relativ präzise Regel. Dabei werden u. a. berücksichtigt:

- längere bzw. stärkere Begriffe
- Modell-/Produktbezeichnungen
- domainspezifische Tokens
- Entfernung unnötiger Zahl-/Preis-/Mengenbestandteile

Beispiel:

```text
^Subject:.*HS-PX410.*original.*packing
```

### Gruppiert / flexibel

Die flexible Variante gruppiert längere Betreffzeilen in wenige sinnvolle Token-Gruppen und erzeugt breiter matchende Regeln, die eher für `SCORE` geeignet sind.

Beispiel:

```text
^Subject:.*(Weekend|Clearance|Sale).*(Eachine|E200|Helicopter)
```

Ziel ist nicht perfekte automatische Klassifikation, sondern ein operativ brauchbarer Regelvorschlag für Administratoren.

### Aktuelle Grundsätze der Subject-Logik

- Domains werden erkannt und korrekt maskiert
- Punkte werden außerhalb domainspezifischer Tokens als Trenner behandelt
- Fließkomma-/Preis-/Mengenmuster werden möglichst aus der konservativen Regelbildung herausgehalten
- sehr lange starre Wortketten werden vermieden
- kurze, starke konservative Regeln werden bevorzugt

---

## Auth-/ARC-Auth-Speziallogik

### Unterstützte Header

Das CGI besitzt eine Spezialbehandlung für:

- `Authentication-Results`
- `ARC-Authentication-Results`

### Teilmerkmal-Extraktion

Bei Auswahl eines dieser Header wird ein zweites Dropdown eingeblendet. Dieses enthält erkannte Teilmerkmale wie z. B.:

- `dkim=pass`
- `spf=pass`
- `dmarc=pass`
- `iprev=pass`
- `header.d=example.org`
- `header.from=example.org`
- `smtp.mailfrom=user@example.org`

Zusätzlich können sinnvolle Kombinationen angeboten werden, etwa:

- `dkim=pass + header.d=example.org`
- `dkim=pass + spf=pass`
- `dkim=pass + dmarc=pass`
- `spf=pass + smtp.mailfrom=user@example.org`

### Standardaktion

Wird einer dieser Header gewählt, setzt das CGI standardmäßig die Aktion auf `SCORE`. Das ist nur eine Voreinstellung; der Administrator kann jederzeit auf `DENY` oder `ALLOW` umstellen.

### Semantische SCORE-Empfehlung

Für `Authentication-Results` und `ARC-Authentication-Results` berechnet das CGI inzwischen eine vorsichtige empfohlene SCORE-Richtung:

- `pass`-Signale können zu negativen SCORE-Vorschlägen führen
- `fail`-Signale eher zu positiven SCORE-Vorschlägen
- Plattform-/Bulk-/Marketing-Kontext kann positive Warn-Scores erzeugen
- offensichtliche Fehlalarme bei legitimen Community-, Projekt- oder First-Party-Mails werden möglichst vermieden

Wichtig:

- diese Logik ist **nur empfehlend**
- sie ersetzt **nicht** die manuelle Entscheidung
- sie nimmt dem Administrator keine Option weg

### Plattform- und Bulk-Kontext

Das CGI versucht inzwischen zwischen mehreren Fällen zu unterscheiden.

#### Eher legitim

- konsistente First-Party-Domain in `From`, `Reply-To`, `Return-Path`, `header.d`, `header.from`
- legitime Listen-/Community-Mails
- Versand über legitime Plattformen wie Amazon SES, solange die eigentliche Absenderdomäne konsistent mitläuft

#### Eher verdächtig

- Marketing-/Kampagnenheader (`X-CampaignID`, `X-MJ-*`, …)
- `Precedence: bulk`
- klarer Plattform-/Absender-Mismatch
- technische Pass-Signale ohne konsistente sichtbare Absenderidentität

### Warnhinweis im Modal

Bei verdächtigem Auth-/Bulk-/Plattformkontext zeigt das Modal einen farbigen Warnhinweis an. Dieser soll keine harte Entscheidung vorwegnehmen, sondern auf potenziell riskante technische Konstellationen aufmerksam machen.

---

## Speichern von Regeln

### Ziel-Datei

Neu erzeugte Regeln werden in eine separate Include-Datei geschrieben, standardmäßig:

```text
/etc/mailfilter/mailfilter-extra-rules-cgi.conf
```

Diese Datei kann anschließend per `INCLUDE` in die Hauptkonfiguration eingebunden werden.

### Schreibverhalten

Beim Klick auf **`Regel speichern`** gilt:

- die erzeugte Regel wird geprüft
- identische Regeln werden nicht doppelt gespeichert
- neue Regeln werden angehängt
- über jeder gespeicherten Regel wird ein Kommentar mit der `msg_log_id` abgelegt

Beispiel:

```text
### msg_log_id: hdr-157 ###
SCORE +50="^Subject:.*(Letzte|Chance|Rabatt).*(Sichere|werbefreies|Postfach)"
```

### Nutzen

Dadurch bleibt nachvollziehbar:

- woher eine Regel stammt
- aus welcher Mail sie abgeleitet wurde
- welche Regeln interaktiv über das CGI erzeugt wurden

---

## Typische praktische Nutzung

Typischer Ablauf:

1. Zeitraum wählen
2. auffälliges oder häufiges Subject in der Tabelle sichten
3. per **`Header`** vollständige Header analysieren
4. per **`Rule`** Regel-Modal öffnen
5. passenden Header oder Subject-Vorschlag wählen
6. Regeltyp (`DENY`, `SCORE`, `ALLOW`) festlegen
7. Wert bei `SCORE` anpassen
8. Regel kopieren oder direkt speichern

Dieses Vorgehen ist besonders nützlich für:

- schnelle Reaktion auf neue Werbe- oder Phishingwellen
- manuelle Nacharbeit nach Headeranalyse
- Ergänzung automatisch erzeugter Regeln
- lokale Feinanpassung in kleinen Administrationsumgebungen

---

## Installation

### Voraussetzungen

Benötigt werden:

- ein Webserver mit CGI-Unterstützung
- Python 3
- Zugriff auf die SQLite-Datenbank von `mailfilter-sqlite`
- Schreibrechte auf die zusätzliche Regeldatei, sofern Speichern im CGI genutzt werden soll

### Ablage

Beispiel:

```text
/srv/web/ipfire/cgi-bin/mailfilter.cgi
```

oder ein anderer CGI-Pfad des Zielsystems.

### Dateirechte

Das Script selbst muss ausführbar sein, z. B.:

```bash
chmod 0755 mailfilter.cgi
```

Falls die Funktion **`Regel speichern`** genutzt werden soll, muss der CGI-/Webserver-User Schreibrechte auf die zusätzliche Regeldatei besitzen.

### Include in der mailfilter-Konfiguration

Die zusätzliche CGI-Datei sollte nicht isoliert bleiben, sondern per `INCLUDE` in die eigentliche `mailfilterrc` eingebunden werden.

Beispiel:

```text
INCLUDE "/etc/mailfilter/mailfilter-extra-rules-cgi.conf"
```

Je nach lokaler Struktur können weitere Generator- oder Hilfsdateien parallel eingebunden werden.

---

## Designziel

Das CGI soll keine vollautomatische Sicherheitsentscheidung treffen. Es ist als interaktives Arbeitswerkzeug gedacht.

Wichtige Leitlinien:

- nachvollziehbar statt magisch
- einfache lokale Bedienung
- keine schweren Abhängigkeiten
- direkte Nutzbarkeit auf kleinen Systemen
- Ergänzung, nicht Ersatz, für umfangreichere Generator-Logik

Gerade deshalb ist das CGI bewusst nah an realen Headern und am tatsächlichen `mailfilter`-Regeltext gehalten.

---

## Grenzen

`mailfilter.cgi` kann Hinweise verdichten und Regelvorschläge erzeugen, ersetzt aber keine vollständige Kampagnen- oder Korrelationsanalyse.

Insbesondere gehören weiterhin eher in andere Werkzeuge wie `mailfilter-rulegen.pl` oder `mailfilter-analyze.py`:

- großflächige Massenanalyse
- zeitliche Korrelationen
- heuristische Aggregation über viele Mails
- automatische Kampagnenerkennung
- tiefe Score-Systeme über mehrere Signale hinweg

Das CGI ist die operative, manuelle Frontend-Schicht dazu.

---

## Fazit

`mailfilter.cgi` hat sich von einer einfachen Betreff-/Headeransicht zu einem kleinen, interaktiven Regelwerkzeug für `mailfilter-sqlite` entwickelt.

Es verbindet:

- reale Headeranalyse
- manuelle Kontextprüfung
- konservative und flexible Subject-Regeln
- Auth-/ARC-Auth-Speziallogik
- direktes Speichern zusätzlicher Regeln
- saubere Trennung über separate Include-Dateien

Damit eignet sich das Script gut für Administratoren, die auf Basis realer Header schnell und kontrolliert nachsteuern möchten.
