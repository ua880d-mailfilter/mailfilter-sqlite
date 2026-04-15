// header.hh - source file for the mailfilter program
// Copyright (c) 2003 - 2009  Andreas Bauer <baueran@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.

#ifndef HEADER_HH
#define HEADER_HH

#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

struct entry
{
  string tag;
  string body;
};

class WrongMessageIDException : public runtime_error
{
public:
  WrongMessageIDException ()
    : runtime_error ("POP timestamp in message-ID invalid.") 
  {
  }
};

class Header
{
private:
  vector<entry>  msg_entries;
  string         msg_log_id;
  int            msg_entry_ordinal;
  string         msg_ID;
  string         msg_from;
  string         msg_to;
  string         msg_subject;
  string         msg_normal_subject;
  string         msg_date;
  int            msg_size;

  static unsigned long header_serial;

public:
  Header	();
  vector<entry>* entries             (void);
  int            rfc822_valid_msgid  (const char*);
  void           add_entry           (const char*, const char*);
  const string*  log_id              (void)                const;
  void           set_log_id          (const string&);
  int            next_entry_ordinal  (void);
  const string*  ID                  (void)                const;
  void           set_ID              (const char*);
  const string*  from                (void)                const;
  void           set_from            (const char*);
  const string*  to                  (void)                const;
  void           set_to              (const char*);
  const string*  subject             (void)                const;
  void           set_subject         (const char*);
  const string*  normal_subject      (void)                const;
  void           set_normal_subject  (string);
  const string*  date                (void)                const;
  void           set_date            (const char*);
  int            size                (void)                const;
  void           set_size            (int);

  static void sync_log_id_counter (unsigned long); 
};

#endif
