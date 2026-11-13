---
name: boost-string
description: >-
  Use Boost.Algorithm for string search, split, replace, trim, and join.
  Do not scan text with text.find, find_first_of, a substr loop, or
  std::string::npos. Use when adding or changing C++ that searches, splits,
  or rewrites a string, or when the user mentions find, npos, split, or
  Boost string algorithms.
---

# Boost string algorithms

Search, split, replace, trim, erase, and join string text with Boost.Algorithm. `<boost/algorithm/string.hpp>` is in `global/pch_library/pch.hpp`. Do not add another include for it.

Do not scan text with `text.find`, `rfind`, `find_first_of`, `find_last_of`, `find_first_not_of`, a `substr` loop, or `std::string::npos`. This is string text only. `find` on a map, a set, or another container stays. When you edit a function that still scans text that way, replace that scan. Leave other functions in the file alone.

| Task | Use |
| --- | --- |
| contains | `boost::contains` |
| starts with | `boost::starts_with` |
| ends with | `boost::ends_with` |
| replace | `boost::algorithm::replace_all`, `replace_first` |
| erase | `boost::algorithm::erase_all` |
| trim | `boost::algorithm::trim` |
| split | `boost::split` with `boost::is_any_of` |
| join | `boost::join` |

A line split normalizes newlines first. Drop one trailing `\n` before `boost::split`, so a terminating newline does not add an empty line. An empty string stays one empty line.

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
