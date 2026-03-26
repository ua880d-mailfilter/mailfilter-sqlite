# QUICKSTART.md

## Goal

Get **mailfilter-sqlite + rulegen** running in ~5 minutes.

---

## 1. Voraussetzungen

- mailfilter (SQLite-Version) kompiliert und installiert? (Binary vorhanden, z. B. /usr/bin/mailfilter)
- SQLite vorhanden
- Perl installiert

---

## 2. Verzeichnisse anlegen

    mkdir -p /etc/mailfilter
    mkdir -p /etc/mailfilter/rulegen
    mkdir -p /var/spool/filter

---

## 3. Dateien kopieren

### rulegen Scripts

    cp rulegen/bin/* /etc/mailfilter/
    chmod +x /etc/mailfilter/*.sh
    chmod +x /etc/mailfilter/*.pl

### Konfigurationsdateien

    cp rulegen/conf/*.example /etc/mailfilter/rulegen/
    cd /etc/mailfilter/rulegen
    for f in *.example; do cp "$f" "${f%.example}"; done

---

## 4. mailfilter konfigurieren

Stelle sicher, dass existiert:

    /etc/mailfilter/.mailfilterrc

Füge (später) hinzu:

    INCLUDE="/etc/mailfilter/generated-rules.conf"

---

## 5. SQLite-DB vorbereiten

Beispiel:

    touch /var/spool/filter/mailheader.sqlite3

(mailfilter erzeugt Struktur automatisch beim Schreiben)

---

## 6. Header sammeln

mailfilter so ausführen, dass Header in SQLite geloggt werden  
(dein bestehendes Setup verwenden)

mailfilter --mailfilterrc=/etc/mailfilter/.mailfilterrc -t -v 3
---

## 7. Rulegen starten

    cd /etc/mailfilter 

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

## 8. Ergebnis prüfen

    less /etc/mailfilter/generated-candidates.conf
    less /etc/mailfilter/generated-rules.conf

---

## 9. Aktivieren

Die Regeln werden automatisch aktiv durch:

    INCLUDE="/etc/mailfilter/generated-rules.conf"

---

## Fertig

Du hast jetzt:

- Header-Daten in SQLite
- Analyse + Kampagnenerkennung
- automatisch generierte Regeln

---

## Hinweise

- `/etc/mailfilter/rulegen/` enthält **kritische Konfiguration**
- `/etc/mailfilter/` enthält **generierte Dateien**
- Regeldateien können jederzeit neu erzeugt werden
