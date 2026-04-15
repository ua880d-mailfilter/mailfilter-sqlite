#!/usr/bin/env python3
# /usr/bin/mailfilter-analyze.py
# V. 15a

from __future__ import annotations

import argparse
import os
import re
import sqlite3
from collections import Counter, defaultdict
from dataclasses import dataclass
from datetime import datetime, timedelta, timezone
from email.header import decode_header
from email.utils import mktime_tz, parsedate_tz
from typing import Iterable


DB_PATH = "/var/spool/filter/mailheader-1.sqlite3"
EXTRA_RULES_FILE = "/etc/mailfilter/generated-extra-scores.conf"
DEFAULT_TIME_FIELD = "created_at"
DEFAULT_AUTH_LIMIT = 15

ALARM_SUBJECT_PATTERNS = (
    r"\bverdächtige aktivität\b",
    r"\bsuspicious activity\b",
    r"\baction required\b",
    r"\bdringend\b",
    r"\burgent\b",
    r"\bjetzt verifizieren\b",
    r"\bverify\b",
    r"\bsecurity alert\b",
    r"\bsicherheitswarnung\b",
)

PAYMENT_SUBJECT_PATTERNS = (
    r"\bbeleg\b",
    r"\bzahlung\b",
    r"\brechnung\b",
    r"\binvoice\b",
    r"\bpayment\b",
    r"\bpaypal\b",
    r"\bgoogle play\b",
)

BULK_HEADER_HINTS = (
    "list-unsubscribe",
    "list-unsubscribe-post",
    "list-id",
)

BULK_DOMAIN_HINTS = (
    "emarsys.net",
    "mailjet.com",
    "sendgrid.net",
    "amazonses.com",
    "mailchimpapp.net",
)

HIGH_TRUST_FROM_DOMAINS = (
    "google.com",
    "youtube.com",
    "paypal.de",
    "paypal.com",
    "github.com",
)

SUSPICIOUS_TLDS = (
    ".top",
    ".xyz",
    ".click",
    ".shop",
    ".online",
    ".site",
    ".icu",
)

@dataclass(frozen=True)
class MessageRow:
    msg_log_id: str
    final_score: float | None
    decision: str | None
    subject: str | None
    date_hdr: str | None
    created_at: str | None
    from_addr: str | None


@dataclass(frozen=True)
class AuthAnalysis:
    msg_log_id: str
    subject: str
    from_addr: str
    decision: str
    final_score: float
    date_hdr: str
    created_at: str
    auth_results_count: int
    arc_auth_results_count: int
    dkim_signature_count: int
    dkim_pass: bool
    spf_pass: bool
    dmarc_pass: bool
    any_fail: bool
    auth_from_domains: tuple[str, ...]
    smtp_mailfrom_domains: tuple[str, ...]
    dkim_domains: tuple[str, ...]
    dkim_selectors: tuple[str, ...]
    flags: tuple[str, ...]
    header_names: tuple[str, ...]
    marker: str
    suspicion_score: int


def get_db_connection() -> sqlite3.Connection:
    if not os.path.exists(DB_PATH):
        raise FileNotFoundError(f"Datenbank nicht gefunden: {DB_PATH}")
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn


def positive_int(value: str) -> int:
    try:
        parsed = int(value)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("muss eine ganze Zahl sein") from exc
    if parsed <= 0:
        raise argparse.ArgumentTypeError("muss > 0 sein")
    return parsed


def parse_date_hdr_to_ts(date_str: str | None) -> int | None:
    if not date_str:
        return None
    try:
        clean = re.sub(r"\s*\([A-Za-z]+\)$", "", date_str.strip())
        parsed = parsedate_tz(clean)
        if parsed:
            return int(mktime_tz(parsed))
    except Exception:
        return None
    return None


def parse_created_at_to_ts(value: str | None) -> int | None:
    if not value:
        return None

    raw = str(value).strip()
    if not raw:
        return None

    candidates = [
        "%Y-%m-%d %H:%M:%S",
        "%Y-%m-%d %H:%M:%S.%f",
        "%Y-%m-%dT%H:%M:%S",
        "%Y-%m-%dT%H:%M:%S.%f",
    ]

    for fmt in candidates:
        try:
            dt = datetime.strptime(raw[:26], fmt)
            dt = dt.replace(tzinfo=timezone.utc)
            return int(dt.timestamp())
        except ValueError:
            continue

    try:
        dt = datetime.fromisoformat(raw.replace("Z", "+00:00"))
    except ValueError:
        return None

    if dt.tzinfo is None:
        dt = dt.replace(tzinfo=timezone.utc)
    else:
        dt = dt.astimezone(timezone.utc)

    return int(dt.timestamp())


