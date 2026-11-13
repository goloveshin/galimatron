// global
#include "global/tests/forward.hpp"

/*

Our understanding of using boost.leaf, plus tests for it.

Dynamic addition of handlers?
    yes, through a tuple, per the docs

Can we build a hierarchy of contexts/refinements?
    yes, this is called 'augmentation' (leaf::on_error)
    ! but it ignores repeated types -> a full history of contexts by type is not possible

What happens on throw(a,b,c) -> catch(a){ throw }
    a is removed from the rethrow

Does catching depend on the order: throw(a,b,c) -> catch(c,b,a) ?
    no

Various cases of catching
    throw(a,b,c) -> catch(a,b,c,c)
        will catch

    throw(a,b,c) -> catch(a,b,c,d)
        will not catch

    throw(a,b,c) -> catch(a,b,c,d*)
        will catch, d = nullpr

Does catching depend on which types came from leaf::throw_exception and which from contexts?
    no

Must something be caught specifically?
    no, everything can be inspected through leaf::diagnostic_details

Can the order of entries in 'Diagnostic details' inside leaf::diagnostic_details be read programmatically?
    no, there is no way to iterate them

*/

namespace global
{

struct test_exception_mark_t{};

enum class context_a
{
    value_1 = 1,
    value_2 = 2
};

enum class context_b
{
    value_1 = 1,
    value_2 = 2
};

enum class context_c
{
    value_1 = 1,
    value_2 = 2
};

enum class context_d
{
    value_1 = 1,
    value_2 = 2
};

std::string normalize_diagnostic_info(const std::string& info)
{
    static std::regex error_with_serial("Error with serial #\\d+");
    static std::regex unknown_exception("Unknown exception");
    static std::regex empty_lines("\\n{2,}");
    const auto tmp1 = std::regex_replace(info, error_with_serial, "");
    const auto tmp2 = std::regex_replace(tmp1, unknown_exception, "std::exception");
    return std::regex_replace(tmp2, empty_lines, "\n");
}

// run test_func and catch everything with leaf::diagnostic_details
//
template <typename TestFunc>
void test(
    // run
    TestFunc&& test_func,
    // if this expectation is set (a non-empty string), only check it; if it is not set, write the actual result to the log
    const std::string& expected_diagnostic_details
)
{
    std::stringstream ss;

    leaf::try_catch(
        std::forward<TestFunc>(test_func),
        [&](const leaf::diagnostic_details& info)
        {
            ss << std::endl << info;
        }
    );

    const auto actual_info = normalize_diagnostic_info(ss.str());

    CHECK(actual_info == expected_diagnostic_details);
}

// run test_func, try to catch with catch_func, and globally catch everything with leaf::diagnostic_details
//
template <typename TestFunc, typename CatchFunc>
void test(
    // run
    TestFunc&& test_func,
    // the first handler
    CatchFunc&& catch_func,
    // whether the handler is expected to fire
    const bool is_expected_catched,
    // if this expectation is set (a non-empty string), only check it; if it is not set, write the actual result to the log
    const std::string& expected_diagnostic_details
)
{
    using catch_func_t = decltype(std::function{std::forward<CatchFunc>(catch_func)});

    bool is_actual_catched = false;
    std::stringstream ss;

    leaf::try_catch(
        [&]
        {
            leaf::try_catch(
                std::forward<TestFunc>(test_func),
                [&]<typename R, typename ... Args>(std::function<R (Args...)>)
                {
                    return [&](Args... args)// Args&& ... args -> problem with pointer
                    {
                        is_actual_catched = true;
                        std::forward<CatchFunc>(catch_func)(args...);
                    };
                }(catch_func_t{})
            );
        },
        [&](const leaf::diagnostic_details& info)
        {
            ss << std::endl << info;
        }
    );

    CHECK(is_actual_catched == is_expected_catched);

    const auto actual_info = normalize_diagnostic_info(ss.str());

    if (!expected_diagnostic_details.empty())
    {
        CHECK(actual_info == expected_diagnostic_details);
    }
    else
    {
        std::cout << actual_info << std::endl;
    }
}

// the tests are meant to confirm our understanding of how boost.leaf behaves
//
TEST_SUITE("global/leaf_spy_library/boost.leaf")
{

// below, contexts are listed in creation order: a(1) -> b(1) - a was created first, then b

// one augmentation of different types at one level
// a(1) -> b(1) -> c(1) => { c(1), b(1), a(1) }
//
TEST_CASE("augmenting 1")
{
    auto func_c = [&]
    {
        auto load = leaf::on_error(context_c{ context_c::value_1 });

        leaf::throw_exception(test_exception_mark_t{});
    };

    auto func_b = [&]
    {
        auto load = leaf::on_error(context_b{ context_b::value_1 });

        func_c();
    };

    auto func_a = [&]
    {
        auto load = leaf::on_error(context_a{ context_a::value_1 });

        func_b();
    };

    test(
        [&]
        {
            func_a();
        },
R"(
Caught:
    std::exception: "std::exception"
Diagnostic details:
    global::test_exception_mark_t
    global::context_c: 1
    global::context_b: 1
    global::context_a: 1
)"
    );
}

// two augmentations of the same type at one level -> the first one from the exception side is taken
// a(1) -> a(2) -> b(1) -> b(2) -> c(1) -> c(2) => { c(2), b(2), a(2) }
//
TEST_CASE("augmenting 2")
{
    auto func_c = [&]
    {
        auto load_1 = leaf::on_error(context_c{ context_c::value_1 });
        auto load_2 = leaf::on_error(context_c{ context_c::value_2 });

        leaf::throw_exception(test_exception_mark_t{});
    };

    auto func_b = [&]
    {
        auto load_1 = leaf::on_error(context_b{ context_b::value_1 });
        auto load_2 = leaf::on_error(context_b{ context_b::value_2 });

        func_c();
    };

    auto func_a = [&]
    {
        auto load1 = leaf::on_error(context_a{ context_a::value_1 });
        auto load2 = leaf::on_error(context_a{ context_a::value_2 });

        func_b();
    };

    test(
        [&]
        {
            func_a();
        },
R"(
Caught:
    std::exception: "std::exception"
Diagnostic details:
    global::test_exception_mark_t
    global::context_c: 2
    global::context_b: 2
    global::context_a: 2
)"
    );
}

