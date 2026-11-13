// galimatron
#include "galimatron/translator_library/translator/detail/transform_sequence_builder.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::translator::transform_sequence_builder
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_tsp);
}


namespace
{

struct state_transition_t
{
    domain_et from_domain;
    state_type_et from_state;
    domain_et to_domain;
    state_type_et to_state;
};
bool has_state_transition(const builder_t::builder_args_t& args, const state_transition_t& transition)
{
    for (const auto& item : args.state_transforms)
    {
        const auto incoming = args.incoming_domain_and_state(item);
        const auto outgoing = args.outgoing_domain_and_state(item);
        if (incoming.domain == transition.from_domain
            && incoming.state_type == transition.from_state
            && outgoing.domain == transition.to_domain
            && outgoing.state_type == transition.to_state)
        {
            return true;
        }
    }
    return false;
}

std::string format_state_transition(const state_transition_t& transition)
{
    return std::format("{} {} -> {} {}",
        magic_enum::enum_name(transition.from_domain),
        magic_enum::enum_name(transition.from_state),
        magic_enum::enum_name(transition.to_domain),
        magic_enum::enum_name(transition.to_state)
    );
}

}// namespace

builder_t::builder_t(const builder_args_t& args)
{
    check_transform_sequence(args);
    if (args.finish_sequence)
    {
        check_state_transforms(args);
        check_transform_sequence_state_types(args);
    }
    complete_transform_sequence(args);
}

void builder_t::check_state_transforms(const builder_args_t& args)
{
    using enum domain_et;
    using enum state_type_et;

    const state_transition_t required[] = {
        { LANGUAGE, GaL, LANGUAGE, CPP },
        { LANGUAGE, CPP, LANGUAGE, GaL },
        { LANGUAGE, CPP, LANGUAGE, XML },
        { LANGUAGE, XML, LANGUAGE, SAXON },
        { LANGUAGE, SAXON, LANGUAGE, CPP },
        { LANGUAGE, CPP, POST_LANGUAGE, CPP },
        { POST_LANGUAGE, CPP, POST_LANGUAGE, XML }
    };

    std::string absent;
    for (const auto& transition : required)
    {
        if (has_state_transition(args, transition))
        {
            continue;
        }

        if (!absent.empty())
        {
            absent += ", ";
        }
        absent += format_state_transition(transition);
    }

    if (absent.empty())
    {
        return;
    }

    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::translator{},
        exception::e_what_info{ std::format("state transforms are absent: {}", absent) }
    );
}

void builder_t::check_transform_sequence(const builder_args_t& args)
{
    bool in_post_language = false;

    const auto fail = [](const domain_et from, const domain_et to)
    {
        leaf::throw_exception(
            exception::domain::translator_library{},
            exception::subdomain::translator_library::translator{},
            exception::e_what_info{ std::format(
                "transform sequence domains are not continuous: {} -> {}",
                magic_enum::enum_name(from),
                magic_enum::enum_name(to)
            ) }
        );
    };

    for (const auto& item : args.transform_sequence)
    {
        const auto incoming = args.incoming_domain_and_state(item).domain;
        const auto outgoing = args.outgoing_domain_and_state(item).domain;

        if (incoming == domain_et::POST_LANGUAGE && outgoing == domain_et::LANGUAGE)
        {
            fail(incoming, outgoing);
        }

        if (in_post_language && incoming == domain_et::LANGUAGE)
        {
            fail(domain_et::POST_LANGUAGE, domain_et::LANGUAGE);
        }

        if (incoming == domain_et::POST_LANGUAGE || outgoing == domain_et::POST_LANGUAGE)
        {
            in_post_language = true;
        }
    }
}