def resolve_row_ts(row: MessageRow, time_field: str) -> int | None:
    if time_field == "created_at":
        return parse_created_at_to_ts(row.created_at)
    if time_field == "date_hdr":
        return parse_date_hdr_to_ts(row.date_hdr)
    raise ValueError(f"Unbekanntes Zeitfeld: {time_field}")


def decode_subject(subject: str | None) -> str:
    if not subject:
        return ""

    try:
        decoded_parts: list[str] = []
        for part, encoding in decode_header(subject):
            if isinstance(part, bytes):
                decoded_parts.append(part.decode(encoding or "utf-8", errors="replace"))
            else:
                decoded_parts.append(str(part))
        return " ".join(decoded_parts).strip()
    except Exception:
        s = re.sub(
            r"=\?UTF-8\?[BQ]\?(.+?)\?=",
            lambda m: m.group(1).replace("_", " "),
            str(subject),
        )
        return re.sub(r"=\?.*?\?=", "", s).strip()


def fetch_messages(conn: sqlite3.Connection) -> list[MessageRow]:
    cur = conn.cursor()
    cur.execute(
        """
        SELECT
            msg_log_id,
            final_score,
            decision,
            subject,
            date_hdr,
            created_at,
            from_addr
        FROM messages
        """
    )

    rows: list[MessageRow] = []
    for row in cur.fetchall():
        rows.append(
            MessageRow(
                msg_log_id=str(row["msg_log_id"]),
                final_score=row["final_score"],
                decision=row["decision"],
                subject=row["subject"],
                date_hdr=row["date_hdr"],
                created_at=row["created_at"],
                from_addr=row["from_addr"],
            )
        )
    return rows


def filter_rows(
    rows: list[MessageRow],
    *,
    days: int | None,
    latest: int | None,
    time_field: str,
) -> tuple[list[MessageRow], dict[str, int]]:
    now_ts = int(datetime.now(timezone.utc).timestamp())
    cutoff_ts = now_ts - (days * 86400) if days else None

    diagnostics = {
        "total_rows": len(rows),
        "parsed_ok": 0,
        "parsed_failed": 0,
        "future_skew": 0,
    }

    prepared: list[tuple[int, MessageRow]] = []

    for row in rows:
        ts = resolve_row_ts(row, time_field)
        if ts is None:
            diagnostics["parsed_failed"] += 1
            continue

        diagnostics["parsed_ok"] += 1
        if ts > now_ts + 2 * 86400:
            diagnostics["future_skew"] += 1
        if cutoff_ts is not None and ts < cutoff_ts:
            continue

        prepared.append((ts, row))

    prepared.sort(key=lambda item: item[0], reverse=True)

    if latest is not None:
        prepared = prepared[:latest]

    return [row for _, row in prepared], diagnostics


def extract_domain(value: str | None) -> str:
    if not value:
        return ""

    text = str(value).strip().lower()
    match = re.search(r"<([^>]+)>", text)
    if match:
        text = match.group(1)

    if "@" in text:
        text = text.rsplit("@", 1)[-1]

    text = text.strip(" <>;,'\"()[]")
    if not text or "." not in text:
        return ""

    return text


def unique_preserve_order(values: Iterable[str]) -> tuple[str, ...]:
    seen: set[str] = set()
    result: list[str] = []
    for value in values:
        item = value.strip().lower()
        if not item or item in seen:
            continue
        seen.add(item)
        result.append(item)
    return tuple(result)


def domains_aligned(reference_domain: str, candidates: Iterable[str]) -> bool:
    if not reference_domain:
        return False

    ref = reference_domain.lower()
    for candidate in candidates:
        cand = candidate.lower()
        if cand == ref or cand.endswith("." + ref) or ref.endswith("." + cand):
            return True
    return False

