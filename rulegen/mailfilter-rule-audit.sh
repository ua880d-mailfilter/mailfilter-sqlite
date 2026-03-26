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
###############################################################################
#
# mailfilter-rule-audit.sh
#
# Vergleicht bestehende Regeln aus der mailfilterrc mit den tatsächlich in der
# SQLite-Datenbank protokollierten Treffern.
#
# Zweck:
#   - aktive Regeln erkennen
#   - inaktive Regeln erkennen
#   - Kandidaten für Bereinigung / Pflege finden
#
# Optionen:
#   --db FILE           SQLite-Datenbank
#   --mailfilterrc FILE mailfilterrc
#   --show inactive     nur inaktive Regeln
#   --show active       nur aktive Regeln
#   --show all          alles (Default)
#
# Beispiel:
#   ./mailfilter-rule-audit.sh \
#       --db /var/spool/filter/mailheader.log.sqlite3 \
#       --mailfilterrc /etc/mailfilter/.mailfilterrc
#
###############################################################################

DB=""
RC=""
SHOW="all"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --db) DB="$2"; shift 2 ;;
        --mailfilterrc) RC="$2"; shift 2 ;;
        --show) SHOW="$2"; shift 2 ;;
        *)
            echo "Unknown option: $1"
            exit 1
        ;;
    esac
done

if [[ -z "$DB" || -z "$RC" ]]; then
    echo "Usage: $0 --db <sqlitefile> --mailfilterrc <file> [--show active|inactive|all]"
    exit 1
fi

if [[ ! -f "$DB" ]]; then
    echo "Database not found: $DB"
    exit 1
fi

if [[ ! -f "$RC" ]]; then
    echo "mailfilterrc not found: $RC"
    exit 1
fi

command -v sqlite3 >/dev/null || {
    echo "sqlite3 not installed"
    exit 1
}

TMP_RULES=$(mktemp)
TMP_HITS=$(mktemp)

# Regeln aus mailfilterrc extrahieren
awk '
BEGIN { OFS="|" }
{
    line=$0
    sub(/\r$/, "", line)
    if (line ~ /^[[:space:]]*#/) next
    if (line ~ /^[[:space:]]*$/) next

    if (match(line, /^[[:space:]]*DENY[[:space:]]*=[[:space:]]*"(.*)"[[:space:]]*$/, a)) {
        print "deny", a[1]
        next
    }

    if (match(line, /^[[:space:]]*ALLOW[[:space:]]*=[[:space:]]*"(.*)"[[:space:]]*$/, a)) {
        print "allow", a[1]
        next
    }

    if (match(line, /^[[:space:]]*SCORE[[:space:]]+\+?([0-9]+)[[:space:]]*=[[:space:]]*"(.*)"[[:space:]]*$/, a)) {
        print "score", a[2]
        next
    }
}
' "$RC" > "$TMP_RULES"

# Treffer aus DB holen
sqlite3 -separator '|' "$DB" "
SELECT phase, expression, COUNT(*)
FROM rule_hits
WHERE matched=1
GROUP BY phase, expression;
" > "$TMP_HITS"

perl -e '
use strict;
use warnings;

my ($rules_file, $hits_file, $show) = @ARGV;

my %hits;

open my $hf, "<", $hits_file or die $!;
while (<$hf>) {
    chomp;
    my ($phase, $expr, $count) = split(/\|/, $_, 3);
    $hits{$phase}{$expr} = $count;
}
close $hf;

open my $rf, "<", $rules_file or die $!;
my @rows;

while (<$rf>) {
    chomp;
    my ($phase, $expr) = split(/\|/, $_, 2);
    my $count = $hits{$phase}{$expr} // 0;
    my $state = $count > 0 ? "active" : "inactive";

    next if $show eq "active"   && $state ne "active";
    next if $show eq "inactive" && $state ne "inactive";

    push @rows, [$state, $phase, $count, $expr];
}
close $rf;

@rows = sort {
       $a->[0] cmp $b->[0]
    || $b->[2] <=> $a->[2]
    || $a->[1] cmp $b->[1]
    || $a->[3] cmp $b->[3]
} @rows;

print "=====================================================================\n";
print "mailfilter rule audit\n";
print "=====================================================================\n\n";

my ($active, $inactive) = (0, 0);
for my $r (@rows) {
    $r->[0] eq "active" ? $active++ : $inactive++;
}

print "active rules  : $active\n";
print "inactive rules: $inactive\n\n";

my $current = "";
for my $r (@rows) {
    my ($state, $phase, $count, $expr) = @$r;

    if ($state ne $current) {
        $current = $state;
        print "---------------------------------------------------------------------\n";
        print uc($state) . " RULES\n";
        print "---------------------------------------------------------------------\n";
    }

    printf "[%s] hits=%-5d %s\n", $phase, $count, $expr;
}
' "$TMP_RULES" "$TMP_HITS" "$SHOW"

rm -f "$TMP_RULES" "$TMP_HITS"
