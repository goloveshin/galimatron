---
name: ranges
description: >-
  Build a container with ranges::views::transform and filter, not a push_back
  loop. Sort with ranges::actions::sort, not std::sort. Use when C++ maps,
  filters, or sorts a collection.
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

`ranges::to_vector` is an object: no `()`. `ranges::to<T>()` has them. Pass an lvalue into `views::transform`: a function that returns a `std::vector` is an rvalue, so store it and pipe the name. `<range/v3/all.hpp>` is in `global/pch_library/pch.hpp`.

A loop that changes other state, returns early, or stops at a match stays a loop. A condition that only drops elements is `filter`. Replace a pure map or filter `push_back` loop in a function you edit. Leave the other loops of the file alone.

## Sort

Sort with `ranges::actions::sort`, never `std::sort` or `std::ranges::sort`. A container the pipeline just built is an rvalue, so the action continues the pipe:

```cpp
return
    records
    | ranges::views::transform([](const record_t& record){ return record.index; })
    | ranges::to_vector
    | ranges::actions::sort
    ;
```

An existing container is an lvalue: assign with `|=`. The default order is `operator<`, and the comparator stays `ranges::less{}`. When the key is not one member, a projection lambda returns it, as `layout_table.cpp` does for the attribute fingerprint. Pass a comparator only when the order is not a key, as the hash rows do to put the root aggregate first.

```cpp
names |= ranges::actions::sort;
version_indexes |= ranges::actions::sort | ranges::actions::unique;
rows |= ranges::actions::sort(ranges::less{}, &statement_t::index_in_version);
```