def text_matches_any(text: str, patterns: Iterable[str]) -> bool:
    lowered = text.lower()
    return any(re.search(pattern, lowered, flags=re.IGNORECASE) for pattern in patterns)


def has_bulk_hint(header_names: Iterable[str], dkim_domains: Iterable[str]) -> bool:
    header_names_l = {name.lower() for name in header_names}
    if any(hint in header_names_l for hint in BULK_HEADER_HINTS):
        return True

    for domain in dkim_domains:
        lowered = domain.lower()
        if any(lowered == hint or lowered.endswith("." + hint) for hint in BULK_DOMAIN_HINTS):
            return True

    return False


def domain_looks_low_trust(from_domain: str) -> bool:
    lowered = from_domain.lower()
    if not lowered:
        return True

    if any(lowered == domain or lowered.endswith("." + domain) for domain in HIGH_TRUST_FROM_DOMAINS):
        return False

    if any(lowered.endswith(tld) for tld in SUSPICIOUS_TLDS):
        return True

    if lowered.count(".") >= 3:
        return True

    return False


def build_combined_auth_signals(
    *,
    subject: str,
    from_domain: str,
    auth_from_domains: tuple[str, ...],
    smtp_mailfrom_domains: tuple[str, ...],
    dkim_domains: tuple[str, ...],
    header_names: tuple[str, ...],
    dkim_pass: bool,
    spf_pass: bool,
    dmarc_pass: bool,
    any_fail: bool,
    has_dkim_sig: bool,
) -> tuple[str, ...]:
    combined: list[str] = []

    auth_from_aligned = domains_aligned(from_domain, auth_from_domains)
    mailfrom_aligned = domains_aligned(from_domain, smtp_mailfrom_domains)
    dkim_aligned = domains_aligned(from_domain, dkim_domains)

    alarm_subject = text_matches_any(subject, ALARM_SUBJECT_PATTERNS)
    payment_subject = text_matches_any(subject, PAYMENT_SUBJECT_PATTERNS)
    bulk_hint = has_bulk_hint(header_names, dkim_domains)
    low_trust_domain = domain_looks_low_trust(from_domain)
    multi_dkim_context = len(dkim_domains) > 1

    if dkim_pass and spf_pass and dmarc_pass:
        combined.append("AUTH_PASS_TRIPLE")

    if dkim_pass and dkim_aligned:
        combined.append("DKIM_PASS_D_ALIGNED")

    if spf_pass and mailfrom_aligned:
        combined.append("SPF_PASS_MAILFROM_ALIGNED")

    if dmarc_pass and auth_from_aligned:
        combined.append("DMARC_PASS_FROM_ALIGNED")

    if has_dkim_sig and not dkim_pass:
        combined.append("DKIM_SIG_ONLY")

    if any_fail:
        combined.append("AUTH_FAIL_PRESENT")

    if alarm_subject:
        combined.append("ALARM_SUBJECT_PATTERN")

    if payment_subject:
        combined.append("PAYMENT_SUBJECT_PATTERN")

    if bulk_hint:
        combined.append("BULK_CONTEXT")

    if low_trust_domain:
        combined.append("LOW_TRUST_DOMAIN_CONTEXT")

    if multi_dkim_context:
        combined.append("MULTI_DKIM_CONTEXT")

    if (
        (dkim_pass or spf_pass or dmarc_pass)
        and not any_fail
        and alarm_subject
        and low_trust_domain
    ):
        combined.append("AUTH_OK_BUT_CONTEXT_SUSPECT")

    if (
        (dkim_pass or spf_pass or dmarc_pass)
        and not any_fail
        and bulk_hint
        and not alarm_subject
    ):
        combined.append("AUTH_OK_BULK_CONTEXT")

    if (
        dkim_pass
        and (spf_pass or dmarc_pass)
        and (dkim_aligned or auth_from_aligned or mailfrom_aligned)
        and not low_trust_domain
        and not alarm_subject
    ):
        combined.append("AUTH_OK_STRONG")

    return tuple(unique_preserve_order(combined))


