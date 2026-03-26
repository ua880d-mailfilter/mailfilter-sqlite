# CONFIGURATION.md

## Overview

This document describes all configuration files used by the **rulegen subsystem**
of mailfilter-sqlite.

All persistent configuration files are located in:

    /etc/mailfilter/rulegen/

Generated rule files are written to:

    /etc/mailfilter/

This separation is intentional and prevents accidental deletion of critical
configuration.

---

## General Format

Most configuration files follow a simple line-based format:

    value1|value2|value3|...

- Lines starting with `#` are comments
- Empty lines are ignored

---

## protected_domains.conf

### Purpose

Defines trusted domains and their protection level.

This is the **most important configuration file** in the system.

### Format

    <domain>|<level>|<category>|<description>

### Example

    fedex.com|2|logistics|parcel service
    ipfire.org|3|project|trusted project

### Fields

- **domain**
  Domain name to protect

- **level**
  Protection level:
  - 1 → weak protection, no auto DENY-Rule
  - 2 → medium , no DENY-RULE, only SCORE rules allowed
  - 3 → strong , no DENY-Rule, no rules allowed

- **category**
  Logical classification (e.g. logistics, finance, project, isp, dev, shop, media)

- **description**
  Free text for documentation/debugging

### Effect

- reduces false positives
- modifies scoring
- can completely suppress rule generation

### Warning

Do NOT delete or accidentally overwrite this file.

---

## allow_subject_tokens.conf

### Purpose

Defines tokens that indicate legitimate messages.

### Example

    invoice
    shipping confirmation
    account update

### Effect

- used for ALLOW-proximity detection
- prevents false positives

---

## bulk_providers.conf

### Purpose

Defines known bulk mail providers / ESPs.

### Example

    mailchimp.com
    sendgrid.net

### Effect

- helps classify traffic
- reduces false positives for legitimate bulk mail

---

## weak_subject_tokens.conf

### Purpose

Defines tokens with low signal value.

### Example

    update
    notification
    message

### Effect

- prevents weak tokens from triggering rules alone

---

## brand_domains.conf

### Purpose

Defines brand names for phishing detection.

### Example

    amazon
    paypal
    fedex.com
	gmail

### Effect

- used for fake-brand detection
- helps detect typosquatting and impersonation

---

## File Placement Summary

### Persistent configuration

    /etc/mailfilter/rulegen/

Contains:

- protected_domains.conf
- allow_subject_tokens.conf
- bulk_providers.conf
- weak_subject_tokens.conf
- brand_domains.conf

### Generated files

    /etc/mailfilter/

Contains:

- generated-candidates.conf
- generated-rules.conf
- generated-conservative-rules.conf
- generated-aggressive-rules.conf

---

## Best Practices

- Always review generated rules before activation
- Start with conservative settings
- Keep configuration files under version control (optional)
- Document custom entries inside the files using comments

---

## Notes

The configuration system is intentionally simple, transparent, and
fully explainable.

All decisions are based on:

- structured data
- explicit rules
- deterministic evaluation

No hidden logic or machine learning is involved.
