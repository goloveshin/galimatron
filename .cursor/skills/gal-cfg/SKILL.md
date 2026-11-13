---
name: gal-cfg
description: >-
  Name the directories in a Galimatron gal.cfg incoming_gal, actual_gal,
  actual_xml, intermediate, and actual_summary. Use when adding or changing a
  gal.cfg, input-gal-dir, output-gal-dir, intermediate-xml-dir,
  intermediate-log-dir, intermediate-summary-dir, or those directories.
---

# gal.cfg directories

```
"input-gal-dir" : "incoming_gal/",
"output-gal-dir" : "actual_gal/",
"intermediate-xml-dir" : "actual_xml/",
"intermediate-log-dir" : "intermediate/",
"intermediate-summary-dir" : "actual_summary/",
```

Use `intermediate/` when that directory is only scratch and nothing compares it. Use `actual_xml/`, `actual_summary/`, and `actual_log/` when those results are kept or compared.

A test `gal.cfg` compares xml and summary, so those are `actual_xml/` and `actual_summary/`. The log stays `intermediate/`. `galimatron/tests/case_folders.hpp` repeats these names.

An example that does not compare xml, log, or summary keeps all three in `intermediate/`.
