#!/usr/bin/env python3
"""
mailfilter-analyze.py – Version 11a
"""

import sqlite3
import argparse
import os
import re
from datetime import datetime, timedelta
from email.header import decode_header
from email.utils import parsedate_to_datetime, mktime_tz, parsedate_tz
from collections import defaultdict

DB_PATH = "/var/spool/filter/mailheader.sqlite3"
EXTRA_RULES_FILE = "/etc/mailfilter/generated-extra-scores.conf"

def get_db_connection():
    if not os.path.exists(DB_PATH):
        raise FileNotFoundError(f"Datenbank nicht gefunden: {DB_PATH}")
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn

def parse_date_to_ts(date_str):
    if not date_str:
        return None
    try:
        clean = re.sub(r'\s*\([A-Za-z]+\)$', '', date_str.strip())
        parsed = parsedate_tz(clean)
        if parsed:
            return mktime_tz(parsed)
    except:
        pass
    return None

def decode_subject(subject):
    if not subject:
        return ""
    try:
        decoded_parts = []
        for part, encoding in decode_header(subject):
            if isinstance(part, bytes):
                decoded_parts.append(part.decode(encoding or 'utf-8', errors='replace'))
            else:
                decoded_parts.append(str(part))
        return " ".join(decoded_parts).strip()
    except:
        s = re.sub(r'=\?UTF-8\?[BQ]\?(.+?)\?=', lambda m: m.group(1).replace('_', ' '), subject)
        return re.sub(r'=\?.*?\?=', '', s).strip()

def print_stats(days=None):
    conn = get_db_connection()
    cur = conn.cursor()

    print("\n=== mailfilter SQLite Analyse (Version 11a) ===\n")
    now = datetime.now()
    print(f"Stand (heute)          : {now.strftime('%Y-%m-%d %H:%M:%S')}")

    cur.execute("SELECT COUNT(*) as total FROM messages")
    total = cur.fetchone()["total"]
    print(f"Gesamt Mails in DB     : {total:,}")

    if days:
        cutoff_date = now - timedelta(days=days)
        print(f"Gefilterter Zeitraum   : ab {cutoff_date.strftime('%Y-%m-%d')}  (letzte {days} Tage)")
    else:
        print("Gefilterter Zeitraum   : alle Mails")

    # Gefilterte Mails sammeln
    filtered_rows = []
    cutoff_ts = int((now - timedelta(days=days or 99999)).timestamp()) if days else 0

    cur.execute("SELECT msg_log_id, final_score, decision, subject, date_hdr FROM messages")
    for row in cur.fetchall():
        if not days:
            filtered_rows.append(row)
            continue
        ts = parse_date_to_ts(row['date_hdr'])
        if ts and ts >= cutoff_ts:
            filtered_rows.append(row)

    print(f"Gefilterte Mails (letzte {days or 'alle'} Tage) : {len(filtered_rows):,}")

    # Decision-Verteilung
    decision_count = defaultdict(int)
    score_sum = defaultdict(float)
    for row in filtered_rows:
        d = row['decision'] or 'unknown'
        decision_count[d] += 1
        score_sum[d] += row['final_score'] or 0

    print("\nDecision-Verteilung (gefiltert):")
    for d in sorted(decision_count, key=decision_count.get, reverse=True):
        avg = score_sum[d] / decision_count[d] if decision_count[d] > 0 else 0
        print(f"  {d:12} -> {decision_count[d]:6,} Mails   (Ø Score: {avg:.1f})")

    # Top 15 Subjects mit msg_log_id
    subject_data = defaultdict(lambda: {'count': 0, 'score_sum': 0.0, 'example_id': None})
    for row in filtered_rows:
        subj = row['subject'] or ""
        data = subject_data[subj]
        data['count'] += 1
        data['score_sum'] += row['final_score'] or 0
        if data['example_id'] is None:
            data['example_id'] = row['msg_log_id']

    print("\nTop 15 Subjects (gefiltert) – mit Beispiel-msg_log_id:")
    for subj, data in sorted(subject_data.items(), key=lambda x: x[1]['count'], reverse=True)[:15]:
        avg = data['score_sum'] / data['count'] if data['count'] > 0 else 0
        clean = decode_subject(subj)
        msg_id = data['example_id'] or '—'
        print(f"  {data['count']:5,} ×  (Ø {avg:.1f})  | msg_log_id: {msg_id:<14} -> {clean[:80]}{'...' if len(clean) > 70 else ''}")

    # Häufige gute Header
    print("\nHäufigste Header bei Mails (final_score <= 10):")
    cur.execute("""
        SELECT tag, COUNT(DISTINCT he.msg_log_id) as mails
        FROM header_entries he
        JOIN messages m ON he.msg_log_id = m.msg_log_id
        WHERE m.final_score <= 10
        GROUP BY tag
        ORDER BY mails DESC LIMIT 15
    """)
    for row in cur.fetchall():
        print(f"  {row['tag']:25} -> {row['mails']:6,} Mails")

    conn.close()

def generate_extra_rules():
    conn = get_db_connection()
    rules = [f"# === Extra-Rules generiert am {datetime.now().strftime('%Y-%m-%d %H:%M')} ==="]

    rules.append("\n# Starke Belohnung für gute Authentifizierung")
    rules.append('SCORE -25="^DKIM-Signature:"')
    rules.append('SCORE -55="^Authentication-Results:.*(pass|dkim=pass|spf=pass|dmarc=pass)"')

    rules.append("\n# Interne Gateway-Header")
    rules.append('SCORE -15="^X-TOI-EXPURGATEID:"')
    rules.append('SCORE -15="^X-TOI-MSGID:"')
    rules.append('SCORE -15="^X-TOI-VIRUSSCAN:"')

    rules.append("\n# Basis-Header")
    for h in ["From:", "Received:", "Message-ID:", "Date:"]:
        rules.append(f'SCORE -8="^{h}"')

    rules.append("\n# Milde Strafe für häufige Kleinanzeigen-Antworten")
    rules.append('SCORE +15="^Subject:.*Re: Nutzer-Anfrage zu deiner Anzeige"')

    with open(EXTRA_RULES_FILE, "w", encoding="utf-8") as f:
        f.write("\n".join(rules) + "\n")

    print(f"✅ Extra-Rules erzeugt -> {EXTRA_RULES_FILE}")
    conn.close()

def main():
    parser = argparse.ArgumentParser(description="mailfilter SQLite Analyse Tool v11a")
    parser.add_argument("--stats", action="store_true", help="Statistik anzeigen")
    parser.add_argument("--days", type=int, help="Nur Mails der letzten N Tage")
    parser.add_argument("--generate-rules", action="store_true", help="Extra SCORE-Regeln generieren")
    args = parser.parse_args()

    if args.stats or args.days is not None:
        print_stats(days=args.days)
    if args.generate_rules:
        generate_extra_rules()

    if not any([args.stats, args.generate_rules]):
        parser.print_help()

if __name__ == "__main__":
    main()
