# SQLITE_INTEGRATION.md

## Overview

The SQLite integration extends the existing mailfilter logging system by adding
a structured database backend for header analysis and rule generation.

---

## Existing Logging Mechanism

Mailfilter already supports optional logging of email headers into a plain text file:

    SHOW_HEADERS = "/var/spool/filter/mailheader.log"

This mechanism writes raw headers sequentially for debugging and inspection.

---

## SQLite Logging

The SQLite backend follows the same concept but stores data in structured form:

    LOG_HEADERS_SQLITE3 = "/var/spool/filter/mailheader.log.sqlite3"

Both logging mechanisms can be enabled independently.

---

## Design Approach

- The SQLite logging is attached to the same processing path where headers are
  parsed and evaluated.

- No separate parsing logic is introduced.

- The original filtering behavior is not modified semantically.

- Additional logging hooks were introduced at relevant points in the code.

---

## Implementation Highlights

- Initialization is performed during program startup if enabled
- Logging occurs during header parsing and rule evaluation
- Database connection is properly closed on shutdown

The implementation is encapsulated in a dedicated module (`dblog.cc`).

---

## Stored Data

The database contains structured information including:

### messages
- message ID
- decision (pass / deny / score-deny)
- final score

### header_entries
- individual header fields (name/value)
- linked to message ID

### rule_hits
- evaluation phase
- expression
- match result
- score contribution

---

## Data Scope

- Only email headers are stored
- No message body is written to the database
- Data reflects the internal processing state of mailfilter

---

## Advantages

- Structured and queryable data
- Direct input for rule generation
- No impact on existing mailfilter behavior
- Fully deterministic and reproducible

---

## Summary

The SQLite integration is a non-intrusive extension of the existing logging
system. It enhances visibility into mailfilter's decision process without
altering its core behavior.
