#!/usr/bin/env python3
"""
mailfilter.cgi – Version 18
Erweitert um:
- Header-Info-Modal
- Rule-Modal mit dynamischer Header-Tag-Auswahl
- intelligentere Regelgenerierung
"""

import cgi
import cgitb
import sqlite3
import re
import os
import html
import json
from datetime import datetime, timedelta
from email.header import decode_header
from email.utils import parsedate_tz, mktime_tz
from collections import defaultdict

cgitb.enable()

print("Content-Type: text/html; charset=utf-8\n")

DB_PATH = "/var/spool/filter/mailheader.sqlite3"
EXTRA_RULES_FILE = "/etc/mailfilter/mailfilter-extra-rules-cgi.conf"

DAYS_OPTIONS = [1, 2, 5, 10, 20, 30, 60, 90, 180, 365]
LIMIT_OPTIONS = [5, 10, 15, 20, 30, 50]


def get_db_connection():
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
    except Exception:
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
    except Exception:
        return re.sub(r'=\?.*?\?=', '', subject).strip()

def save_rule_if_new(msg_log_id, rule_text):
    if not rule_text or not rule_text.strip():
        return False, "Keine Regel zum Speichern übergeben."

    rule_text = rule_text.strip()
    msg_log_id = (msg_log_id or "").strip()

    existing_rules = set()

    if os.path.exists(EXTRA_RULES_FILE):
        with open(EXTRA_RULES_FILE, "r", encoding="utf-8") as f:
            for line in f:
                stripped = line.strip()
                if stripped and not stripped.startswith("#"):
                    existing_rules.add(stripped)

    if rule_text in existing_rules:
        return False, "Regel existiert bereits und wurde nicht doppelt gespeichert."

    with open(EXTRA_RULES_FILE, "a", encoding="utf-8") as f:
        if os.path.getsize(EXTRA_RULES_FILE) > 0:
            f.write("\n")
        f.write(f"### msg_log_id: {msg_log_id} ###\n")
        f.write(rule_text + "\n")

    return True, f"Regel gespeichert in {EXTRA_RULES_FILE}"


print("""<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="utf-8">
    <title>mailfilter Analyse</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f9f9f9; font-size: 14px; }
        table { border-collapse: collapse; width: 100%; margin-top: 15px; }
        th, td { padding: 10px; border: 1px solid #ccc; text-align: left; vertical-align: top; }
        th { background: #e0e0e0; }
        tr:nth-child(even) { background: #f2f2f2; }
        tr.subject-row:hover { background: #e6f0ff; cursor: pointer; }
        .modal {
            display: none; position: fixed; z-index: 1000; left: 0; top: 0;
            width: 100%; height: 100%; background: rgba(0,0,0,0.7);
        }
        .modal-content {
            background: white; margin: 4% auto; padding: 25px; width: 94%;
            max-width: 900px; border-radius: 10px;
            box-shadow: 0 4px 20px rgba(0,0,0,0.3);
            max-height: 86vh; overflow-y: auto;
        }
        .rule-modal-content {
            max-width: 760px;
        }
        button {
            padding: 5px 10px; margin: 3px 3px; font-size: 13px; line-height: 1.2;
        }
        .btn-rule { background: #0066cc; color: white; border: none; border-radius: 4px; }
        .btn-header { background: #666; color: white; border: none; border-radius: 4px; }
        .btn-close { background: #999; color: white; border: none; border-radius: 4px; }
        .score-input, .tag-select-wrap, .value-preview-wrap {
            margin-top: 15px;
        }
        pre {
            background: #f4f4f4; padding: 12px; border: 1px solid #ddd;
            white-space: pre-wrap; word-break: break-word;
        }
        .small-note {
            color: #555; font-size: 12px; margin-top: 6px;
        }
        .header-table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 12px;
        }
        .header-table th, .header-table td {
            border: 1px solid #ccc;
            padding: 8px;
            vertical-align: top;
        }
        .header-table th {
            background: #ececec;
        }
        .header-body {
            white-space: pre-wrap;
            word-break: break-word;
            font-family: monospace;
            font-size: 13px;
        }
        .actions-cell {
            white-space: nowrap;
        }
        input[type="number"], select {
            width: 100%;
            padding: 8px;
            box-sizing: border-box;
        }
        .preview-box {
            background: #f7f7f7;
            border: 1px solid #ddd;
            padding: 8px 8px;
            white-space: pre-wrap;
            word-break: break-word;
            min-height: 20px;
        }
        .top-filter-form {
            margin-bottom: 12px;
        }
        .top-filter-form select {
            width: auto;
            min-width: 60px;
            padding: 4px 8px;
            margin-right: 10px;
        }
        .top-filter-form input[type="submit"] {
            width: auto;
            padding: 6px 12px;
            margin-left: 4px;
            vertical-align: middle;
        }
        #authResultsWrap select:disabled {
           background: #f1f1f1;
           color: #666;
       }
    </style>
</head>
<body>
<h1>mailfilter Analyse</h1>
""")

def get_single_form_value(form, name, default=None):
    value = form.getvalue(name, default)
    if isinstance(value, list):
        return value[-1] if value else default
    return value

form = cgi.FieldStorage()

