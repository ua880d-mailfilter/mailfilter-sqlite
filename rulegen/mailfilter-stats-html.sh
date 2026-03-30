#!/bin/bash

###############################################################################
#
# mailfilter-stats-html.sh
#
# HTML-Ausgabe für Statistikdaten aus einer mailfilter-sqlite Datenbank.
#
# Beispiel:
#   ./mailfilter-stats-html.sh --db /var/spool/filter/mailheader.log.sqlite3 > stats.html
#
###############################################################################

set -euo pipefail

DB=""
LIMIT=15
TITLE="mailfilter SQLite statistics"

usage() {
  cat <<EOF
Usage: $0 --db FILE [--limit N] [--title TEXT]

Options:
  --db FILE      SQLite-Datenbank
  --limit N      Anzahl Zeilen pro Block (Default: 15)
  --title TEXT   HTML-Titel (Default: "mailfilter SQLite statistics")
EOF
  exit 1
}

normalize_cell() {
  local s="${1:-}"

  # CR/LF/Tabs in Leerzeichen umwandeln
  s="${s//$'\r'/ }"
  s="${s//$'\n'/ }"
  s="${s//$'\t'/ }"

  # Mehrfache Leerzeichen glätten
  s="$(printf '%s' "$s" | sed 's/[[:space:]][[:space:]]*/ /g')"

  # Führende / abschließende Leerzeichen entfernen
  s="${s#"${s%%[![:space:]]*}"}"
  s="${s%"${s##*[![:space:]]}"}"

  printf '%s' "$s"
}

html_escape() {
  normalize_cell "${1:-}" | sed \
    -e 's/&/\&amp;/g' \
    -e 's/</\&lt;/g' \
    -e 's/>/\&gt;/g' \
    -e 's/"/\&quot;/g' \
    -e "s/'/\&#39;/g"
}

sql_table_html() {
  local heading="$1"
  local query="$2"
  local columns="$3"

  echo "<section class=\"card\">"
  echo "  <h2>$(html_escape "$heading")</h2>"
  echo "  <table>"
  echo "    <thead><tr>"

  IFS='|' read -r -a hdrs <<< "$columns"

for i in "${!hdrs[@]}"; do
  h="${hdrs[$i]}"

  if [[ $i -eq $((${#hdrs[@]} - 1)) ]]; then
    echo "      <th class=\"num\">$(html_escape "$h")</th>"
  else
    echo "      <th>$(html_escape "$h")</th>"
  fi
done

  echo "    </tr></thead>"
  echo "    <tbody>"

  sqlite3 -separator $'\t' "$DB" "$query" | while IFS=$'\t' read -r -a row; do
    echo "      <tr>"

for i in "${!row[@]}"; do
  cell="${row[$i]}"

  if [[ $i -eq $((${#row[@]} - 1)) ]]; then
    echo "        <td class=\"num\">$(html_escape "$cell")</td>"
  else
    echo "        <td>$(html_escape "$cell")</td>"
  fi
done

    echo "      </tr>"
  done

  echo "    </tbody>"
  echo "  </table>"
  echo "</section>"
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --db)
      DB="${2:-}"
      shift 2
      ;;
    --limit)
      LIMIT="${2:-}"
      shift 2
      ;;
    --title)
      TITLE="${2:-}"
      shift 2
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage
      ;;
  esac
done

[[ -n "$DB" ]] || usage
[[ -f "$DB" ]] || { echo "Database not found: $DB" >&2; exit 1; }
command -v sqlite3 >/dev/null || { echo "sqlite3 not installed" >&2; exit 1; }

DB_ESCAPED="$(html_escape "$DB")"
NOW="$(date '+%Y-%m-%d %H:%M:%S')"

cat <<EOF
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>$(html_escape "$TITLE")</title>
<style>
  body {
    font-family: Arial, Helvetica, sans-serif;
    margin: 0;
    background: #f4f6f8;
    color: #1f2937;
  }
  .wrap {
    max-width: 1400px;
    margin: 32px auto;
    padding: 0 20px 40px;
  }
  header {
    background: white;
    border: 1px solid #d7dee7;
    border-radius: 12px;
    padding: 20px 24px;
    margin-bottom: 24px;
  }
  h1 {
    margin: 0 0 8px;
    font-size: 30px;
  }
  .meta {
    color: #475569;
    font-size: 14px;
    line-height: 1.5;
  }
  .grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(500px, 1fr));
    gap: 18px;
  }
  .card {
    background: white;
    border: 1px solid #d7dee7;
    border-radius: 12px;
    padding: 16px 18px 18px;
    box-shadow: 0 2px 8px rgba(0,0,0,0.04);
    overflow-x: auto;
  }
  .card h2 {
    margin: 0 0 12px;
    font-size: 20px;
    color: #0f3d75;
  }
  table {
    width: 100%;
    border-collapse: collapse;
    font-size: 14px;
    table-layout: auto;
  }
  th, td {
    padding: 8px 10px;
    border-bottom: 1px solid #e5e7eb;
    text-align: left;
    vertical-align: top;
    overflow-wrap: anywhere;
    word-break: break-word;
  }
  th {
    background: #eef4fb;
    color: #0f3d75;
  }
  th.num, td.num {
   white-space: nowrap;
    width: 1%;
    min-width: 70px;
    text-align: right;
  }
  tr:hover td {
    background: #fafcff;
  }
  footer {
    margin-top: 24px;
    color: #64748b;
    font-size: 13px;
    text-align: center;
  }
  code {
    background: #eef4fb;
    padding: 2px 6px;
    border-radius: 4px;
  }
</style>
</head>
<body>
<div class="wrap">
  <header>
    <h1>$(html_escape "$TITLE")</h1>
    <div class="meta">
      <strong>Database:</strong> $DB_ESCAPED<br>
      <strong>Generated:</strong> $(html_escape "$NOW")<br>
      <strong>Row limit per section:</strong> $(html_escape "$LIMIT")
    </div>
  </header>

  <div class="grid">
EOF

sql_table_html "Decisions" \
  "SELECT decision, COUNT(*) FROM messages GROUP BY decision ORDER BY COUNT(*) DESC;" \
  "Decision|Count"

sql_table_html "Top Header Tags" \
  "SELECT tag, COUNT(*) FROM header_entries GROUP BY tag ORDER BY COUNT(*) DESC LIMIT $LIMIT;" \
  "Header Tag|Count"

sql_table_html "Top From Addresses" \
  "SELECT REPLACE(REPLACE(REPLACE(body, char(9), ' '), char(10), ' '), char(13), ' '), COUNT(*) \
   FROM header_entries \
   WHERE tag='From' \
   GROUP BY body \
   ORDER BY COUNT(*) DESC \
   LIMIT $LIMIT;" \
  "From|Count"

sql_table_html "Top Reply-To Values" \
  "SELECT REPLACE(REPLACE(REPLACE(body, char(9), ' '), char(10), ' '), char(13), ' '), COUNT(*) \
   FROM header_entries \
   WHERE tag='Reply-To' \
   GROUP BY body \
   ORDER BY COUNT(*) DESC \
   LIMIT $LIMIT;" \
  "Reply-To|Count"

sql_table_html "Top Return-Path Values" \
  "SELECT REPLACE(REPLACE(REPLACE(body, char(9), ' '), char(10), ' '), char(13), ' '), COUNT(*) \
   FROM header_entries \
   WHERE tag='Return-Path' \
   GROUP BY body \
   ORDER BY COUNT(*) DESC \
   LIMIT $LIMIT;" \
  "Return-Path|Count"

sql_table_html "Top List-Unsubscribe Values" \
  "SELECT REPLACE(REPLACE(REPLACE(body, char(9), ' '), char(10), ' '), char(13), ' '), COUNT(*) \
   FROM header_entries \
   WHERE tag='List-Unsubscribe' \
   GROUP BY body \
   ORDER BY COUNT(*) DESC \
   LIMIT $LIMIT;" \
  "List-Unsubscribe|Count"

sql_table_html "Top Subjects" \
  "SELECT REPLACE(REPLACE(REPLACE(subject, char(9), ' '), char(10), ' '), char(13), ' '), COUNT(*) \
   FROM messages \
   WHERE subject IS NOT NULL AND subject != '' \
   GROUP BY subject \
   ORDER BY COUNT(*) DESC \
   LIMIT $LIMIT;" \
  "Subject|Count"

sql_table_html "Top Deny Rules" \
  "SELECT REPLACE(REPLACE(REPLACE(expression, char(9), ' '), char(10), ' '), char(13), ' '), COUNT(*) \
   FROM rule_hits \
   WHERE phase='deny' AND matched=1 \
   GROUP BY expression \
   ORDER BY COUNT(*) DESC \
   LIMIT $LIMIT;" \
  "Expression|Hits"

sql_table_html "Top Score Rules" \
  "SELECT REPLACE(REPLACE(REPLACE(expression, char(9), ' '), char(10), ' '), char(13), ' '), COUNT(*), SUM(COALESCE(score_delta,0)) \
   FROM rule_hits \
   WHERE phase='score' AND matched=1 \
   GROUP BY expression \
   ORDER BY COUNT(*) DESC, SUM(COALESCE(score_delta,0)) DESC \
   LIMIT $LIMIT;" \
  "Expression|Hits|Total Score Delta"

sql_table_html "Top Scored Messages" \
  "SELECT REPLACE(REPLACE(REPLACE(COALESCE(subject,''), char(9), ' '), char(10), ' '), char(13), ' '), final_score \
   FROM messages \
   WHERE final_score > 0 \
   ORDER BY final_score DESC, subject \
   LIMIT $LIMIT;" \
  "Subject|Final Score"

cat <<EOF
  </div>

  <footer>
    Generated by <code>mailfilter-stats-html.sh</code>
  </footer>
</div>
</body>
</html>
EOF