def auth_suspicion_score(flags: Iterable[str]) -> int:
    weights = {
        "AUTH_FAIL_PRESENT": 5,
        "AUTH_OK_BUT_CONTEXT_SUSPECT": 5,
        "ALARM_SUBJECT_PATTERN": 3,
        "LOW_TRUST_DOMAIN_CONTEXT": 3,
        "MULTI_DKIM_CONTEXT": 2,
        "PAYMENT_SUBJECT_PATTERN": 1,
        "DKIM_SIG_ONLY": 1,
        "AUTH_OK_BULK_CONTEXT": -1,
        "AUTH_OK_STRONG": -2,
        "AUTH_PASS_TRIPLE": -2,
        "DKIM_PASS_D_ALIGNED": -1,
        "SPF_PASS_MAILFROM_ALIGNED": -1,
        "DMARC_PASS_FROM_ALIGNED": -1,
    }
    return sum(weights.get(flag, 0) for flag in flags)


def classify_marker(flags: Iterable[str]) -> str:
    flags_set = set(flags)

    if "AUTH_OK_BUT_CONTEXT_SUSPECT" in flags_set or "AUTH_FAIL_PRESENT" in flags_set:
        return "!!! suspicious"
    if "AUTH_OK_BULK_CONTEXT" in flags_set:
        return "bulk/auth-ok"
    if "AUTH_OK_STRONG" in flags_set or "AUTH_PASS_TRIPLE" in flags_set:
        return "auth-strong"
    return "auth-neutral"


def fetch_relevant_headers(
    conn: sqlite3.Connection,
    msg_ids: list[str],
) -> dict[str, dict[str, list[str]]]:
    if not msg_ids:
        return {}

    placeholders = ",".join(["?"] * len(msg_ids))
    cur = conn.cursor()
    cur.execute(
        f"""
        SELECT msg_log_id, tag, body
        FROM header_entries
        WHERE msg_log_id IN ({placeholders})
          AND tag IN (
            'Authentication-Results',
            'ARC-Authentication-Results',
            'DKIM-Signature',
            'List-Unsubscribe',
            'List-Unsubscribe-Post',
            'List-Id'
          )
        ORDER BY msg_log_id, ordinal
        """,
        msg_ids,
    )

    result: dict[str, dict[str, list[str]]] = defaultdict(lambda: defaultdict(list))
    for row in cur.fetchall():
        result[str(row["msg_log_id"])][str(row["tag"])].append(str(row["body"] or ""))
    return result


def parse_auth_domains(header_value: str) -> tuple[list[str], list[str]]:
    auth_from = re.findall(r"header\.from=([^\s;]+)", header_value, flags=re.IGNORECASE)
    smtp_mailfrom = re.findall(r"smtp\.mailfrom=([^\s;]+)", header_value, flags=re.IGNORECASE)

    auth_domains = [extract_domain(value) for value in auth_from]
    mailfrom_domains = [extract_domain(value) for value in smtp_mailfrom]
    return auth_domains, mailfrom_domains


def parse_dkim_signature(header_value: str) -> tuple[list[str], list[str]]:
    dkim_domains = re.findall(r"(?:^|;\s*)d=([^;\s]+)", header_value, flags=re.IGNORECASE)
    selectors = re.findall(r"(?:^|;\s*)s=([^;\s]+)", header_value, flags=re.IGNORECASE)
    return [value.strip().lower() for value in dkim_domains], [value.strip().lower() for value in selectors]


