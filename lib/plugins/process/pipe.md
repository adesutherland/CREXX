# CREXX Level-b PIPE Engine

The CREXX Level-b PIPE Engine is a lightweight, CMS-Pipelines–inspired stream processor for the CREXX Level-b environment.

It provides a flexible and modular way to process text streams by chaining simple stages — sources, filters, and sinks — into a single pipeline expression.
All stages are implemented in plain REXX, with seamless integration of external commands and programs through the Process API.

This design allows combining built-in REXX text-processing functions with system-level utilities, enabling powerful data transformations in a compact, readable form.---

## 🌱 Overview

The `PIPE` command connects multiple processing stages with the pipe (`|`) character.

```rexx
PIPE < "in.txt" | LOCATE "error" | CHANGE "error" "warn" | > "out.txt"
```

- Each stage processes a stream of text lines.
- The first stage is a **source**.
- The last stage is a **sink**.
- Quotes (`"…"`, `'…'`) protect spaces and `|`.
- Keywords are **case-insensitive**; arguments preserve case.

---

## ⚙️ Syntax Summary

```
PIPE <source> | filter | filter | ... | <sink>
```

**Sources**
- `< "filename"` — read file
- `CMD "command"` — run command and use its stdout

**Filters**
- Built-in REXX filters (`LOCATE`, `CHANGE`, `COUNT`, `FIELD`, `SORT`, etc.)
- External filters via `CMD "command [args]"`

**Sinks**
- `> "filename"` — write to file
- `> CONSOLE` — display on screen
- `> NULL` — discard output
- `CMD "command"` — send stream to an external process

---

## 🧰 Implemented Commands

### 🟢 Sources & Sinks

| Command | Type | Description |
|----------|------|-------------|
| `< "file"` | Source | Read a text file into the stream. Target stem is reset before reading. |
| `> "file"` | Sink | Write stream to file. Each line ends with CRLF except the last (avoids phantom blank). |
| `> CONSOLE` | Sink | Output each line with index to the console. |
| `> NULL` | Sink | Discard stream silently. |
| `CMD "command"` | Source / Filter / Sink | External process, connected via stdin/stdout. Supports `SYNC` and `TIMEOUT=ms`. |

---

### 🔹 Built-in Filters

| Command | Description | Example |
|----------|--------------|----------|
| **LOCATE "pat"** | Keep lines containing the substring `pat`. | `PIPE < "log.txt" | LOCATE "ERROR" | > "errs.txt"` |
| **CHANGE "old" "new"** | Replace all occurrences of `old` with `new`. | `PIPE < "in.txt" | CHANGE foo bar | > "out.txt"` |
| **SPECS start-end** | Extract substring from position `start` to `end` (1-based inclusive). | `PIPE < "in.txt" | SPECS 1-20 | > "first20.txt"` |
| **DROP n** | Skip the first `n` lines. | `PIPE < "in.txt" | DROP 2 | > "rest.txt"` |
| **TAKE n** | Keep only the first `n` lines. | `PIPE < "in.txt" | TAKE 10 | > "head.txt"` |
| **COUNT** | Replace stream with one line = number of input lines. | `PIPE < "in.txt" | COUNT | > CONSOLE` |
| **TRACE [label]** | Print each line (optional label) to console, pass through unchanged. | `PIPE < "in.txt" | TRACE before | > CONSOLE` |
| **SORT [args]** | OS sort; shorthand for `CMD "sort [args]"`. | `PIPE < "in.txt" | SORT /R | > "sorted.txt"` |
| **UNIQUE** | Remove duplicate lines, keep first occurrence (stable). | `PIPE < "in.txt" | UNIQUE | > "unique.txt"` |
| **FIELD n [SEP "c"]** | Extract the `n`-th field (whitespace default; or custom separator). | `PIPE < "data.csv" | FIELD 2 SEP "," | > "col2.txt"` |
| **UPPER** | Convert each line to uppercase. | `PIPE < "data.txt" | UPPER | > "up.txt"` |
| **LOWER** | Convert each line to lowercase. | `PIPE < "data.txt" | LOWER | > "low.txt"` |
| **TRIM [L|R]** | Trim blanks. Default both; `L`=left; `R`=right. | `PIPE < "pad.txt" | TRIM R | > "noR.txt"` |
| **PAD width [LEFT|RIGHT]** | Pad lines with spaces to width. Default RIGHT. | `PIPE < "names.txt" | PAD 20 LEFT | > CONSOLE` |
| **TIME** | Placeholder for timing/diagnostics (passes data through). | `PIPE < "in.txt" | TIME | > "out.txt"` |

---