// two augmentations of the same type at different levels -> the first one from the exception side is taken
// a(2) -> b(1) -> a(1) -> c(1) => { c(1), a(1), b(1) }
//
TEST_CASE("augmenting 3")
{
    auto func_c = [&]
    {
        auto load = leaf::on_error(context_c{ context_c::value_1 });

        leaf::throw_exception(test_exception_mark_t{});
    };

    auto func_b = [&]
    {
        auto load1 = leaf::on_error(context_b{ context_b::value_1 });
        auto load2 = leaf::on_error(context_a{ context_a::value_1 });

        func_c();
    };

    auto func_a = [&]
    {
        // will be discarded, because func_b() already took context_a
        auto load = leaf::on_error(context_a{ context_a::value_2 });

        func_b();
    };

    test(
        [&]
        {
            func_a();
        },
R"(
Caught:
    std::exception: "std::exception"
Diagnostic details:
    global::test_exception_mark_t
    global::context_c: 1
    global::context_a: 1
    global::context_b: 1
)"
    );
}

// augmentation with two types at once, at one level
//
TEST_CASE("augmenting 4")
{
    auto func_c = [&]
    {
        auto load = leaf::on_error(context_c{ context_c::value_1 });

        leaf::throw_exception(test_exception_mark_t{});
    };

    auto func_b = [&]
    {
        auto load = leaf::on_error(context_b{ context_b::value_1 }, context_a{ context_a::value_1 });

        func_c();
    };

    auto func_a = [&]
    {
        func_b();
    };

    test(
        [&]
        {
            func_a();
        },
R"(
Caught:
    std::exception: "std::exception"
Diagnostic details:
    global::test_exception_mark_t
    global::context_c: 1
    global::context_b: 1
    global::context_a: 1
)"
    );
}

// catch by test_exception_mark_t -> it is not thrown further
//
TEST_CASE("augmenting and explicit catch by mark and rethrow")
{
    auto func_c = [&]
    {
        auto load = leaf::on_error(context_c{ context_c::value_1 });

        leaf::throw_exception(test_exception_mark_t{});
    };

    auto func_b = [&]
    {
        auto load = leaf::on_error(context_b{ context_b::value_1 });

        func_c();
    };

    auto func_a = [&]
    {
        auto load = leaf::on_error(context_a{ context_a::value_1 });

        func_b();
    };

    test(
        [&]
        {
            func_a();
        },
        [&](const test_exception_mark_t& info)
        {
            throw;
        },
        true,
R"(
Caught:
    std::exception: "std::exception"
Diagnostic details:
    global::context_c: 1
    global::context_b: 1
    global::context_a: 1
)"
    );
}

