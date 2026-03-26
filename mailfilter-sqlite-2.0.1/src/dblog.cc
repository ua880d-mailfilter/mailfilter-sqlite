#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <sstream>
#include <iostream>
#include "dblog.hh"
#include "preferences.hh"
#include "feedback.hh"
#include "mailfilter.hh"

#ifdef USE_SQLITE3_HEADERLOG
#include <sqlite3.h>
#endif

using namespace std;

Dblog :: Dblog ()
{
#ifdef USE_SQLITE3_HEADERLOG
  db_handle = NULL;
#endif
  is_enabled = false;
  is_ready = false;
}

Dblog :: ~Dblog ()
{
  close ();
}

Dblog& Dblog :: Instance ()
{
  static Dblog* instance = new Dblog;
  return *instance;
}

bool Dblog :: enabled (void) const
{
  return is_enabled;
}

bool Dblog :: ready (void) const
{
  return is_ready;
}

void Dblog :: init (const char* db_path)
{
#ifndef USE_SQLITE3_HEADERLOG
  (void)db_path;
  is_enabled = false;
  is_ready = false;
#else
  Feedback* logger = Feedback :: Instance ();

  if (!db_path || !*db_path)
    {
      is_enabled = false;
      is_ready = false;
      return;
    }

  is_enabled = true;

  sqlite3* db = NULL;
  if (sqlite3_open (db_path, &db) != SQLITE_OK)
    {
      logger->print_err ("Could not open sqlite3 header log database.");
      if (db)
        sqlite3_close (db);
      is_ready = false;
      return;
    }

  db_handle = db;
  is_ready = true;

  try
    {
      create_schema ();
    }
  catch (...)
    {
      logger->print_err ("Could not initialise sqlite3 header log schema.");
      close ();
    }
#endif
}

void Dblog :: close (void)
{
#ifdef USE_SQLITE3_HEADERLOG
  if (db_handle)
    {
      sqlite3_close ((sqlite3*)db_handle);
      db_handle = NULL;
    }
#endif
  is_ready = false;
}

string Dblog :: sql_quote (const string& input) const
{
  string out = "'";
  for (string::const_iterator i = input.begin (); i != input.end (); ++i)
    {
      if (*i == '\'')
        out += "''";
      else
        out += *i;
    }
  out += "'";
  return out;
}

void Dblog :: exec (const string& sql) const
{
#ifdef USE_SQLITE3_HEADERLOG
  if (!is_ready || !db_handle)
    return;

  char* err = NULL;
  if (sqlite3_exec ((sqlite3*)db_handle, sql.c_str (), NULL, NULL, &err) != SQLITE_OK)
    {
      Feedback* logger = Feedback :: Instance ();
      if (err)
        {
          logger->print_err ((string)"sqlite3 header log failed: " + err);
          sqlite3_free (err);
        }
      else
        logger->print_err ("sqlite3 header log failed.");
    }
#else
  (void)sql;
#endif
}

void Dblog :: create_schema (void)
{
  exec ("CREATE TABLE IF NOT EXISTS messages ("
        "msg_log_id TEXT PRIMARY KEY,"
        "message_id TEXT,"
        "from_addr TEXT,"
        "to_addr TEXT,"
        "subject TEXT,"
        "normal_subject TEXT,"
        "date_hdr TEXT,"
        "msg_size INTEGER,"
        "decision TEXT,"
        "final_score INTEGER DEFAULT 0,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");");

  exec ("CREATE TABLE IF NOT EXISTS header_entries ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "msg_log_id TEXT NOT NULL,"
        "ordinal INTEGER NOT NULL,"
        "tag TEXT NOT NULL,"
        "body TEXT NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");");

  exec ("CREATE TABLE IF NOT EXISTS rule_hits ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "msg_log_id TEXT NOT NULL,"
        "phase TEXT NOT NULL,"
        "expression TEXT,"
        "is_negative INTEGER NOT NULL DEFAULT 0,"
        "matched INTEGER NOT NULL DEFAULT 0,"
        "header_tag TEXT,"
        "header_body TEXT,"
        "normalized_subject INTEGER NOT NULL DEFAULT 0,"
        "score_delta INTEGER NOT NULL DEFAULT 0,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");");

  exec ("CREATE INDEX IF NOT EXISTS idx_header_entries_msg_log_id "
        "ON header_entries(msg_log_id);");

  exec ("CREATE INDEX IF NOT EXISTS idx_rule_hits_msg_log_id "
        "ON rule_hits(msg_log_id);");

  exec ("CREATE INDEX IF NOT EXISTS idx_rule_hits_phase "
        "ON rule_hits(phase);");
}

