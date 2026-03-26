#!/bin/bash
#
# This file is part of mailfilter-sqlite.
# mailfilter-sqlite is based on the original mailfilter project by (C) Andreas Bauer.
#
# Copyright (C) 2026 Rico Dummis <ua880d@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License ...
#
# ==========================================================
# mailfilter-rulegen.sh
# ==========================================================
#
# Zweck:
#   Shell-Wrapper für die Generierung von Mailfilter-Regeln
#   auf Basis einer SQLite-Header-Datenbank.
#
# Aufgaben:
#   - Extraktion relevanter Daten via sqlite3
#   - Aufbereitung der Daten (Domains, Subjects, Hosts)
#   - Übergabe an mailfilter-rulegen.pl
#   - Steuerung von Parametern und Filtern
#
# Hauptparameter:
#   --db <file>                 SQLite-Datenbank (Quelle)
#   --mailfilterrc <file>       bestehende Regeln (optional für internen Abgleich)
#   --out <file>                Kandidaten-Ausgabe (Prüfung/Ergebnisse/Vorschläge)
#   --export-rules <file>       generierte Rules-Datei (Standard)
#   --export-cons <file>        generierte Rules-Datei (conservative)
#   --export-aggr <file>        generierte Rules-Datei (aggressieve)
#
# Filterparameter:
#   --min-deny-hits <n>
#   --max-pass-hits <n>
#   --min-phrase-size <n>
#   --max-phrase-size <n>
#   --highscore <n>
#
# Zeitparameter:
#   --time-threshold-days <n>   (Default: 365)
#
# Ausgabe:
#   1. Analyse-Datei:
#      generated-candidates.conf
#
#   2. Optionale Export-Datei:
#      generated-rules.conf
#
#   3. Optionale Export-Datei:
#      generated-conservative-rules.conf
#
#   4. Optionale Export-Datei:
#      generated-aggresive-rules.conf
#
# Workflow:
#   1. SQL-Abfragen auf SQLite DB
#   2. Aggregation von:
#        - Subject-Phrasen
#        - Domains
#        - Received Hosts
#   3. Übergabe an Perl-Engine
#   4. Ausgabe + optionaler Export
#
# Besonderheiten:
#   - funktioniert unabhängig von MTA
#   - ideal für periodische Ausführung (Cron)
#   - unterstützt Test-DBs
#
# ==========================================================
#
#
# ==========================================================
###############################################################################
#
# mailfilter-rulegen.sh
#
# Analysewerkzeug für mailfilter-SQLite-Logs zur Generierung neuer
# Regex-Regelkandidaten.
#
# Dieses Script liest die SQLite-Datenbank, extrahiert relevante Daten
# (Subjects und Header) und übergibt sie an das Perl-Auswertungsskript
# mailfilter-rulegen.pl.
#
# Ziel:
#   Unterstützung beim manuellen Aufbau neuer mailfilter-Regeln.
#
# Es werden KEINE Regeln automatisch aktiviert.
# Alle Vorschläge werden auskommentiert ausgegeben.
#
# ---------------------------------------------------------------------------
#
# Optionen
#
# --db FILE
#       SQLite-Datenbank mit mailfilter Headerlogs
#
# --out FILE
#       Zieldatei für erzeugte Regelkandidaten
#       Default: ./generated-candidates.conf
#
# --mailfilterrc FILE
#       Bestehende mailfilterrc zum Vergleich mit vorhandenen Regeln
#
# --highscore N
#       Referenzwert für HIGHSCORE aus mailfilterrc
#       Default: 100
#
# --min-deny-hits N
#       Mindestanzahl an Treffer in Spam-Mails
#       Default: 5
#
# --max-pass-hits N
#       Maximale Treffer in legitimen Mails
#       Default: 0
#
# --min-phrase-size N
#       Minimale Wortanzahl pro Phrase
#       Default: 1
#
# --max-phrase-size N
#       Maximale Wortanzahl pro Phrase
#       Default: 3
#
# --protected-domains FILE
#       Datei mit geschützten Domains (policy-Datei!)
#
# --bulk-providers FILE
#       Datei mit Bulk-Mail/ESP-Domains
#
# --allow-subject-tokens FILE
#       Datei mit zulässigen Subject-Tokens
#
# --weak-subject-tokens FILE
#       Datei mit schwachen Subject-Tokens
#
###############################################################################

DB=""
OUT="./generated-candidates.conf"
PROTECTED_DOMAINS="/etc/mailfilter/rulegen/protected_domains.conf"
BULK_PROVIDERS="/etc/mailfilter/rulegen/bulk_mail_providers.conf"
WEAK_SUBJECT_TOKENS="/etc/mailfilter/rulegen/weak_subject_tokens.conf"
ALLOW_SUBJECT_TOKENS="/etc/mailfilter/rulegen/allow_subject_tokens.conf"
BRAND_DOMAINS="/etc/mailfilter/rulegen/brand_domains.conf"
EXPORT_RULES=""
EXPORT_CONS=""
EXPORT_AGGR=""
MAILFILTERRC=""

