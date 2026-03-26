# QUICKSTART.md

## Goal
Get mailfilter-sqlite + rulegen running in ~5 minutes.

---

## 1. Requirements
- mailfilter installed
- SQLite available
- Perl installed

---

## 2. Create directories
mkdir -p /etc/mailfilter
mkdir -p /etc/mailfilter/rulegen
mkdir -p /var/spool/filter

---

## 3. Prepare database

Option A (recommended):
Copy provided schema template:
cp testdata/mailheader-schema.sqlite3 /var/spool/filter/mailheader.sqlite3

Option B:
Let mailfilter create it automatically during logging

Option C:
Create via import script from test data

---

## 4. Copy files
cp rulegen/bin/* /etc/mailfilter/
chmod +x /etc/mailfilter/*.sh
chmod +x /etc/mailfilter/*.pl

cp rulegen/conf/*.example /etc/mailfilter/rulegen/
cd /etc/mailfilter/rulegen
for f in *.example; do cp "$f" "${f%.example}"; done

---

## 5. Configure mailfilter
Ensure:
INCLUDE="/etc/mailfilter/generated-rules.conf"

---

## 6. Run rulegen

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

## Done
