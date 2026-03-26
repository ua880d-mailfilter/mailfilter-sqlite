#!/usr/bin/perl
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
# ==========================================================
# mailfilter-header-import.pl
# ==========================================================
#
# Ziel:
#   - reproduzierbare Testdatenbasis für mailfilter-rulegen
#   - Umschalten zwischen verschiedenen Test-DBs
#   - keine Vermischung mit der produktiven DB
#
# Wichtige Hinweise:
#   - Dieses Script importiert nur Header, keine Mail-Bodies.
#   - Für rulegen-Tests sollten als decision i. d. R. "deny", "score-deny"
#     oder "pass" verwendet werden, da die bestehende Logik darauf prüft.
#   - Es schreibt in messages und header_entries. rule_hits bleibt leer.
#
# Beispiel:
#   ./mailfilter-header-import.pl \
#       --db /var/spool/filter/mailheader-test.sqlite3 \
#       --schema-from /var/spool/filter/mailheader.log.sqlite3 \
#       --input /tmp/spam-samples \
#       --decision deny \
#       --id-prefix tst
#
# Zweck:
#   Importiert Mail-Header (z. B. aus .eml/.hdr/.txt-Dateien) in eine
#   SQLite-Datenbank zur Analyse und Testzwecken.
#
# Einsatz:
#   - Aufbau von Test-Datenbanken
#   - Simulation von Spam-/Ham-Szenarien
#   - reproduzierbare Regeltests
#
# Funktionen:
#   - Parsen von Headern aus Text / .eml
#   - Speicherung in:
#       * messages
#       * header_entries
#   - Setzen von:
#       * decision (pass / deny)
#       * message_id (synthetisch möglich)
#
# Parameter:
#   --db <file>             Ziel-DB
#   --schema-from <file>    Schema-Quelle (optional)
#   --input <dir/file>      Input-Dateien
#   --decision <pass|deny>
#   --id-prefix <string>
#   --reset                 DB neu aufbauen
#
# Typische Nutzung:
#   Test-DB erstellen:
#
#     ./mailfilter-header-import.pl \
#       --db mailheader-test.sqlite3 \
#       --schema-from mailheader.log.sqlite3 \
#       --input ./testdata/spam \
#       --decision deny \
#       --reset
#
# Vorteile:
#   - vollständige Kontrolle über Testdaten und parallele DBs
#   - keine Abhängigkeit von echten Mailservern
#   - gezielte Kampagnen-Simulation möglich
#
# ==========================================================
#   (C) Rico Dummis - 2026
# ==========================================================


use strict;
use warnings;
use Getopt::Long;
use File::Find;
use File::Spec;

my $db = "";
my $schema_from = "";
my @inputs;
my $decision = "deny";
my $id_prefix = "imp";
my $reset = 0;
my $verbose = 0;

GetOptions(
    "db=s"          => \$db,
    "schema-from=s" => \$schema_from,
    "input=s@"      => \@inputs,
    "decision=s"    => \$decision,
    "id-prefix=s"   => \$id_prefix,
    "reset!"        => \$reset,
    "verbose!"      => \$verbose,
) or die usage();

die usage() unless $db && @inputs;

die "sqlite3 not found in PATH\n" unless command_exists("sqlite3");

if (!-f $db) {
    die "--schema-from is required when target DB does not yet exist\n"
        unless $schema_from && -f $schema_from;
    clone_schema($schema_from, $db);
}

my @files = collect_files(@inputs);
die "No readable input files found\n" unless @files;

my $next_id = next_import_id($db, $id_prefix);

my @sql;
push @sql, "BEGIN TRANSACTION;";

if ($reset) {
    my $prefix_like = sql_quote($id_prefix . "-%");
    push @sql, "DELETE FROM rule_hits WHERE msg_log_id LIKE $prefix_like;";
    push @sql, "DELETE FROM header_entries WHERE msg_log_id LIKE $prefix_like;";
    push @sql, "DELETE FROM messages WHERE msg_log_id LIKE $prefix_like;";
    $next_id = 1;
}

my $imported = 0;
for my $file (@files) {
    my $raw = slurp_file($file);
    my $headers = extract_headers($raw);
    next unless length $headers;

    my @entries = parse_headers($headers);
    next unless @entries;

    my %first;
    for my $e (@entries) {
        my ($tag, $body) = @$e;
        $first{lc $tag} = $body if !exists $first{lc $tag};
    }

    my $msg_log_id   = sprintf("%s-%d", $id_prefix, $next_id++);
    my $message_id   = $first{'message-id'}      // "";
    my $from_addr    = $first{'from'}            // "";
    my $to_addr      = $first{'to'}              // "";
    my $subject      = $first{'subject'}         // "";
    my $normal_subj  = normalize_subject($subject);
    my $date_hdr     = $first{'date'}            // ($first{'delivery-date'} // "");
    my $msg_size     = length($raw);
    my $final_score  = 0;

    push @sql, sprintf(
        "INSERT INTO messages (msg_log_id, message_id, from_addr, to_addr, subject, normal_subject, date_hdr, msg_size, decision, final_score) VALUES (%s, %s, %s, %s, %s, %s, %s, %d, %s, %d);",
        sql_quote($msg_log_id),
        sql_quote($message_id),
        sql_quote($from_addr),
        sql_quote($to_addr),
        sql_quote($subject),
        sql_quote($normal_subj),
        sql_quote($date_hdr),
        $msg_size,
        sql_quote($decision),
        $final_score,
    );

    my $ord = 1;
    for my $e (@entries) {
        my ($tag, $body) = @$e;
        push @sql, sprintf(
            "INSERT INTO header_entries (msg_log_id, ordinal, tag, body) VALUES (%s, %d, %s, %s);",
            sql_quote($msg_log_id),
            $ord++,
            sql_quote($tag),
            sql_quote($body),
        );
    }

    $imported++;
    print "Imported: $file -> $msg_log_id\n" if $verbose;
}

