---
name: boost-string
description: >-
  Use Boost.Algorithm for string search, split, replace, trim, and join, not
  text.find, find_first_of, a substr loop, or npos. Use when C++ searches,
  splits, or rewrites a string.
---

# Boost string algorithms

Search, split, replace, trim, erase, and join string text with Boost.Algorithm. `<boost/algorithm/string.hpp>` is in `global/pch_library/pch.hpp`: add no include.

Do not scan string text with `find`, `rfind`, `find_first_of`, `find_last_of`, `find_first_not_of`, a `substr` loop, or `std::string::npos`. `find` on a map, set, or other container stays. Replace such a scan in a function you edit. Leave the other functions of the file alone.

| Task | Use |
| --- | --- |
| contains, starts with, ends with | `boost::contains`, `boost::starts_with`, `boost::ends_with` |
| replace | `boost::algorithm::replace_all`, `replace_first` |
| erase, trim | `boost::algorithm::erase_all`, `trim` |
| split, join | `boost::split` with `boost::is_any_of`, `boost::join` |

A line split normalizes newlines first and drops one trailing `\n` before `boost::split`, so a final newline adds no empty line. An empty string stays one empty line.

```cpp
strings_t lines_of(std::string text)
{
    boost::algorithm::replace_all(text, "\r\n", "\n");
    boost::algorithm::replace_all(text, "\r", "\n");
    if (!text.empty() && text.back() == '\n')
    {
        text.pop_back();
    }

    strings_t lines;
    boost::split(lines, text, boost::is_any_of("\n"));
    return lines;
}
```

A test checks text with the same functions: `CHECK(boost::contains(text, "crash\n"));`.
