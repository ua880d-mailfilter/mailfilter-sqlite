mailfilter synthetic starter test set
=================================

Inhalt
------
Dieses Starterset enthält nur synthetische Header-Beispiele. Die Bodies sind
absichtlich bedeutungslos. Ziel ist ein kontrollierbarer Import in eine
separate Test-DB.

Verzeichnisstruktur
-------------------
spam/synthetic/
ham/synthetic/

Vorgeschlagene Nutzung
----------------------
1. Separate Test-DB aus der Live-DB-Schema-Struktur erzeugen
2. Spam-Dateien mit --decision deny importieren
3. Ham-Dateien mit --decision pass importieren
4. mailfilter-rulegen gegen die Test-DB laufen lassen

Beispiel
--------
Spam importieren:
  ./mailfilter-header-import.pl \
    --db /var/spool/filter/mailheader-test.sqlite3 \
    --schema-from /var/spool/filter/mailheader.log.sqlite3 \
    --input spam/synthetic \
    --decision deny \
    --id-prefix tstspam \
    --reset

Ham importieren:
  ./mailfilter-header-import.pl \
    --db /var/spool/filter/mailheader-test.sqlite3 \
    --input ham/synthetic \
    --decision pass \
    --id-prefix tstham

Anschließend:
  ./mailfilter-rulegen.sh --db /var/spool/filter/mailheader-test.sqlite3 ...

Ziel der Beispiele
------------------
Spam:
- promo / urgency / price patterns
- domain / campaign indicators
- fake trusted-brand patterns
- old historical bounce pattern

Ham:
- protected domains
- allow-near subjects (invoice, paket, versandt, password reset)
- typische Shop-/Provider-/Plattform-Mails
