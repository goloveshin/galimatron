// global
#include "global/tests/forward.hpp"

// global
#include "global/common_library/utils.hpp"


namespace global
{

namespace
{

struct record_t
{
    std::string name;
    int value;
};

}// namespace

TEST_SUITE("global/common_library/utils")
{

TEST_CASE("processes the first element of each key and skips the repeats")
{
    const std::vector<record_t> records{
        record_t{ .name = "a", .value = 1 },
        record_t{ .name = "b", .value = 2 },
        record_t{ .name = "a", .value = 3 },
        record_t{ .name = "c", .value = 4 },
        record_t{ .name = "b", .value = 5 }
    };

    std::vector<std::string> names;
    std::vector<int> values;
    std::vector<int> states;

    find_and_process_unique(
        records,
        [&](const record_t& record)
        {
            names.push_back(record.name);
            values.push_back(record.value);
            states.push_back(record.value * 2);
        },
        [](const record_t& record)
        {
            return record.name;
        }
    );

    CHECK(names == std::vector<std::string>{ "a", "b", "c" });
    CHECK(values == std::vector<int>{ 1, 2, 4 });
    CHECK(states == std::vector<int>{ 2, 4, 8 });
}

TEST_CASE("does nothing for an empty range")
{
    bool is_called = false;

    find_and_process_unique(
        std::vector<int>{},
        [&](const int){ is_called = true; },
        [](const int value){ return value; }
    );

    CHECK_FALSE(is_called);
}

}// TEST_SUITE

}// namespace global
