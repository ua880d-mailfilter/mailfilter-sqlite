#ifndef DBLOG_HH
#define DBLOG_HH

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include "header.hh"

class Dblog
{
public:
  static Dblog& Instance ();

  void init              (const char* db_path);
  void close             (void);

  bool enabled           (void) const;
  bool ready             (void) const;

  void log_header_entry  (const Header*,
                          const char*,
                          const char*,
                          int);

  void log_message_meta  (const Header*);

  void log_rule_hit      (const Header*,
                          const char*,
                          const char*,
                          bool,
                          bool,
                          const char*,
                          const char*,
                          bool,
                          int);

  void log_decision      (const Header*,
                          const char*,
                          int);

private:
  Dblog                  ();
  ~Dblog                 ();

  Dblog                  (const Dblog&);
  Dblog& operator=       (const Dblog&);

  void create_schema     (void);
  unsigned long max_msg_log_serial (void) const;
  std::string ensure_msg_log_id (const Header*) const;
  std::string sql_quote  (const std::string&) const;
  void exec              (const std::string&) const;

#ifdef USE_SQLITE3_HEADERLOG
  void* db_handle;
#endif

  bool is_enabled;
  bool is_ready;
};

#endif
