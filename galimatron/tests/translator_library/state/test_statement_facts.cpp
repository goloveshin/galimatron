// galimatron
#include "galimatron/translator_library/state/statement_facts.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


using namespace galimatron::translator::state;

TEST_SUITE("galimatron/translator_library/state/statement_facts" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

TEST_CASE("a tag with a name names a tag")
{
    const entity_t entity = tag_statement_t{ .index_in_version = 3, .name = "price" };
    const auto facts = statement_facts(entity);

    REQUIRE(facts);
    CHECK(facts->name == "price");
    CHECK(facts->index_in_version == 3);
    CHECK(facts->names_tag);
}

TEST_CASE("an unnamed tag has no name and names no tag")
{
    const entity_t entity = tag_statement_t{ .index_in_version = 1 };
    const auto facts = statement_facts(entity);

    REQUIRE(facts);
    CHECK_FALSE(facts->name);
    CHECK_FALSE(facts->names_tag);
}

TEST_CASE("a label and an ordering carry a name but name no tag")
{
    const entity_t label = label_statement_t{ .index_in_version = 2, .name = "field" };
    const auto label_facts = statement_facts(label);
    REQUIRE(label_facts);
    CHECK(label_facts->name == "field");
    CHECK(label_facts->index_in_version == 2);
    CHECK_FALSE(label_facts->names_tag);

    const entity_t ordering = ordering_statement_t{ .index_in_version = 4, .name = "shelf" };
    const auto ordering_facts = statement_facts(ordering);
    REQUIRE(ordering_facts);
    CHECK(ordering_facts->name == "shelf");
    CHECK(ordering_facts->index_in_version == 4);
    CHECK_FALSE(ordering_facts->names_tag);
}

TEST_CASE("a delete of a tag names a tag, a delete of a version does not")
{
    const entity_t tag_delete = operation_statement_t{ .index_in_version = 5, .name = "price", .is_version = false };
    const auto tag_facts = statement_facts(tag_delete);
    REQUIRE(tag_facts);
    CHECK(tag_facts->name == "price");
    CHECK(tag_facts->names_tag);

    const entity_t version_delete = operation_statement_t{ .index_in_version = 0, .name = "", .is_version = true };
    const auto version_facts = statement_facts(version_delete);
    REQUIRE(version_facts);
    CHECK_FALSE(version_facts->names_tag);
}

TEST_CASE("an entity that is not a statement has no facts")
{
    entity_t entity = root_t{};

    CHECK_FALSE(statement_facts(entity));
    CHECK_FALSE(set_index_in_version(entity, 7));
}

TEST_CASE("the index in the version is set on every kind of statement")
{
    entity_t tag = tag_statement_t{ .index_in_version = 0, .name = "price" };
    entity_t label = label_statement_t{ .index_in_version = 0, .name = "field" };
    entity_t ordering = ordering_statement_t{ .index_in_version = 0, .name = "shelf" };
    entity_t operation = operation_statement_t{ .index_in_version = 0, .name = "price" };

    for (auto* entity : { &tag, &label, &ordering, &operation })
    {
        REQUIRE(set_index_in_version(*entity, 9));
        CHECK(statement_facts(*entity)->index_in_version == 9);
    }
}

}
