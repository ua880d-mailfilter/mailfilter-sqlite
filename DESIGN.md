# DESIGN.md

## Design Philosophy

The mailfilter-sqlite rule generation system follows a clear and deliberate design philosophy.

It is built to be transparent, deterministic, and controllable.

---

## Core Principles

### 1. Header-First Filtering

The system operates primarily on email headers before full message retrieval.

Benefits:

- Reduces I/O and bandwidth usage
- Enables early decision making
- Prevents unnecessary downstream processing

---

### 2. Offline Rule Generation

Rules are not created during live traffic processing.

Instead:

- Data is collected first
- Analysis is performed offline
- Rules are generated in controlled batches

This ensures:

- Stability
- Reproducibility
- No unpredictable runtime behavior

---

### 3. Deterministic Behavior

All decisions are rule-based and reproducible.

- Same input → same output
- No randomness
- No hidden state

This makes the system:

- Debuggable
- Explainable
- Auditable

---

### 4. No Machine Learning

The system intentionally avoids machine learning.

Reasons:

- Full transparency of decisions
- No black-box behavior
- Predictable outcomes
- Easy tuning and control

---

### 5. Explainable Results

Every rule and decision can be traced back to:

- Header data
- Statistical patterns
- Explicit logic

There are no hidden heuristics.

---

### 6. False-Positive Awareness

The system actively avoids aggressive blocking.

Mechanisms:

- ALLOW proximity detection
- protected domains
- conservative scoring

Goal:

- Minimize disruption of legitimate emails

---

### 7. Separation of Concerns

The system is divided into clear stages:

1. Data collection (SQLite logging)
2. Analysis (rulegen)
3. Rule application (mailfilter runtime)

This separation allows:

- Independent tuning
- Easier debugging
- Flexible integration

---

### 8. Infrastructure Awareness

The system evaluates not only domains but also:

- Received hosts
- Mail relays
- Delivery patterns

This enables detection of:

- Campaigns
- Shared infrastructure
- Rotating sender domains

---

## What This System Is NOT

- Not a real-time adaptive system
- Not self-learning during runtime
- Not based on probabilistic models
- Not dependent on external services

---

## What This System IS

- A deterministic pre-filter
- A rule generation engine
- A structured mail telemetry system
- A foundation for further analysis tools

---

## Summary

The system prioritizes:

- Transparency over automation
- Control over complexity
- Stability over adaptability

It is designed for administrators who want full insight and control over mail filtering behavior.