void builder_t::check_transform_sequence_state_types(const builder_args_t& args)
{
    std::string different;
    for (const auto& item : args.transform_sequence)
    {
        const auto incoming = args.incoming_domain_and_state(item).state_type;
        const auto outgoing = args.outgoing_domain_and_state(item).state_type;
        if (incoming == outgoing)
        {
            continue;
        }

        if (!different.empty())
        {
            different += ", ";
        }
        different += std::format("{} -> {}",
            magic_enum::enum_name(incoming),
            magic_enum::enum_name(outgoing)
        );
    }

    if (different.empty())
    {
        return;
    }

    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::translator{},
        exception::e_what_info{ std::format("transform sequence state types differ: {}", different) }
    );
}

namespace
{

struct node_t
{
    domain_et domain;
    state_type_et state;
};

int domain_index(const domain_et domain)
{
    return domain == domain_et::LANGUAGE ? 0 : 1;
}

int state_index(const state_type_et state)
{
    switch (state)
    {
    case state_type_et::GaL:
        return 0;
    case state_type_et::CPP:
        return 1;
    case state_type_et::XML:
        return 2;
    case state_type_et::SAXON:
        return 3;
    }
    return 0;
}

void fail_cannot_complete(const node_t& from, const node_t& to)
{
    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::translator{},
        exception::e_what_info{ std::format(
            "transform sequence cannot be completed: {} {} -> {} {}",
            magic_enum::enum_name(from.domain),
            magic_enum::enum_name(from.state),
            magic_enum::enum_name(to.domain),
            magic_enum::enum_name(to.state)
        ) }
    );
}

bool is_post_transform(const builder_t::builder_args_t& args, const builder_t::container_type_t& item)
{
    const auto incoming = args.incoming_domain_and_state(item);
    const auto outgoing = args.outgoing_domain_and_state(item);
    return incoming.domain == domain_et::POST_LANGUAGE || outgoing.domain == domain_et::POST_LANGUAGE;
}

bool is_language_unparse(const builder_t::builder_args_t& args, const builder_t::container_type_t& item)
{
    const auto incoming = args.incoming_domain_and_state(item);
    const auto outgoing = args.outgoing_domain_and_state(item);
    return incoming.domain == domain_et::LANGUAGE
        && incoming.state_type == state_type_et::CPP
        && outgoing.domain == domain_et::LANGUAGE
        && outgoing.state_type == state_type_et::GaL;
}

builder_t::container_t shortest_path(
    const builder_t::builder_args_t& args,
    const node_t& from,
    const node_t& to,
    const bool language_only
)
{
    if (from.domain == to.domain && from.state == to.state)
    {
        return {};
    }

    struct step_t
    {
        node_t node;
        builder_t::container_t path;
    };

    bool seen[2][4] = {};
    std::list<step_t> queue;
    queue.push_back(step_t{ from, {} });
    seen[domain_index(from.domain)][state_index(from.state)] = true;

    while (!queue.empty())
    {
        const auto step = std::move(queue.front());
        queue.pop_front();

        for (const auto& item : args.state_transforms)
        {
            const auto incoming = args.incoming_domain_and_state(item);
            const auto outgoing = args.outgoing_domain_and_state(item);
            if (incoming.domain != step.node.domain || incoming.state_type != step.node.state)
            {
                continue;
            }
            if (language_only && outgoing.domain != domain_et::LANGUAGE)
            {
                continue;
            }

            const node_t next{ outgoing.domain, outgoing.state_type };
            if (seen[domain_index(next.domain)][state_index(next.state)])
            {
                continue;
            }

            auto next_path = step.path;
            next_path.push_back(item);
            if (next.domain == to.domain && next.state == to.state)
            {
                return next_path;
            }

            seen[domain_index(next.domain)][state_index(next.state)] = true;
            queue.push_back(step_t{ next, std::move(next_path) });
        }
    }

    fail_cannot_complete(from, to);
    return {};
}

