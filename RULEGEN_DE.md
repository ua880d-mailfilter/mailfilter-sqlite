# RULEGEN.md

## Überblick

Das **rulegen-Subsystem** ist die zentrale Intelligenzschicht von mailfilter-sqlite.

Es transformiert strukturierte Header-Daten aus SQLite in:

- analysierte Kampagnen-Signaturen
- bewertete Risikoeinstufungen
- strukturierte Regelkandidaten
- exportierbare mailfilter-Regeldateien

Das System ist deterministisch, nachvollziehbar und vollständig datengetrieben.

---

## Verarbeitungs-Pipeline

1. Header-Erfassung durch mailfilter
2. Speicherung in SQLite
3. Extraktion und Aggregation (mailfilter-rulegen.sh)
4. Analyse und Bewertung (mailfilter-rulegen.pl)
5. Generierung von Kandidaten
6. Export von Regeln

---

## Kernkomponenten

### mailfilter-rulegen.sh

Verantwortlich für:

- SQL-Datenextraktion
- Aggregation von:
  - Subject-Phrasen
  - Absender-Domains
  - Received-Hosts
- Parameterverarbeitung
- Steuerung der Pipeline

---

### mailfilter-rulegen.pl

Zentrale Analyse-Engine:

- Kampagnenerkennung
- Clustering ähnlicher Nachrichten
- Bewertung und Klassifizierung
- Fake-Brand-Erkennung
- Regelgenerierung

---

## Eingabedaten

### SQLite-Tabellen

- `messages`
- `header_entries`
- `rule_hits`

Diese liefern:

- Metadaten der Nachrichten
- zerlegte Header
- Ergebnisse der Regelbewertung

---

## Policy-Modell

Konfigurationsdateien befinden sich unter:

/etc/mailfilter/rulegen/

### protected_domains.conf

Format:

    <domain>|<level>|<category>|<description>

Beispiel:

    fedex.com|2|logistics|parcel service
    ipfire.org|3|project|trusted project

Definiert:

- Vertrauenslevel (1–3)
- semantische Klassifikation
- Kontextinformationen

Auswirkung:

- Reduzierung von False Positives
- Anpassung der Bewertung
- ggf. vollständige Unterdrückung von Regeln

---

### allow_subject_tokens.conf

Definiert erlaubte Subject-Tokens.

Verwendung:

- ALLOW-Proximity-Erkennung
- False-Positive-Vermeidung

---

### bulk_providers.conf

Bekannte Bulk-Mail-/ESP-Anbieter.

Verwendung:

- Klassifizierung von Traffic
- Reduktion von False Positives

---

### weak_subject_tokens.conf

Tokens mit geringer Aussagekraft, die allein keine Regeln auslösen sollen.

---

### brand_domains.conf

Referenzliste für Brand-/Phishing-Erkennung.

---

## Bewertungskonzepte

### Zeitbasierte Bewertung

- `recent` → aktuell relevant
- `aged` → eingeschränkt relevant
- `stale` → nur historisch (kein Regelexport)

---

### Protected Domains

- Level 1 → schwach geschützt
- Level 2 → mittel (nur SCORE)
- Level 3 → stark (keine Regelgenerierung)

---

### Empfehlungstypen

- `allow`
- `score`
- `deny`
- `manual review`
- `high-risk phishing candidate`

---

## Kampagnenerkennung

Nachrichten werden gruppiert anhand von:

- Subject-Ähnlichkeit
- Absender-Domain-Mustern
- Infrastruktur (Received-Hosts)

Jede Kampagne liefert:

- aggregierte Statistiken
- repräsentative Beispiele
- Regelvorschläge

---

## Fake-Brand-Erkennung

Erkennt:

- Typosquatting-Domains
- Marken-Imitationen
- verdächtige Domain-Muster

Verwendet:

- brand_domains.conf
- Ähnlichkeitsheuristiken

---

## Regelgenerierung

### Kandidatenblöcke

Werden erzeugt in:

    generated-candidates.conf

Enthalten:

- Metadaten (Confidence, Risiko, Häufigkeit)
- Begründung (Warum erkannt)
- vorgeschlagene Regeln

---

### Exportierte Regeln

Werden erzeugt in:

- generated-rules.conf
- generated-conservative-rules.conf
- generated-aggressive-rules.conf

---

## Exportbedingungen

Regeln werden nur exportiert, wenn:

- nicht `stale`
- kein Konflikt mit stark geschützten Domains besteht
- kein ALLOW-Konflikt vorliegt
- ausreichendes Risiko bzw. Relevanz vorhanden ist

---

## CLI-Steuerung (mailfilter-rulegen.sh)

Beispiel:

    ./mailfilter-rulegen.sh \
      --db /var/spool/filter/mailheader.sqlite3 \
      --mailfilterrc /etc/mailfilter/.mailfilterrc \
      --out generated-candidates.conf \
      --highscore 100 \
      --min-deny-hits 2 \
      --max-pass-hits 0 \
      --min-phrase-size 2 \
      --max-phrase-size 3 \
      --export-rules /etc/mailfilter/generated-rules.conf \
      --export-cons /etc/mailfilter/generated-conservative-rules.conf \
      --export-aggr /etc/mailfilter/generated-aggressive-rules.conf

---

## Laufzeit-Trennung

Persistente Konfiguration:

    /etc/mailfilter/rulegen/

Generierte Ausgaben:

    /etc/mailfilter/

Diese Trennung schützt wichtige Policy-Dateien vor versehentlichem Löschen.

---

## Designprinzipien

- deterministische Analyse (kein Machine Learning erforderlich)
- nachvollziehbare Entscheidungen
- Trennung von Core und Intelligenzschicht
- reproduzierbare Ergebnisse
- sichere Regelgenerierung (keine automatische Aktivierung)

---

## Zusammenfassung

Das rulegen-Subsystem wandelt rohe Header-Daten in:

- strukturiertes Wissen
- Kampagnen-Intelligenz
- verwertbare mailfilter-Regeln

Es ermöglicht eine dynamische Weiterentwicklung von mailfilter bei voller Kontrolle und Transparenz.
