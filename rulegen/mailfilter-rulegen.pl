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
# ==========================================================
# mailfilter-rulegen.pl
# ==========================================================
#
# Zweck:
#   Analysiert Mail-Header aus einer SQLite-Datenbank und
#   erzeugt:
#     - kommentierte Analyseblöcke (Candidates / Campaigns)
#     - einen kompakten Empfehlungsblock
#     - optional eine exportierbare Regeldatei
#
# Hauptfunktionen:
#   - Subject-, Domain-, Host- und Campaign-Analyse
#   - Zeitbasierte Bewertung (recent / aged / stale)
#   - False-Positive-Schutz:
#       * ALLOW-Proximity (Subject Tokens)
#       * protected_domains (Level 1-3)
#       * bulk provider detection
#   - Fake-Brand-Detection (Typosquatting / Phishing)
#   - Priorisierte Risiko-/Empfehlungslogik
#   - Export nach generated-rules.conf
#
# Eingaben:
#   - Daten aus mailfilter-rulegen.sh (stdin / Parameter)
#   - Konfigurationsdateien:
#       * protected_domains.conf
#       * allow_subject_tokens.conf
#       * bulk_providers.conf
#       * weak_subject_tokens.conf
#       * brand_domains.conf
#
# Wichtige Variablen:
#   $time_threshold_days   (Default: 365)
#   $export_rules_file     (optional, via CLI gesetzt)
#
# Bewertungskonzepte:
#   time_risk:
#     recent  -> aktiv relevant
#     aged    -> eingeschränkt relevant
#     stale   -> historisch, keine Rule-Generierung
#
#   protected_level:
#     1 = schwach geschützt
#     2 = mittel (nur SCORE)
#     3 = stark (keine Rules)
#
#   recommendation:
#     - allow / score / deny / manual review
#     - high-risk phishing candidate (Fake Brand)
#
# Exportlogik:
#   Export erfolgt nur wenn:
#     - nicht stale
#     - nicht stark geschützt
#     - kein ALLOW-Konflikt
#     - hohe Relevanz / Risiko
#
# Hinweis:
#   Diese Datei ist der Kern der Analyse- und Entscheidungslogik.
#
# ==========================================================
# 
# End Header
# ==========================================================


use strict;
use warnings;
use Getopt::Long;

my $subjects_file;
my $headers_file;
my $mailfilterrc;
my $protected_domains_file   = "/etc/mailfilter/rulegen/protected_domains.conf";
my $bulk_providers_file      = "/etc/mailfilter/rulegen/bulk_mail_providers.conf";
my $weak_subject_tokens_file = "/etc/mailfilter/rulegen/weak_subject_tokens.conf";
my $allow_subject_tokens_file= "/etc/mailfilter/rulegen/allow_subject_tokens.conf";
my $brand_domains_file       = "/etc/mailfilter/rulegen/brand_domains.conf";
my $export_rules_file	     = "";
my $export_conservative 	 = "";
my $export_aggressive 	     = "";

my $stale_days   = 365;		# older than this = stale
my $recent_days  = 30;		# recent window (optional)
my $highscore    = 100;
my $min_deny     = 5;
my $max_pass     = 0;
my $min_phrase   = 1;
my $max_phrase   = 3;
my $max_examples = 5;
my $max_froms    = 5;

GetOptions(
  "subjects=s"            => \$subjects_file,
  "headers=s"             => \$headers_file,
  "mailfilterrc=s"        => \$mailfilterrc,
  "protected-domains=s"   => \$protected_domains_file,
  "bulk-providers=s"      => \$bulk_providers_file,
  "weak-subject-tokens=s" => \$weak_subject_tokens_file,
  "allow-subject-tokens=s"=> \$allow_subject_tokens_file,
  "highscore=i"           => \$highscore,
  "min-deny=i"            => \$min_deny,
  "max-pass=i"            => \$max_pass,
  "min-phrase=i"          => \$min_phrase,
  "max-phrase=i"          => \$max_phrase,
  "stale-days=i"  	  	  => \$stale_days,
  "recent-days=i" 	  	  => \$recent_days,
  "brand-domains=s"       => \$brand_domains_file,
  "export-rules=s"        => \$export_rules_file,
  "export-cons=s"		  => \$export_conservative,
  "export-aggr=s"		  => \$export_aggressive,  
) or die "Invalid arguments\n";

die "--subjects is required\n" unless $subjects_file;
die "--headers is required\n"  unless $headers_file;

open my $SUB, "<", $subjects_file or die "Cannot open $subjects_file: $!";
open my $HDR, "<", $headers_file  or die "Cannot open $headers_file: $!";

my %deny;
my %pass;
my %msg_date;
my %msg_ts;
my %examples;
my %phrase_froms;
my %msg_subject;
my %msg_decision;
my %msg_from;
my %msg_replyto;
my %msg_returnpath;
my %msg_list_unsub;
my %msg_received_hosts;

my @deny_subjects;
my @pass_subjects;

my @recommended_domain_rules;
my @recommended_score_rules;
my @recommended_campaign_rules;

my %top_subject_tokens;
my %top_subject_phrases;
my %top_from_addr;
my %top_from_domain;
my %top_replyto_domain;
my %top_returnpath_domain;
my %top_listunsub_domain;
my %top_received_hosts;

my %phrase_first_seen;
my %phrase_last_seen;
my %domain_first_seen;
my %domain_last_seen;

my %from_domain_deny;
my %from_domain_pass;
my %from_domain_examples;

my %received_host_deny;
my %received_host_pass;
my %received_host_examples;

my %host_first_seen;
my %host_last_seen;

my %campaigns;

my @existing_deny;
my @existing_allow;
my @existing_score;
my @export_rules;
my @export_rules_conservative;
my @export_rules_aggressive;

my $fh_cons;
my $fh_aggr;

if ($export_conservative) {
  open($fh_cons, ">", $export_conservative);
}

if ($export_aggressive) {
  open($fh_aggr, ">", $export_aggressive);
}

my $now = time;

my $stale_threshold  = $now - ($stale_days  * 86400);
my $recent_threshold = $now - ($recent_days * 86400);

sub allow_conservative {
  my ($time_risk, $is_protected, $level, $allow_near, $deny_hits, $brand_hit) = @_;
  
  return 0 if $time_risk eq "stale";
#  return 1 if $brand_hit;
  
  return 0 if $allow_near;
  return 0 if $level >= 2;
  return 0 if $deny_hits < 3;

  return 1;
}

sub allow_aggressive {
  my ($time_risk, $level, $deny_hits, $brand_hit) = @_;

 # return 1 if $brand_hit;

  return 0 if $time_risk eq "stale";
  return 1 if $brand_hit;

  return 0 if $level >= 3;
  return 1 if $deny_hits >= 1;

  return 0;
}

sub classify_time_risk {
  my ($ts, $stale_th, $recent_th) = @_;

  return "unknown" unless defined $ts;

  return "recent" if $ts >= $recent_th;
  return "stale"  if $ts <  $stale_th;
  return "aged";
}

sub load_list_file {
  my ($file) = @_;
  my %set;
  return %set unless defined $file && -f $file;

  open my $fh, "<", $file or return %set;
  while (<$fh>) {
    chomp;
    s/#.*//;
    s/^\s+|\s+$//g;
    next unless length;
    $set{lc $_} = 1;
  }
  close $fh;
  return %set;
}

sub load_domain_policy_file {
  my ($file) = @_;
  my %map;

  return %map unless defined $file && -f $file;

  open my $fh, "<", $file or return %map;

  while (<$fh>) {
    chomp;
    s/\r$//;
    next if /^\s*#/;
    next if /^\s*$/;

    my ($domain, $level, $type, $comment) = split(/\|/, $_, 4);

    next unless defined $domain;
    $domain =~ s/^\s+|\s+$//g;
    next unless length $domain;

    $level   = 0 unless defined $level && $level =~ /^\d+$/;
    $type    = "" unless defined $type;
    $comment = "" unless defined $comment;

    $type    =~ s/^\s+|\s+$//g;
    $comment =~ s/^\s+|\s+$//g;

    $map{lc $domain} = {
      level   => int($level),
      type    => $type,
      comment => $comment,
    };
  }

  close $fh;
  return %map;
}

