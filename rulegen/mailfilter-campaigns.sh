#!/bin/bash
#
# This file is part of mailfilter-sqlite.
# mailfilter-sqlite is based on the original mailfilter project by (C) Andreas Bauer.
#
# Copyright (C) 2026 Rico Dummis <ua880d@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License ...
###############################################################################
#
# mailfilter-campaigns.sh
#
# Zeitbasierte Kampagnenanalyse für die mailfilter SQLite3-Datenbank.
#
# Dieses Werkzeug ist bewusst extern gehalten. Es wertet vorhandene Headerdaten
# aus, ohne die Mailfilter-C-Quellen erneut anfassen zu müssen.
#
# Optionen:
#   --db FILE      SQLite-Datenbank
#   --days N       Zeitraum in Tagen (Default: 30)
#   --limit N      Anzahl Kampagnen (Default: 20)
#
# Beispiel:
#   ./mailfilter-campaigns.sh --db /var/spool/filter/mailheader.log.sqlite3
#
###############################################################################

DB=""
DAYS=30
LIMIT=20

while [[ $# -gt 0 ]]; do
    case "$1" in
        --db) DB="$2"; shift 2 ;;
        --days) DAYS="$2"; shift 2 ;;
        --limit) LIMIT="$2"; shift 2 ;;
        *)
            echo "Unknown option: $1"
            exit 1
        ;;
    esac
done

if [[ -z "$DB" ]]; then
    echo "Usage: $0 --db <sqlitefile> [--days N] [--limit N]"
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

TMP=$(mktemp)

sqlite3 -separator $'\t' "$DB" <<SQL > "$TMP"
SELECT
  m.msg_log_id,
  IFNULL(m.decision,''),
  IFNULL(m.subject,''),
  IFNULL(MAX(CASE WHEN h.tag='From' THEN h.body END),''),
  IFNULL(MAX(CASE WHEN h.tag='Delivery-date' THEN h.body END),''),
  IFNULL(MAX(CASE WHEN h.tag='Return-path' THEN h.body END),'')
FROM messages m
LEFT JOIN header_entries h
  ON h.msg_log_id = m.msg_log_id
GROUP BY m.msg_log_id, m.decision, m.subject;
SQL

perl -MTime::Piece -e '
use strict;
use warnings;

my $days  = shift @ARGV;
my $limit = shift @ARGV;
my $file  = shift @ARGV;

my $now = time();
my %cluster;

sub parse_date {
    my ($date) = @_;
    return undef unless defined $date && length $date;

    my @fmts = (
        "%a, %d %b %Y %H:%M:%S %z",
        "%a, %d %b %Y %H:%M:%S %Z",
        "%d %b %Y %H:%M:%S %z",
        "%d %b %Y %H:%M:%S %Z",
    );

    for my $fmt (@fmts) {
        my $epoch;
        eval {
            my $tp = Time::Piece->strptime($date, $fmt);
            $epoch = $tp->epoch;
        };
        return $epoch if defined $epoch;
    }

    return undef;
}

sub extract_domain {
    my ($text) = @_;
    return "" unless defined $text;
    if ($text =~ /([A-Za-z0-9._%+\-]+\@[A-Za-z0-9.\-]+)/) {
        my $addr = lc($1);
        $addr =~ /\@(.+)$/;
        return $1 // "";
    }
    return "";
}

sub subject_marker {
    my ($s) = @_;
    return "price+urgency" if $s =~ /(?:\$\s*\d|\b\d{1,4}\.\d{2}\b)/i
                           && $s =~ /\b(?:last chance|last wave|countdown|today only|48hrs?|72hrs?|sold out soon)\b/i;
    return "price+promo"   if $s =~ /(?:\$\s*\d|\b\d{1,4}\.\d{2}\b)/i
                           && $s =~ /\b(?:sale|clearance|limited|exclusive|offer|save|deal|discount|bottom price)\b/i;
    return "promo"         if $s =~ /\b(?:sale|clearance|limited|exclusive|offer|save|deal|discount|bottom price)\b/i;
    return "generic";
}

open my $fh, "<", $file or die $!;
while (<$fh>) {
    chomp;
    my ($id,$decision,$subject,$from,$date,$returnpath) = split(/\t/, $_, 6);
    next unless $decision =~ /deny/;

    my $epoch = parse_date($date);
    next unless defined $epoch;
    next if ($now - $epoch) > ($days * 86400);

    my $domain = extract_domain($from);
    $domain = extract_domain($returnpath) unless length $domain;
    $domain = "unknown-domain" unless length $domain;

    my $marker = subject_marker(lc($subject // ""));
    my $key = join(" || ", $domain, $marker);

    $cluster{$key}->{count}++;
    $cluster{$key}->{froms}->{$from} = 1 if defined $from && length $from;
    push @{ $cluster{$key}->{subjects} }, $subject
        if defined $subject
        && length $subject
        && @{ $cluster{$key}->{subjects} || [] } < 5;
}

close $fh;

print "============================================================\n";
print "mailfilter campaign activity (last $days days)\n";
print "============================================================\n\n";

my $count = 0;
for my $k (sort { $cluster{$b}->{count} <=> $cluster{$a}->{count} } keys %cluster) {
    last if $count >= $limit;
    next if $cluster{$k}->{count} < 2;

    print "------------------------------------------------------------\n";
    print "cluster: $k\n";
    print "count: $cluster{$k}->{count}\n";

    if ($cluster{$k}->{froms} && %{ $cluster{$k}->{froms} }) {
        print "froms:\n";
        for my $f (sort keys %{ $cluster{$k}->{froms} }) {
            print "  $f\n";
        }
    }

    if ($cluster{$k}->{subjects} && @{ $cluster{$k}->{subjects} }) {
        print "subjects:\n";
        for my $s (@{ $cluster{$k}->{subjects} }) {
            print "  $s\n";
        }
    }

    print "\n";
    $count++;
}
' "$DAYS" "$LIMIT" "$TMP"

rm -f "$TMP"
