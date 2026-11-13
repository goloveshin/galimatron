---
name: ranges
description: >-
  Build a container with ranges::views::transform and ranges::views::filter
  instead of a loop that push_back's each element. Sort with
  ranges::actions::sort instead of std::sort or std::ranges::sort. Use when
  adding or changing C++ that maps, filters, or sorts a collection, or when
  the user mentions push_back, token_to_text, ranges, transform, filter,
  sort, or actions.
---

# Ranges

A loop whose only job is to map or filter into a new container is a pipeline. Keep the source's order.

```cpp
return
    tokens
    | ranges::views::transform([](auto* token){ return token_to_text(token); })
    | ranges::views::filter([](const auto& name){ return !name.empty(); })
    | ranges::to<strings_t>()
    ;
```

`ranges::to_vector` is an object. No `()`. `ranges::to<T>()` has the parentheses. Pass an lvalue into `views::transform`. A function that returns a `std::vector` is an rvalue; store it, then pipe the name. `<range/v3/all.hpp>` is in `global/pch_library/pch.hpp`.

A loop that also changes other state, returns early, or stops at a match stays a loop. A condition that only drops elements is `filter`. When you edit a function that still `push_back`s a pure map or filter, replace that loop. Leave other loops in the file alone.

## Sort

Sort with `ranges::actions::sort`. Do not call `std::sort` or `std::ranges::sort`. A container the pipeline just built is an rvalue, so the action continues the pipe:

```cpp
return
    records
    | ranges::views::transform([](const record_t& record){ return record.index; })
    | ranges::to_vector
    | ranges::actions::sort
    ;
```

An existing container is an lvalue. Assign with `|=`. The default order is `operator<`. The comparator stays `ranges::less{}`. A projection lambda returns the key when the key is not one member, as `layout_table.cpp` does for the attribute fingerprint. Pass a comparator only when the order is not a key, as the hash rows do to put the root aggregate first.

```cpp
names |= ranges::actions::sort;
version_indexes |= ranges::actions::sort | ranges::actions::unique;
rows |= ranges::actions::sort(ranges::less{}, &statement_t::index_in_version);
```
