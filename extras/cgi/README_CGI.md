# mailfilter.cgi

Interaktive CGI-Oberfläche für **mailfilter-sqlite** zur Analyse gespeicherter Mail-Header, zur schnellen Ableitung von `mailfilter`-Regeln und zum direkten Schreiben zusätzlicher Regeln in eine separate Include-Datei.

Diese Komponente ist als leichtgewichtiges Web-Frontend für kleine bis mittlere Administrationsumgebungen gedacht, insbesondere für Systeme wie **IPFire**, auf denen Header bereits durch **mailfilter-sqlite** in einer SQLite-Datenbank protokolliert werden.

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

1. **Sichtung auffälliger oder häufiger Betreffzeilen** innerhalb eines konfigurierbaren Zeitraums
2. **Analyse vollständiger Header einer konkreten Nachricht** über `msg_log_id`
3. **Interaktive Generierung und Speicherung zusätzlicher Regeln** (`DENY`, `SCORE`, `ALLOW`)

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
- Prüfung konkreter Header (`From`, `Return-path`, `Subject`, `List-Unsubscribe`, `X-Mailer`, …)
- direktes Speichern neuer Regeln in eine zusätzliche Include-Datei

Beide Werkzeuge ergänzen sich sehr gut:

- **analyze.py** für Statistik, Brainstorming und Automatisierung
- **mailfilter.cgi** für interaktive Nacharbeit und manuelle Präzisierung

---

## Datenbasis

`mailfilter.cgi` arbeitet direkt auf der von **mailfilter-sqlite** gefüllten SQLite-Datenbank, typischerweise:

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

Die Oberfläche filtert standardmäßig anhand von **`date_hdr`**, also des echten Datums aus dem Mail-Header.

Das ist bei `mailfilter-sqlite` bewusst sinnvoll, weil `created_at` bei vollständigen POP3-Abfragen bzw. beim erstmaligen Import vieler älterer Mails auf denselben Erfassungszeitpunkt fallen kann. Für Administratoren ist in solchen Fällen das tatsächliche Mail-Datum in `date_hdr` wesentlich aussagekräftiger.

---

## Hauptfunktionen der Oberfläche

## 1. Zeitraum- und Listenfilter

Im oberen Bereich der Oberfläche kann gesteuert werden:

- Zeitraum: z. B. `1`, `2`, `5`, `10`, `20`, `30`, `60`, `90`, `180`, `365` Tage
- maximale Anzahl der anzuzeigenden Top-Einträge, z. B. `5`, `10`, `15`, `20`, `30`, `50`

Die Ansicht bleibt beim Speichern neuer Regeln erhalten.

---

## 2. Decision-Verteilung

Die Oberfläche zeigt die aktuelle Verteilung der in `messages.decision` gespeicherten Entscheidungen für den gewählten Zeitraum, z. B.:

- `pass`
- `allow`
- `deny`
- `unknown`

So lässt sich schnell erkennen, ob sich innerhalb des gefilterten Zeitraums auffällige Muster häufen.

---

## 3. Top Subjects

Anschließend werden die häufigsten Betreffzeilen im gewählten Zeitraum angezeigt, inklusive:

- Anzahl
- durchschnittlichem Score
- Betreffzeile
- Beispiel-`msg_log_id`

Jeder Eintrag repräsentiert eine gruppierte Sicht auf eine Betreffzeile und dient als Einstieg für die weitere Detailanalyse.

---

## 4. Header-Modal

Über den Button **`Header`** lässt sich für die zugehörige `msg_log_id` ein eigenes Modal öffnen, das alle zugehörigen Header aus `header_entries` zeigt.

Dadurch kann ein Administrator sofort prüfen:

- welche Header tatsächlich vorhanden sind
- in welcher Form `From`, `Return-path`, `Subject`, `List-Unsubscribe` etc. vorliegen
- ob bestimmte Header als Regelbasis besser geeignet sind als der Betreff selbst

Das ist besonders nützlich, um stabile Regeln zu identifizieren.

---

## 5. Rule-Modal

Über Klick auf die Tabellenzeile oder den Button **`Rule`** öffnet sich das Rule-Modal.

Dort stehen aktuell folgende Funktionen zur Verfügung:

- Auswahl des Regeltyps:
  - `DENY`
  - `SCORE`
  - `PASS` / `ALLOW`
- Eingabe eines Score-Werts für `SCORE`
- Auswahl eines konkreten Header-Tags aus den wirklich vorhandenen Headern der Nachricht
- automatische Vorschau einer geeigneten Regelbasis
- Generierung der finalen `mailfilter`-Regel
- Kopieren der Regel in die Zwischenablage
- direktes Speichern der Regel in eine separate Include-Datei