my %protected_domain_policy = load_domain_policy_file($protected_domains_file);
my %bulk_providers          = load_list_file($bulk_providers_file);
my %weak_subject_tokens     = load_list_file($weak_subject_tokens_file);
my %allow_subject_tokens    = load_list_file($allow_subject_tokens_file);
my %brand_domains           = load_list_file($brand_domains_file);

sub lookup_domain_policy {
  my ($domain, $mapref) = @_;
  return undef unless defined $domain && length $domain;

  my $d = lc($domain);

  for my $entry (keys %$mapref) {
    if ($d =~ /\Q$entry\E$/i) {
      return $mapref->{$entry};
    }
  }

  return undef;
}

sub domain_matches_list {
  my ($domain, $setref) = @_;
  return 0 unless defined $domain && length $domain;

  for my $d (keys %$setref) {
    return 1 if $domain =~ /\Q$d\E$/i;
  }
  return 0;
}

sub phrase_matches_allow_tokens {
  my ($phrase, $setref) = @_;
  return 0 unless defined $phrase && length $phrase;

  my $p = lc($phrase);

  for my $tok (keys %$setref) {
    next unless length $tok;
    return 1 if $p =~ /\Q$tok\E/;
  }

  return 0;
}

sub normalize_rule {
  my ($r) = @_;
  return "" unless defined $r;
  $r = lc($r);
  $r =~ s/\^subject:\.\*//g;
  $r =~ s/\^from:\.\*//g;
  $r =~ s/\^reply-to:\.\*//g;
  $r =~ s/\^return-path:\.\*//g;
  $r =~ s/\^list-unsubscribe:\.\*//g;
  $r =~ s/\^\(from\|received\):\.\*//g;
  $r =~ s/[\^\$\(\)\[\]\{\}\?\+\*\.\|\\]/ /g;
  $r =~ s/\s+/ /g;
  $r =~ s/^\s+|\s+$//g;
  return $r;
}

sub token_set {
  my ($r) = @_;
  my %t;
  my $n = normalize_rule($r);
  for my $tok (split(/\s+/, $n)) {
    next unless length($tok) > 2;
    $t{$tok} = 1;
  }
  return \%t;
}

sub compare_rule_to_existing {
  my ($candidate, $listref) = @_;

  for my $r (@$listref) {
    return ("exact-match", $r) if $r eq $candidate;
  }

  my $cand_tokens = token_set($candidate);

  for my $r (@$listref) {
    my $ref_tokens = token_set($r);
    my $common = 0;
    my $cand_count = scalar keys %$cand_tokens;
    next if $cand_count == 0;

    for my $k (keys %$cand_tokens) {
      $common++ if exists $ref_tokens->{$k};
    }

    if ($common >= 2 || ($cand_count > 0 && $common == $cand_count)) {
      return ("similar-to-existing", $r);
    }
  }

  return ("new", "");
}

#####Date-Time Parse
sub parse_date_to_ts {
  my ($d) = @_;
  return undef unless defined $d;

  # Beispiel: Tue, 09 Apr 2024 11:32:10 +0200
  if ($d =~ /\w{3},\s+(\d{1,2})\s+(\w{3})\s+(\d{4})\s+(\d{2}):(\d{2}):(\d{2})/) {
    my ($day, $mon, $year, $h, $m, $s) = ($1, $2, $3, $4, $5, $6);

    my %mon_map = (
      Jan=>0, Feb=>1, Mar=>2, Apr=>3, May=>4, Jun=>5,
      Jul=>6, Aug=>7, Sep=>8, Oct=>9, Nov=>10, Dec=>11
    );

    return undef unless exists $mon_map{$mon};

    require Time::Local;
    return Time::Local::timelocal($s, $m, $h, $day, $mon_map{$mon}, $year);
  }

  return undef;
}
#####

sub normalize_subject {
  my ($s) = @_;
  return "" unless defined $s;
  $s = lc($s);
  $s =~ s/=\?[^\?]+\?[BQbq]\?[^\?]+\?= / /g;
  $s =~ s/=\?[^\?]+\?[BQbq]\?[^\?]+\?=/ /g;
  $s =~ s/[^a-z0-9\$%\.\- ]/ /g;
  $s =~ s/\s+/ /g;
  $s =~ s/^\s+|\s+$//g;
  return $s;
}

sub compact_list_line {
  my ($href, $limit) = @_;
  my @vals = sort keys %$href;
  if (@vals > $limit) {
    @vals = @vals[0 .. $limit - 1];
    push @vals, "...";
  }
  return join(" ", @vals);
}

sub extract_email {
  my ($s) = @_;
  return "" unless defined $s;
  if ($s =~ /([A-Za-z0-9._%+\-]+\@[A-Za-z0-9.\-]+)/) {
    return lc($1);
  }
  return "";
}

sub extract_domain {
  my ($s) = @_;
  my $addr = extract_email($s);
  return "" unless length $addr;
  $addr =~ /\@(.+)$/;
  return $1 // "";
}

