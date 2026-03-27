# INSTALL.md

## Overview

This guide describes how to build and deploy **mailfilter-sqlite** and set up the
runtime environment, including the `rulegen` subsystem.

---

## Requirements

- Linux/Unix system
- build tools (gcc/g++, make, autoconf/automake)
- SQLite3 (library + headers) , tested with v3.51.1 
- Perl
- Shell (bash)

Optional:
- fetchmail (for scheduled retrieval)

---

## Build & Install

### 1. Prepare source

Extract the source archive:

    tar xzf mailfilter-sqlite-2.0.1.tar.gz
    cd mailfilter-sqlite-2.0.1

### 2. Generate configure script (needed!)

    autoreconf -fi

### 3. Configure

    ./configure --enable-sqlite3-headerlog

### 4. Build

    make

### 5. Install

    make install

---

## Runtime Setup

### 1. Create directories

    mkdir -p /etc/mailfilter
    mkdir -p /etc/mailfilter/rulegen

---

### 2. Copy example configuration

From the repository:

    cp rulegen/conf/*.example /etc/mailfilter/rulegen/

Then rename as needed:

    cd /etc/mailfilter/rulegen
    for f in *.example; do cp "$f" "${f%.example}"; done

---

### 3. Main config

Ensure your main config exists:

    /etc/mailfilter/.mailfilterrc

Add include for generated rules (after testing!):

    INCLUDE="/etc/mailfilter/generated-rules.conf"

Better: 

    INCLUDE="/etc/mailfilter/generated-conservative-rules.conf"

---

## rulegen Setup

### 1. Place scripts

Copy tools:

    cp rulegen/bin/*.sh /usr/local/bin/
    cp rulegen/bin/*.pl /usr/local/bin/

Make executable:

    chmod +x /usr/local/bin/mailfilter-rulegen.sh
    chmod +x /usr/local/bin/mailfilter-rulegen.pl
    chmod +x /usr/local/bin/mailfilter-header-import.pl

---

### 2. Database location

Example:

    /var/spool/filter/mailheader.log.sqlite3

Ensure writable:

    mkdir -p /var/spool/filter
    chown -R root:root /var/spool/filter

---

## First Run

### 1. Collect headers

Run mailfilter in header mode (depending on your setup).

---

### 2. Generate rules

    mailfilter-rulegen.sh ... 

This will:

- read SQLite data
- analyze campaigns
- generate rule candidates
- export rules

---

### 3. Verify output

Check:

    /etc/mailfilter/generated-rules.conf

---

## Integration with fetchmail (optional)

Example (preconnect):

    preconnect "mailfilter ..."

This allows filtering before retrieval.

---

## Testing

Use separate (new) database:

	./mailfilter-header-import.pl --db mailheader-test.sqlite3 \
	--schema-from /etc/mailfilter/rulegen/testdata/mailheader-schema.sqlite3 --input /tmp --reset

Run generator on test DB.

	./mailfilter-rulegen.sh --db mailheader-test.sqlite3 --mailfilterrc /etc/mailfilter/.mailfilterrc \
	--out generated-candidates.conf --highscore 100 --min-deny-hits 2 --max-pass-hits 0 \
	--min-phrase-size 2 --max-phrase-size 3 --export-rules /etc/mailfilter/generated-rules.conf \
	--export-cons /etc/mailfilter/generated-conservative-rules.conf \
	--export-aggr /etc/mailfilter/generated-aggressive-rules.conf

---

## Notes

- Keep production and test databases separate
- Review generated rules before enabling aggressive modes
- Start with conservative configuration

---

## Troubleshooting

### SQLite not detected

Ensure:

    sqlite3.h installed
    libsqlite3 available

Re-run:

    ./configure --enable-sqlite3-headerlog

---

### No rules generated

Check:

- database contains data
- rulegen scripts executable
- config files present

---

## Summary

mailfilter-sqlite consists of:

- core filter (C/C++)
- SQLite logging
- external rule generation (rulegen)

Proper setup ensures a fully adaptive header-based filtering system.
