# Frozen Unicode Input Sources

Captured: 2026-08-25

The files below are committed fixtures, not live build dependencies. Their
exact bytes are checked by `SHA256SUMS`.

## Unicode 17.0.0

Base URL: `https://www.unicode.org/Public/17.0.0/ucd/`

| Retained file | Upstream role |
| --- | --- |
| `ucd/ReadMe.txt` | UCD version and directory provenance |
| `ucd/UnicodeData.txt` | categories, combining classes, decomposition, simple case mappings |
| `ucd/NormalizationTest.txt` | normative normalization conformance corpus |
| `ucd/DerivedNormalizationProps.txt` | quick checks and derived normalization mappings |
| `ucd/CompositionExclusions.txt` | canonical composition exclusions for later NFC work |
| `ucd/CaseFolding.txt` | simple/full/default/Turkic case-fold mappings |
| `ucd/SpecialCasing.txt` | full, contextual, and locale case mappings |
| `ucd/DerivedCoreProperties.txt` | derived properties, including identifier inputs |
| `ucd/PropList.txt` | additional normative/informative binary properties |
| `ucd/PropertyAliases.txt` | stable property aliases for future data parsing |
| `ucd/PropertyValueAliases.txt` | stable property-value aliases for future data parsing |

Each UCD file was downloaded by appending its retained filename to the base
URL. `LICENSE.txt` was captured from `https://www.unicode.org/license.txt` and
contains Unicode License V3 (`SPDX-License-Identifier: Unicode-3.0`). The data
file headers also refer to the Unicode Terms of Use.

Normative specifications:

- UAX #15, Unicode Normalization Forms:
  `https://www.unicode.org/reports/tr15/tr15-57.html`
- UAX #44, Unicode Character Database:
  `https://www.unicode.org/reports/tr44/tr44-36.html`

## ICU 78.3 `gennorm2` Reference Notation

Tag: `release-78.3`

- `nfc.txt`:
  `https://raw.githubusercontent.com/unicode-org/icu/release-78.3/icu4c/source/data/unidata/norm2/nfc.txt`
- `LICENSE.txt`:
  `https://raw.githubusercontent.com/unicode-org/icu/release-78.3/LICENSE`
- notation documentation:
  `https://unicode-org.github.io/icu/userguide/transforms/normalization/#data-file-syntax`

The retained `nfc.txt` identifies itself as machine-generated complete NFC data
for Unicode 17.0.0. It is a reference transcription source, not a second
normative standard: Unicode 17.0.0 UAX #15 and the UCD remain authoritative.
ICU's rule file is valuable because it makes the IBM-origin `gennorm2` notation
and its mapping/combining-class decisions executable and auditable.

