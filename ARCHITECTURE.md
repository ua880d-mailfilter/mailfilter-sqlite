# Architecture

## Overview

The system is structured into three distinct layers:

1. **Collection**
   - `mailfilter` retrieves email headers
   - headers are parsed and stored in SQLite

2. **Analysis**
   - SQL queries and Perl-based processing evaluate patterns
   - messages are grouped, classified, and scored

3. **Rule Deployment**
   - structured candidate blocks are generated
   - rules are written to `generated-rules.conf`
   - rules are included dynamically via `INCLUDE` in `mailfilterrc`

---

## Design Principles

- **Header-only processing**
  - no message body required
  - fast and lightweight

- **Minimal core modification**
  - original mailfilter behavior preserved
  - SQLite logging added without altering core logic

- **External intelligence layer**
  - all adaptive logic is implemented outside the core
  - rule generation handled via scripts (Perl + shell)

- **Deterministic behavior**
  - no machine learning required
  - all decisions are explainable and reproducible

---

## Data Flow

1. mailfilter retrieves headers
2. headers are split into structured components
3. data is stored in SQLite
4. analysis pipeline evaluates patterns
5. campaigns and anomalies are detected
6. rule candidates are generated
7. rules are exported and included into runtime configuration

---

## SQLite Schema

### messages

Stores one row per message:

- `msg_log_id`
- `message_id`
- `from_addr`
- `to_addr`
- `subject`
- `normal_subject`
- `date_hdr`
- `msg_size`
- `decision`
- `final_score`

---

### header_entries

Stores parsed header fields:

- `msg_log_id`
- `ordinal`
- `tag`
- `body`

---

### rule_hits

Tracks rule evaluation results:

- `msg_log_id`
- `phase`
- `expression`
- `matched`

---

## rulegen Subsystem

The rule generation subsystem acts as the intelligence layer.

### Responsibilities

- campaign detection
- clustering similar messages
- fake-brand detection
- temporal scoring (recent / aged / stale)
- false-positive reduction
- rule suggestion and scoring
- rule export

---

### Components

#### mailfilter-rulegen.sh

- orchestrates SQL queries
- controls pipeline execution

#### mailfilter-rulegen.pl

- core analysis engine
- implements scoring logic
- generates rule candidates

#### mailfilter-header-import.pl

- imports test data into SQLite
- enables reproducible testing

---

## Configuration Model

Runtime configuration is separated from code.

Location:

/etc/mailfilter/
/etc/mailfilter/rulegen/

### Configuration Files

- `protected_domains.conf`
- `allow_subject_tokens.conf`
- `bulk_providers.conf`
- `weak_subject_tokens.conf`
- `brand_domains.conf`

These files define:

- trust relationships
- allow-list behavior
- provider classification
- phishing indicators

---

## Rule Generation

Generated files:

- `generated-candidates.conf`
- `generated-rules.conf`
- `generated-conservative-rules.conf`
- `generated-aggressive-rules.conf`

### Workflow

1. candidates are generated with metadata
2. candidates are evaluated and filtered
3. production rules are exported
4. rules are included via `mailfilterrc`

---

## Runtime Integration

Rules are dynamically included:

```
INCLUDE="/etc/mailfilter/generated-rules.conf"
```
or:

```
INCLUDE="/etc/mailfilter/generated-conservative-rules.conf"
```
or:
```
INCLUDE="/etc/mailfilter/generated-aggressive-rules.conf"
```

This allows:

- dynamic adaptation
- no restart required
- separation of static and generated rules

---

## Autonomy

The system can operate without:

- local MTA
- fetchmail

mailfilter can directly retrieve headers and populate the database.

---

## Testing Strategy

- separate SQLite databases for testing
- synthetic datasets supported
- no interference with production data
- deterministic replay possible

---

## Architectural Summary

mailfilter-sqlite transforms a classic filter into:

- a data collection engine
- a structured analysis system
- a rule generation platform

The separation between core and intelligence layer ensures:

- stability of the original codebase
- flexibility of rule evolution
- long-term maintainability
