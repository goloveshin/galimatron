# Third-party notices

Galimatron is distributed under the Boost Software License, Version 1.0 (see [LICENSE](LICENSE)).

It uses the libraries below. The full license text of each one is in [third_party_licenses](third_party_licenses), and the installers copy that folder.

## Why the Boost license

Every dependency is under a permissive license: the Boost license, MIT, BSD, Apache 2.0, or MPL 2.0 for SaxonC-HE. None of them is a copyleft license that spreads to the code that uses it, so Galimatron is free to choose its own license, and it uses the Boost license.

- **MIT, BSD-3-Clause, Apache-2.0** require that their copyright notice and license text travel with a distributed copy. The installers carry them in `third_party_licenses`.
- **MPL-2.0** (SaxonC-HE) is a file-level license. It applies to the Saxon files, which are used and shipped unmodified. It does not apply to Galimatron's own code. If you redistribute SaxonC-HE, its source is available from [Saxonica](https://www.saxonica.com).

## Libraries

| | Library | License | Used for |
|---|---|---|---|
| | [Boost](https://www.boost.org) 1.92.0 | BSL-1.0 | exceptions (LEAF), string algorithms, and other utilities |
| | [ANTLR 4](https://www.antlr.org) 4.13.2 | BSD-3-Clause | the parser of the GaL language (the C++ runtime is linked in, the generator is used at build time) |
| | [SaxonC-HE](https://www.saxonica.com) 13.0.0 | MPL-2.0, with the James Clark notice (MIT) and the JLine2 notice (BSD) | XSLT 3.0 processor |
| <img src="art/third_party/taskflow.png" width="48" alt="Taskflow"> | [Taskflow](https://github.com/taskflow/taskflow) 4.0.0 | MIT | the chain of translation steps |
| | [range-v3](https://github.com/ericniebler/range-v3) | BSL-1.0 | ranges, views and actions |
| | [pugixml](https://pugixml.org) | MIT | reading and writing XML |
| | [nlohmann/json](https://github.com/nlohmann/json) | MIT | parsing the JSON of the logging configuration |
| <img src="art/third_party/inja.svg" width="64" alt="inja"> | [inja](https://github.com/pantor/inja) 3.4.0 | MIT | the text templates of the post step |
| | [alpaca](https://github.com/p-ranav/alpaca) | MIT | serialization in the UUID protocol example only, not in the installers |
| | [magic_enum](https://github.com/Neargye/magic_enum) | MIT | names of enumerations |
| <img src="art/third_party/logme.png" width="48" alt="logme"> | [logme](https://github.com/efmsoft/logme) | Apache-2.0 | logging |
| | [JsonCpp](https://github.com/open-source-parsers/jsoncpp) 1.9.6 | MIT or public domain | reading the JSON of the logging configuration (inside logme) |
| | [leaf_spy](https://github.com/goloveshin/leaf_spy) | BSL-1.0 | reading Boost.LEAF errors |
| | [multi_index_lookup](https://github.com/goloveshin/multi_index_lookup) | BSL-1.0 | lookup over containers |
| <img src="art/third_party/doctest.svg" width="64" alt="doctest"> | [doctest](https://github.com/doctest/doctest) | MIT | the tests only, not in the installers |