---

## Strategien der Regelgenerierung

Die Oberfläche versucht, für unterschiedliche Header-Typen sinnvolle Regelvorschläge zu erzeugen.

## `From`, `Return-path`, `Reply-To`

Hier wird bevorzugt eine E-Mail-Adresse extrahiert. Das ist oft die stabilste Basis für:

- `ALLOW`
- gezielte `DENY`
- moderate `SCORE`

Beispiel:

```text
SCORE +50="^From:.*produkte@service\.freenet\.de"
```

Wichtig: Wird explizit `From` oder `Return-path` gewählt, fällt die Logik nicht stillschweigend auf `Subject` zurück.

---

## `List-Unsubscribe`

Falls vorhanden, kann `List-Unsubscribe` besonders bei Newslettern oder Massenmailern eine stabile Regelbasis sein. Es wird bevorzugt versucht, eine Adresse oder einen markanten Teilwert zu extrahieren.

---

## `Subject`

Für `Subject` existieren bewusst zwei Strategien:

### Konservativ

Ziel ist eine eher präzise und kurze Regel mit **wenigen starken Tokens**.

Aktuell basiert die konservative Strategie im Wesentlichen auf:

- Vorverarbeitung des Betreffs
- Entfernung bzw. Entwertung dynamischer Zahlen-/Preisbestandteile
- Erkennung und Erhalt domain-artiger Tokens
- Gewichtung stärkerer Tokens
- Auswahl der **drei stärksten Tokens** als Basis

Beispiel:

```text
ALLOW="^Subject:.*HS-PX410.*original.*packing"
```

Diese Variante eignet sich besonders für:

- `ALLOW`
- präzisere `DENY`
- engere, nachvollziehbare Subject-Regeln

### Gruppiert / flexibel

Die flexible Strategie ist für längere oder kampagnenartige Betreffzeilen gedacht.

Hier werden bevorzugt:

- Promo-/Werbewörter gebündelt
- starke Inhalts-/Produktbegriffe gruppiert
- maximal wenige, kompakte Gruppen gebildet

Beispiel:

```text
SCORE +50="^Subject:.*(Weekend|Clearance|Sale).*(Eachine|E200|Helicopter)"
```

Diese Variante eignet sich besonders für:

- `SCORE`
- ähnliche oder leicht variierte Werbe-/Spam-Betreffzeilen
- breitere, aber noch kontrollierte Subject-Regeln

### Empfehlung

Im praktischen Einsatz gilt meist:

- **ALLOW** eher konservativ
- **SCORE** oft flexibel
- **DENY** mit Subject-Regeln vorsichtig einsetzen

Wo immer möglich, sind stabile Header wie `From` oder `Return-path` oft besser als ein reiner Subject-Match.

---

## Speichern von Regeln

Unterhalb der erzeugten Regel stehen aktuell zwei Aktionen zur Verfügung:

- **`Regel kopieren`**
- **`Regel speichern`**

Beim Speichern wird die Regel in folgende Datei geschrieben:

```text
/etc/mailfilter/mailfilter-extra-rules-cgi.conf
```

Vor dem Schreiben wird geprüft, ob dieselbe Regel bereits vorhanden ist.

- existiert sie bereits, wird nichts doppelt gespeichert
- existiert sie noch nicht, wird sie **append** angefügt

Jede gespeicherte Regel erhält zusätzlich einen Kommentar mit der zugehörigen `msg_log_id`, z. B.:

```text
### msg_log_id: hdr-157 ###
SCORE +50="^Subject:.*(Letzte|Chance|Rabatt).*(Sichere|werbefreies|Postfach)"
```

Das erleichtert die Rückverfolgung, woher eine Regel stammt.

---

## Warum eine separate Regeldatei?

Die Speicherung in eine separate Datei ist bewusst Teil des Konzepts.

`mailfilter` kann über `INCLUDE` weitere Konfigurationsdateien einbinden. Dadurch lässt sich die Gesamtregelbasis sauber strukturieren.

Beispielsweise können getrennt gepflegt werden:

- Hauptkonfiguration
- manuelle Regeln
- automatisch erzeugte Regeln
- per CGI interaktiv gespeicherte Regeln
- spätere Rule-Generator-Ausgaben

Für kleine und mittlere Administrationsumgebungen ist das deutlich wartbarer als eine einzige große Konfigurationsdatei.

---

## Einbindung in `mailfilterrc`