// the seed stays. a catalog edge is inserted only where two seed steps do not meet
//
void complete_between_steps(const builder_t::builder_args_t& args)
{
    builder_t::container_t result;
    bool started = false;
    node_t current{ domain_et::LANGUAGE, state_type_et::GaL };

    for (const auto& item : args.transform_sequence)
    {
        const auto incoming = args.incoming_domain_and_state(item);
        const auto outgoing = args.outgoing_domain_and_state(item);
        if (started && current.state != incoming.state_type)
        {
            const bool language_only = current.domain == domain_et::LANGUAGE
                && incoming.domain == domain_et::LANGUAGE;
            for (const auto& step : shortest_path(
                args,
                current,
                node_t{ incoming.domain, incoming.state_type },
                language_only
            ))
            {
                result.push_back(step);
                const auto step_out = args.outgoing_domain_and_state(step);
                current = node_t{ step_out.domain, step_out.state_type };
            }
        }
        result.push_back(item);
        current = node_t{ outgoing.domain, outgoing.state_type };
        started = true;
    }

    args.transform_sequence = std::move(result);
}

}// namespace

void builder_t::complete_transform_sequence(const builder_args_t& args)
{
    if (!args.finish_sequence)
    {
        complete_between_steps(args);
        return;
    }

    check_state_transforms(args);
    check_transform_sequence(args);

    container_t language;
    container_t post;
    for (const auto& item : args.transform_sequence)
    {
        if (is_post_transform(args, item))
        {
            post.push_back(item);
        }
        else if (post.empty())
        {
            language.push_back(item);
        }
        else
        {
            post.push_back(item);
        }
    }

    container_t result;
    node_t current{ domain_et::LANGUAGE, state_type_et::GaL };
    bool started = false;

    const auto append_item = [&](const container_type_t& item)
    {
        const auto incoming = args.incoming_domain_and_state(item);
        const auto outgoing = args.outgoing_domain_and_state(item);
        if (!started && incoming.state_type != state_type_et::GaL)
        {
            fail_cannot_complete(
                node_t{ domain_et::LANGUAGE, state_type_et::GaL },
                node_t{ incoming.domain, incoming.state_type }
            );
        }
        if (started && current.state != incoming.state_type)
        {
            fail_cannot_complete(current, node_t{ incoming.domain, incoming.state_type });
        }

        result.push_back(item);
        current = node_t{ outgoing.domain, outgoing.state_type };
        started = true;
    };

    const auto append_path = [&](const node_t& target, const bool language_only)
    {
        const node_t from = started
            ? current
            : node_t{ domain_et::LANGUAGE, state_type_et::GaL };
        for (const auto& item : shortest_path(args, from, target, language_only))
        {
            append_item(item);
        }
    };

    const auto ensure_state = [&](const domain_and_state_t& needed, const bool language_only)
    {
        if (started && current.state == needed.state_type)
        {
            return;
        }
        append_path(node_t{ needed.domain, needed.state_type }, language_only);
    };

    for (const auto& item : language)
    {
        ensure_state(args.incoming_domain_and_state(item), true);
        append_item(item);
    }

    const bool closed_by_unparse = started && is_language_unparse(args, result.back());
    if (!closed_by_unparse)
    {
        append_path(node_t{ domain_et::LANGUAGE, state_type_et::CPP }, true);
        for (const auto& item : args.state_transforms)
        {
            if (!is_language_unparse(args, item))
            {
                continue;
            }
            append_item(item);
            break;
        }
    }

    if (post.empty())
    {
        append_path(node_t{ domain_et::POST_LANGUAGE, state_type_et::XML }, false);
    }
    else
    {
        for (const auto& item : post)
        {
            ensure_state(args.incoming_domain_and_state(item), false);
            append_item(item);
        }
        // a post xslt ends in SAXON. the adopt step is already in front of it
        if (current.state != state_type_et::XML && current.state != state_type_et::SAXON)
        {
            append_path(node_t{ domain_et::POST_LANGUAGE, state_type_et::XML }, false);
        }
    }

    args.transform_sequence = std::move(result);
}

}// namespace galimatron::translator::translator::transform_sequence_builder