string Dblog :: ensure_msg_log_id (const Header* the_header) const
{
  if (!the_header)
    return "";

  if (the_header->log_id ()->length () > 0)
    return *(the_header->log_id ());

  return "";
}

void Dblog :: log_header_entry (const Header* the_header,
                                const char* tag,
                                const char* body,
                                int ordinal)
{
  if (!is_ready || !the_header || !tag || !body)
    return;

  string msg_log_id = ensure_msg_log_id (the_header);
  if (!msg_log_id.length ())
    return;

  // safer integer formatting
  ostringstream ord;
  ord << ordinal;

 string sql = "INSERT INTO header_entries "
        "(msg_log_id, ordinal, tag, body) VALUES ("
        + sql_quote (msg_log_id) + ", "
        + ord.str () + ", "
        + sql_quote (tag) + ", "
        + sql_quote (body) + ");";

  exec (sql);
}

void Dblog :: log_message_meta (const Header* the_header)
{
  if (!is_ready || !the_header)
    return;

  string msg_log_id = ensure_msg_log_id (the_header);
  if (!msg_log_id.length ())
    return;

  string message_id;
  string from_addr;
  string to_addr;
  string subject;
  string normal_subject;
  string date_hdr;
  int msg_size = 0;

  if (the_header->ID ())
    message_id = *(the_header->ID ());
  if (the_header->from ())
    from_addr = *(the_header->from ());
  if (the_header->to ())
    to_addr = *(the_header->to ());
  if (the_header->subject ())
    subject = *(the_header->subject ());
  if (the_header->normal_subject ())
    normal_subject = *(the_header->normal_subject ());
  if (the_header->date ())
    date_hdr = *(the_header->date ());
  msg_size = the_header->size ();

  ostringstream size_str;
  size_str << msg_size;

  string sql = "INSERT OR REPLACE INTO messages "
               "(msg_log_id, message_id, from_addr, to_addr, subject, "
               "normal_subject, date_hdr, msg_size) VALUES ("
               + sql_quote (msg_log_id) + ", "
               + sql_quote (message_id) + ", "
               + sql_quote (from_addr) + ", "
               + sql_quote (to_addr) + ", "
               + sql_quote (subject) + ", "
               + sql_quote (normal_subject) + ", "
               + sql_quote (date_hdr) + ", "
               + size_str.str () + ");";

  exec (sql);
}

void Dblog :: log_rule_hit (const Header* the_header,
                            const char* phase,
                            const char* expression,
                            bool is_negative,
                            bool matched,
                            const char* header_tag,
                            const char* header_body,
                            bool normalized_subject,
                            int score_delta)
{
  if (!is_ready || !the_header || !phase)
    return;

  string msg_log_id = ensure_msg_log_id (the_header);
  if (!msg_log_id.length ())
    return;

  ostringstream score_str;
  score_str << score_delta;

  string sql = "INSERT INTO rule_hits "
               "(msg_log_id, phase, expression, is_negative, matched, "
               "header_tag, header_body, normalized_subject, score_delta) "
               "VALUES ("
               + sql_quote (msg_log_id) + ", "
               + sql_quote (phase) + ", "
               + sql_quote (expression ? expression : "") + ", "
               + (is_negative ? "1" : "0") + ", "
               + (matched ? "1" : "0") + ", "
               + sql_quote (header_tag ? header_tag : "") + ", "
               + sql_quote (header_body ? header_body : "") + ", "
               + (normalized_subject ? "1" : "0") + ", "
               + score_str.str () + ");";

  exec (sql);
}

void Dblog :: log_decision (const Header* the_header,
                            const char* decision,
                            int final_score)
{
  if (!is_ready || !the_header || !decision)
    return;

  string msg_log_id = ensure_msg_log_id (the_header);
  if (!msg_log_id.length ())
    return;

  ostringstream score_str;
  score_str << final_score;

  string sql = "UPDATE messages SET "
               "decision = " + sql_quote (decision) + ", "
               "final_score = " + score_str.str ()
               + " WHERE msg_log_id = " + sql_quote (msg_log_id) + ";";

  exec (sql);
}
