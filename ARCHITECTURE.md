# Architecture Overview

## 1. Design Goals

`mailfilter-sqlite` is not just a mail filter extension.  
It is designed as a **header-centric analysis and rule feedback system**.

Core principles:

- **Header-only processing** (no body parsing)
- **Deterministic rule evaluation**
- **Structured logging (SQLite)**
- **Offline analysis and rule generation**
- **Human-controlled feedback loop**
- **No self-modifying runtime behavior**

The system separates **runtime filtering** from **rule generation and analysis**.

---

## 2. High-Level Architecture

```
Incoming Mail
     │
     ▼
 mailfilter (runtime)
     │
     ├── Rule evaluation (ALLOW / DENY / SCORE)
     │
     └── Structured logging → SQLite database
                             │
                             ▼
                    Analysis / Rulegen
                             │
                             ▼
                   Generated rule files
                             │
                             ▼
                     mailfilterrc (INCLUDE)
                             │
                             ▼
                        Runtime reuse
```

This forms a **controlled feedback loop**, not an autonomous system.

---

## 3. Runtime Layer (mailfilter)

At runtime, `mailfilter`:

1. Parses email headers (RFC822)
2. Applies rules from `mailfilterrc`
3. Evaluates decisions:
   - `pass`
   - `deny`
   - `score-deny`
   - `deny-maxlength`
4. Logs structured data into SQLite (optional, configurable)

### Key property

The runtime remains:

- fast
- deterministic
- independent of rule generation logic

---

## 4. Database Layer (SQLite)

The SQLite database is the **central analysis component**.

It is not just a log — it is a **data source for rule generation**.

### Typical tables

- `messages`
  - decision
  - final_score
  - subject
- `header_entries`
  - tag (From, Subject, etc.)
  - body
- `rule_hits`
  - phase (deny / score)
  - expression
  - matched
  - score_delta

### Purpose

- detect patterns
- identify frequent senders/domains
- evaluate rule effectiveness
- support data-driven rule generation

---

## 5. Analysis & Rule Generation

### Components

- `mailfilter-rulegen.sh`
  - data preparation
  - SQL extraction
- `mailfilter-rulegen.pl`
  - core logic
  - decision engine

### Inputs

- SQLite database
- Existing rules (`mailfilterrc`)
- Policy/config files:
  - `protected_domains.conf`
  - `allow_subject_tokens.conf`
  - `weak_subject_tokens.conf`
  - `bulk_mail_providers.conf`
  - `brand_domains.conf`

### Processing

The rule generator:

- identifies suspicious patterns (domains, headers, subjects)
- evaluates frequency and risk
- checks for conflicts with:
  - protected domains
  - existing ALLOW rules
- assigns scoring levels
- filters unstable or low-quality candidates

### Important

Rules are **not blindly generated**.  
They are filtered through multiple safety layers.

---

## 6. Output Files

### 6.1 Candidate Output

```
generated-candidates.conf
```

- raw suggestions
- not used in runtime
- intended for review

---

### 6.2 Exported Rule Sets

```
generated-rules.conf
generated-conservative-rules.conf
generated-aggressive-rules.conf
```

- ready-to-use rules
- separated by aggressiveness level
- intended for controlled integration

---

## 7. Feedback Loop

The system uses a **controlled feedback loop**:

1. Rules are generated offline
2. Admin reviews and selects rules
3. Rules are included in `mailfilterrc`
4. Runtime applies rules
5. New data is logged
6. Cycle repeats

### Key property

> The system is **human-supervised**, not autonomous.

---

## 8. Safety Mechanisms

To prevent false positives and unstable rules:

- Protected domains (multi-level trust)
- ALLOW proximity checks
- Bulk provider detection
- Weak signal filtering
- Frequency thresholds
- Conflict detection with existing rules

---

## 9. Statistics & Analysis Tools

### CLI Tool

```
mailfilter-stats.sh
```

- fast terminal output
- useful for debugging and quick inspection

### HTML Tool

```
mailfilter-stats-html.sh
```

- structured HTML reports
- visual analysis
- suitable for documentation and sharing
- handles long header values safely

---

## 10. Integration Scenario

Typical deployment:

```
fetchmail → mailfilter → MTA / SpamAssassin
```

or:

```
mail source → mailfilter → downstream processing
```

---

## 11. Positioning

`mailfilter-sqlite` is:

- lightweight
- transparent
- auditable
- scriptable
- suitable for automation

---

## 12. Future Direction

The architecture allows:

- improved data-driven rule generation
- better scoring heuristics
- domain clustering
- pattern detection across headers
- integration into distribution-specific packages

---

## 13. Summary

`mailfilter-sqlite` combines:

- deterministic filtering
- structured logging
- offline analysis
- controlled rule generation

into a **closed but human-controlled feedback system**.
