<!-- Logo + Titel -->

<p align="center">
  <img src="docs/images/logo/mf-sqlite-logo2-128p.png" width="110"/>
</p>

<h1 align="center">mailfilter-sqlite</h1>

<p align="center">
  <b>Deterministic mail pre-filter with SQLite-based header analysis and offline rule generation</b>
</p>

<p align="center">
  <i>Explainable. Predictable. No black box.</i>
</p>

<br>

<!-- Screenshot -->

##  Example: Generated Analysis Candidate

![Generated Candidate Example](docs/images/candidates/testset-bewertungsblock.png)

**mailfilter-sqlite** is an extended fork of the original **mailfilter** https://mailfilter.sourceforge.io/ (C) by Andreas Bauer.

The example above demonstrates:

* automatic phishing campaign detection
* fake brand recognition (e.g. `github-security-check.example`)
* risk classification and confidence scoring
* automatic rule suggestion generation

---

##  Why this project exists

Traditional mail filtering often happens too late in the pipeline.

**mailfilter-sqlite shifts detection to the earliest possible stage:**

* headers are analyzed before full message retrieval
* spam campaigns are identified offline
* rules are generated proactively
* unwanted messages can be blocked before reaching MTA or SpamAssassin

 Result: **less load, more control, full transparency**

---

##  Key Features

* **Header-first filtering**

  * no message body required
  * fast and lightweight

* **SQLite integration**

  * structured storage of:

    * messages
    * header entries
    * rule hits
    * final decisions

* **Offline rule generation**

  * no live self-modifying behavior
  * rules are generated in controlled batches

* **Campaign detection**

  * subject similarity
  * sender domains
  * received hosts
  * repeated infrastructure patterns

* **Fake-brand / typosquatting detection**

  * examples:

    * `arnazon`, `amaz0n`, `paypa1`, `g00gle`, `micr0soft`

* **False-positive protection**

  * ALLOW proximity
  * protected domains
  * bulk provider awareness
  * conservative export logic

* **Deterministic behavior**

  * same input ? same output
  * no hidden heuristics
  * no machine learning required

---

##  Architecture

```
mailfilter -> SQLite -> rulegen -> rules -> mailfilter
```

**Pipeline:**

1. **Collection**

   * mailfilter retrieves and parses headers
   * headers are logged to SQLite

2. **Analysis**

   * `rulegen` evaluates domains, subjects, campaigns and infrastructure

3. **Deployment**

   * rules are exported into include files
   * mailfilter applies them on later runs

---

##  Quick Start

```bash
mkdir -p /etc/mailfilter
mkdir -p /etc/mailfilter/rulegen
mkdir -p /var/spool/filter
```

Prepare a SQLite database (choose one):

* copy a provided schema template
* let mailfilter create it automatically
* or build a test database using `.eml` datasets

Add to your `mailfilterrc`:

```conf
INCLUDE="/etc/mailfilter/generated-rules.conf"
```

Run:

```bash
cd /etc/mailfilter

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
```

---

##  Documentation

Start here:

* **QUICKSTART.md** — fastest setup
* **INSTALL.md** — build & deployment
* **CONFIGURATION.md** — policy files
* **RULEGEN.md** — analysis logic
* **EXAMPLES.md** — real rules
* **DESIGN.md** — philosophy
* **SQLITE_INTEGRATION.md** — implementation details

---

##  What this project is not

* not a machine-learning spam filter
* not a black box
* not a live self-learning system

 Rules are generated **offline and deliberately applied**

---

##  Attribution

Based on the original **mailfilter** by Andreas Bauer.

Extended with:

* SQLite-based structured logging
* rule generation subsystem
* additional fixes and enhancements

---

##  License

GNU General Public License (GPL)

Original copyright notices are preserved.
