# RULEGEN.md

## Overview

The **rulegen subsystem** is the intelligence layer of mailfilter-sqlite.

It transforms structured header data from SQLite into:

- analyzed campaign signatures
- scored risk assessments
- structured rule candidates
- exportable mailfilter rule sets

The system is deterministic, explainable, and fully data-driven.

---

## Processing Pipeline

1. Header collection via mailfilter
2. Storage in SQLite
3. Extraction and aggregation (mailfilter-rulegen.sh)
4. Analysis and scoring (mailfilter-rulegen.pl)
5. Candidate generation
6. Rule export

---

## Core Components

### mailfilter-rulegen.sh

Responsible for:

- SQL data extraction
- aggregation of:
  - subject phrases
  - sender domains
  - received hosts
- parameter handling
- pipeline orchestration

---

### mailfilter-rulegen.pl

Core analysis engine:

- campaign detection
- clustering similar messages
- scoring and classification
- fake-brand detection
- rule generation

---

## Input Data

### SQLite Tables

- `messages`
- `header_entries`
- `rule_hits`

These provide:

- message metadata
- parsed headers
- rule evaluation results

---

## Policy Model

Configuration files are located in:

/etc/mailfilter/rulegen/

### protected_domains.conf

Format:

    <domain>|<level>|<category>|<description>

Example:

    fedex.com|2|logistics|parcel service
    ipfire.org|3|project|trusted project

Defines:

- trust level (1–3)
- semantic classification
- context for evaluation

Impact:

- suppresses false positives
- modifies scoring
- can fully block rule generation

---

### allow_subject_tokens.conf

Defines tokens that indicate legitimate content.

Used for:

- ALLOW proximity detection
- false-positive prevention

---

### bulk_providers.conf

Known bulk senders / ESPs.

Used to:

- reduce false positives
- classify traffic

---

### weak_subject_tokens.conf

Low-signal tokens that should not trigger rules alone.

---

### brand_domains.conf

Reference list for brand-based phishing detection.

---

## Evaluation Concepts

### Time-Based Risk

- `recent` → active and relevant
- `aged` → partially relevant
- `stale` → historical only (no rule export)

---

### Protected Domains

- Level 1 → weak protection
- Level 2 → medium (SCORE only)
- Level 3 → strong (no rule generation)

---

### Recommendation Types

- `allow`
- `score`
- `deny`
- `manual review`
- `high-risk phishing candidate`

---

## Campaign Detection

Messages are grouped based on:

- subject similarity
- sender domain patterns
- infrastructure (received hosts)

Each campaign produces:

- aggregated statistics
- representative examples
- rule suggestions

---

## Fake Brand Detection

Detects:

- typosquatting domains
- brand impersonation
- suspicious domain patterns

Uses:

- brand_domains.conf
- similarity heuristics

---

## Rule Generation

### Candidate Blocks

Generated in:

    generated-candidates.conf

Contain:

- metadata (confidence, risk, counts)
- reasoning (why detected)
- suggested rules

---

### Exported Rules

Generated in:

- generated-rules.conf
- generated-conservative-rules.conf
- generated-aggressive-rules.conf

---

## Export Conditions

Rules are only exported if:

- not `stale`
- no strong protected-domain conflict
- no ALLOW conflict
- sufficient risk / relevance

---

## CLI Control (mailfilter-rulegen.sh)

Example:

    ./mailfilter-rulegen.sh \
      --db /var/spool/filter/mailheader.sqlite3 \
      --mailfilterrc /etc/mailfilter/.mailfilterrc \
      --out generated-candidates.conf \
      --highscore 100 \
      --min-deny-hits 2 \
      --max-pass-hits 0 \
      --min-phrase-size 2 \
      --max-phrase-size 3 \
      --export-rules /etc/mailfilter/generated-rules.conf \
      --export-cons /etc/mailfilter/generated-conservative-rules.conf \
      --export-aggr /etc/mailfilter/generated-aggressive-rules.conf

---

## Runtime Separation

Persistent configuration:

    /etc/mailfilter/rulegen/

Generated output:

    /etc/mailfilter/

This prevents accidental deletion of critical policy data.

---

## Design Principles

- deterministic analysis (no ML required)
- explainable decisions
- separation of core and intelligence layer
- reproducible results
- safe rule generation (no automatic activation)

---

## Summary

The rulegen subsystem converts raw header data into:

- structured knowledge
- campaign intelligence
- actionable mailfilter rules

It enables mailfilter to evolve dynamically while preserving full control and transparency.