save_rule_action = get_single_form_value(form, "save_rule", "")
save_rule_msg_id = get_single_form_value(form, "save_msg_id", "")
save_rule_text = get_single_form_value(form, "save_rule_text", "")
save_status_message = ""
save_status_ok = False

if save_rule_action == "1":
    save_status_ok, save_status_message = save_rule_if_new(save_rule_msg_id, save_rule_text)

try:
    days = int(get_single_form_value(form, "days", 90))
except (TypeError, ValueError):
    days = 90

if days not in DAYS_OPTIONS:
    days = 90

try:
    limit = int(get_single_form_value(form, "limit", 30))
except (TypeError, ValueError):
    limit = 30

if limit not in LIMIT_OPTIONS:
    limit = 30

now = datetime.now()
cutoff_date = now - timedelta(days=days)
print(f"<p><strong>Stand:</strong> {now.strftime('%Y-%m-%d %H:%M:%S')}</p>")
print(f"<p><strong>Gefilterter Zeitraum:</strong> ab {cutoff_date.strftime('%Y-%m-%d')} (letzte {days} Tage)</p>")

print('<form method="get" class="top-filter-form">')
print('Letzte <select name="days">')
for d in DAYS_OPTIONS:
    selected = ' selected' if d == days else ''
    print(f'<option value="{d}"{selected}>{d} Tage</option>')
print('</select>')

print('&nbsp;&nbsp;Max. Zeilen <select name="limit">')
for l in LIMIT_OPTIONS:
    selected = ' selected' if l == limit else ''
    print(f'<option value="{l}"{selected}>{l}</option>')
print('</select>')

print(' <input type="submit" value="Aktualisieren">')
print('</form><hr>')

if save_status_message:
    color = "#155724" if save_status_ok else "#856404"
    bg = "#d4edda" if save_status_ok else "#fff3cd"
    border = "#c3e6cb" if save_status_ok else "#ffeeba"
    print(f'''
    <div style="margin:10px 0; padding:10px 12px; border:1px solid {border}; background:{bg}; color:{color}; border-radius:4px;">
        {html.escape(save_status_message)}
    </div>
    ''')

conn = get_db_connection()
cur = conn.cursor()
cutoff_ts = int(cutoff_date.timestamp())

cur.execute("SELECT msg_log_id, final_score, decision, subject, date_hdr, from_addr FROM messages")

filtered_rows = []
for row in cur.fetchall():
    ts = parse_date_to_ts(row['date_hdr'])
    if ts is not None and ts >= cutoff_ts:
        filtered_rows.append(row)

print("<h2>Decision-Verteilung</h2>")
decision_count = defaultdict(int)
for row in filtered_rows:
    decision_count[row['decision'] or 'unknown'] += 1
for d in sorted(decision_count, key=decision_count.get, reverse=True):
    print(f"<strong>{html.escape(str(d))}:</strong> {decision_count[d]} Mails<br>")

print(f"<h2>Top Subjects (letzte {days} Tage, max. {limit} Zeilen)</h2>")
print('<table><tr><th>Anzahl</th><th>Ø Score</th><th>Subject</th><th>msg_log_id</th><th>Aktionen</th></tr>')

subject_data = defaultdict(lambda: {
    'count': 0,
    'score_sum': 0.0,
    'example_id': None,
    'from_addr': '',
    'subject_clean': ''
})

for row in filtered_rows:
    subj = row['subject'] or ""
    data = subject_data[subj]
    data['count'] += 1
    data['score_sum'] += row['final_score'] or 0
    if data['example_id'] is None:
        data['example_id'] = row['msg_log_id']
        data['from_addr'] = row['from_addr'] or ''
        data['subject_clean'] = decode_subject(subj)

top_subjects = sorted(subject_data.items(), key=lambda x: x[1]['count'], reverse=True)[:limit]
visible_msg_ids = []
messages_meta = {}

for subj, data in top_subjects:
    avg = data['score_sum'] / data['count'] if data['count'] > 0 else 0
    clean = data['subject_clean'] or decode_subject(subj)
    msg_id = data['example_id'] or '—'
    from_addr = data['from_addr'] or ''

    visible_msg_ids.append(msg_id)
    messages_meta[msg_id] = {
        "msg_id": msg_id,
        "subject": clean,
        "from_addr": from_addr
    }

    safe_msg_id = html.escape(str(msg_id), quote=True)
    safe_subject = html.escape(clean, quote=True)
    safe_from = html.escape(from_addr, quote=True)

    print(f'<tr class="subject-row" data-msg-id="{safe_msg_id}" data-subject="{safe_subject}" data-from="{safe_from}">')
    print(f'  <td>{data["count"]}</td>')
    print(f'  <td>{avg:.1f}</td>')
    print(f'  <td>{html.escape(clean[:80])}{"..." if len(clean) > 80 else ""}</td>')
    print(f'  <td>{html.escape(str(msg_id))}</td>')
    print('  <td class="actions-cell">')
    print(f"    <button type=\"button\" class=\"btn-rule\" onclick='openRuleFromButton(event, {json.dumps(str(msg_id))})'>Rule</button>")
    print(f"    <button type=\"button\" class=\"btn-header\" onclick='openHeadersFromButton(event, {json.dumps(str(msg_id))})'>Header</button>")
    print('  </td>')
    print('</tr>')

print('</table>')

headers_by_msg = defaultdict(list)

