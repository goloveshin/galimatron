// global
#include "global/tests/forward.hpp"
#include "global/common_library/private_memory.hpp"


namespace global
{

TEST_SUITE("global/common_library/private_memory")
{

TEST_CASE("private_memory_bytes")
{
    const auto bytes = private_memory_bytes();
    REQUIRE(bytes);
    CHECK(*bytes > 0);
}

}

}// namespace global