// various orders and compositions
//
TEST_CASE("different sequences in throw and catch")
{
    auto func_c = [&]
    {
        auto load = leaf::on_error(context_c{ context_c::value_1 });

        leaf::throw_exception(test_exception_mark_t{});
    };

    auto func_b = [&]
    {
        auto load = leaf::on_error(context_b{ context_b::value_1 });

        func_c();
    };

    auto func_a = [&]
    {
        auto load = leaf::on_error(context_a{ context_a::value_1 });

        func_b();
    };

    // exact match -> will catch
    //
    SUBCASE("exactly: throw(c,b,a) -> catch(c,b,a)")
    {
        test(
            [&]
            {
                func_a();
            },
            [&](const context_c&, const context_b&, const context_a&)
            {
                throw;
            },
            true,
R"(
Caught:
    std::exception: "std::exception"
Diagnostic details:
    global::test_exception_mark_t
)"
        );
    }

    // a different order -> will catch
    //
    SUBCASE("different: throw(c,b,a) -> catch(a,b,c)")
    {
        test(
            [&]
            {
                func_a();
            },
            [&](const context_a&, const context_c&, const context_b&)
            {
                throw;
            },
            true,
R"(
Caught:
    std::exception: "std::exception"
Diagnostic details:
    global::test_exception_mark_t
)"
        );
    }

    // a different composition with a duplicate -> will catch
    //
    SUBCASE("duplicate: throw(c,b,a) -> catch(c,b,a,a)")
    {
        test(
            [&]
            {
                func_a();
            },
            [&](const context_c&, const context_b&, const context_a&, const context_a&)
            {
                throw;
            },
            true,
R"(
Caught:
    std::exception: "std::exception"
Diagnostic details:
    global::test_exception_mark_t
)"
        );
    }

    // no such type, and it is required -> will not catch
    //
    SUBCASE("no such type and must: throw(c,b,a) -> catch(c,b,a,d)")
    {
        test(
            [&]
            {
                func_a();
            },
            [&](const context_c&, const context_b&, const context_a&, const context_d&)
            {
                throw;
            },
            false,
R"(
Caught:
    std::exception: "std::exception"
Diagnostic details:
    global::test_exception_mark_t
    global::context_a: 1
    global::context_b: 1
    global::context_c: 1
)"
        );
    }

    // no such type, and it is optional -> will catch, d == nullptr
    //
    SUBCASE("no such type and maybe: throw(c,b,a) -> catch(c,b,a,d*)")
    {
        test(
            [&]
            {
                func_a();
            },
            [&](const context_c&, const context_b&, const context_a&, const context_d* d)
            {
                CHECK(d == nullptr);
                throw;
            },
            true,
R"(
Caught:
    std::exception: "std::exception"
Diagnostic details:
    global::test_exception_mark_t
)"
        );
    }

    // no such type, and it is optional -> will catch, d == nullptr
    //
    SUBCASE("no such type and maybe: throw(c,b,a) -> catch(d*,c,b,a)")
    {
        test(
            [&]
            {
                func_a();
            },
            [&](const context_d* d, const context_c&, const context_b&, const context_a&)
            {
                CHECK(d == nullptr);
                throw;
            },
            true,
R"(
Caught:
    std::exception: "std::exception"
Diagnostic details:
    global::test_exception_mark_t
)"
        );
    }
}

// catching does not depend on where the type came from - throw_exception or on_error
//
TEST_CASE("types in throw_exception() and on_error() are equivalent")
{
    auto func_c = [&]
    {
        auto load = leaf::on_error(context_c{ context_c::value_1 });

        leaf::throw_exception(context_b{ context_b::value_1 });
    };

    auto func_b = [&]
    {
        auto load = leaf::on_error(test_exception_mark_t{});

        func_c();
    };

    auto func_a = [&]
    {
        auto load = leaf::on_error(context_a{ context_a::value_1 });

        func_b();
    };

    test(
        [&]
        {
            func_a();
        },
        [&](const context_b&, const context_c&, const test_exception_mark_t&, const context_a&)
        {
            throw;
        },
        true,
R"(
Caught:
    std::exception: "std::exception"
)"
    );
}

}

}// namespace global