Die zusätzliche Datei muss in der eigentlichen Konfiguration eingebunden werden, z. B.:

```text
INCLUDE = "/etc/mailfilter/mailfilter-extra-rules-cgi.conf"
```

Die genaue Einbindung hängt vom restlichen lokalen Aufbau ab. Wichtig ist nur, dass die Datei von `mailfilter` beim Lauf berücksichtigt wird.

---

## Installationshinweise

## Voraussetzungen

Benötigt werden mindestens:

- ein funktionierendes `mailfilter-sqlite`
- eine SQLite-Datenbank mit den Tabellen `messages` und `header_entries`
- ein Webserver mit CGI-Unterstützung
- Python 3
- Schreibrechte für die zusätzliche Regeldatei, falls die Save-Funktion genutzt werden soll

Typischer Zielpfad für die Datenbank:

```text
/var/spool/filter/mailheader.sqlite3
```

Typischer Zielpfad für die per CGI gespeicherten Regeln:

```text
/etc/mailfilter/mailfilter-extra-rules-cgi.conf
```

---

## Ablage im Repository

Empfohlene Projektstruktur:

```text
extras/
├── cgi/
│   └── mailfilter.cgi/
│       ├── mailfilter.cgi
│       └── readme_cgi.md
└── python/
    └── mailfilter-analyze.py
```

---

## Beispielinstallation auf dem Zielsystem

### 1. CGI-Script kopieren

Beispielhaft nach:

```text
/srv/web/ipfire/cgi-bin/mailfilter.cgi
```

oder in das jeweilige CGI-Verzeichnis des Zielsystems.

### 2. Ausführbar machen

```bash
chmod 0755 /srv/web/ipfire/cgi-bin/mailfilter.cgi
```

### 3. Regeldatei anlegen

Falls die zusätzliche CGI-Regeldatei noch nicht existiert:

```bash
touch /etc/mailfilter/mailfilter-extra-rules-cgi.conf
chmod 0644 /etc/mailfilter/mailfilter-extra-rules-cgi.conf
```

Die Rechte müssen zur Webserver-/CGI-Umgebung passen. Falls der Webserver die Datei direkt beschreiben soll, sind die Eigentümer- und Gruppenrechte entsprechend anzupassen.

### 4. Include in `mailfilterrc` ergänzen

```text
INCLUDE = "/etc/mailfilter/mailfilter-extra-rules-cgi.conf"
```

### 5. CGI im Browser aufrufen

Beispiel:

```text
https://<system>/cgi-bin/mailfilter.cgi
```

---

## Sicherheits- und Betriebsaspekte

Die CGI-Oberfläche ist bewusst für kontrollierte Administratorumgebungen gedacht.

Empfehlungen:

- nur intern oder abgesichert erreichbar machen
- Schreibrechte für die Regeldatei bewusst vergeben
- Webserver-Logs im Blick behalten
- generierte Regeln regelmäßig prüfen
- insbesondere `ALLOW`-Regeln mit Bedacht einsetzen

Da Regeln direkt gespeichert werden können, sollte der Zugriff auf die Oberfläche nicht unkontrolliert offen sein.

---

## Grenzen der aktuellen Version

Der aktuelle Stand ist bewusst pragmatisch und leichtgewichtig. Einige mögliche spätere Ausbaustufen sind:

- Anzeige zusätzlicher Details aus `rule_hits`
- manuelles Selektieren einzelner Subject-Tokens per Klick
- gruppenweise Auswahl stärkerer/flexibler Subject-Regeln
- getrennte Ausgabedateien für `ALLOW`, `SCORE`, `DENY`
- direkte Bearbeitung oder Deaktivierung bereits gespeicherter Regeln
- Referenzierung gespeicherter Regeln innerhalb der Oberfläche
- stärkere heuristische Unterscheidung zwischen legitimen Mails und Kampagnen-/Werbemustern

---

## Fazit

`mailfilter.cgi` ist als schlanke Web-Oberfläche ein sehr praktischer Baustein für **mailfilter-sqlite**.

Die Kombination aus:

- SQLite-basierter Headeranalyse
- interaktiver Sichtung
- Header-Detailansicht
- konservativer und flexibler Regelgenerierung
- direktem Speichern in eine separate Include-Datei

macht das Script besonders nützlich für Administratoren, die in kleinen oder mittleren Umgebungen Mailverkehr schnell analysieren und regelbasiert nachsteuern wollen.

Durch die Trennung von Hauptkonfiguration, Generatorlogik und CGI-regelbasiertem Schreiben bleibt die Gesamtkonfiguration sauber, nachvollziehbar und erweiterbar.
