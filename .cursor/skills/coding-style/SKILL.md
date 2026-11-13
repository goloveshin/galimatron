---
name: coding-style
description: >-
  Apply this repository's C++ coding style when writing or editing code:
  comment separators, the closing parenthesis of multi-line functions,
  type suffixes, includes, the precompiled header, namespaces, and MSVC constraints.
  A comment above wizard { in a .gal file ends with a blank //.
  Use when adding or changing C++ in global, galimatron, examples, or tests,
  or when the user mentions coding style, comments, naming, formatting, or wizard.
---

# Coding style

Match the surrounding file. These are the rules that are easy to get wrong.

## Comments

A comment directly above a function, class, struct, or their `template <...>` ends with a blank `//` at the same indent. English. Say what it checks or does. Name the exception when a check throws.

`// testing:` is its own comment, with a blank `//` before it and after it. Values: `// testing: no`, `// testing: yes (unit)`, `// testing: will (unit)`.

A parameter note sits on the line above that parameter, with no blank `//`. A call, a macro, and an unmarked function have no blank `//`.

When the arguments are one struct, the comment sits above that struct. The function follows the closing `};` on the next line.

```cpp
// compiles the xslt and replaces the document with the primary result.
// parameters are set before compile. side results are captured when asked
//
struct compile_and_apply_t
{
    const std::string& xslt;
    bool capture_side_results = false;
};
void compile_and_apply(const compile_and_apply_t&);
```

```cpp
// flushes the deferred write, then deletes files that were never rewritten
//
// testing: yes (unit)
//
void flush(
    const path_t& path,
    // the user's catch functions
    DirectHandlers&& ... handlers
)
```

A comment above `wizard {` in a `.gal` file ends with a blank `//`. The generated console keeps that `//` between the description and the commented command.

```
// sea_state_e gains gale. vessel_kind_e gains launch. versions 0, 1, 2, 3, and 4 stay closed.
//
wizard {
```

```
// sea_state_e gains gale. vessel_kind_e gains launch. versions 0, 1, 2, 3, and 4 stay closed.
//
// wizard {
```

## Names

| Suffix | Meaning |
| --- | --- |
| `_t` | type |
| `_ot` | `std::optional` |
| `_ft` | `std::function` |
| `_et` | enum |
| `_at` | `std::any` |
| `_rt` | `std::reference_wrapper` |
| `_sptr_t` | `std::shared_ptr` |

Enum values are `SCREAMING_SNAKE`. Data members end with `_`.

Reuse aliases from `global/common_library/type_declaration.hpp` (`index_t`, `indices_t`, `bool_ot`, `name_t`, `strings_t`, `path_t`, and the `_ot` forms). Add a new alias there when the same spelling would be repeated. Nested `galimatron` namespaces see them through `using namespace global` in `galimatron/common_library/type_declaration.hpp`.

Boost.LEAF exception structs in `exception_declaration.hpp` have no `_t` (`struct error`, `struct e_what_info`). A named string constant is `inline constexpr std::string_view`, not `char[]`.

## Includes

Group includes. A short comment names the group. A blank line separates groups. Paths are relative to the repository root.

```cpp
// global
#include "global/common_library/scope_guard.hpp"

// Boost.LEAF
#include <boost/leaf/context.hpp>
```

`global` does not include `galimatron`. A new global library is `global/<name>_library/` with the same `CMakeLists.txt` shape as `global/filesystem_library`. Wizard headers include `galimatron/common_library/forward.hpp`.

Standard C++ headers come from `global/pch_library/pch.hpp`. Add a missing one there. Do not `#include` it in a `.hpp` or `.cpp`.

## Parentheses

For a function declaration, definition, or call, `)` is on its own line, indented with the line that opens `(`. `;` or `,` stays on the `)` line. A one-line signature stays one line. `if`, `for`, `while`, `switch`, and `catch` are not functions.

When `(` and the last argument's `}` share an indent, `)` stays on the `}` line: `}),`. When `}` is indented further, `)` is the next line at the call's indent. A call that opens on one line closes on that same expression: `foo(bar({ .a = 1 }));`.

```cpp
leaf_spy_args_t make_args(
    const leaf_spy_args_t::job_that_may_throw_ft& job,
    const bool is_throw_out
)

filesystem.read(
{
    .path = path_to_test_data / "file_2.txt"
}),

auto guard = scope_guard_t(
    [&]{
        replace_path_func = [](auto&){};
    },
    []{
        replace_path_func = [](auto&){};
    }
);
```

## Parameter packs

A space before `...`: `template <typename ... T>` and `T&& ... args`.

## Layout

4 spaces. The brace of a function, type, or namespace is on the next line. Close with `}// namespace qualified::name`. A blank line separates a namespace from a nested namespace. Prefer designated initializers.

`Logme::SID SUBSID` is the only declaration in its anonymous namespace. That closer has no comment.

```cpp
namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_cmp);
}
```

## MSVC

- `std::format`'s first argument is a string literal.
- `std::string` from `std::string_view` is explicit: `name_t{ the_view }`.
- `ranges::to_vector` is an object. No `()`.
- Pass an lvalue into `views::transform`. An rvalue `std::vector` piped into that view does not compile.
- `using enum entity_et` and `using enum context_et` together do not compile. Use one, or qualify the enumerators.

## Tests

Doctest. The suite name is the feature path: `"global/common_library/scope_guard"`. The case name is a short English phrase. A comment above the case uses the blank `//` rule.

## Scope of an edit

Change the code the task needs. Leave an existing typo, a mismatched namespace closer, and an unused overload unless the task is that fix.