def analyze_auth_for_rows(
    rows: list[MessageRow],
    conn: sqlite3.Connection,
) -> tuple[list[AuthAnalysis], Counter[str]]:
    headers_by_msg = fetch_relevant_headers(conn, [row.msg_log_id for row in rows])
    analyses: list[AuthAnalysis] = []
    flag_counter: Counter[str] = Counter()

    for row in rows:
        header_map = headers_by_msg.get(row.msg_log_id, {})
        auth_headers = header_map.get("Authentication-Results", [])
        arc_headers = header_map.get("ARC-Authentication-Results", [])
        dkim_headers = header_map.get("DKIM-Signature", [])
        header_names = tuple(sorted(header_map.keys()))

        auth_blob = "\n".join(auth_headers + arc_headers).lower()

        dkim_pass = "dkim=pass" in auth_blob
        spf_pass = "spf=pass" in auth_blob
        dmarc_pass = "dmarc=pass" in auth_blob
        any_fail = any(
            token in auth_blob
            for token in (
                "dkim=fail",
                "dkim=temperror",
                "dkim=permerror",
                "spf=fail",
                "spf=softfail",
                "dmarc=fail",
            )
        )

        auth_domains: list[str] = []
        smtp_mailfrom_domains: list[str] = []
        for header_value in auth_headers + arc_headers:
            parsed_auth_domains, parsed_mailfrom_domains = parse_auth_domains(header_value)
            auth_domains.extend(parsed_auth_domains)
            smtp_mailfrom_domains.extend(parsed_mailfrom_domains)

        dkim_domains: list[str] = []
        dkim_selectors: list[str] = []
        for header_value in dkim_headers:
            parsed_dkim_domains, parsed_selectors = parse_dkim_signature(header_value)
            dkim_domains.extend(parsed_dkim_domains)
            dkim_selectors.extend(parsed_selectors)

        from_domain = extract_domain(row.from_addr)

        base_flags: list[str] = []

        if dkim_pass:
            base_flags.append("DKIM_PASS")
        if spf_pass:
            base_flags.append("SPF_PASS")
        if dmarc_pass:
            base_flags.append("DMARC_PASS")
        if any_fail:
            base_flags.append("AUTH_FAIL")

        if auth_headers or arc_headers:
            base_flags.append("AUTH_PRESENT")
        if dkim_headers:
            base_flags.append("DKIM_SIG_PRESENT")

        if from_domain:
            if auth_domains and not domains_aligned(from_domain, auth_domains):
                base_flags.append("FROM_AUTH_MISMATCH")
            if smtp_mailfrom_domains and not domains_aligned(from_domain, smtp_mailfrom_domains):
                base_flags.append("FROM_MAILFROM_MISMATCH")
            if dkim_domains and not domains_aligned(from_domain, dkim_domains):
                base_flags.append("FROM_DKIM_MISMATCH")

        if (dkim_pass or spf_pass or dmarc_pass) and not any_fail:
            base_flags.append("AUTH_TECH_OK")

        if not (dkim_pass or spf_pass or dmarc_pass):
            base_flags.append("NO_PASS_SIGNAL")

        if not auth_headers and not arc_headers and not dkim_headers:
            base_flags.append("NO_AUTH_HEADERS")

        combined_flags = build_combined_auth_signals(
            subject=decode_subject(row.subject),
            from_domain=from_domain,
            auth_from_domains=unique_preserve_order(auth_domains),
            smtp_mailfrom_domains=unique_preserve_order(smtp_mailfrom_domains),
            dkim_domains=unique_preserve_order(dkim_domains),
            header_names=header_names,
            dkim_pass=dkim_pass,
            spf_pass=spf_pass,
            dmarc_pass=dmarc_pass,
            any_fail=any_fail,
            has_dkim_sig=bool(dkim_headers),
        )

        all_flags = unique_preserve_order(list(base_flags) + list(combined_flags))
        marker = classify_marker(all_flags)
        suspicion_score = auth_suspicion_score(all_flags)

        flag_counter.update(all_flags)

        analyses.append(
            AuthAnalysis(
                msg_log_id=row.msg_log_id,
                subject=decode_subject(row.subject),
                from_addr=row.from_addr or "",
                decision=row.decision or "unknown",
                final_score=float(row.final_score or 0.0),
                date_hdr=row.date_hdr or "",
                created_at=row.created_at or "",
                auth_results_count=len(auth_headers),
                arc_auth_results_count=len(arc_headers),
                dkim_signature_count=len(dkim_headers),
                dkim_pass=dkim_pass,
                spf_pass=spf_pass,
                dmarc_pass=dmarc_pass,
                any_fail=any_fail,
                auth_from_domains=unique_preserve_order(auth_domains),
                smtp_mailfrom_domains=unique_preserve_order(smtp_mailfrom_domains),
                dkim_domains=unique_preserve_order(dkim_domains),
                dkim_selectors=unique_preserve_order(dkim_selectors),
                flags=all_flags,
                header_names=header_names,
                marker=marker,
                suspicion_score=suspicion_score,
            )
        )

    return analyses, flag_counter


def auth_priority(item: AuthAnalysis) -> tuple[int, float]:
    return (item.suspicion_score, item.final_score)


