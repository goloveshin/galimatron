---
name: product-test
description: >-
  Stop when a Product or Slice test would need direct file manipulations and
  tell the user. Use when adding or changing a Product test, a Slice test,
  translate_production, translate_slice, test_axes::product,
  test_axes::translator_slice, gal.cfg test cases, shared .cfg and .xsl
  files, or directory compares of actual_gal and actual_xml.
---

# Product tests

if making Product test needed direct file manipulations - stop and let me know!

A Product test is a suite marked `galimatron::test_axes::product`. It calls `translate_production` with `gal.cfg` and `log.cfg` on real files. The pattern is `test_wizard_create_version.cpp`: compare `test_folders::actual_gal` with `expected_gal`, and `actual_xml` with `expected_xml`. `compare_directories` is allowed. An exception check on a failing case is allowed.

Direct file manipulations are the test opening, rewriting, or deleting file bytes: `ifstream`, `ofstream`, `filesystem.read`, `filesystem.write`, `filesystem.remove`, a path rewrite, or reading XML to count attributes. The product writes the files. The test does not.

Stop before adding that code. Say which manipulation the case needs and why a directory compare does not cover it. Do not implement a workaround.

# Staging before a run

Two product cases prepare files, then call `translate_production`, then compare directories. The helpers are `stage_file` and `stage_directory` in `galimatron/tests/utils.hpp`.

`stage_file` copies one file and creates the parent directories. `test_xslt_catalog.cpp` uses it to put `catalog.xml` and `gal-functions.xsl` into `share/galimatron`. The missing-catalog case deletes `share` before the run so the log says the catalog did not resolve. That delete stays in the test.

`stage_directory` replaces a directory with a copy of another. `test_normal_mode.cpp` uses it to copy `source_gal` into `incoming_gal` before every run. That folder is both input and output, so the run deletes a `.gal` file it does not rewrite. The source stays untouched, and the next run stages it again.

A product test still does not open or rewrite file bytes, and it does not remove files except that catalog delete.

# Slice tests

A Slice test is a suite marked `galimatron::test_axes::translator_slice`. It calls `translate_slice` with the steps that case runs. `translate_slice` still loads `gal.cfg` and `log.cfg`. Compare the directory that slice writes. `compare_directories` is allowed. An exception check on a failing case is allowed. The same direct file manipulations are forbidden. The slice writes the files.

`test_cpp2gal.cpp` compares `test_folders::actual_gal` with `expected_gal`. `test_cpp2post_cpp.cpp` compares `test_folders::actual_xml` with `expected_xml`. Its compiler failure checks the message, the path, the aggregate, and the version. Normalizing that path for the check is allowed.

# Shared configs

Share the maximum set of `.cfg` and `.xsl` files that are the same across cases. Put one copy in a subfolder of `galimatron/tests/test_data/translator_library/common`. CMake copies it onto the cases. A file that differs stays in the case folder.

`common/default` holds `gal.cfg`, `log.cfg`, and `xslt/config.xml` for ordinary cases. `common/xslt_seal` holds the `gal.cfg` every seal case uses and the `edit.xsl` that `reassign`, `inherit`, and `open` share. Those three also receive `galimatron/translator_library/xslt/gal-functions.xsl` beside `edit.xsl`. `present` and `unchanged` keep their own `edit.xsl`.
