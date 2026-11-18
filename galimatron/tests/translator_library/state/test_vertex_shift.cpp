// galimatron
#include "galimatron/translator_library/state/holder/vertex_shift.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


using namespace galimatron::translator::state;
using galimatron::translator::state::holder::vertex_shift_t;

TEST_SUITE("galimatron/translator_library/state/holder/vertex_shift" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

TEST_CASE("a descriptor moves down by the removed vertices below it")
{
    const vertex_shift_t shift{ { 7, 2, 4 } };

    CHECK(shift(0) == 0);
    CHECK(shift(1) == 1);
    CHECK(shift(3) == 2);
    CHECK(shift(5) == 3);
    CHECK(shift(6) == 4);
    CHECK(shift(8) == 5);
    CHECK(shift(100) == 97);
}

TEST_CASE("a removed vertex becomes absent")
{
    const vertex_shift_t shift{ { 7, 2, 4 } };

    CHECK(shift(2) == vertex_descriptor_not_present);
    CHECK(shift(4) == vertex_descriptor_not_present);
    CHECK(shift(7) == vertex_descriptor_not_present);
}

TEST_CASE("an absent descriptor stays absent, and a repeated removal counts once")
{
    const vertex_shift_t shift{ { 3, 3, 3 } };

    CHECK(shift(vertex_descriptor_not_present) == vertex_descriptor_not_present);
    CHECK(shift(4) == 3);
    CHECK_FALSE(shift.is_empty());
    CHECK(vertex_shift_t{ vertex_descriptors_vector_t{} }.is_empty());
}

TEST_CASE("an optional descriptor")
{
    const vertex_shift_t shift{ { 1 } };

    vertex_descriptor_ot moved{ 5 };
    holder::pull_down(moved, shift);
    REQUIRE(moved);
    CHECK(*moved == 4);

    vertex_descriptor_ot removed{ 1 };
    holder::pull_down(removed, shift);
    CHECK_FALSE(removed);

    vertex_descriptor_ot empty;
    holder::pull_down(empty, shift);
    CHECK_FALSE(empty);
}

}