HIGHSCORE=100
MIN_DENY_HITS=5
MAX_PASS_HITS=0
MIN_PHRASE=1
MAX_PHRASE=3

while [[ $# -gt 0 ]]; do
    case "$1" in
        --db) DB="$2"; shift 2 ;;
        --out) OUT="$2"; shift 2 ;;
        --mailfilterrc) MAILFILTERRC="$2"; shift 2 ;;
        --highscore) HIGHSCORE="$2"; shift 2 ;;
        --min-deny-hits) MIN_DENY_HITS="$2"; shift 2 ;;
        --max-pass-hits) MAX_PASS_HITS="$2"; shift 2 ;;
        --min-phrase-size) MIN_PHRASE="$2"; shift 2 ;;
        --max-phrase-size) MAX_PHRASE="$2"; shift 2 ;;
		--protected-domains) PROTECTED_DOMAINS="$2"; shift 2 ;;
		--bulk-providers) BULK_PROVIDERS="$2"; shift 2 ;;
		--weak-subject-tokens) WEAK_SUBJECT_TOKENS="$2"; shift 2 ;;
		--allow-subject-tokens) ALLOW_SUBJECT_TOKENS="$2"; shift 2 ;;
		--brand-domains) BRAND_DOMAINS="$2"; shift 2 ;;
		--export-rules) EXPORT_RULES="$2"; shift 2 ;;
		--export-cons)  EXPORT_CONS="$2"; shift 2 ;;
		--export-aggr)  EXPORT_AGGR="$2"; shift 2 ;;
        *)
            echo "Unknown option: $1"
            exit 1
        ;;
    esac
done

if [[ -z "$DB" ]]; then
    echo "Usage: $0 --db <sqlitefile>"
    exit 1
fi

if [[ ! -f "$DB" ]]; then
    echo "Database not found: $DB"
    exit 1
fi

if [[ -n "$MAILFILTERRC" && ! -f "$MAILFILTERRC" ]]; then
    echo "mailfilterrc not found: $MAILFILTERRC"
    exit 1
fi

#if [ -n "$EXPORT_RULES" ]; then
#  PERL_ARGS="$PERL_ARGS --export-rules \"$EXPORT_RULES\""
#fi

command -v sqlite3 >/dev/null || {
    echo "sqlite3 not installed"
    exit 1
}

TMP=$(mktemp)

sqlite3 "$DB" <<SQL > "$TMP.subjects"
SELECT msg_log_id || '|' || decision || '|' || subject
FROM messages
WHERE subject IS NOT NULL
AND subject != '';
SQL

sqlite3 "$DB" <<SQL > "$TMP.headers"
SELECT h.msg_log_id || '|' || m.decision || '|' || h.tag || '|' || h.body
FROM header_entries h
JOIN messages m ON m.msg_log_id = h.msg_log_id
WHERE h.tag IN ('Received','From','Reply-To','Return-path','Sender','List-Unsubscribe','Date','Delivery-date');
SQL

PERL_ARGS=(
  --subjects "$TMP.subjects"
  --headers "$TMP.headers"
  --highscore "$HIGHSCORE"
  --min-deny "$MIN_DENY_HITS"
  --max-pass "$MAX_PASS_HITS"
  --min-phrase "$MIN_PHRASE"
  --max-phrase "$MAX_PHRASE"
  --protected-domains "$PROTECTED_DOMAINS"
  --bulk-providers "$BULK_PROVIDERS"
  --weak-subject-tokens "$WEAK_SUBJECT_TOKENS"
  --allow-subject-tokens "$ALLOW_SUBJECT_TOKENS"
  --brand-domains "$BRAND_DOMAINS"
)

if [[ -n "$MAILFILTERRC" ]]; then
  PERL_ARGS+=( --mailfilterrc "$MAILFILTERRC" )
fi

if [[ -n "$EXPORT_RULES" ]]; then
  PERL_ARGS+=( --export-rules "$EXPORT_RULES" )
fi

if [[ -n "$EXPORT_CONS" ]]; then
  PERL_ARGS+=( --export-cons "$EXPORT_CONS" )
fi

if [[ -n "$EXPORT_AGGR" ]]; then
  PERL_ARGS+=( --export-aggr "$EXPORT_AGGR" )
fi

perl mailfilter-rulegen.pl "${PERL_ARGS[@]}" > "$TMP.out" || {
    rm -f "$TMP.subjects" "$TMP.headers" "$TMP.out"
    exit 1
}

mv "$TMP.out" "$OUT"
rm -f "$TMP.subjects" "$TMP.headers"

echo "Generated candidates written to $OUT"
