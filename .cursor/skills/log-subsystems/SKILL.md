---
name: log-subsystems
description: >-
  Keep Logme subsystem ids aligned with log_subsystems.hpp and every
  log.cfg allowed list. Use when adding or changing Logme::SID::Build,
  LOGME_SUBSYSTEM, a subsystem abbreviation, log_subsystems.hpp, or
  subsystems.allowed in a log.cfg.
---

# Log subsystems

A `.cpp` that logs sets one file-scope id. That id is the only declaration in its namespace.

```cpp
namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_cmp);
}// namespace
```

`global/common_library/log_subsystems.hpp` is the list. The constants are `const char*` because `SID::Build` takes `const char*` or `std::string`, and a `string_view` does not bind. The header sits in `global` because the ids cover the executable, the runtime, and the translator. `galimatron/executable/main.cpp` uses `LOGME_SUBSYSTEM(SUBSID, global::log_subsystems::exec);`.

The same ids are listed in `subsystems.allowed` of every `log.cfg`:

- `galimatron/tests/test_data/translator_library/common/default/log.cfg`
- `galimatron/tests/test_data/translator_library/no_xslt/log.cfg`
- `galimatron/tests/test_data/translator_library/xslt_catalog/log.cfg`
- `galimatron/tests/test_data/translator_library/xslt_catalog_resolved/log.cfg`
- `examples/benchmark/log.cfg`
- `examples/uuid_protocol/steps/<step>/generated/log.cfg`

CMake copies the common test file onto each case at configure. Edit that file. Leave the copies under the build tree. `//` comments in these files are part of the Logme format. `xslt_catalog` and `xslt_catalog_resolved` leave out `RT-FS`, `RT-FSD`, and `RT-CFG`: those debug lines name absolute paths.

## Which id

Reuse the id of the folder you are editing. A folder that already has an id does not get a second one.

| Id | Folder |
| --- | --- |
| `EXEC` | `galimatron/executable` |
| `EXLIB` | `galimatron/executable_library` |
| `RT` | `galimatron/runtime_library`, when no narrower id fits |
| `RT-FS` | `runtime_library/filesystem` |
| `RT-FSD` | `runtime_library/filesystem_decorator` |
| `RT-CFG` | `runtime_library/galimatron_config` |
| `RT-TRN` | `runtime_library/translate` |
| `RT-SUM` | `runtime_library/translate_summary_xml` |
| `TR` | `translator_library`, when no narrower id fits |
| `TR-PAR` | `translator_library/parser` |
| `TR-BLD` | `translator_library/state/builder` |
| `TR-HLD` | `translator_library/state/holder` |
| `TR-WIZ` | `translator_library/state/wizard` |
| `TR-G2S` | `transforms/gal2cpp` |
| `TR-S2G` | `transforms/cpp2gal` |
| `TR-S2S` | `transforms/cpp2cpp_wizard` |
| `TR-S2X` | `transforms/cpp2xml` |
| `TR-X2S` | `transforms/xml2cpp` |
| `TR-XSLT` | `transforms/xml2xml` |
| `TR-CMP` | `transforms/cpp2post_cpp` |
| `TR-TSP` | `translator_library/translator`, and `transforms/cpp2cpp_validate` |

The wrappers in `translator/detail/transforms` stay on `TR-TSP`. The transform's own `.cpp` files use the transform's id.

## A new id

Add the `inline constexpr const char*` in `global/common_library/log_subsystems.hpp`, beside the other ids. Above it, `brief` says what the id does and `part` names the folder. A blank `//` follows those lines, and a blank line follows the constant:

```cpp
// brief: from the language cpp presentation to the language gal presentation
// part: galimatron/translator_library/transforms/cpp2gal
//
inline constexpr const char* tr_s2g = "TR-S2G";
```

Pass that constant to `SID::Build`. Add it in the same place in each `log.cfg` above: executable, then runtime, then translator. Every line except the last has a comma. Add a row to the table above.

A rename changes the header, the `.cpp`, every `log.cfg`, and the row in this skill.
