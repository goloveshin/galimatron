---
name: exception-using
description: >-
  Apply Galimatron exception rules: declare exceptions in
  galimatron/common_library/exception_declaration.hpp, throw them with
  Boost.LEAF only inside galimatron, catch them in runtime leaf_spy,
  and print them in tests with print_leaf_exception. Keep the full
  message in e_what_info and attach e_path_info, e_aggregate_info, and
  e_version_info with leaf::on_error. Use when adding or changing a
  throw, a catch, an exception type, leaf::throw_exception,
  leaf::on_error, augment_where, leaf_spy, print_leaf_exception, or
  translator_library error handling.
---

# Exception using

Types live only in `galimatron/common_library/exception_declaration.hpp`, inside `namespace exception`. No `_t` suffix. Throw them with Boost.LEAF, and only inside galimatron. `global` and `examples` do not include this header and do not throw these types.

- `exception::domain`: `executable`, `executable_library`, `runtime_library`, `translator_library`.
- `exception::subdomain::runtime_library` and `exception::subdomain::translator_library` mark the part, for example `runtime_library::filesystem` or `translator_library::translator`.
- `exception::details` holds `io_operation_et` and `validation_error_code_et`.
- Payloads: `e_what_info`, `e_path_info`, `e_aggregate_info`, `e_version_info`, `e_stacktrace_info`. `e_stacktrace_info` fills its stacktrace by default.

```cpp
leaf::throw_exception(
    exception::domain::translator_library{},
    exception::subdomain::translator_library::translator{},
    code,
    exception::e_what_info{ std::move(what) }
);
```

A new leaf type is listed in `galimatron/runtime_library/leaf_spy/leaf_spy.hpp`: `spy_handler_t::operator()` and `print_exception_part::visit`. Domain and subdomain marks count as a matched exception. Detail enums and the `e_*` payloads do not. Antlr `ParseCancellationException` and `nlohmann::json::parse_error` stay out of the declaration header. The binder recognizes them with `dynamic_cast`.

`e_path_info`, `e_aggregate_info`, and `e_version_info` are already registered and printed. The generic `leaf_spy` library stays free of Galimatron types. `galimatron::runtime::try_catch` binds them through `bound_exceptions_t`.

## Where

`e_what_info::what` is the whole message, including the file, the aggregate, and the version when the text already names them. The payloads repeat those facts for the handler. Attach a payload only when that fact is known. `e_path_info::path` is empty when the file is unknown. `augment_path` copies the run's path report onto `e_path_info`. The catch spells the path from those copies and does not read the run slot. Empty means spell the path as stored. An empty aggregate or version is not a payload.

Helpers in `galimatron/common_library/exception_augment.hpp`:

- `augment_path` loads `e_path_info` from a `path_ot`. An empty path loads nothing. The path is a `.gal` file for a language error, or a filesystem or xslt path.
- `e_path_info_for` builds that same payload for a throw. An empty `path_ot` stays empty.
- `augment_aggregate` loads `e_aggregate_info`.
- `augment_version` loads `e_version_info`.
- `augment_where` loads the set fields of `exception::error_where_t`.
- `augment_domain` loads the caller's domain and subdomain. Put it in the function that calls a callback, before the call, and keep it alive across the call. A throw that already names the same type keeps its own value.

Keep the returned object alive until the throws it covers have left the scope. A temporary that dies at the semicolon loads nothing.

```cpp
const auto aggregate_info = exception::augment_aggregate(aggregate.name);
const auto where = exception::augment_where({
    .path = file_path,
    .version = version_index
});
```

Load on the widest scope that still has the right value: the aggregate for a whole aggregate, the version for one version, the path for one statement. An inner load of the same type wins. A later load of a type that is already present is ignored.

On a Taskflow worker the load is inside the `leaf::try_capture_all` lambda, as `fail()` does in `galimatron/translator_library/transforms/cpp2cpp_validate/cpp2cpp_validate.cpp`. A load in the caller attaches to `captured_leaf_exception`, and `try_catch` unpacks the captured result without those values. Pass `exception::error_where_t` into `fail`.

## Throw

```cpp
leaf::throw_exception(
    exception::domain::runtime_library{},
    exception::subdomain::runtime_library::filesystem{},
    exception::details::io_operation_et::READ,
    exception::e_path_info_for(path)
);
```

On a Taskflow worker there is no active leaf context. Capture, then throw `captured_leaf_exception`, as `fail()` does. Otherwise `try_catch` sees only the exception type.

## Catch

Handle these exceptions only through `galimatron::runtime::try_catch` (`galimatron/runtime_library/leaf_spy/leaf_spy.hpp`).

`translator_library` throws and does not catch. The exceptions are `transforms::xml2xml::saxon_session_t::apply` and `apply_primary`: they catch `SaxonApiException` while `newXslt30Processor` is still alive. `throw_translator` reads it with `printout_native_saxon_exception`, passes the path to `leaf::throw_exception` inside `leaf::try_capture_all`, then throws `captured_leaf_exception`. Same constraint on both functions in `transforms/xml2xml/detail/xml2xml_impl.cpp`.

## Tests

Failure tests call `print_leaf_exception` from `galimatron/tests/utils.hpp`. It runs the check inside `try_catch` with `is_throw_out = true`. Do not use `REGISTER_EXCEPTION_TRANSLATOR` for leaf exceptions.

```cpp
print_leaf_exception([&]
{
    // call that must throw
});
```

A handler that must see the location takes those payloads by reference. A missing payload then fails the match.

```cpp
[&](
    const exception::domain::translator_library&,
    const exception::subdomain::translator_library::compiler&,
    const exception::e_what_info& info,
    const exception::e_path_info& path,
    const exception::e_aggregate_info& aggregate,
    const exception::e_version_info& version
)
{
    CHECK(info.what == expected_what);
    REQUIRE(path.path);
    CHECK(path.path->lexically_normal() == expected_path);
    CHECK(aggregate.name == expected_aggregate);
    CHECK(version.version == expected_version);
}
```
