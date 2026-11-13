// galimatron
#include "galimatron/translator_library/translator/detail/taskflow_dump.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

TEST_SUITE("galimatron/translator_library/translator/taskflow_dump" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

// the pointer ids are rewritten in the order they first appear. a later use keeps that name
//
TEST_CASE("pointer ids become n1, n2 in appearance order")
{
    const std::string dump =
        "subgraph cluster_p00000000000000AA {\n"
        "label=\"Taskflow: p00000000000000BB\";\n"
        "p00000000000000CC[label=\"gal2cpp\" ];\n"
        "p00000000000000CC -> p00000000000000DD;\n"
        "p00000000000000DD[label=\"cpp2gal\" ];\n"
        "label=\"Subflow: cpp2cpp_validate\";\n"
        "}\n";

    const auto actual = translator::translator::readable_taskflow_dump(dump);

    CHECK(actual ==
        "subgraph cluster_n1 {\n"
        "label=\"Taskflow: n2\";\n"
        "n3[label=\"gal2cpp\" ];\n"
        "n3 -> n4;\n"
        "n4[label=\"cpp2gal\" ];\n"
        "label=\"Subflow: cpp2cpp_validate\";\n"
        "}\n"
    );
}

}

}// namespace galimatron