def print_auth_analysis(
    rows: list[MessageRow],
    conn: sqlite3.Connection,
    auth_limit: int,
) -> None:
    analyses, flag_counter = analyze_auth_for_rows(rows, conn)

    print("\nAuth-/DKIM-Analyse (gefilterte Menge):")
    print(f"  Untersuchte Mails        : {len(analyses):,}")

    if not analyses:
        print("  Keine Datensätze verfügbar.")
        return

    print("  Häufigste Flags:")
    for flag, count in flag_counter.most_common(12):
        print(f"    {flag:28} -> {count:6,}")

    ranked = sorted(analyses, key=auth_priority, reverse=True)[:auth_limit]

    print(f"\nTop {auth_limit} Auth-Kandidaten:")
    for item in ranked:
        subject_short = item.subject[:88] + ("..." if len(item.subject) > 88 else "")
        domains = []
        if item.auth_from_domains:
            domains.append("auth_from=" + ",".join(item.auth_from_domains))
        if item.smtp_mailfrom_domains:
            domains.append("mailfrom=" + ",".join(item.smtp_mailfrom_domains))
        if item.dkim_domains:
            domains.append("dkim_d=" + ",".join(item.dkim_domains))
        if item.dkim_selectors:
            domains.append("dkim_s=" + ",".join(item.dkim_selectors))

                print(
            f"  {'msg_log_id=' + item.msg_log_id:<22}"
            f"{'marker=' + item.marker:<30}"
            f"{'decision=' + item.decision:<18}"
            f"{'score=' + format(item.final_score, '7.1f')}"
        )

        flags_text = ", ".join(item.flags) if item.flags else "—"
        details_text = " | ".join(domains) if domains else "—"

        print(f"    {'Subject':<8}: {subject_short}")
        print(f"    {'From':<8}: {item.from_addr or '—'}")
        print(f"    {'Flags':<8}: {flags_text}")
        print(f"    {'Details':<8}: {details_text}")