if visible_msg_ids:
    placeholders = ",".join(["?"] * len(visible_msg_ids))
    cur.execute(
        f"""
        SELECT msg_log_id, ordinal, tag, body
        FROM header_entries
        WHERE msg_log_id IN ({placeholders})
        ORDER BY msg_log_id, ordinal
        """,
        visible_msg_ids
    )

    for row in cur.fetchall():
        headers_by_msg[row["msg_log_id"]].append({
            "ordinal": row["ordinal"],
            "tag": row["tag"],
            "body": row["body"]
        })

print("""
<div id="ruleModal" class="modal">
  <div class="modal-content rule-modal-content">
    <h3>Regel für diese Mail erstellen</h3>
    <p><strong>Subject:</strong> <span id="modalSubject"></span></p>
    <p><strong>msg_log_id:</strong> <span id="modalMsgId"></span></p>
    <p><strong>From:</strong> <span id="modalFrom"></span></p>

    <label><strong>Aktion wählen:</strong></label><br>
    <select id="actionSelect" onchange="toggleScoreField(); updateRulePreview();" style="margin:10px 0;">
      <option value="DENY">DENY – hart blocken</option>
      <option value="SCORE">SCORE – Strafpunkte vergeben</option>
      <option value="PASS">PASS – immer durchlassen</option>
    </select>

    <div id="scoreField" class="score-input">
      <label><strong>Straf-Score (positiv = schlechter):</strong></label><br>
      <input type="number" id="scoreValue" value="50" step="5" onchange="updateRulePreview()" oninput="updateRulePreview()">
    </div>

    <div class="tag-select-wrap">
      <label><strong>Header-Feld für Regel:</strong></label><br>
      <select id="headerTagSelect" onchange="handleHeaderTagChange()"></select>
      <div class="small-note">Es werden nur Header-Tags angezeigt, die für diese Mail tatsächlich vorhanden sind.</div>
    </div>

    <div id="authResultsWrap" class="tag-select-wrap" style="display:none;">
      <label><strong>Authentication-Results Merkmal:</strong></label><br>
      <select id="authResultsSelect" onchange="updateRulePreview()" disabled></select>
      <div class="small-note">Erkannte Teilmerkmale aus Authentication-Results, z. B. dkim=pass oder header.d=example.com.</div>
    </div>

    <div class="value-preview-wrap">
      <label><strong>Verwendeter Wert / Vorschlag:</strong></label>
      <div id="ruleValuePreview" class="preview-box"></div>
    </div>

    <div id="subjectVariantsWrap" class="value-preview-wrap" style="display:none;">
      <label><strong>Subject-Vorschläge:</strong></label>

      <div style="margin-bottom:10px;">
        <div><strong>Konservativ</strong></div>
        <div id="subjectVariantConservative" class="preview-box"></div>
        <button type="button" onclick="useSubjectVariant('conservative')">Diesen Vorschlag verwenden</button>
      </div>

      <div>
        <div><strong>Gruppiert / flexibel</strong></div>
        <div id="subjectVariantFlexible" class="preview-box"></div>
        <button type="button" onclick="useSubjectVariant('flexible')">Diesen Vorschlag verwenden</button>
      </div>
    </div>

    <br>
    <button type="button" class="btn-rule" onclick="generateRule()">Regel generieren</button>
    <button type="button" class="btn-close" onclick="closeRuleModal()">Abbrechen</button>

    <pre id="generatedRule" style="margin-top:20px; display:none;"></pre>
    <button id="copyBtn" type="button" onclick="copyRule()" style="display:none;">Regel kopieren</button>
    <button id="saveBtn" type="button" onclick="saveRule()" style="display:none;">Regel speichern</button>
  </div>
</div>
""")

print("""
<div id="headerModal" class="modal">
  <div class="modal-content">
    <h3>Header-Details</h3>
    <p><strong>msg_log_id:</strong> <span id="headerModalMsgId"></span></p>
    <p><strong>Subject:</strong> <span id="headerModalSubject"></span></p>
    <table class="header-table">
      <thead>
        <tr>
          <th style="width:70px;">Nr</th>
          <th style="width:180px;">Tag</th>
          <th>Wert</th>
        </tr>
      </thead>
      <tbody id="headerTableBody"></tbody>
    </table>
    <br>
    <button type="button" class="btn-close" onclick="closeHeaderModal()">Schließen</button>
  </div>
</div>
""")

print("""
    <form id="saveRuleForm" method="post" style="display:none;">
      <input type="hidden" name="days" id="saveDaysField">
      <input type="hidden" name="limit" id="saveLimitField">
      <input type="hidden" name="save_rule" value="1">
      <input type="hidden" name="save_msg_id" id="saveMsgIdField">
      <input type="hidden" name="save_rule_text" id="saveRuleTextField">
    </form>
""")

messages_meta_json = json.dumps(messages_meta, ensure_ascii=False)
headers_by_msg_json = json.dumps(headers_by_msg, ensure_ascii=False)

print("<script>")
print("const messagesMeta = " + messages_meta_json + ";")
print("const headersByMsg = " + headers_by_msg_json + ";")

