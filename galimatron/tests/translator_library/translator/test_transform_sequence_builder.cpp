// galimatron
#include "galimatron/translator_library/translator/detail/transform_sequence_builder.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

namespace tsb = translator::translator::transform_sequence_builder;

namespace
{

struct stored_transition_t
{
    tsb::domain_and_state_t incoming;
    tsb::domain_and_state_t outgoing;
    int id = 0;
};

struct fixture_t
{
    tsb::builder_t::container_t state_transforms;
    tsb::builder_t::container_t transform_sequence;
    tsb::builder_t::builder_args_t args;

    fixture_t()
        : args{
            .state_transforms = state_transforms,
            .incoming_domain_and_state = [](const std::any& item)
            {
                return std::any_cast<const stored_transition_t&>(item).incoming;
            },
            .outgoing_domain_and_state = [](const std::any& item)
            {
                return std::any_cast<const stored_transition_t&>(item).outgoing;
            },
            .transform_sequence = transform_sequence
        }
    {
    }

    void add(
        const tsb::domain_et in_domain,
        const tsb::state_type_et in_state,
        const tsb::domain_et out_domain,
        const tsb::state_type_et out_state
    )
    {
        state_transforms.push_back(stored_transition_t{
            .incoming = { .domain = in_domain, .state_type = in_state },
            .outgoing = { .domain = out_domain, .state_type = out_state }
        });
    }

    void add_sequence(
        const tsb::domain_et in_domain,
        const tsb::state_type_et in_state,
        const tsb::domain_et out_domain,
        const tsb::state_type_et out_state,
        const int id = 0
    )
    {
        transform_sequence.push_back(stored_transition_t{
            .incoming = { .domain = in_domain, .state_type = in_state },
            .outgoing = { .domain = out_domain, .state_type = out_state },
            .id = id
        });
    }

    void add_sequence(const tsb::domain_et in_domain, const tsb::domain_et out_domain)
    {
        add_sequence(in_domain, tsb::state_type_et::XML, out_domain, tsb::state_type_et::XML);
    }
};

void expect_error(
    const std::function<void ()>& action,
    const std::string_view expected_what
)
{
    bool caught = false;

    leaf::try_catch(
        action,
        [&](
            const exception::domain::translator_library&,
            const exception::subdomain::translator_library::translator&,
            const exception::e_what_info& info
        )
        {
            caught = true;
            CHECK(info.what == expected_what);
        }
    );

    CHECK(caught);
}

void expect_absent(const tsb::builder_t::builder_args_t& args, const std::string_view expected_what)
{
    expect_error(
        [&]{ tsb::builder_t::check_state_transforms(args); },
        expected_what
    );
}

void expect_sequence(const tsb::builder_t::builder_args_t& args, const std::string_view expected_what)
{
    expect_error(
        [&]{ tsb::builder_t::check_transform_sequence(args); },
        expected_what
    );
}

void expect_state_types(const tsb::builder_t::builder_args_t& args, const std::string_view expected_what)
{
    expect_error(
        [&]{ tsb::builder_t::check_transform_sequence_state_types(args); },
        expected_what
    );
}

void add_required_state_transforms(fixture_t& fixture)
{
    using enum tsb::domain_et;
    using enum tsb::state_type_et;

    fixture.add(LANGUAGE, GaL, LANGUAGE, CPP);
    fixture.add(LANGUAGE, CPP, LANGUAGE, GaL);
    fixture.add(LANGUAGE, CPP, LANGUAGE, XML);
    fixture.add(LANGUAGE, XML, LANGUAGE, SAXON);
    fixture.add(LANGUAGE, SAXON, LANGUAGE, CPP);
    fixture.add(LANGUAGE, CPP, POST_LANGUAGE, CPP);
    fixture.add(POST_LANGUAGE, CPP, POST_LANGUAGE, XML);
}

std::string sequence_text(const fixture_t& fixture)
{
    std::string text;
    for (const auto& item : fixture.transform_sequence)
    {
        const auto stored = std::any_cast<stored_transition_t>(item);
        if (!text.empty())
        {
            text += " | ";
        }
        text += std::format("{} {} -> {} {}",
            magic_enum::enum_name(stored.incoming.domain),
            magic_enum::enum_name(stored.incoming.state_type),
            magic_enum::enum_name(stored.outgoing.domain),
            magic_enum::enum_name(stored.outgoing.state_type)
        );
        if (stored.id != 0)
        {
            text += std::format(" #{}", stored.id);
        }
    }
    return text;
}

}// namespace