def print_stats(
    *,
    days: int | None = None,
    latest: int | None = None,
    time_field: str = DEFAULT_TIME_FIELD,
    verbose: bool = False,
    analyze_auth: bool = False,
    auth_limit: int = DEFAULT_AUTH_LIMIT,
) -> None:
    conn = get_db_connection()
    try:
        rows = fetch_messages(conn)
        cur = conn.cursor()

        print("\n=== mailfilter SQLite Analyse ===\n")
        now = datetime.now()
        print(f"Stand (heute)            : {now.strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"Zeitfeld                 : {time_field}")

        cur.execute("SELECT COUNT(*) AS total FROM messages")
        total = cur.fetchone()["total"]
        print(f"Gesamt Mails in DB       : {total:,}")

        if days is not None:
            cutoff_date = now - timedelta(days=days)
            print(
                f"Gefilterter Zeitraum     : ab {cutoff_date.strftime('%Y-%m-%d')} "
                f"(letzte {days} Tage)"
            )
        else:
            print("Gefilterter Zeitraum     : alle Mails")

        if latest is not None:
            print(f"Max. neueste Ergebnisse  : {latest}")
        else:
            print("Max. neueste Ergebnisse  : unbegrenzt")

        filtered_rows, diagnostics = filter_rows(
            rows,
            days=days,
            latest=latest,
            time_field=time_field,
        )

        print(f"Gefilterte Mails         : {len(filtered_rows):,}")

        if verbose:
            print("\nZeit-Diagnose:")
            print(f"  parsebar               : {diagnostics['parsed_ok']:,}")
            print(f"  unparsebar             : {diagnostics['parsed_failed']:,}")
            print(f"  future_skew            : {diagnostics['future_skew']:,}")

        decision_count: dict[str, int] = defaultdict(int)
        score_sum: dict[str, float] = defaultdict(float)

        for row in filtered_rows:
            decision = row.decision or "unknown"
            decision_count[decision] += 1
            score_sum[decision] += row.final_score or 0.0

        print("\nDecision-Verteilung (gefiltert):")
        for decision in sorted(decision_count, key=decision_count.get, reverse=True):
            count = decision_count[decision]
            avg = score_sum[decision] / count if count > 0 else 0.0
            print(f"  {decision:12} -> {count:6,} Mails (Ø Score: {avg:.1f})")

        subject_data: dict[str, dict[str, object]] = defaultdict(
            lambda: {"count": 0, "score_sum": 0.0, "example_id": None}
        )

        for row in filtered_rows:
            subject = row.subject or ""
            data = subject_data[subject]
            data["count"] += 1
            data["score_sum"] += row.final_score or 0.0
            if data["example_id"] is None:
                data["example_id"] = row.msg_log_id

        print("\nTop 15 Subjects (gefiltert) – mit Beispiel-msg_log_id:")
        for subject, data in sorted(
            subject_data.items(),
            key=lambda item: int(item[1]["count"]),
            reverse=True,
        )[:15]:
            count = int(data["count"])
            avg = float(data["score_sum"]) / count if count > 0 else 0.0
            msg_id = data["example_id"] or "—"
            clean = decode_subject(subject)
            suffix = "..." if len(clean) > 70 else ""
            print(
                f"  {count:5,} × (Ø {avg:.1f}) | msg_log_id: {msg_id:<14} -> "
                f"{clean[:80]}{suffix}"
            )

        if analyze_auth:
            print_auth_analysis(
                rows=filtered_rows,
                conn=conn,
                auth_limit=auth_limit,
            )

    finally:
        conn.close()


def generate_extra_rules() -> None:
    conn = get_db_connection()
    try:
        rules = [f"# === Extra-Rules generiert am {datetime.now().strftime('%Y-%m-%d %H:%M')} ==="]
        rules.append("\n# Starke Belohnung für gute Authentifizierung")
        rules.append('SCORE -25="^DKIM-Signature:"')
        rules.append('SCORE -55="^Authentication-Results:.*(pass|dkim=pass|spf=pass|dmarc=pass)"')

        rules.append("\n# Interne Gateway-Header")
        rules.append('SCORE -15="^X-TOI-EXPURGATEID:"')
        rules.append('SCORE -15="^X-TOI-MSGID:"')
        rules.append('SCORE -15="^X-TOI-VIRUSSCAN:"')

        rules.append("\n# Basis-Header")
        for header in ["From:", "Received:", "Message-ID:", "Date:"]:
            rules.append(f'SCORE -8="^{header}"')

        rules.append("\n# Milde Strafe für häufige Kleinanzeigen-Antworten")
        rules.append('SCORE +15="^Subject:.*Re: Nutzer-Anfrage zu deiner Anzeige"')

        with open(EXTRA_RULES_FILE, "w", encoding="utf-8") as handle:
            handle.write("\n".join(rules) + "\n")

        print(f"? Extra-Rules erzeugt -> {EXTRA_RULES_FILE}")
    finally:
        conn.close()


def main() -> None:
    parser = argparse.ArgumentParser(description="mailfilter SQLite Analyse Tool")
    parser.add_argument("--stats", action="store_true", help="Statistik anzeigen")
    parser.add_argument(
        "--days",
        type=positive_int,
        help="Nur Mails der letzten N Tage berücksichtigen",
    )
    parser.add_argument(
        "--latest",
        type=positive_int,
        help="Nur die N neuesten Ergebnisse berücksichtigen",
    )
    parser.add_argument(
        "--time-field",
        choices=("created_at", "date_hdr"),
        default=DEFAULT_TIME_FIELD,
        help="Zeitfeld für Filter und Sortierung",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Zusätzliche Zeit-Diagnose ausgeben",
    )
    parser.add_argument(
        "--analyze-auth",
        action="store_true",
        help="Authentication-Results, ARC und DKIM-Signature auswerten",
    )
    parser.add_argument(
        "--auth-limit",
        type=positive_int,
        default=DEFAULT_AUTH_LIMIT,
        help="Maximale Anzahl Auth-Kandidaten in der Ausgabe",
    )
    parser.add_argument(
        "--generate-rules",
        action="store_true",
        help="Extra SCORE-Regeln generieren",
    )

    args = parser.parse_args()

    if args.stats or args.days is not None or args.latest is not None or args.analyze_auth:
        print_stats(
            days=args.days,
            latest=args.latest,
            time_field=args.time_field,
            verbose=args.verbose,
            analyze_auth=args.analyze_auth,
            auth_limit=args.auth_limit,
        )

    if args.generate_rules:
        generate_extra_rules()

    if not any(
        [
            args.stats,
            args.generate_rules,
            args.days is not None,
            args.latest is not None,
            args.analyze_auth,
        ]
    ):
        parser.print_help()


if __name__ == "__main__":
    main()