push @sql, "COMMIT;";

run_sql($db, join("\n", @sql));

print "Imported $imported message header set(s) into $db\n";
print "Decision used: $decision\n";
print "ID prefix     : $id_prefix\n";

exit 0;

sub usage {
    return <<"USAGE";
Usage:
  $0 --db FILE --input PATH [--input PATH ...] [options]

Required:
  --db FILE             Target SQLite database
  --input PATH          Input file or directory (may be given multiple times)

Optional:
  --schema-from FILE    Clone schema from existing DB if target DB does not exist
  --decision VALUE      deny | pass | score-deny  (default: deny)
  --id-prefix PREFIX    Prefix for imported msg_log_id values (default: imp)
  --reset               Delete prior imports with same prefix before importing
  --verbose             Print imported filenames

Notes:
  - For separate test databases, use --schema-from with your live mailfilter DB.
  - Bodies are ignored. Only RFC-822 style headers are imported.
USAGE
}

sub command_exists {
    my ($cmd) = @_;
    for my $dir (File::Spec->path()) {
        my $full = File::Spec->catfile($dir, $cmd);
        return 1 if -x $full;
    }
    return 0;
}

sub clone_schema {
    my ($src, $dst) = @_;

    my $schema = qx(sqlite3 "$src" ".schema");
    die "Could not read schema from $src\n" unless defined $schema && length $schema;

    my $tmp = "$dst.schema.sql";
    open my $fh, ">", $tmp or die "Cannot write temp schema file: $!";
    print {$fh} $schema;
    close $fh;

    my $rc = system("sqlite3", $dst, ".read $tmp");
    unlink $tmp;
    die "Schema clone failed for $dst\n" if $rc != 0;
}

sub collect_files {
    my @in = @_;
    my @out;

    for my $p (@in) {
        next unless defined $p && length $p;

        if (-f $p && -r $p) {
            push @out, $p;
            next;
        }

        if (-d $p) {
            find(
                sub {
                    return unless -f $_ && -r $_;
                    push @out, $File::Find::name;
                },
                $p
            );
        }
    }

    @out = sort @out;
    return @out;
}

sub next_import_id {
    my ($dbfile, $prefix) = @_;
    my $sql = <<"SQL";
SELECT COALESCE(MAX(CAST(SUBSTR(msg_log_id, LENGTH('$prefix-') + 1) AS INTEGER)), 0)
FROM messages
WHERE msg_log_id LIKE '$prefix-%';
SQL
    my $res = qx(sqlite3 "$dbfile" "$sql");
    chomp $res;
    $res = 0 unless defined $res && $res =~ /^\d+$/;
    return $res + 1;
}

sub slurp_file {
    my ($file) = @_;
    open my $fh, "<:raw", $file or die "Cannot open $file: $!";
    local $/;
    my $raw = <$fh>;
    close $fh;
    return $raw // "";
}

sub extract_headers {
    my ($raw) = @_;
    return "" unless defined $raw && length $raw;

    $raw =~ s/\r\n/\n/g;
    $raw =~ s/\r/\n/g;

    if ($raw =~ /(.*?)(?:\n\n|\z)/s) {
        return $1;
    }

    return $raw;
}

sub parse_headers {
    my ($headers) = @_;
    my @lines = split /\n/, $headers;
    my @unfolded;

    for my $line (@lines) {
        next if $line =~ /^\s*$/;
        if (@unfolded && $line =~ /^[ \t]+/) {
            $unfolded[-1] .= " " . trim($line);
        } else {
            push @unfolded, $line;
        }
    }

    my @entries;
    for my $line (@unfolded) {
        next unless $line =~ /^([^:]+):(.*)$/;
        my $tag  = trim($1);
        my $body = trim($2);
        push @entries, [$tag, $body] if length $tag;
    }

    return @entries;
}

sub trim {
    my ($s) = @_;
    $s = "" unless defined $s;
    $s =~ s/^\s+//;
    $s =~ s/\s+$//;
    return $s;
}

sub normalize_subject {
    my ($s) = @_;
    return "" unless defined $s;
    my $n = lc($s);
    $n =~ s/=\?[^\?]+\?[BQbq]\?[^\?]+\?= / /g;
    $n =~ s/=\?[^\?]+\?[BQbq]\?[^\?]+\?=/ /g;
    $n =~ s/[^a-z0-9\$%\.\- ]/ /g;
    $n =~ s/\s+/ /g;
    $n =~ s/^\s+|\s+$//g;
    return $n;
}

sub sql_quote {
    my ($s) = @_;
    $s = "" unless defined $s;
    $s =~ s/'/''/g;
    return "'$s'";
}

sub run_sql {
    my ($dbfile, $sql) = @_;

    my $tmp = "$dbfile.import.sql";
    open my $fh, ">", $tmp or die "Cannot write temp SQL file: $!";
    print {$fh} $sql;
    close $fh;

    my $rc = system("sqlite3", $dbfile, ".read $tmp");
    unlink $tmp;
    die "SQL import failed\n" if $rc != 0;
}