TEST_SUITE("galimatron/translator_library/translator/transform_sequence_builder" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

TEST_CASE("check_state_transforms")
{
    using enum tsb::domain_et;
    using enum tsb::state_type_et;

    SUBCASE("all required state transitions")
    {
        fixture_t fixture;
        fixture.add(LANGUAGE, GaL, LANGUAGE, CPP);
        fixture.add(LANGUAGE, CPP, LANGUAGE, GaL);
        fixture.add(LANGUAGE, CPP, LANGUAGE, XML);
        fixture.add(LANGUAGE, XML, LANGUAGE, SAXON);
        fixture.add(LANGUAGE, SAXON, LANGUAGE, CPP);
        fixture.add(LANGUAGE, CPP, POST_LANGUAGE, CPP);
        fixture.add(POST_LANGUAGE, CPP, POST_LANGUAGE, XML);

        CHECK_NOTHROW(tsb::builder_t::check_state_transforms(fixture.args));
    }

    SUBCASE("one transition is absent")
    {
        fixture_t fixture;
        fixture.add(LANGUAGE, GaL, LANGUAGE, CPP);
        fixture.add(LANGUAGE, CPP, LANGUAGE, GaL);
        fixture.add(LANGUAGE, CPP, LANGUAGE, XML);
        fixture.add(LANGUAGE, SAXON, LANGUAGE, CPP);
        fixture.add(LANGUAGE, CPP, POST_LANGUAGE, CPP);
        fixture.add(POST_LANGUAGE, CPP, POST_LANGUAGE, XML);

        expect_absent(fixture.args, "state transforms are absent: LANGUAGE XML -> LANGUAGE SAXON");
    }

    SUBCASE("transition has another domain")
    {
        fixture_t fixture;
        fixture.add(POST_LANGUAGE, GaL, POST_LANGUAGE, CPP);
        fixture.add(LANGUAGE, CPP, LANGUAGE, GaL);
        fixture.add(LANGUAGE, CPP, LANGUAGE, XML);
        fixture.add(LANGUAGE, XML, LANGUAGE, SAXON);
        fixture.add(LANGUAGE, SAXON, LANGUAGE, CPP);
        fixture.add(LANGUAGE, CPP, POST_LANGUAGE, CPP);
        fixture.add(POST_LANGUAGE, CPP, POST_LANGUAGE, XML);

        expect_absent(fixture.args, "state transforms are absent: LANGUAGE GaL -> LANGUAGE CPP");
    }

    SUBCASE("LANGUAGE -> POST_LANGUAGE is not CPP -> CPP then CPP -> XML")
    {
        fixture_t fixture;
        fixture.add(LANGUAGE, GaL, LANGUAGE, CPP);
        fixture.add(LANGUAGE, CPP, LANGUAGE, GaL);
        fixture.add(LANGUAGE, CPP, LANGUAGE, XML);
        fixture.add(LANGUAGE, XML, LANGUAGE, SAXON);
        fixture.add(LANGUAGE, SAXON, LANGUAGE, CPP);
        fixture.add(LANGUAGE, GaL, POST_LANGUAGE, CPP);

        expect_absent(
            fixture.args,
            "state transforms are absent: LANGUAGE CPP -> POST_LANGUAGE CPP, POST_LANGUAGE CPP -> POST_LANGUAGE XML"
        );
    }

    SUBCASE("several transitions are absent")
    {
        fixture_t fixture;
        fixture.add(LANGUAGE, GaL, POST_LANGUAGE, CPP);
        fixture.add(LANGUAGE, CPP, LANGUAGE, XML);

        expect_absent(
            fixture.args,
            "state transforms are absent: LANGUAGE GaL -> LANGUAGE CPP, LANGUAGE CPP -> LANGUAGE GaL, LANGUAGE XML -> LANGUAGE SAXON, LANGUAGE SAXON -> LANGUAGE CPP, LANGUAGE CPP -> POST_LANGUAGE CPP, POST_LANGUAGE CPP -> POST_LANGUAGE XML"
        );
    }

    SUBCASE("state transforms are empty")
    {
        const fixture_t fixture;

        expect_absent(
            fixture.args,
            "state transforms are absent: LANGUAGE GaL -> LANGUAGE CPP, LANGUAGE CPP -> LANGUAGE GaL, LANGUAGE CPP -> LANGUAGE XML, LANGUAGE XML -> LANGUAGE SAXON, LANGUAGE SAXON -> LANGUAGE CPP, LANGUAGE CPP -> POST_LANGUAGE CPP, POST_LANGUAGE CPP -> POST_LANGUAGE XML"
        );
    }
}

TEST_CASE("check_transform_sequence")
{
    using enum tsb::domain_et;

    const auto not_continuous = "transform sequence domains are not continuous: POST_LANGUAGE -> LANGUAGE";

    SUBCASE("both domain sets are absent")
    {
        const fixture_t fixture;

        CHECK_NOTHROW(tsb::builder_t::check_transform_sequence(fixture.args));
    }

    SUBCASE("only LANGUAGE")
    {
        fixture_t fixture;
        fixture.add_sequence(LANGUAGE, LANGUAGE);
        fixture.add_sequence(LANGUAGE, LANGUAGE);

        CHECK_NOTHROW(tsb::builder_t::check_transform_sequence(fixture.args));
    }

    SUBCASE("only POST_LANGUAGE")
    {
        fixture_t fixture;
        fixture.add_sequence(POST_LANGUAGE, POST_LANGUAGE);
        fixture.add_sequence(POST_LANGUAGE, POST_LANGUAGE);

        CHECK_NOTHROW(tsb::builder_t::check_transform_sequence(fixture.args));
    }

    SUBCASE("LANGUAGE then POST_LANGUAGE")
    {
        fixture_t fixture;
        fixture.add_sequence(LANGUAGE, LANGUAGE);
        fixture.add_sequence(LANGUAGE, POST_LANGUAGE);
        fixture.add_sequence(POST_LANGUAGE, POST_LANGUAGE);

        CHECK_NOTHROW(tsb::builder_t::check_transform_sequence(fixture.args));
    }

    SUBCASE("POST_LANGUAGE then LANGUAGE")
    {
        fixture_t fixture;
        fixture.add_sequence(POST_LANGUAGE, POST_LANGUAGE);
        fixture.add_sequence(LANGUAGE, LANGUAGE);

        expect_sequence(fixture.args, not_continuous);
    }

    SUBCASE("return to LANGUAGE")
    {
        fixture_t fixture;
        fixture.add_sequence(LANGUAGE, LANGUAGE);
        fixture.add_sequence(POST_LANGUAGE, POST_LANGUAGE);
        fixture.add_sequence(LANGUAGE, LANGUAGE);

        expect_sequence(fixture.args, not_continuous);
    }

    SUBCASE("transform goes back to LANGUAGE")
    {
        fixture_t fixture;
        fixture.add_sequence(POST_LANGUAGE, LANGUAGE);

        expect_sequence(fixture.args, not_continuous);
    }
}

TEST_CASE("check_transform_sequence_state_types")
{
    using enum tsb::domain_et;
    using enum tsb::state_type_et;

    SUBCASE("state types are equal")
    {
        fixture_t fixture;
        fixture.add_sequence(LANGUAGE, GaL, LANGUAGE, GaL);
        fixture.add_sequence(LANGUAGE, XML, POST_LANGUAGE, XML);
        fixture.add_sequence(POST_LANGUAGE, CPP, POST_LANGUAGE, CPP);

        CHECK_NOTHROW(tsb::builder_t::check_transform_sequence_state_types(fixture.args));
    }

    SUBCASE("transform sequence is empty")
    {
        const fixture_t fixture;

        CHECK_NOTHROW(tsb::builder_t::check_transform_sequence_state_types(fixture.args));
    }

    SUBCASE("one state type differs")
    {
        fixture_t fixture;
        fixture.add_sequence(LANGUAGE, XML, LANGUAGE, XML);
        fixture.add_sequence(LANGUAGE, GaL, LANGUAGE, CPP);

        expect_state_types(fixture.args, "transform sequence state types differ: GaL -> CPP");
    }

    SUBCASE("several state types differ")
    {
        fixture_t fixture;
        fixture.add_sequence(LANGUAGE, GaL, LANGUAGE, CPP);
        fixture.add_sequence(LANGUAGE, XML, POST_LANGUAGE, XML);
        fixture.add_sequence(POST_LANGUAGE, XML, POST_LANGUAGE, GaL);

        expect_state_types(
            fixture.args,
            "transform sequence state types differ: GaL -> CPP, XML -> GaL"
        );
    }
}

TEST_CASE("complete_transform_sequence")
{
    using enum tsb::domain_et;
    using enum tsb::state_type_et;

    const auto entry =
        "LANGUAGE GaL -> LANGUAGE CPP"
        " | LANGUAGE CPP -> LANGUAGE GaL"
        " | LANGUAGE GaL -> LANGUAGE CPP"
        " | LANGUAGE CPP -> POST_LANGUAGE CPP"
        " | POST_LANGUAGE CPP -> POST_LANGUAGE XML";

    SUBCASE("empty sequence")
    {
        fixture_t fixture;
        add_required_state_transforms(fixture);

        tsb::builder_t::complete_transform_sequence(fixture.args);

        CHECK(sequence_text(fixture) == entry);
    }

    SUBCASE("no language xslt does not adopt into Saxon")
    {
        fixture_t fixture;
        add_required_state_transforms(fixture);

        tsb::builder_t::complete_transform_sequence(fixture.args);

        CHECK(sequence_text(fixture) == entry);
    }

    SUBCASE("language transform is kept and bridged")
    {
        fixture_t fixture;
        add_required_state_transforms(fixture);
        fixture.add_sequence(LANGUAGE, SAXON, LANGUAGE, SAXON, 1);

        tsb::builder_t::complete_transform_sequence(fixture.args);

        CHECK(sequence_text(fixture) ==
            "LANGUAGE GaL -> LANGUAGE CPP"
            " | LANGUAGE CPP -> LANGUAGE XML"
            " | LANGUAGE XML -> LANGUAGE SAXON"
            " | LANGUAGE SAXON -> LANGUAGE SAXON #1"
            " | LANGUAGE SAXON -> LANGUAGE CPP"
            " | LANGUAGE CPP -> LANGUAGE GaL"
            " | LANGUAGE GaL -> LANGUAGE CPP"
            " | LANGUAGE CPP -> POST_LANGUAGE CPP"
            " | POST_LANGUAGE CPP -> POST_LANGUAGE XML"
        );
    }

    SUBCASE("original order is preserved")
    {
        fixture_t fixture;
        add_required_state_transforms(fixture);
        fixture.add_sequence(LANGUAGE, SAXON, LANGUAGE, SAXON, 1);
        fixture.add_sequence(LANGUAGE, CPP, LANGUAGE, CPP, 2);
        fixture.add_sequence(POST_LANGUAGE, XML, POST_LANGUAGE, XML, 3);

        tsb::builder_t::complete_transform_sequence(fixture.args);

        CHECK(sequence_text(fixture) ==
            "LANGUAGE GaL -> LANGUAGE CPP"
            " | LANGUAGE CPP -> LANGUAGE XML"
            " | LANGUAGE XML -> LANGUAGE SAXON"
            " | LANGUAGE SAXON -> LANGUAGE SAXON #1"
            " | LANGUAGE SAXON -> LANGUAGE CPP"
            " | LANGUAGE CPP -> LANGUAGE CPP #2"
            " | LANGUAGE CPP -> LANGUAGE GaL"
            " | LANGUAGE GaL -> LANGUAGE CPP"
            " | LANGUAGE CPP -> POST_LANGUAGE CPP"
            " | POST_LANGUAGE CPP -> POST_LANGUAGE XML"
            " | POST_LANGUAGE XML -> POST_LANGUAGE XML #3"
        );
    }

    SUBCASE("no post xslt does not adopt into Saxon")
    {
        fixture_t fixture;
        add_required_state_transforms(fixture);
        fixture.add(POST_LANGUAGE, XML, POST_LANGUAGE, SAXON);

        tsb::builder_t::complete_transform_sequence(fixture.args);

        CHECK(sequence_text(fixture) == entry);
    }

    SUBCASE("post xslt adopts into Saxon")
    {
        fixture_t fixture;
        add_required_state_transforms(fixture);
        fixture.add(POST_LANGUAGE, XML, POST_LANGUAGE, SAXON);
        fixture.add_sequence(POST_LANGUAGE, SAXON, POST_LANGUAGE, SAXON, 1);

        tsb::builder_t::complete_transform_sequence(fixture.args);

        CHECK(sequence_text(fixture) ==
            std::string{ entry }
            + " | POST_LANGUAGE XML -> POST_LANGUAGE SAXON"
            " | POST_LANGUAGE SAXON -> POST_LANGUAGE SAXON #1"
        );
    }

    SUBCASE("post transform only")
    {
        fixture_t fixture;
        add_required_state_transforms(fixture);
        fixture.add_sequence(POST_LANGUAGE, XML, POST_LANGUAGE, XML, 1);

        tsb::builder_t::complete_transform_sequence(fixture.args);

        CHECK(sequence_text(fixture) ==
            std::string{ entry } + " | POST_LANGUAGE XML -> POST_LANGUAGE XML #1"
        );
    }

    SUBCASE("cannot reach XML")
    {
        fixture_t fixture;
        add_required_state_transforms(fixture);
        fixture.add_sequence(POST_LANGUAGE, GaL, POST_LANGUAGE, GaL);

        expect_error(
            [&]{ tsb::builder_t::complete_transform_sequence(fixture.args); },
            "transform sequence cannot be completed: POST_LANGUAGE GaL -> POST_LANGUAGE XML"
        );
    }
}

}

}// namespace galimatron
