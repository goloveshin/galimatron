---
name: gal-cfg
description: >-
  Name the gal.cfg directories incoming_gal, actual_gal, actual_xml, intermediate,
  actual_summary. Use when changing a gal.cfg or its input-gal-dir, output-gal-dir,
  intermediate-xml-dir, intermediate-log-dir, or intermediate-summary-dir.
---

# gal.cfg directories

```
"input-gal-dir" : "incoming_gal/",
"output-gal-dir" : "actual_gal/",
"intermediate-xml-dir" : "actual_xml/",
"intermediate-log-dir" : "intermediate/",
"intermediate-summary-dir" : "actual_summary/",
```

Use `intermediate/` when the directory is only scratch and nothing compares it. Use `actual_xml/`, `actual_summary/`, and `actual_log/` when those results are kept or compared.

A test `gal.cfg` compares xml and summary: `actual_xml/` and `actual_summary/`. The log stays `intermediate/`. `galimatron/tests/case_folders.hpp` repeats these names.

An example that compares no xml, log, or summary keeps all three in `intermediate/`.
