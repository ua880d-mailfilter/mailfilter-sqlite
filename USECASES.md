# USECASES.md

## Overview

This document describes typical use cases for the mailfilter-sqlite rule generation system.

The system is flexible and can be deployed in different roles depending on requirements.

---

## 1. Lightweight Mail Pre-Filter (Home / IPFire)

### Scenario

A small system (e.g. IPFire) with:

- fetchmail
- postfix (or similar MTA)
- spamassassin

### Goal

Reduce load on downstream components by filtering early.

### How it works

- mailfilter processes headers first
- obvious spam is denied early
- remaining mails continue to MTA / SpamAssassin

### Benefits

- lower CPU usage
- reduced spam volume
- faster processing

---

## 2. Analysis-Only Mode (Passive Monitoring)

### Scenario

No active filtering, only data collection.

### Goal

Understand incoming mail patterns before enabling rules.

### How it works

- headers are logged into SQLite
- rulegen analyzes data
- rules are generated but NOT applied

### Benefits

- zero risk
- full visibility
- safe tuning phase

---

## 3. Incremental Rule Deployment

### Scenario

Gradual introduction of filtering rules.

### Goal

Minimize false positives.

### How it works

- start with SCORE rules only
- monitor behavior
- later introduce DENY rules selectively

### Benefits

- controlled rollout
- safe optimization
- high confidence rules

---

## 4. Campaign Detection and Tracking

### Scenario

Identify recurring spam or phishing campaigns.

### Goal

Detect patterns across multiple messages.

### How it works

- group by domain, subject, infrastructure
- analyze frequency and timing
- generate campaign-based rules

### Benefits

- early detection of campaigns
- efficient rule generation
- better coverage with fewer rules

---

## 5. Security Analysis / Mail Intelligence

### Scenario

Use the SQLite database as a data source.

### Goal

Perform deeper analysis or integrate with other tools.

### How it works

- query header data directly
- analyze trends and anomalies
- export data to external systems

### Benefits

- structured telemetry
- flexible integration
- foundation for advanced tooling

---

## 6. High-Volume Mail Gateway (Advanced)

### Scenario

Deployment in larger environments.

### Goal

Reduce load on central filtering systems.

### How it works

- pre-filter at entry point
- drop obvious spam early
- forward only relevant mail

### Benefits

- scalability
- reduced processing cost
- improved performance

---

## Summary

The system can be used as:

- a pre-filter
- an analysis engine
- a rule generator
- a telemetry source

It adapts to both small and large environments while maintaining full transparency and control.
