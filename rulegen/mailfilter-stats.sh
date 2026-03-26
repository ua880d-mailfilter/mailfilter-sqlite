#!/bin/bash
###############################################################################
#
# This file is part of mailfilter-sqlite.
# mailfilter-sqlite is based on the original mailfilter project by (C) Andreas Bauer.
#
# Copyright (C) 2026 Rico Dummis <ua880d@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License ...
#
###############################################################################
#
# mailfilter-stats.sh
#
# Kleines Statistikwerkzeug für die mailfilter SQLite3-Datenbank(en).
#
# Zweck:
#   Schneller Überblick über den aktuellen Header-/Spam-Datenbestand.
#
# Optionen:
#   --db FILE        SQLite-Datenbank
#   --limit N        Anzahl der Ausgaben pro Block (Default: 15)
#
# Beispiel:
#   ./mailfilter-stats.sh --db /var/spool/filter/mailheader.log.sqlite3
#
###############################################################################

DB=""
LIMIT=15

while [[ $# -gt 0 ]]; do
    case "$1" in
        --db) DB="$2"; shift 2 ;;
        --limit) LIMIT="$2"; shift 2 ;;
        *)
            echo "Unknown option: $1"
            exit 1
        ;;
    esac
done

if [[ -z "$DB" ]]; then
    echo "Usage: $0 --db <sqlitefile> [--limit N]"
    exit 1
fi

if [[ ! -f "$DB" ]]; then
    echo "Database not found: $DB"
    exit 1
fi

command -v sqlite3 >/dev/null || {
    echo "sqlite3 not installed"
    exit 1
}

echo "============================================================"
echo "mailfilter SQLite statistics"
echo "database: $DB"
echo "============================================================"
echo

echo "== decisions =="
sqlite3 "$DB" "
SELECT decision, COUNT(*)
FROM messages
GROUP BY decision
ORDER BY COUNT(*) DESC;
"
echo

echo "== top tags =="
sqlite3 "$DB" "
SELECT tag, COUNT(*)
FROM header_entries
GROUP BY tag
ORDER BY COUNT(*) DESC
LIMIT $LIMIT;
"
echo

echo "== top from addresses =="
sqlite3 "$DB" "
SELECT body, COUNT(*)
FROM header_entries
WHERE tag='From'
GROUP BY body
ORDER BY COUNT(*) DESC
LIMIT $LIMIT;
"
echo

echo "== top reply-to values =="
sqlite3 "$DB" "
SELECT body, COUNT(*)
FROM header_entries
WHERE tag='Reply-To'
GROUP BY body
ORDER BY COUNT(*) DESC
LIMIT $LIMIT;
"
echo

echo "== top return-path values =="
sqlite3 "$DB" "
SELECT body, COUNT(*)
FROM header_entries
WHERE tag='Return-path'
GROUP BY body
ORDER BY COUNT(*) DESC
LIMIT $LIMIT;
"
echo

echo "== top list-unsubscribe values =="
sqlite3 "$DB" "
SELECT body, COUNT(*)
FROM header_entries
WHERE tag='List-Unsubscribe'
GROUP BY body
ORDER BY COUNT(*) DESC
LIMIT $LIMIT;
"
echo

echo "== top subjects =="
sqlite3 "$DB" "
SELECT subject, COUNT(*)
FROM messages
WHERE subject IS NOT NULL AND subject != ''
GROUP BY subject
ORDER BY COUNT(*) DESC
LIMIT $LIMIT;
"
echo

echo "== top deny rules =="
sqlite3 "$DB" "
SELECT expression, COUNT(*)
FROM rule_hits
WHERE phase='deny' AND matched=1
GROUP BY expression
ORDER BY COUNT(*) DESC
LIMIT $LIMIT;
"
echo

echo "== top score rules =="
sqlite3 "$DB" "
SELECT expression, COUNT(*), SUM(COALESCE(score_delta,0))
FROM rule_hits
WHERE phase='score' AND matched=1
GROUP BY expression
ORDER BY COUNT(*) DESC, SUM(COALESCE(score_delta,0)) DESC
LIMIT $LIMIT;
"
echo

echo "== top scored messages =="
sqlite3 "$DB" "
SELECT final_score, subject
FROM messages
WHERE final_score > 0
ORDER BY final_score DESC, subject
LIMIT $LIMIT;
"
echo
