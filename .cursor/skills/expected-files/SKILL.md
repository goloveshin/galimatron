---
name: expected-files
description: >-
  Keep Galimatron expected oracles free of absolute paths. Use when adding or
  changing expected-gal, expected-xml, expected-log, a
  committed example log or xml, committed-report, or report_path.
---

# Expected files

An expected directory is the oracle. It lives only under `galimatron/tests/test_data`. Names are in `galimatron/tests/case_folders.hpp`.

| Oracle | Compared with |
| --- | --- |
| `expected-gal` | `actual_gal` |
| `expected-xml` | `actual_xml` |
| `expected-log` | `actual_log` |

`compare_directories` is the check. There is no `expected-summary`. The summary is written to `actual_summary` and is not an oracle.

`expected-xml` is the pugixml print. Tests round-trip `.xml` through pugixml on read and on write, so one oracle covers every platform.

The only log oracle is `translator_library/no_xslt`. Its `gal.cfg` sets `"committed-report" : true` and `"intermediate-log-dir" : "actual_log/"`. Other cases leave the log in `intermediate/` and do not compare it.

## No absolute path

A path in an oracle is relative to the `gal.cfg` directory, with `/`. `report_path` spells it that way when `committed-report` is on.

```
incoming_gal/payload/fog.gal
```

A drive letter, a `\\` UNC path, `/home/`, `/Users/`, or a machine folder name does not belong in `expected-gal`, `expected-xml`, `expected-log`, or in a committed example log, summary, or xml. `<location>` in language XML stays empty.

The chunk path inside the translator stays absolute. cpp2gal and the wizard write with it. Spell a path only in text that is logged or stored as an oracle. If a fresh `actual_*` file contains an absolute path, fix that writer and run again. Then copy the produced files into the expected directory.