print(r"""
let currentMsgId = null;

document.querySelectorAll('tr.subject-row[data-msg-id]').forEach(row => {
    row.querySelectorAll('td:not(.actions-cell)').forEach(cell => {
        cell.addEventListener('click', function() {
            showRuleModal(row.getAttribute('data-msg-id'));
        });
    });
});

function openRuleFromButton(event, msgId) {
    event.stopPropagation();
    showRuleModal(msgId);
}

function openHeadersFromButton(event, msgId) {
    event.stopPropagation();
    showHeaderModal(msgId);
}

function toggleScoreField() {
    var action = document.getElementById("actionSelect").value;
    document.getElementById("scoreField").style.display = (action === "SCORE") ? "block" : "none";
}

function escapeRegex(text) {
    return String(text).replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
}

function extractEmail(text) {
    if (!text) return "";
    const m = String(text).match(/<([^>]+)>/);
    if (m && m[1]) return m[1].trim();
    const m2 = String(text).match(/[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,}/i);
    return m2 ? m2[0].trim() : "";
}

function stripAngles(text) {
    if (!text) return "";
    return String(text).replace(/^\s*<+/, '').replace(/>+\s*$/, '').trim();
}

function getHeadersForMsg(msgId) {
    return headersByMsg[msgId] || [];
}

function getUniqueTags(headers) {
    const seen = new Set();
    const tags = [];
    headers.forEach(h => {
        if (!seen.has(h.tag)) {
            seen.add(h.tag);
            tags.push(h.tag);
        }
    });
    return tags;
}

function chooseDefaultTag(tags) {
    const preferred = ["From", "Return-path", "Reply-To", "Authentication-Results", "List-Unsubscribe", "Subject", "X-Mailer"];
    for (const p of preferred) {
        if (tags.includes(p)) return p;
    }
    return tags.length ? tags[0] : "Subject";
}

function getHeaderBodies(msgId, tag) {
    return getHeadersForMsg(msgId)
        .filter(h => h.tag === tag)
        .map(h => h.body || "");
}

function normalizeAuthResultValue(value) {
    return String(value || '').trim();
}

function parseAuthenticationResults(msgId) {
    const headers = getHeaderBodies(msgId, "Authentication-Results");
    const features = [];
    const seen = new Set();

    function addFeature(key, label, regexParts) {
        const normalizedKey = String(key || '').trim();
        if (!normalizedKey || seen.has(normalizedKey)) return;
        seen.add(normalizedKey);
        features.push({
            key: normalizedKey,
            label: label,
            regexParts: regexParts || [normalizedKey]
        });
    }

    headers.forEach(h => {
        const text = String(h || '');

        const resultMatches = text.match(/\b(?:dkim|spf|dmarc|iprev|arc)=\w+\b/g) || [];
        resultMatches.forEach(m => {
            addFeature(m, m, [m]);
        });

        const headerDMatches = text.match(/\bheader\.d=([A-Za-z0-9.-]+\.[A-Za-z]{2,})\b/g) || [];
        headerDMatches.forEach(m => {
            addFeature(m, m, [m]);
        });

        const strongCombos = [];

        const dkimPass = /\bdkim=pass\b/.test(text);
        const spfPass = /\bspf=pass\b/.test(text);
        const dmarcPass = /\bdmarc=pass\b/.test(text);
        const iprevPass = /\biprev=pass\b/.test(text);

        const hd = text.match(/\bheader\.d=([A-Za-z0-9.-]+\.[A-Za-z]{2,})\b/);
        const headerDomain = hd ? hd[1] : "";

        if (dkimPass && headerDomain) {
            strongCombos.push({
                key: 'dkim=pass + header.d=' + headerDomain,
                label: 'dkim=pass + header.d=' + headerDomain,
                regexParts: ['dkim=pass', 'header.d=' + headerDomain]
            });
        }

        if (dkimPass && spfPass) {
            strongCombos.push({
                key: 'dkim=pass + spf=pass',
                label: 'dkim=pass + spf=pass',
                regexParts: ['dkim=pass', 'spf=pass']
            });
        }

        if (dkimPass && dmarcPass) {
            strongCombos.push({
                key: 'dkim=pass + dmarc=pass',
                label: 'dkim=pass + dmarc=pass',
                regexParts: ['dkim=pass', 'dmarc=pass']
            });
        }

        if (spfPass && dmarcPass) {
            strongCombos.push({
                key: 'spf=pass + dmarc=pass',
                label: 'spf=pass + dmarc=pass',
                regexParts: ['spf=pass', 'dmarc=pass']
            });
        }

        if (iprevPass && dkimPass) {
            strongCombos.push({
                key: 'iprev=pass + dkim=pass',
                label: 'iprev=pass + dkim=pass',
                regexParts: ['iprev=pass', 'dkim=pass']
            });
        }

        strongCombos.forEach(c => addFeature(c.key, c.label, c.regexParts));
    });

    return features;
}

function buildAuthResultsRegexFromFeature(feature) {
    if (!feature || !feature.regexParts || !feature.regexParts.length) {
        return '^Authentication-Results:';
    }

    const parts = feature.regexParts.map(p => escapeRegex(p));
    return '^Authentication-Results:.*' + parts.join('.*');
}

function buildSubjectTokens(subject) {
    if (!subject) return [];

    let s = String(subject);

    /* MIME-Reste grob entfernen */
    s = s.replace(/=\?[^?]+\?[BQbq]\?[^?]+\?=/g, ' ');

    /*
     * Domain-artige Muster zuerst schützen:
     * example.com, mail.example.de, foo.bar.net usw.
     * Punkte werden intern maskiert, damit sie nicht als Trenner verloren gehen.
     */
    s = s.replace(/\b([A-Za-z0-9-]+(?:\.[A-Za-z0-9-]+)+)\b/g, function(match) {
        return match.replace(/\./g, '__DOT__');
    });

    /*
     * Zahlen mit Dezimalpunkt / Tausenderpunkt / Plus / Prozent markieren,
     * damit sie später gezielt verworfen werden können.
     */
    s = s.replace(/\b[0-9]+(?:[.,][0-9]+)+(?:[%+])?\b/g, ' __NUM__ ');
    s = s.replace(/\b[0-9]{2,}%\b/g, ' __NUM__ ');
    s = s.replace(/\$[0-9]+(?:[.,][0-9]{2})?/g, ' __NUM__ ');
    s = s.replace(/[€$£¥]/g, ' ');

    /*
     * Punkte als normale Trenner behandeln,
     * nachdem Domains bereits geschützt wurden.
     */
    s = s.replace(/\./g, ' ');

    /*
     * Restliche Satzzeichen glätten, Bindestrich aber erhalten
     * wegen Modellnamen wie HS-PX410 oder I7-12700H.
     */
    s = s.replace(/[\[\](){}!,?:;]+/g, ' ');
    s = s.replace(/[|/\\]+/g, ' ');
    s = s.replace(/\s+/g, ' ').trim();

    const rawWords = s.split(' ').filter(Boolean);

    const stopwords = new Set([
        'the','and','for','your','this','with','from','that','have','are','you',
        'our','now','new','all','more','just','out','get','big','best',
        'und','der','die','das','mit','für','ein','eine','oder','jetzt',
        'nur','noch','heute','dein','ihre','ihren','unser','euer',
        'hello','hallo','hi','dear','status'
    ]);

    const promoSet = new Set([
        'sale', 'clearance', 'deal', 'offer', 'special', 'weekend',
        'promo', 'discount', 'aktion', 'angebot', 'rabatt', 'gutschein',
        'save', 'amazing'
    ]);

    const tokens = [];
    const seen = new Set();

    rawWords.forEach((w, idx) => {
        let cleaned = w.trim();
        if (!cleaned) return;

        if (cleaned === '__NUM__') return;

        cleaned = cleaned.replace(/__DOT__/g, '.');

        const lower = cleaned.toLowerCase();

        if (stopwords.has(lower)) return;
        if (cleaned.length < 3) return;

        /*
         * Reine Zahlen oder weitgehend numerische Tokens raus.
         */
        if (/^[0-9]+$/.test(cleaned)) return;
        if (/^[0-9]+[.,][0-9]+$/.test(cleaned)) return;
        if (/^[0-9]+(?:[.,][0-9]+)*\+?$/.test(cleaned)) return;
        if (/^[0-9]{2,}%$/.test(cleaned)) return;
        if (/^[0-9].*[%+]$/.test(cleaned)) return;

        const hasAlpha = /[A-Za-zÄÖÜäöüß]/.test(cleaned);
        const hasDigit = /[0-9]/.test(cleaned);
        const hasDot = /\./.test(cleaned);
        const hasHyphen = /-/.test(cleaned);

        const isModelLike = hasAlpha && hasDigit;
        const isDomainLike = hasDot && /[A-Za-z]/.test(cleaned);

        if (!hasAlpha && !isModelLike && !isDomainLike) return;

        if (seen.has(lower)) return;
        seen.add(lower);

        let weight = 1;

        if (promoSet.has(lower)) weight = 2;
        if (cleaned.length >= 10) weight = 4;
        else if (cleaned.length >= 7) weight = 3;
        else if (cleaned.length >= 5) weight = 2;

        if (isModelLike) weight = Math.max(weight, 5);
        if (isDomainLike) weight = Math.max(weight, 4);
        if (hasHyphen && hasAlpha) weight = Math.max(weight, 4);

        tokens.push({
            text: cleaned,
            lower: lower,
            pos: idx,
            weight: weight,
            kind: promoSet.has(lower) ? 'promo' : (isModelLike ? 'model' : (isDomainLike ? 'domain' : 'word'))
        });
    });

    return tokens;
}

function buildSubjectRegexConservative(subject) {
    const tokens = buildSubjectTokens(subject);

    if (!tokens.length) {
        return '^Subject:.*' + escapeRegex(subject || '');
    }

    /*
     * Drei stärkste Tokens wählen:
     * - hohe Gewichtung zuerst
     * - bei gleicher Gewichtung längeres Wort bevorzugen
     * - danach wieder in Originalreihenfolge bringen
     */
    const selected = tokens
        .slice()
        .sort((a, b) => {
            if (b.weight !== a.weight) return b.weight - a.weight;
            if (b.text.length !== a.text.length) return b.text.length - a.text.length;
            return a.pos - b.pos;
        })
        .slice(0, 3)
        .sort((a, b) => a.pos - b.pos)
        .map(t => t.text);

    return '^Subject:.*' + selected.map(t => escapeRegex(t)).join('.*');
}

function buildOrderedSubjectGroups(subject) {
    const tokens = buildSubjectTokens(subject);

    if (!tokens.length) return [];

    const promoSet = new Set([
        'sale', 'clearance', 'deal', 'offer', 'special', 'weekend',
        'promo', 'discount', 'aktion', 'angebot', 'rabatt', 'gutschein',
        'save', 'amazing', 'letzte', 'chance'
    ]);

    /*
     * Relevante Tokens vorfiltern:
     * - stärkere Tokens bevorzugen
     * - maximal 6 Tokens insgesamt für flexible Regeln
     */
    let selected = tokens.filter(t => t.weight >= 2);

    if (selected.length < 3) {
        selected = tokens.slice(0, 5);
    } else {
        selected = selected.slice(0, 6);
    }

	/*
	 * Sehr kurze flexible Subjects:
	 * Bei bis zu 3 ausgewählten Tokens alles in eine gemeinsame Gruppe legen,
	 * damit keine falsche Reihenfolge wie "Chance.*(Doppelte|gewinnen)" entsteht.
	 */
	if (selected.length > 0 && selected.length <= 3) {
		return [selected.map(t => t.text)];
	}

    selected.sort((a, b) => a.pos - b.pos);

    const promoTokens = [];
    const strongTokens = [];

    selected.forEach(t => {
        if (promoSet.has(t.lower)) {
            promoTokens.push(t);
        } else {
            strongTokens.push(t);
        }
    });

    const groups = [];

    /*
     * Gruppe 1: Promo/Spam-Wörter bündeln, aber max. 3
     */
    if (promoTokens.length) {
        groups.push(promoTokens.slice(0, 3).map(t => t.text));
    }

    /*
     * Restliche starke Tokens in Originalreihenfolge,
     * auf 1–2 Gruppen mit max. 3 Tokens aufteilen.
     */
    if (strongTokens.length) {
        const firstGroup = strongTokens.slice(0, 3).map(t => t.text);
        if (firstGroup.length) {
            groups.push(firstGroup);
        }

        const remaining = strongTokens.slice(3, 6).map(t => t.text);
        if (remaining.length) {
            groups.push(remaining);
        }
    }

    /*
     * Falls keine Promo-Gruppe existiert und nur eine lange Gruppe da ist,
     * teile sie in zwei kleinere Gruppen.
     */
    if (groups.length === 1 && groups[0].length > 3) {
        const g = groups[0];
        return [
            g.slice(0, 2),
            g.slice(2, 4)
        ].filter(x => x.length);
    }

    /*
     * Maximal 3 Gruppen behalten
     */
    return groups.slice(0, 3);
}

function buildSubjectRegexFlexible(subject) {
    const groups = buildOrderedSubjectGroups(subject);
    const tokens = buildSubjectTokens(subject);

    if (!groups.length) {
        if (!tokens.length) {
            return '^Subject:.*' + escapeRegex(subject || '');
        }
        const fallback = tokens.slice(0, 3).map(t => t.text);
        return '^Subject:.*' + fallback.map(t => escapeRegex(t)).join('.*');
    }

    const groupRegex = groups.map(group => {
        if (group.length === 1) {
            return escapeRegex(group[0]);
        }
        return '(' + group.map(t => escapeRegex(t)).join('|') + ')';
    });

    return '^Subject:.*' + groupRegex.join('.*');
}

function buildSubjectDisplay(subject) {
    const tokens = buildSubjectTokens(subject);

    if (!tokens.length) {
        return subject || '';
    }

    if (tokens.length <= 6) {
        return tokens.slice(0, 4).map(t => t.text).join(' .* ');
    }

    if (tokens.length <= 9) {
        const selected = tokens
            .filter(t => t.weight >= 2)
            .slice(0, 5)
            .map(t => t.text);

        const fallback = selected.length ? selected : tokens.slice(0, 5).map(t => t.text);
        return fallback.join(' .* ');
    }

    const groups = buildOrderedSubjectGroups(subject);

    if (!groups.length) {
        return tokens.slice(0, 5).map(t => t.text).join(' .* ');
    }

    return groups.map(group => {
        if (group.length === 1) {
            return group[0];
        }
        return '(' + group.join(' | ') + ')';
    }).join(' .* ');
}

function buildSubjectDisplayConservative(subject) {
    const tokens = buildSubjectTokens(subject);

    if (!tokens.length) {
        return subject || '';
    }

    const selected = tokens
        .slice()
        .sort((a, b) => {
            if (b.weight !== a.weight) return b.weight - a.weight;
            if (b.text.length !== a.text.length) return b.text.length - a.text.length;
            return a.pos - b.pos;
        })
        .slice(0, 3)
        .sort((a, b) => a.pos - b.pos)
        .map(t => t.text);

    return selected.join(' .* ');
}

function buildSubjectDisplayFlexible(subject) {
    const groups = buildOrderedSubjectGroups(subject);
    const tokens = buildSubjectTokens(subject);

    if (!groups.length) {
        if (!tokens.length) return subject || '';
        return tokens.slice(0, 3).map(t => t.text).join(' .* ');
    }

    return groups.map(group => {
        if (group.length === 1) {
            return group[0];
        }
        return '(' + group.join(' | ') + ')';
    }).join(' .* ');
}

function buildSuggestedValue(msgId, tag) {
    const meta = messagesMeta[msgId] || {};
    const headers = getHeaderBodies(msgId, tag);

    if (tag === "From" || tag === "Return-path" || tag === "Reply-To") {
        for (const h of headers) {
            const email = extractEmail(h) || stripAngles(h);
            if (email && /@/.test(email)) {
                return {
                    display: email,
                    regex: '^' + escapeRegex(tag) + ':.*' + escapeRegex(email),
                    source: tag
                };
            }
        }

        return {
            display: 'Keine brauchbare Mailadresse in ' + tag + ' gefunden',
            regex: '^' + escapeRegex(tag) + ':',
            source: tag
        };
    }

	if (tag === "Authentication-Results") {
		const features = parseAuthenticationResults(msgId);
		const select = document.getElementById("authResultsSelect");
		let feature = null;

		if (features.length) {
			const idx = parseInt(select.value || "0", 10);
			feature = features[idx] || features[0];
		}

		if (feature) {
			return {
				display: feature.label,
				regex: buildAuthResultsRegexFromFeature(feature),
				source: tag
			};
		}

		return {
			display: 'Keine brauchbaren Authentication-Results-Merkmale gefunden',
			regex: '^Authentication-Results:',
			source: tag
		};
	}

    if (tag === "List-Unsubscribe") {
        for (const h of headers) {
            const email = extractEmail(h);
            if (email) {
                return {
                    display: email,
                    regex: '^List-Unsubscribe:.*' + escapeRegex(email),
                    source: tag
                };
            }
        }
        if (headers.length) {
            const shortened = headers[0].substring(0, 80);
            return {
                display: shortened,
                regex: '^List-Unsubscribe:.*' + escapeRegex(shortened),
                source: tag
            };
        }

        return {
            display: 'Kein brauchbarer Wert in List-Unsubscribe gefunden',
            regex: '^List-Unsubscribe:',
            source: tag
        };
    }

		if (tag === "Subject") {
			const subjectText = meta.subject || "";
			return {
				display: buildSubjectDisplayFlexible(subjectText),
				regex: buildSubjectRegexFlexible(subjectText),
				source: "Subject",
				variants: {
					conservative: {
						display: buildSubjectDisplayConservative(subjectText),
						regex: buildSubjectRegexConservative(subjectText)
					},
					flexible: {
						display: buildSubjectDisplayFlexible(subjectText),
						regex: buildSubjectRegexFlexible(subjectText)
					}
				}
			};
		}

    if (tag === "X-Mailer") {
        if (headers.length) {
            return {
                display: headers[0],
                regex: '^X-Mailer:.*' + escapeRegex(headers[0]),
                source: tag
            };
        }

        return {
            display: 'Kein Wert in X-Mailer gefunden',
            regex: '^X-Mailer:',
            source: tag
        };
    }

    if (headers.length) {
        const val = headers[0].trim();
        return {
            display: val,
            regex: '^' + escapeRegex(tag) + ':.*' + escapeRegex(val),
            source: tag
        };
    }

    return {
        display: 'Kein sinnvoller Wert gefunden',
        regex: '^' + escapeRegex(tag || 'Subject') + ':',
        source: tag || 'Subject'
    };
}

function populateHeaderTagSelect(msgId) {
    const select = document.getElementById("headerTagSelect");
    const headers = getHeadersForMsg(msgId);
    const tags = getUniqueTags(headers);

    select.innerHTML = "";

    if (!tags.length) {
        const opt = document.createElement("option");
        opt.value = "Subject";
        opt.textContent = "Subject";
        select.appendChild(opt);
        return;
    }

    tags.forEach(tag => {
        const opt = document.createElement("option");
        opt.value = tag;
        opt.textContent = tag;
        select.appendChild(opt);
    });

    select.value = chooseDefaultTag(tags);

    if (select.value === "Authentication-Results") {
        document.getElementById("actionSelect").value = "SCORE";
    }
}

function populateAuthResultsSelect(msgId) {
    const wrap = document.getElementById("authResultsWrap");
    const select = document.getElementById("authResultsSelect");
    const currentTag = document.getElementById("headerTagSelect").value;

    select.innerHTML = "";
    select.disabled = true;
    wrap.style.display = "none";

    if (currentTag !== "Authentication-Results") {
        return;
    }

    wrap.style.display = "block";

    const features = parseAuthenticationResults(msgId);

    if (!features.length) {
        const opt = document.createElement("option");
        opt.value = "";
        opt.textContent = "Keine Teilmerkmale erkannt";
        select.appendChild(opt);
        select.disabled = true;
        return;
    }

    features.forEach((feature, idx) => {
        const opt = document.createElement("option");
        opt.value = String(idx);
        opt.textContent = feature.label;
        select.appendChild(opt);
    });

    select.disabled = false;
}

function handleHeaderTagChange() {
    if (currentMsgId) {
        populateAuthResultsSelect(currentMsgId);
    }

    const selectedTag = document.getElementById("headerTagSelect").value;
    if (selectedTag === "Authentication-Results") {
        document.getElementById("actionSelect").value = "SCORE";
    }

    toggleScoreField();
    updateRulePreview();
}

function useSubjectVariant(variantName) {
    currentSubjectVariant = variantName;
    updateRulePreview();
}

let currentSubjectVariant = 'flexible';

function updateRulePreview() {
    if (!currentMsgId) return;

    const tag = document.getElementById("headerTagSelect").value;

    const suggestion = buildSuggestedValue(currentMsgId, tag);

    const preview = document.getElementById("ruleValuePreview");
    const variantsWrap = document.getElementById("subjectVariantsWrap");
    const conservativeBox = document.getElementById("subjectVariantConservative");
    const flexibleBox = document.getElementById("subjectVariantFlexible");

    if (tag === "Subject" && suggestion.variants) {
        variantsWrap.style.display = "block";

        conservativeBox.textContent = suggestion.variants.conservative.display || "—";
        flexibleBox.textContent = suggestion.variants.flexible.display || "—";

        if (currentSubjectVariant === 'conservative') {
            preview.textContent = suggestion.variants.conservative.display || "Kein sinnvoller Wert gefunden";
        } else {
            preview.textContent = suggestion.variants.flexible.display || "Kein sinnvoller Wert gefunden";
        }
    } else {
        variantsWrap.style.display = "none";
        preview.textContent = suggestion.display || "Kein sinnvoller Wert gefunden";
    }
}

function showRuleModal(msgId) {
    currentMsgId = msgId;
	currentSubjectVariant = 'flexible';
    const meta = messagesMeta[msgId] || {};

    document.getElementById("modalMsgId").textContent = msgId;
    document.getElementById("modalSubject").textContent = meta.subject || "—";
    document.getElementById("modalFrom").textContent = meta.from_addr || "—";

    populateHeaderTagSelect(msgId);
    handleHeaderTagChange();

    document.getElementById("generatedRule").style.display = "none";
    document.getElementById("copyBtn").style.display = "none";
    document.getElementById("generatedRule").textContent = "";
    document.getElementById("saveBtn").style.display = "none";
    document.getElementById("ruleModal").style.display = "block";
}

function generateRule() {
    if (!currentMsgId) return;

    const action = document.getElementById("actionSelect").value;
    const score = document.getElementById("scoreValue").value;
    const tag = document.getElementById("headerTagSelect").value;
    let suggestion = buildSuggestedValue(currentMsgId, tag);

    if (tag === "Subject" && suggestion.variants) {
        suggestion = suggestion.variants[currentSubjectVariant] || suggestion.variants.flexible;
    }

    let rule = "";

    if (action === "DENY") {
        rule = 'DENY="' + suggestion.regex + '"';
    } else if (action === "SCORE") {
        const numericScore = parseInt(score, 10) || 0;
        const formattedScore = numericScore >= 0 ? ('+' + numericScore) : String(numericScore);
        rule = 'SCORE ' + formattedScore + '="' + suggestion.regex + '"';
    } else if (action === "PASS") {
        rule = 'ALLOW="' + suggestion.regex + '"';
    }

    document.getElementById("generatedRule").textContent = rule;
    document.getElementById("generatedRule").style.display = "block";
    document.getElementById("copyBtn").style.display = "inline";
    document.getElementById("saveBtn").style.display = "inline";
}

function copyRule() {
    navigator.clipboard.writeText(document.getElementById("generatedRule").textContent)
        .then(() => alert("Regel kopiert!"));
}

function saveRule() {
    const ruleText = document.getElementById("generatedRule").textContent || "";
    const msgId = document.getElementById("modalMsgId").textContent || "";

    if (!ruleText.trim()) {
        alert("Es ist keine Regel zum Speichern vorhanden.");
        return;
    }

    const params = new URLSearchParams(window.location.search);
    const days = params.get("days") || "90";
    const limit = params.get("limit") || "30";

    document.getElementById("saveDaysField").value = days;
    document.getElementById("saveLimitField").value = limit;
    document.getElementById("saveMsgIdField").value = msgId;
    document.getElementById("saveRuleTextField").value = ruleText;

    document.getElementById("saveRuleForm").submit();
}

function closeRuleModal() {
    document.getElementById("ruleModal").style.display = "none";
}

function showHeaderModal(msgId) {
    const meta = messagesMeta[msgId] || {};
    const headers = getHeadersForMsg(msgId);
    const tbody = document.getElementById("headerTableBody");

    document.getElementById("headerModalMsgId").textContent = msgId;
    document.getElementById("headerModalSubject").textContent = meta.subject || "—";

    tbody.innerHTML = "";

    if (!headers.length) {
        const tr = document.createElement("tr");
        tr.innerHTML = '<td colspan="3">Keine Header-Einträge gefunden.</td>';
        tbody.appendChild(tr);
    } else {
        headers.forEach(h => {
            const tr = document.createElement("tr");

            const tdOrdinal = document.createElement("td");
            tdOrdinal.textContent = h.ordinal;

            const tdTag = document.createElement("td");
            tdTag.textContent = h.tag;

            const tdBody = document.createElement("td");
            tdBody.className = "header-body";
            tdBody.textContent = h.body || "";

            tr.appendChild(tdOrdinal);
            tr.appendChild(tdTag);
            tr.appendChild(tdBody);
            tbody.appendChild(tr);
        });
    }

    document.getElementById("headerModal").style.display = "block";
}

function closeHeaderModal() {
    document.getElementById("headerModal").style.display = "none";
}

window.onclick = function(event) {
    const ruleModal = document.getElementById("ruleModal");
    const headerModal = document.getElementById("headerModal");

    if (event.target === ruleModal) {
        closeRuleModal();
    }
    if (event.target === headerModal) {
        closeHeaderModal();
    }
};
""")

print("</script>")
print("</body>")
print("</html>")

conn.close()

