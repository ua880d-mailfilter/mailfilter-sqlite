# CONFIGURATION.md

## Überblick

Dieses Dokument beschreibt alle Konfigurationsdateien des **rulegen-Subsystems**
von mailfilter-sqlite.

Alle persistenten Konfigurationsdateien befinden sich unter:

    /etc/mailfilter/rulegen/

Generierte Regeldateien werden geschrieben mit/nach:

   --export-rules /etc/mailfilter/generated-rules.conf
   --export-cons /etc/mailfilter/generated-conservative-rules.conf 
   --export-aggr /etc/mailfilter/generated-aggressive-rules.conf

Diese Trennung ist bewusst gewählt und schützt kritische Konfigurationen vor
versehentlichem Löschen.

---

## Allgemeines Format

Die meisten Konfigurationsdateien folgen einem einfachen zeilenbasierten Format:

    value1|value2|value3|...

- Zeilen mit `#` sind Kommentare
- Leere Zeilen werden ignoriert

---

## protected_domains.conf

### Zweck

Definiert vertrauenswürdige Domains und deren Schutzlevel.

Dies ist die **wichtigste Konfigurationsdatei** im gesamten System.

### Format

    <domain>|<level>|<category>|<description>

### Beispiel

    fedex.com|2|logistics|parcel service
    ipfire.org|3|project|trusted project

### Felder

- **domain**  
  Domain, die geschützt werden soll

- **level**  
  Schutzlevel:
  - 1 → schwach geschützt, keine automatische DENY-Regel
  - 2 → mittel (keine DENY-Regel, nur SCORE-Regeln erlaubt)
  - 3 → stark (keine DENY-Regel erlaubt, keine Regeln empfohlen)

- **category**  
  Logische Klassifikation (e.g. logistics, finance, project, isp, dev, shop, media)

- **description**  
  Freitext für Dokumentation und Debugging/Info

### Wirkung

- reduziert False Positives
- beeinflusst das Scoring
- kann Regelgenerierung vollständig verhindern

### Warnung

Diese Datei darf nicht gelöscht oder unbeabsichtigt überschrieben werden.

---

## allow_subject_tokens.conf

### Zweck

Definiert Tokens, die legitime Nachrichten kennzeichnen.

### Beispiel

    invoice
    shipping confirmation
    account update

### Wirkung

- wird für ALLOW-Proximity verwendet
- reduziert False Positives

---

## bulk_providers.conf

### Zweck

Liste bekannter Bulk-Mail-/ESP-Anbieter.

### Beispiel

    mailchimp.com
    sendgrid.net

### Wirkung

- hilft bei der Klassifikation von Traffic
- reduziert False Positives bei legitimen Bulk-Mails

---

## weak_subject_tokens.conf

### Zweck

Tokens mit geringer Aussagekraft.

### Beispiel

    update
    notification
    message

### Wirkung

- verhindert, dass schwache Tokens alleine Regeln auslösen

---

## brand_domains.conf

### Zweck

Referenzliste für Marken zur Phishing-Erkennung.

### Beispiel

    amazon
    paypal
    dhl
    github
    pollin
    google
	
### Wirkung

- unterstützt Fake-Brand-Erkennung
- hilft bei der Erkennung von Typosquatting

---

## Verzeichnisstruktur

### Persistente Konfiguration

    /etc/mailfilter/rulegen/

Enthält:

- protected_domains.conf
- allow_subject_tokens.conf
- bulk_providers.conf
- weak_subject_tokens.conf
- brand_domains.conf

### Generierte Dateien:

    /etc/mailfilter/

Enthält:

- generated-candidates.conf
- generated-rules.conf
- generated-conservative-rules.conf
- generated-aggressive-rules.conf

---

## Beste Vorgehensweise:

- Generierte Regeln zunächst vor Aktivierung prüfen
- Mit konservativen Einstellungen beginnen
- Konfigurationsdateien optional versionieren (z. B. Git)
- Eigene Einträge mit Kommentaren dokumentieren

---

## Hinweise

Das Konfigurationssystem ist bewusst einfach, transparent und nachvollziehbar gestaltet.

Alle Entscheidungen basieren auf:

- strukturierten Daten
- expliziten Regeln
- deterministischer Auswertung

Es gibt keine versteckte Logik und kein Machine Learning.