sub extract_domain_generic {
  my ($s) = @_;
  return "" unless defined $s;

  if ($s =~ m{https?://([A-Za-z0-9.\-]+\.[A-Za-z]{2,})}i) {
    return lc($1);
  }

  my $maildom = extract_domain($s);
  return $maildom if length $maildom;

  if ($s =~ /\b([A-Za-z0-9.\-]+\.[A-Za-z]{2,})\b/) {
    return lc($1);
  }

  return "";
}
#### neue Hilfs-Funktionen
sub normalize_brand_string {
  my ($s) = @_;
  return "" unless defined $s;

  $s = lc($s);

  # typische Phishing-Ersetzungen angleichen
  $s =~ tr/0134578/olieast/;

 # häufige Homoglyph-/Lookalike-Muster vorab glätten
  $s =~ s/rn/m/g;     # arnazon -> amazon
  $s =~ s/vv/w/g;     # vvindows -> windows
  $s =~ s/cl/d/g;     # dhl-artige Verwechslungen / selten, aber nützlich

  # Trenner und Nicht-Alnum entfernen
  $s =~ s/[^a-z0-9]//g;

  return $s;
}

sub levenshtein_distance {
  my ($a, $b) = @_;
  return length($b // "") unless defined $a && length $a;
  return length($a // "") unless defined $b && length $b;

  my @a = split //, $a;
  my @b = split //, $b;

  my @d;
  $d[$_][0] = $_ for 0 .. @a;
  $d[0][$_] = $_ for 0 .. @b;

  for my $i (1 .. @a) {
    for my $j (1 .. @b) {
      my $cost = ($a[$i - 1] eq $b[$j - 1]) ? 0 : 1;

      my $del = $d[$i - 1][$j] + 1;
      my $ins = $d[$i][$j - 1] + 1;
      my $sub = $d[$i - 1][$j - 1] + $cost;

      my $min = $del;
      $min = $ins if $ins < $min;
      $min = $sub if $sub < $min;

      $d[$i][$j] = $min;
    }
  }

  return $d[@a][@b];
}

##
sub extract_domain_tokens {
  my ($dom) = @_;
  return () unless defined $dom && length $dom;

  my @labels = split(/\./, lc($dom));
  my @tokens;

  for my $label (@labels) {
    next unless defined $label && length $label;
    push @tokens, grep { defined $_ && length $_ } split(/[-_]/, $label);
  }

  return @tokens;
}
##

sub extract_domain_labels {
  my ($dom) = @_;
  return () unless defined $dom && length $dom;

  my @labels = split(/\./, lc($dom));
  @labels = grep { defined $_ && length $_ } @labels;
  return @labels;
}
###
sub detect_fake_brand {
  my ($domain, $brandref, $protectedref) = @_;
  return ("", 0) unless defined $domain && length $domain;

  # echte/geschützte Domains nie als fake brand markieren
  if (domain_matches_list($domain, $protectedref)) {
    return ("", 0);
  }

  my @labels = extract_domain_labels($domain);
  my @tokens = extract_domain_tokens($domain);

  return ("", 0) unless @labels || @tokens;

  my $joined = normalize_brand_string(join("", @labels));

  for my $brand (sort { length($b) <=> length($a) } keys %$brandref) {
    next unless defined $brand && length $brand;

    my $brand_base = lc($brand);
    $brand_base =~ s/^\s+|\s+$//g;
    $brand_base =~ s/\..*$//;   # falls doch mal paypal.com etc. eingetragen wird

    my $nb = normalize_brand_string($brand_base);
    next unless length $nb >= 4;

    # 1. exakte Einbettung im zusammengesetzten Domain-String
    if (index($joined, $nb) >= 0) {
      return ($brand_base, 1);
    }

    # 2. einzelne Labels prüfen
    for my $label (@labels) {
      my $nl = normalize_brand_string($label);
      next unless length $nl;

      my $dist = levenshtein_distance($nl, $nb);

      if ((length($nb) <= 6 && $dist <= 1) ||
          (length($nb) >  6 && $dist <= 2)) {
        return ($brand_base, 1);
      }
    }

    # 3. Tokens innerhalb eines Labels prüfen (wichtig für paypai-secure-check)
    for my $tok (@tokens) {
      my $nt = normalize_brand_string($tok);
      next unless length $nt;

      my $dist = levenshtein_distance($nt, $nb);
###
	if ((length($nb) <= 4 && $dist == 0) ||
	  (length($nb) <= 6 && $dist <= 1) ||
	  (length($nb) >  6 && $dist <= 2)) {
           #print STDERR "brand_hit: domain=$domain token=$tok brand=$brand_base\n";
	   return ($brand_base, 1);
	}

	# kurze Marken nur dann als Teilstring erkennen,
	# wenn das Token deutlich nach zusammengesetzter Markenfälschung aussieht
	if (length($nb) <= 4) {
	   if (index($nt, $nb) >= 0 && $nt ne $nb && length($nt) >= length($nb) + 5) {
	      return ($brand_base, 1);
	 }
	}
	else {
	   if (index($nt, $nb) >= 0 && $nt ne $nb) {
	       return ($brand_base, 1);
	   }
	}
###
    }
  }

  return ("", 0);
}
####

sub extract_received_host {
  my ($line) = @_;
  return "" unless defined $line;
  my $h = lc($line);

  if ($h =~ /\bfrom\s+([a-z0-9][a-z0-9._\-]+\.[a-z]{2,})\b/) {
    return $1;
  }
  if ($h =~ /\bhelo=([a-z0-9][a-z0-9._\-]+\.[a-z]{2,})\b/) {
    return $1;
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

sub regex_escape_domain {
  my ($d) = @_;
  $d =~ s/\./\\./g;
  return $d;
}

sub bulk_reason {
  my ($domain, $host) = @_;
  return "bulk mail provider domain" if domain_matches_list($domain, \%bulk_providers);
  return "bulk mail provider host"   if domain_matches_list($host, \%bulk_providers);
  return "";
}

if ($mailfilterrc) {
  open my $RC, "<", $mailfilterrc or die "Cannot open $mailfilterrc: $!";
  while (my $line = <$RC>) {
    chomp $line;
    next if $line =~ /^\s*#/;
    next if $line =~ /^\s*$/;

    if ($line =~ /^\s*DENY\s*=\s*"(.+)"\s*$/) {
      push @existing_deny, $1;
      next;
    }

    if ($line =~ /^\s*ALLOW\s*=\s*"(.+)"\s*$/) {
      push @existing_allow, $1;
      next;
    }

    if ($line =~ /^\s*SCORE\s+\+?([0-9]+)\s*=\s*"(.+)"\s*$/) {
      push @existing_score, { score => $1, rule => $2 };
      next;
    }
  }
  close $RC;
}

while (<$HDR>) {
  chomp;
  my ($msgid, $decision, $tag, $body) = split(/\|/, $_, 4);
  next unless defined $msgid && defined $tag && defined $body;
  my $ltag = lc($tag);

  if ($ltag eq 'from' && !exists $msg_from{$msgid}) {
    $msg_from{$msgid} = $body;
  }
  if ($ltag eq 'reply-to' && !exists $msg_replyto{$msgid}) {
    $msg_replyto{$msgid} = $body;
  }
  if ($ltag eq 'return-path' && !exists $msg_returnpath{$msgid}) {
    $msg_returnpath{$msgid} = $body;
  }
  if ($ltag eq 'list-unsubscribe' && !exists $msg_list_unsub{$msgid}) {
    $msg_list_unsub{$msgid} = $body;
  }
  if (($ltag eq 'date' || $ltag eq 'delivery-date') && !exists $msg_date{$msgid}) {
    $msg_date{$msgid} = $body;

    my $ts = parse_date_to_ts($body);
    $msg_ts{$msgid} = $ts if defined $ts;
  }
  if ($ltag eq 'received') {
    my $host = extract_received_host($body);
    if (length $host) {
      $msg_received_hosts{$msgid} ||= {};
      $msg_received_hosts{$msgid}->{$host} = 1;
    }
  }
}

while (<$SUB>) {
  chomp;
  my ($msgid, $decision, $subject) = split(/\|/, $_, 3);
  next unless defined $msgid && defined $subject && length $subject;

  $msg_subject{$msgid}  = $subject;
  $msg_decision{$msgid} = $decision;

  my $raw_subject = $subject;
  my $norm = normalize_subject($subject);
  next unless length $norm;

  if ($decision =~ /deny/) {
    push @deny_subjects, $norm;
  } elsif ($decision eq 'pass') {
    push @pass_subjects, $norm;
  }

  my @tokens = grep { length($_) > 3 } split(/\s+/, $norm);

  if ($decision =~ /deny/) {
    for my $tok (@tokens) {
      $top_subject_tokens{$tok}++;
    }
  }

  for my $size ($min_phrase .. $max_phrase) {
    next if $size < 1;
    next if @tokens < $size;

    for (my $i = 0; $i <= $#tokens - $size + 1; $i++) {
      my @slice = @tokens[$i .. $i + $size - 1];

      if ($size == 1) {
        next if exists $weak_subject_tokens{ $slice[0] };
      }

      my $phrase = join(" ", @slice);
      next unless length($phrase);
###############
  if ($decision =~ /deny/) {
    $deny{$phrase}++;
    $top_subject_phrases{$phrase}++;
    $examples{$phrase} ||= [];
    push @{ $examples{$phrase} }, $raw_subject
      if @{ $examples{$phrase} } < $max_examples;

    if (exists $msg_from{$msgid} && length $msg_from{$msgid}) {
      $phrase_froms{$phrase} ||= {};
      $phrase_froms{$phrase}->{ $msg_from{$msgid} } = 1;
    }

    if (exists $msg_ts{$msgid}) {
      my $ts = $msg_ts{$msgid};

      if (!defined $phrase_first_seen{$phrase}->{ts} || $ts < $phrase_first_seen{$phrase}->{ts}) {
        $phrase_first_seen{$phrase}->{ts}  = $ts;
        $phrase_first_seen{$phrase}->{raw} = $msg_date{$msgid};
      }

      if (!defined $phrase_last_seen{$phrase}->{ts} || $ts > $phrase_last_seen{$phrase}->{ts}) {
        $phrase_last_seen{$phrase}->{ts}  = $ts;
        $phrase_last_seen{$phrase}->{raw} = $msg_date{$msgid};
      }
    }
  }
################
      if ($decision eq 'pass') {
        $pass{$phrase}++;
      }
    }
  }

  my $from        = $msg_from{$msgid}       // "";
  my $replyto     = $msg_replyto{$msgid}    // "";
  my $returnpath  = $msg_returnpath{$msgid} // "";
  my $listunsub   = $msg_list_unsub{$msgid} // "";

  my $from_domain       = extract_domain($from);
  my $replyto_domain    = extract_domain($replyto);
  my $returnpath_domain = extract_domain($returnpath);
  my $listunsub_domain  = extract_domain_generic($listunsub);
###############
  for my $dom (grep { length $_ } ($from_domain, $replyto_domain, $returnpath_domain, $listunsub_domain)) {
    if ($decision =~ /deny/) {
      $from_domain_deny{$dom}++;
      $from_domain_examples{$dom} ||= [];
      push @{ $from_domain_examples{$dom} }, $raw_subject
        if @{ $from_domain_examples{$dom} } < $max_examples;

      if (exists $msg_ts{$msgid}) {
        my $ts = $msg_ts{$msgid};

        if (!defined $domain_first_seen{$dom}->{ts} || $ts < $domain_first_seen{$dom}->{ts}) {
          $domain_first_seen{$dom}->{ts}  = $ts;
          $domain_first_seen{$dom}->{raw} = $msg_date{$msgid};
        }

        if (!defined $domain_last_seen{$dom}->{ts} || $ts > $domain_last_seen{$dom}->{ts}) {
          $domain_last_seen{$dom}->{ts}  = $ts;
          $domain_last_seen{$dom}->{raw} = $msg_date{$msgid};
        }
      }
    } elsif ($decision eq 'pass') {
      $from_domain_pass{$dom}++;
    }
  }
###############
  if ($decision =~ /deny/ && length $from) {
    $top_from_addr{$from}++;
    $top_from_domain{$from_domain}++             if length $from_domain;
    $top_replyto_domain{$replyto_domain}++       if length $replyto_domain;
    $top_returnpath_domain{$returnpath_domain}++ if length $returnpath_domain;
    $top_listunsub_domain{$listunsub_domain}++   if length $listunsub_domain;
  }

	if (exists $msg_received_hosts{$msgid}) {
	  for my $host (keys %{ $msg_received_hosts{$msgid} }) {
		if ($decision =~ /deny/) {
		  $received_host_deny{$host}++;
		  $received_host_examples{$host} ||= [];
		  push @{ $received_host_examples{$host} }, $raw_subject
			if @{ $received_host_examples{$host} } < $max_examples;
		  $top_received_hosts{$host}++;

		  if (exists $msg_ts{$msgid}) {
			my $ts = $msg_ts{$msgid};

			if (!defined $host_first_seen{$host}->{ts} || $ts < $host_first_seen{$host}->{ts}) {
			  $host_first_seen{$host}->{ts}  = $ts;
			  $host_first_seen{$host}->{raw} = $msg_date{$msgid};
			}

			if (!defined $host_last_seen{$host}->{ts} || $ts > $host_last_seen{$host}->{ts}) {
			  $host_last_seen{$host}->{ts}  = $ts;
			  $host_last_seen{$host}->{raw} = $msg_date{$msgid};
			}
		  }
		} elsif ($decision eq 'pass') {
		  $received_host_pass{$host}++;
		}
	  }
	}

  if ($decision =~ /deny/) {
    my $cluster_dom =
         $from_domain
      || $replyto_domain
      || $returnpath_domain
      || $listunsub_domain
      || "unknown-domain";

    my $cluster_host = "no-host";
    if (exists $msg_received_hosts{$msgid}) {
      my @hosts = sort keys %{ $msg_received_hosts{$msgid} };
      $cluster_host = $hosts[0] if @hosts;
    }

    my $marker = subject_marker($norm);
    my $cluster_key = join("|", $cluster_dom, $cluster_host, $marker);

    $campaigns{$cluster_key}->{count}++;

######
  if (exists $msg_ts{$msgid}) {
    my $ts = $msg_ts{$msgid};

    if (!defined $campaigns{$cluster_key}->{first_seen} || $ts < $campaigns{$cluster_key}->{first_seen}) {
      $campaigns{$cluster_key}->{first_seen} = $ts;
      $campaigns{$cluster_key}->{first_seen_raw} = $msg_date{$msgid};
    }

    if (!defined $campaigns{$cluster_key}->{last_seen} || $ts > $campaigns{$cluster_key}->{last_seen}) {
      $campaigns{$cluster_key}->{last_seen} = $ts;
      $campaigns{$cluster_key}->{last_seen_raw} = $msg_date{$msgid};
    }
  }
#####
    $campaigns{$cluster_key}->{froms} ||= {};
    $campaigns{$cluster_key}->{froms}->{$from} = 1 if length $from;
    $campaigns{$cluster_key}->{examples} ||= [];
    push @{ $campaigns{$cluster_key}->{examples} }, $raw_subject
      if @{ $campaigns{$cluster_key}->{examples} } < $max_examples;
  }
}

print "# ==========================================================\n";
print "# mailfilter-rulegen candidate rules\n";
print "# generated: " . localtime() . "\n";
print "# ==========================================================\n\n";

print "# ==========================================================\n";
print "# top spam analysis\n";
print "# ==========================================================\n\n";

print "# top subject tokens\n";
for my $tok (sort { $top_subject_tokens{$b} <=> $top_subject_tokens{$a} } keys %top_subject_tokens) {
  last if $top_subject_tokens{$tok} < 2;
  print "#   $tok\t$top_subject_tokens{$tok}\n";
}
print "\n";

print "# top subject phrases\n";
for my $ph (sort { $top_subject_phrases{$b} <=> $top_subject_phrases{$a} } keys %top_subject_phrases) {
  last if $top_subject_phrases{$ph} < 2;
  print "#   $ph\t$top_subject_phrases{$ph}\n";
}
print "\n";

print "# top from addresses\n";
for my $fa (sort { $top_from_addr{$b} <=> $top_from_addr{$a} } keys %top_from_addr) {
  last if $top_from_addr{$fa} < 2;
  print "#   $fa\t$top_from_addr{$fa}\n";
}
print "\n";

print "# top from domains\n";
for my $fd (sort { $top_from_domain{$b} <=> $top_from_domain{$a} } keys %top_from_domain) {
  last if $top_from_domain{$fd} < 2;
  print "#   $fd\t$top_from_domain{$fd}\n";
}
print "\n";

print "# top reply-to domains\n";
for my $rd (sort { $top_replyto_domain{$b} <=> $top_replyto_domain{$a} } keys %top_replyto_domain) {
  last if $top_replyto_domain{$rd} < 2;
  print "#   $rd\t$top_replyto_domain{$rd}\n";
}
print "\n";

print "# top return-path domains\n";
for my $rd (sort { $top_returnpath_domain{$b} <=> $top_returnpath_domain{$a} } keys %top_returnpath_domain) {
  last if $top_returnpath_domain{$rd} < 2;
  print "#   $rd\t$top_returnpath_domain{$rd}\n";
}
print "\n";

print "# top list-unsubscribe domains\n";
for my $ld (sort { $top_listunsub_domain{$b} <=> $top_listunsub_domain{$a} } keys %top_listunsub_domain) {
  last if $top_listunsub_domain{$ld} < 2;
  print "#   $ld\t$top_listunsub_domain{$ld}\n";
}
print "\n";

print "# top received hosts\n";
for my $rh (sort { $top_received_hosts{$b} <=> $top_received_hosts{$a} } keys %top_received_hosts) {
  last if $top_received_hosts{$rh} < 2;
  print "#   $rh\t$top_received_hosts{$rh}\n";
}
print "\n";

print "# ==========================================================\n";
print "# campaign signatures\n";
print "# ==========================================================\n\n";

for my $ck (sort { $campaigns{$b}->{count} <=> $campaigns{$a}->{count} } keys %campaigns) {
  next if $campaigns{$ck}->{count} < $min_deny;
  my ($dom, $host, $marker) = split(/\|/, $ck, 3);
  my $from_line = compact_list_line($campaigns{$ck}->{froms}, $max_froms);
  my $policy = lookup_domain_policy($dom, \%protected_domain_policy);
  my $bulk_reason = bulk_reason($dom, $host);
##  
  my ($brand_match, $brand_hit) = detect_fake_brand($dom, \%brand_domains, \%protected_domain_policy);
##
  my $is_protected = defined $policy ? 1 : 0;
  my $level        = defined $policy ? $policy->{level}   : 0;
  my $ptype        = defined $policy ? $policy->{type}    : "";
  my $pcomment     = defined $policy ? $policy->{comment} : "";

  my $allow_near = 0;
  for my $e (@{ $campaigns{$ck}->{examples} || [] }) {
    if (phrase_matches_allow_tokens($e, \%allow_subject_tokens)) {
      $allow_near = 1;
      last;
    }
  }
##
  my $risk = "low";
  my $recommendation = "hard deny plausible";

  if ($level >= 3) {
    $risk = "high";
    $recommendation = "do not auto-deny";
  }
  elsif ($level == 2) {
    $risk = "medium";
    $recommendation = "prefer SCORE over DENY";
  }
  elsif ($level == 1) {
    $risk = "medium";
    $recommendation = "no automatic DENY";
  }
  elsif ($bulk_reason) {
    $risk = "medium";
    $recommendation = "prefer soft scoring";
  }

  # fake-brand phishing has higher priority than ALLOW-like wording
  if ($brand_hit) {
    $risk = "high";
    $recommendation = "high-risk phishing candidate";
  }
  elsif ($allow_near) {
    $risk = "high";
    $recommendation = "do not auto-deny (ALLOW proximity)";
  }
##
  print "# --------------------------------------------------\n";
	my $time_risk = "unknown";
	my $suppress_rule_output = 0;
	
	if (exists $campaigns{$ck}->{last_seen}) {
	  $time_risk = classify_time_risk($campaigns{$ck}->{last_seen}, $stale_threshold, $recent_threshold);
	}
	if ($time_risk eq 'stale') {
	  $recommendation = "historical candidate, manual review only";
	  $suppress_rule_output = 1;
	}  
  print "# campaign signature\n";
  print "# count: $campaigns{$ck}->{count}\n";
  print "# confidence: " . ($campaigns{$ck}->{count} >= 4 ? "high" : "medium") . "\n";
  print "# risk: $risk\n";
  print "# recommendation: $recommendation\n";
##
  print "# brand_match: $brand_match\n" if $brand_hit;
  print "# brand_risk: high\n" if $brand_hit;
  print "# reason: fake-brand similarity\n" if $brand_hit;
##
  print "# protected_level: $level\n" if $is_protected;
  print "# protected_type: $ptype\n" if $is_protected && length $ptype;
  print "# protected_comment: $pcomment\n" if $is_protected && length $pcomment;
  print "# reason: ALLOW proximity\n" if $allow_near;
  print "# reason: $bulk_reason\n" if $bulk_reason;
  print "# from_domain: $dom\n";
  print "# received_host: $host\n";
  print "# subject_marker: $marker\n";
  print "# from_addresses: $from_line\n" if length $from_line;
  print "# first_seen_date: $campaigns{$ck}->{first_seen_raw}\n"
    if exists $campaigns{$ck}->{first_seen_raw};
  print "# last_seen_date : $campaigns{$ck}->{last_seen_raw}\n"
    if exists $campaigns{$ck}->{last_seen_raw};
  print "# time_risk: $time_risk\n";
  print "# examples:\n";
  for my $e (@{ $campaigns{$ck}->{examples} || [] }) {
    print "#   $e\n";
  }
########
# recommended campaign rule collection
if (!$suppress_rule_output
    && !$allow_near
    && !$is_protected
    && !$bulk_reason
    && $campaigns{$ck}->{count} >= $min_deny
    && $dom ne "unknown-domain") {

  my $re_dom = regex_escape_domain($dom);

#######
  push @recommended_campaign_rules,
    "# SCORE +20=\"^(From|Reply-To|Return-path|List-Unsubscribe):.*$re_dom\"";
  push @export_rules,
	"SCORE +20=\"^(From|Reply-To|Return-path|List-Unsubscribe):.*$re_dom\"";
		
	if (allow_conservative($time_risk, $is_protected, $level, $allow_near, $campaigns{$ck}->{count})) {
	  push @export_rules_conservative,
		"SCORE +20=\"^(From|Reply-To|Return-path|List-Unsubscribe):.*$re_dom\"";
	}

	if (allow_aggressive($time_risk, $level, $campaigns{$ck}->{count})) {
	  push @export_rules_aggressive,
		"SCORE +20=\"^(From|Reply-To|Return-path|List-Unsubscribe):.*$re_dom\"";
	}
 
 
 if ($host ne "no-host" && $level < 2) {
    my $re_host = regex_escape_domain($host);
    push @recommended_campaign_rules,
      "# SCORE +20=\"^(From|Received):.*$re_host\"";
    push @export_rules,
      "SCORE +20=\"^(From|Received):.*$re_host\"";	  
  
		if (allow_conservative($time_risk, $is_protected, $level, $allow_near, $campaigns{$ck}->{count})) {
		  push @export_rules_conservative,
			"SCORE +20=\"^(From|Received):.*$re_host\"";
		}

		if (allow_aggressive($time_risk, $level, $campaigns{$ck}->{count})) {
		  push @export_rules_aggressive,
			"SCORE +20=\"^(From|Received):.*$re_host\"";
		}
 }
  if ($marker eq "price+urgency") {
    push @recommended_campaign_rules,
      "# SCORE +15=\"^Subject:.*(Last Chance|Last Wave|Countdown|Today Only|48Hrs|72Hrs|Sold Out Soon).*(\\\$[0-9]+|[0-9]{1,4}\\.[0-9]{2})\"";
    push @export_rules,
      "SCORE +15=\"^Subject:.*(Last Chance|Last Wave|Countdown|Today Only|48Hrs|72Hrs|Sold Out Soon).*(\\\$[0-9]+|[0-9]{1,4}\\.[0-9]{2})\"";	  
		if (allow_conservative($time_risk, $is_protected, $level, $allow_near, $campaigns{$ck}->{count})) {
		  push @export_rules_conservative,
			"SCORE +15=\"^Subject:.*(Last Chance|Last Wave|Countdown|Today Only|48Hrs|72Hrs|Sold Out Soon).*(\\\$[0-9]+|[0-9]{1,4}\\.[0-9]{2})\"";
		}

		if (allow_aggressive($time_risk, $level, $campaigns{$ck}->{count})) {
		  push @export_rules_aggressive,
			"SCORE +15=\"^Subject:.*(Last Chance|Last Wave|Countdown|Today Only|48Hrs|72Hrs|Sold Out Soon).*(\\\$[0-9]+|[0-9]{1,4}\\.[0-9]{2})\"";
		}
 }
  elsif ($marker eq "price+promo") {
    push @recommended_campaign_rules,
      "# SCORE +15=\"^Subject:.*(Sale|Clearance|Limited|Exclusive|Offer|Save|Deal|Discount|Bottom Price).*(\\\$[0-9]+|[0-9]{1,4}\\.[0-9]{2})\"";
    push @export_rules,
      "SCORE +15=\"^Subject:.*(Last Chance|Last Wave|Countdown|Today Only|48Hrs|72Hrs|Sold Out Soon).*(\\\$[0-9]+|[0-9]{1,4}\\.[0-9]{2})\"";	  
		if (allow_conservative($time_risk, $is_protected, $level, $allow_near, $campaigns{$ck}->{count})) {
		  push @export_rules_conservative,
			"SCORE +15=\"^Subject:.*(Sale|Clearance|Limited|Exclusive|Offer|Save|Deal|Discount|Bottom Price).*(\\\$[0-9]+|[0-9]{1,4}\\.[0-9]{2})\"";
		}

		if (allow_aggressive($time_risk, $level, $campaigns{$ck}->{count})) {
		  push @export_rules_aggressive,
			"SCORE +15=\"^Subject:.*(Sale|Clearance|Limited|Exclusive|Offer|Save|Deal|Discount|Bottom Price).*(\\\$[0-9]+|[0-9]{1,4}\\.[0-9]{2})\"";
		}	
	  }
  elsif ($marker eq "promo") {
    push @recommended_campaign_rules,
      "# SCORE +10=\"^Subject:.*(Sale|Clearance|Limited|Exclusive|Offer|Save|Deal|Discount|Bottom Price)\"";
    push @export_rules,
      "SCORE +10=\"^Subject:.*(Sale|Clearance|Limited|Exclusive|Offer|Save|Deal|Discount|Bottom Price)\"";
		if (allow_conservative($time_risk, $is_protected, $level, $allow_near, $campaigns{$ck}->{count})) {
		  push @export_rules_conservative,
			"SCORE +10=\"^Subject:.*(Sale|Clearance|Limited|Exclusive|Offer|Save|Deal|Discount|Bottom Price)\"";
		}

		if (allow_aggressive($time_risk, $level, $campaigns{$ck}->{count})) {
		  push @export_rules_aggressive,
			"SCORE +10=\"^Subject:.*(Sale|Clearance|Limited|Exclusive|Offer|Save|Deal|Discount|Bottom Price)\"";
		}
  }
##
}
########
	 print "# suggested hard rules:\n";
	 if ($suppress_rule_output) {
	  print "# (none)\n";
	 }
	 elsif (!$allow_near && !$is_protected && !$bulk_reason && $dom ne "unknown-domain") {
	  my $re_dom = regex_escape_domain($dom);
	  print "# DENY=\"^From:.*$re_dom\"\n";
	  print "# DENY=\"^Return-path:.*$re_dom\"\n";
	  print "# DENY=\"^List-Unsubscribe:.*$re_dom\"\n";
	 } else {
	  print "# (none)\n";
	 }

	print "# suggested soft rules:\n";
	if ($suppress_rule_output) {
	  print "# (none)\n";
	}
	elsif ($level >= 3) {
	  print "# (none)\n";
	}
	else {
	  if ($dom ne "unknown-domain") {
		my $re_dom = regex_escape_domain($dom);
		my $domain_score = 20;
		$domain_score = 5 if $level == 2;
		print "# SCORE +$domain_score=\"^(From|Reply-To|Return-path|List-Unsubscribe):.*$re_dom\"\n";
	  }

	  if ($host ne "no-host" && $level < 2) {
		my $re_host = regex_escape_domain($host);
		print "# SCORE +20=\"^(From|Received):.*$re_host\"\n";
	  }

	  if ($marker eq "price+urgency") {
		print "# SCORE +15=\"^Subject:.*(Last Chance|Last Wave|Countdown|Today Only|48Hrs|72Hrs|Sold Out Soon).*(\\\$[0-9]+|[0-9]{1,4}\\.[0-9]{2})\"\n";
	  }
	  elsif ($marker eq "price+promo") {
		print "# SCORE +15=\"^Subject:.*(Sale|Clearance|Limited|Exclusive|Offer|Save|Deal|Discount|Bottom Price).*(\\\$[0-9]+|[0-9]{1,4}\\.[0-9]{2})\"\n";
	  }
	  elsif ($marker eq "promo") {
		print "# SCORE +10=\"^Subject:.*(Sale|Clearance|Limited|Exclusive|Offer|Save|Deal|Discount|Bottom Price)\"\n";
	  }
	}
  print "# --------------------------------------------------\n\n";
}

print "# ==========================================================\n";
print "# suggested DENY rules (subject based)\n";
print "# ==========================================================\n\n";

foreach my $p (sort { $deny{$b} <=> $deny{$a} || length($b) <=> length($a) } keys %deny) {
  my $deny_hits = $deny{$p};
  my $pass_hits = $pass{$p} // 0;

  next if $deny_hits < $min_deny;
  next if $pass_hits > $max_pass;

  my $word_count = scalar split(/\s+/, $p);
  next if $word_count == 1 && exists $weak_subject_tokens{$p};

  my $confidence = ($word_count >= 3) ? "medium" : ($word_count == 2 ? "medium" : "low");
  my $risk = ($word_count == 1) ? "high" : "medium";
  my $recommendation = ($word_count == 1) ? "prefer SCORE, not DENY" : "manual review";

  my $allow_near = phrase_matches_allow_tokens($p, \%allow_subject_tokens);
  if ($allow_near) {
    $risk = "high";
    $recommendation = "do not auto-deny (ALLOW proximity)";
  }

  my $candidate_rule = '^Subject:.*' . $p;
  my ($status, $existing) = compare_rule_to_existing($candidate_rule, \@existing_deny);
  my $from_line = "";
  if (exists $phrase_froms{$p}) {
    $from_line = compact_list_line($phrase_froms{$p}, $max_froms);
  }

  print "# --------------------------------------------------\n";
  print "# auto-generated candidate\n";
  print "# from_addresses: $from_line\n" if length $from_line;
  print "# type: DENY\n";
  print "# source: subject phrase\n";
  print "# phrase: $p\n";
  print "# words: $word_count\n";
  print "# deny_hits: $deny_hits\n";
  print "# pass_hits: $pass_hits\n";
#######
  my $time_risk = "unknown";
  my $suppress_rule_output = 0;
  if (exists $phrase_last_seen{$p}->{ts}) {
    $time_risk = classify_time_risk($phrase_last_seen{$p}->{ts}, $stale_threshold, $recent_threshold);
  }

  if ($time_risk eq 'stale') {
    $recommendation = "historical candidate, manual review only";
	$suppress_rule_output = 1;
  }
  print "# confidence: $confidence\n";
  print "# risk: $risk\n";
  print "# recommendation: $recommendation\n";
  print "# reason: ALLOW proximity\n" if $allow_near;
  print "# status: $status\n";
#######
  print "# existing_rule: $existing\n" if $existing;
#########
  print "# first_seen_date: $phrase_first_seen{$p}->{raw}\n"
    if exists $phrase_first_seen{$p}->{raw};

  print "# last_seen_date : $phrase_last_seen{$p}->{raw}\n"
    if exists $phrase_last_seen{$p}->{raw};

  ##my $ts = $phrase_last_seen{$p}->{ts};

  print "# time_risk: $time_risk\n";
#########
  print "# examples:\n";
  for my $e (@{ $examples{$p} || [] }) {
    print "#   $e\n";
  }
  print "# suggested rule:\n";
####
# recommended subject rule collection
my $recommended_subject_min_hits = 3;

my $re_phrase = $p;
$re_phrase =~ s/([\\.^\$|(){}\[\]+*?])/\\$1/g;

if (!$suppress_rule_output
    && !$allow_near
    && $deny_hits >= $recommended_subject_min_hits
    && $confidence ne "low"
    && $word_count >= 3) {

  push @recommended_score_rules,
    "# SCORE +10=\"^Subject:.*$re_phrase\"";
  push @export_rules,
    "SCORE +10=\"^Subject:.*$re_phrase\"";
	
  if (allow_conservative($time_risk, 0, 0, $allow_near, $deny_hits)) {
    push @export_rules_conservative,
      "SCORE +10=\"^Subject:.*$re_phrase\"";
  }
}

if (allow_aggressive($time_risk, 0, $deny_hits)) {
  push @export_rules_aggressive,
    "SCORE +10=\"^Subject:.*$re_phrase\"";
}
####
  if ($allow_near) {
    print "# SCORE +5=\"^Subject:.*$p\"\n";
  } else {
	if ($suppress_rule_output) {
	  print "# (none)\n";
	} else {
	  print "# DENY=\"^Subject:.*$p\"\n";
	}
  }
  print "# --------------------------------------------------\n\n";
}

print "# ==========================================================\n";
print "# suggested DENY rules (domain based)\n";
print "# ==========================================================\n\n";

foreach my $dom (sort { $from_domain_deny{$b} <=> $from_domain_deny{$a} } keys %from_domain_deny) {
  my $deny_hits = $from_domain_deny{$dom};
  my $pass_hits = $from_domain_pass{$dom} // 0;
  next if $deny_hits < $min_deny;
  next if $pass_hits > $max_pass;

  my $regex_dom = regex_escape_domain($dom);
  my $candidate_rule = '^From:.*' . $regex_dom;
  my ($status, $existing) = compare_rule_to_existing($candidate_rule, \@existing_deny);

  my $policy = lookup_domain_policy($dom, \%protected_domain_policy);
  my $is_bulk = domain_matches_list($dom, \%bulk_providers);
####
 my ($brand_match, $brand_hit) = detect_fake_brand($dom, \%brand_domains, \%protected_domain_policy);
####
  my $level        = defined $policy ? $policy->{level}   : 0;
  my $ptype        = defined $policy ? $policy->{type}    : "";
  my $pcomment     = defined $policy ? $policy->{comment} : "";
  my $is_protected = defined $policy ? 1 : 0;
######

my $risk = "low";
my $recommendation = "hard deny plausible";

if ($level >= 3) {
  $risk = "high";
  $recommendation = "do not auto-deny";
}
elsif ($level == 2) {
  $risk = "medium";
  $recommendation = "prefer SCORE over DENY";
}
elsif ($level == 1) {
  $risk = "medium";
  $recommendation = "no automatic DENY";
}
elsif ($is_bulk) {
  $risk = "medium";
  $recommendation = "prefer SCORE over DENY";
}

# fake-brand phishing has higher priority than generic domain heuristics
if ($brand_hit) {
  $risk = "high";
  $recommendation = "high-risk phishing candidate";
}

######
  print "# --------------------------------------------------\n";
  print "# auto-generated candidate\n";
  print "# from_addresses: $dom\n";
  print "# type: DENY\n";
  print "# source: from/reply-to/return-path/list-unsubscribe domain\n";
  print "# deny_hits: $deny_hits\n";
  print "# pass_hits: $pass_hits\n";
#####
  my $time_risk = "unknown";
  my $suppress_rule_output = 0;
  if (exists $domain_last_seen{$dom}->{ts}) {
    $time_risk = classify_time_risk($domain_last_seen{$dom}->{ts}, $stale_threshold, $recent_threshold);
  }

  if ($time_risk eq 'stale') {
    $recommendation = "historical candidate, manual review only";
	$suppress_rule_output = 1;
  }
######
  print "# confidence: medium\n";
  print "# risk: $risk\n";
  print "# recommendation: $recommendation\n";
##
  print "# brand_match: $brand_match\n" if $brand_hit;
  print "# brand_risk: high\n" if $brand_hit;
  print "# reason: fake-brand similarity\n" if $brand_hit;
##
  print "# protected_level: $level\n" if $is_protected;
  print "# protected_type: $ptype\n" if $is_protected && length $ptype;
  print "# protected_comment: $pcomment\n" if $is_protected && length $pcomment;
  print "# reason: bulk mail provider\n" if $is_bulk;
  print "# status: $status\n";
  print "# existing_rule: $existing\n" if $existing;

  ########
  print "# first_seen_date: $domain_first_seen{$dom}->{raw}\n"
    if exists $domain_first_seen{$dom}->{raw};

  print "# last_seen_date : $domain_last_seen{$dom}->{raw}\n"
    if exists $domain_last_seen{$dom}->{raw};
  print "# time_risk: $time_risk\n";
########
  print "# examples:\n";
  for my $e (@{ $from_domain_examples{$dom} || [] }) {
    print "#   $e\n";
  }
####Empfehlungen sammeln
 # recommended domain rule collection
   if ((!$suppress_rule_output
      && !$is_bulk
      && (!$is_protected || $level < 2)
      && $deny_hits >= $min_deny)
	  || $brand_hit) {

    push @recommended_domain_rules,
      "# DENY=\"^(From|Reply-To|Return-path|List-Unsubscribe):.*$regex_dom\"";
    push @export_rules,
      "DENY=\"^(From|Reply-To|Return-path|List-Unsubscribe):.*$regex_dom\"";	  
	  

	if (allow_conservative($time_risk, $is_protected, $level, 0, $deny_hits, $brand_hit)) {
	  if ($brand_hit){
	    push @export_rules_conservative,
		  "SCORE +75=\"^(From|Reply-To|Return-path|List-Unsubscribe):.*$regex_dom\"";
	  }
	  else {
#####
	 	my $cons_score = 20;
	 	$cons_score = 10 if $deny_hits <= 3;
		$cons_score = 25 if $deny_hits > 3 && $deny_hits < 10;
		$cons_score = 35 if $deny_hits >= 10 && $deny_hits < 20;
		$cons_score = 45 if $deny_hits >= 20;
            	push @export_rules_conservative,
                  "SCORE +$cons_score=\"^(From|Reply-To|Return-path|List-Unsubscribe):.*$regex_dom\"";
#####	  
	  }
	}

	if (allow_aggressive($time_risk, $level, $deny_hits, $brand_hit)) {
	  push @export_rules_aggressive,
		"DENY=\"^(From|Reply-To|Return-path|List-Unsubscribe):.*$regex_dom\"";
	}
  }  
####Ende_sammeln
  print "# suggested rule:\n";
  if (!$is_protected && !$is_bulk) {
	if ($suppress_rule_output) {
	  print "# (none)\n";
	} else {
	  print "# DENY=\"^(From|Reply-To|Return-path|List-Unsubscribe):.*$regex_dom\"\n";
	}
  } else {
    if ($level >= 3) {
      print "# (none)\n";
    } else {
      my $soft_score = 20;
      $soft_score = 5 if $level == 2;
      print "# SCORE +$soft_score=\"^(From|Reply-To|Return-path|List-Unsubscribe):.*$regex_dom\"\n";
    }
  }
  print "# --------------------------------------------------\n\n";
}

print "# ==========================================================\n";
print "# suggested DENY rules (received host based)\n";
print "# ==========================================================\n\n";

foreach my $host (sort { $received_host_deny{$b} <=> $received_host_deny{$a} } keys %received_host_deny) {
  my $deny_hits = $received_host_deny{$host};
  my $pass_hits = $received_host_pass{$host} // 0;
  next if $deny_hits < $min_deny;
  next if $pass_hits > $max_pass;

  my $regex_host = regex_escape_domain($host);
  my $candidate_rule = '^(From|Received):.*' . $regex_host;
  my ($status, $existing) = compare_rule_to_existing($candidate_rule, \@existing_deny);

  my $is_bulk = domain_matches_list($host, \%bulk_providers);
  my $time_risk = "unknown";
  if (exists $host_last_seen{$host}->{ts}) {
    $time_risk = classify_time_risk($host_last_seen{$host}->{ts}, $stale_threshold, $recent_threshold);
  }

#if ($time_risk eq 'stale') {
#  $recommendation = "historical candidate, manual review only";
#}

  print "# --------------------------------------------------\n";
  print "# auto-generated candidate\n";
  print "# from_addresses: $host\n";
  print "# type: DENY\n";
  print "# source: received host\n";
  print "# deny_hits: $deny_hits\n";
  print "# pass_hits: $pass_hits\n";
	my $risk = "medium";
	my $suppress_rule_output = 0;
	my $recommendation = "prefer SCORE over DENY";
	if ($time_risk eq 'stale') {
	  $recommendation = "historical candidate, manual review only";
	  $suppress_rule_output = 1;
	}
	print "# confidence: medium\n";
	print "# risk: $risk\n";
	print "# recommendation: $recommendation\n";
	print "# reason: bulk mail provider\n" if $is_bulk;
	print "# status: $status\n";
	print "# existing_rule: $existing\n" if $existing;
	print "# first_seen_date: $host_first_seen{$host}->{raw}\n"
	  if exists $host_first_seen{$host}->{raw};
	print "# last_seen_date : $host_last_seen{$host}->{raw}\n"
	  if exists $host_last_seen{$host}->{raw};
	print "# time_risk: $time_risk\n";
	print "# examples:\n";
  for my $e (@{ $received_host_examples{$host} || [] }) {
    print "#   $e\n";
  }
# recommended received-host rule collection
if (!$suppress_rule_output
    && !$is_bulk
    && $deny_hits >= $min_deny) {

  push @recommended_score_rules,
    "# SCORE +20=\"^(From|Received):.*$regex_host\"";
  push @export_rules,
    "SCORE +20=\"^(From|Received):.*$regex_host\"";
	
	if (allow_conservative($time_risk, 0, 0, 0, $deny_hits)) {
	  push @export_rules_conservative,
		"SCORE +20=\"^(From|Received):.*$regex_host\"";
	}

	if (allow_aggressive($time_risk, 0, $deny_hits)) {
	  push @export_rules_aggressive,
		"SCORE +20=\"^(From|Received):.*$regex_host\"";
	}
}
##
  print "# suggested rule:\n";
	if ($suppress_rule_output) {
	  print "# (none)\n";
	} else {
	  print "# SCORE +20=\"^(From|Received):.*$host\"\n";
	}
  print "# --------------------------------------------------\n\n";
}

my @patterns = (
  {
    id    => 'price_pattern',
    desc  => 'price-like amount in subject',
    score => 10,
    regex => qr/(?:\$\s*\d{1,4}(?:\.\d{2})?|\b\d{1,4}\.\d{2}\b)/i,
    rule  => '^Subject:.*(\$[0-9]+|[0-9]{1,4}\.[0-9]{2})',
  },
  {
    id    => 'percent_pattern',
    desc  => 'percentage / discount pattern',
    score => 10,
    regex => qr/\b\d{1,3}\s*%\b/i,
    rule  => '^Subject:.*[0-9]{1,3}%.*',
  },
  {
    id    => 'promo_words',
    desc  => 'commercial promotion keywords',
    score => 10,
    regex => qr/\b(sale|clearance|limited|exclusive|offer|save|deal|discount|bottom price)\b/i,
    rule  => '^Subject:.*(Sale|Clearance|Limited|Exclusive|Offer|Save|Deal|Discount|Bottom Price)',
  },
  {
    id    => 'urgency_words',
    desc  => 'urgency wording',
    score => 15,
    regex => qr/\b(last chance|last wave|countdown|today only|final call|48hrs?|72hrs?|sold out soon)\b/i,
    rule  => '^Subject:.*(Last Chance|Last Wave|Countdown|Today Only|Final Call|48Hrs|72Hrs|Sold Out Soon)',
  },
  {
    id    => 'price_plus_promo',
    desc  => 'price plus promotion wording combination',
    score => 15,
    regex => qr/(?=.*(?:\$\s*\d{1,4}(?:\.\d{2})?|\b\d{1,4}\.\d{2}\b))(?=.*\b(sale|clearance|limited|exclusive|offer|save|deal|discount|bottom price)\b)/i,
    rule  => '^Subject:.*(Sale|Clearance|Limited|Exclusive|Offer|Save|Deal|Discount|Bottom Price).*(\$[0-9]+|[0-9]{1,4}\.[0-9]{2})',
  },
  {
    id    => 'price_plus_urgency',
    desc  => 'price plus urgency wording combination',
    score => 15,
    regex => qr/(?=.*(?:\$\s*\d{1,4}(?:\.\d{2})?|\b\d{1,4}\.\d{2}\b))(?=.*\b(last chance|last wave|countdown|today only|48hrs?|72hrs?|sold out soon)\b)/i,
    rule  => '^Subject:.*(Last Chance|Last Wave|Countdown|Today Only|48Hrs|72Hrs|Sold Out Soon).*(\$[0-9]+|[0-9]{1,4}\.[0-9]{2})',
  },
  {
    id    => 'price_plus_product',
    desc  => 'price plus product-sales wording',
    score => 10,
    regex => qr/(?=.*(?:\$\s*\d{1,4}(?:\.\d{2})?|\b\d{1,4}\.\d{2}\b))(?=.*\b(smart watch|router|drone|gaming chair|power station|ultrasonic)\b)/i,
    rule  => '^Subject:.*(Smart Watch|Router|Drone|Gaming Chair|Power Station|Ultrasonic).*(\$[0-9]+|[0-9]{1,4}\.[0-9]{2})',
  },
);

print "# ==========================================================\n";
print "# suggested SCORE rules\n";
print "# ==========================================================\n\n";

my @score_rules_only = map { $_->{rule} } @existing_score;

for my $pat (@patterns) {
  my $deny_hits = 0;
  my $pass_hits = 0;
  my @pat_examples;
  my %pat_froms;

  for my $msgid (keys %msg_subject) {
    my $decision = $msg_decision{$msgid} // "";
    my $subject  = normalize_subject($msg_subject{$msgid});
    next unless length $subject;

    if ($decision =~ /deny/ && $subject =~ $pat->{regex}) {
      $deny_hits++;
      push @pat_examples, $subject if @pat_examples < $max_examples;
      if (exists $msg_from{$msgid}) {
        $pat_froms{$msg_from{$msgid}} = 1;
      }
    }

    if ($decision eq 'pass' && $subject =~ $pat->{regex}) {
      $pass_hits++;
    }
  }

  next if $deny_hits < $min_deny;
  next if $pass_hits > $max_pass;

  my $confidence = "medium";
  $confidence = "high" if $deny_hits >= 8 && $pass_hits == 0;

  my $candidate_rule = $pat->{rule};
  my ($status, $existing) = compare_rule_to_existing($candidate_rule, \@score_rules_only);
  my $from_line = compact_list_line(\%pat_froms, $max_froms);

  print "# --------------------------------------------------\n";
  print "# auto-generated candidate\n";
  print "# from_addresses: $from_line\n" if length $from_line;
  print "# type: SCORE\n";
  print "# pattern_id: $pat->{id}\n";
  print "# description: $pat->{desc}\n";
  print "# deny_hits: $deny_hits\n";
  print "# pass_hits: $pass_hits\n";
  print "# confidence: $confidence\n";
  print "# risk: low\n";
  print "# recommendation: suitable for additive scoring\n";
  print "# status: $status\n";
  print "# existing_rule: $existing\n" if $existing;
  print "# suggested_score: $pat->{score}\n";
  print "# highscore_reference: $highscore\n";
  print "# examples:\n";
  for my $e (@pat_examples) {
    print "#   $e\n";
  }
  print "# suggested rule:\n";
  print "# SCORE +$pat->{score}=\"$candidate_rule\"\n";
  push @export_rules,
	"SCORE +$pat->{score}=\"$candidate_rule\"";
	
	 if ($deny_hits >= $min_deny && $pass_hits <= $max_pass) {

	  if (allow_conservative("recent", 0, 0, 0, $deny_hits)) {
		push @export_rules_conservative,
		  "SCORE +$pat->{score}=\"$candidate_rule\"";
	  }

	  if (allow_aggressive("recent", 0, $deny_hits)) {
		push @export_rules_aggressive,
		  "SCORE +$pat->{score}=\"$candidate_rule\"";
	  }
	}
  print "# --------------------------------------------------\n\n";
}

print "\n";
print "# ==========================================================\n";
print "# recommended rules\n";
print "# ==========================================================\n\n";

# optional: very simple deduplication
my %seen;

if (@recommended_domain_rules) {
  print "# domain candidates\n";
  for my $r (@recommended_domain_rules) {
    next if $seen{$r}++;
    print "$r\n";
  }
  print "\n";
}

if (@recommended_score_rules) {
  print "# subject/score candidates\n";
  for my $r (@recommended_score_rules) {
    next if $seen{$r}++;
    print "$r\n";
  }
  print "\n";
}

if (@recommended_campaign_rules) {
  print "# campaign candidates\n";
  for my $r (@recommended_campaign_rules) {
    next if $seen{$r}++;
    print "$r\n";
  }
  print "\n";
}

if (!@recommended_domain_rules && !@recommended_score_rules && !@recommended_campaign_rules) {
  print "# (no recommended rules)\n\n";
}

sub write_rules_file {
  my ($file, $rules_ref, $title) = @_;

  return unless $file && @$rules_ref;

  my %seen;
  my @unique = grep { !$seen{$_}++ } @$rules_ref;

  open my $fh, ">", $file or do {
    warn "Cannot write $file\n";
    return;
  };

  print $fh "# ==========================================================\n";
  print $fh "# $title\n";
  print $fh "# auto-generated by mailfilter-rulegen\n";
  print $fh "# generated: " . localtime() . "\n";
  print $fh "# ==========================================================\n\n";

  for my $r (@unique) {
    print $fh "$r\n";
  }

  close $fh;

  print "# wrote $file (" . scalar(@unique) . " rules)\n";
}

write_rules_file(
  $export_rules_file,
  \@export_rules,
  "generated-rules.conf"
);

write_rules_file(
  $export_conservative,
  \@export_rules_conservative,
  "generated-conservative-rules.conf"
);

write_rules_file(
  $export_aggressive,
  \@export_rules_aggressive,
  "generated-aggressive-rules.conf"
);
