// galimatron
#include "galimatron/translator_library/translator/detail/translator_impl.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::translator
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_tsp);
}

namespace
{

// builder_t completes transform_sequence in place.
// finish_sequence is production: the full catalog, GaL in, post XML out.
// finish_sequence false is testing: only a catalog edge whose step was supplied
// can be inserted, and only between two seed steps that do not meet
//
void complete_sequence(
    transform_sequence_builder::builder_t::container_t& transform_sequence,
    const transform_sequence_builder::builder_t::container_t& state_transforms,
    const bool finish_sequence
)
{
    namespace tsb = transform_sequence_builder;
    tsb::builder_t{{
        .state_transforms = state_transforms,
        .incoming_domain_and_state = [](const std::any& item) -> tsb::domain_and_state_t
        {
            return std::any_cast<const transform_t&>(item).incoming_domain_and_state;
        },
        .outgoing_domain_and_state = [](const std::any& item) -> tsb::domain_and_state_t
        {
            return std::any_cast<const transform_t&>(item).outgoing_domain_and_state;
        },
        .transform_sequence = transform_sequence,
        .finish_sequence = finish_sequence
    }};
}

void take_sequence(
    transforms_t& transforms,
    const transform_sequence_builder::builder_t::container_t& transform_sequence
)
{
    for (const auto& item : transform_sequence)
    {
        transforms.push_back(std::any_cast<const transform_t&>(item));
    }
}

}// namespace

const galimatron_config::transform_t& translator_t::store_builtin_transform(
    const std::string_view name,
    const path_ot principal_xml
)
{
    config_transforms_.push_back({
        .name = name_t{ name },
        .input = {},
        .output = {
            .principal_xml = {
                .file = principal_xml
            }
        }
    });
    return config_transforms_.back();
}

void translator_t::prepare_transforms()
{
    if (args_.testing)
    {
        prepare_testing_transforms();
    }
    else
    {
        prepare_prod_transforms();
    }
}

void translator_t::prepare_prod_transforms()
{
    // builder_t completes transform_sequence in place.
    // state_transforms is the catalog of edges between representations.
    // the seed is validating, the wizard, and the enabled user transforms; those stay, in that order.
    // edges from the catalog are inserted so each step's outgoing domain and state_type
    // match the next step's incoming ones.
    // the sequence starts at language GaL, the language part ends with CPP -> GaL,
    // and the sequence ends at post-language XML.
    namespace tsb = transform_sequence_builder;

    tsb::builder_t::container_t transform_sequence;
    {
        // wizard checks see the file before the command is applied
        transform_sequence.emplace_back(get_cpp2cpp_validate_transform());
        transform_sequence.emplace_back(get_cpp2cpp_wizard_transform());

        // the language xslt edits language xml. the graph is kept before that print,
        // and sealed text is restored after xml2cpp when the xslt changed it.
        // only an enabled language xslt opens the seal
        const bool has_language_xslt = ranges::any_of(
            language_transforms_.user_transforms.transforms,
            galimatron_config::is_enabled
        );
        if (has_language_xslt)
        {
            transform_sequence.emplace_back(get_cpp2cpp_before_xslt_transform());
        }

        for (const auto& tf : language_transforms_.user_transforms.transforms | ranges::views::filter(galimatron_config::is_enabled))
        {
            transform_sequence.emplace_back(get_xml2xml_transform(tf));
        }

        if (has_language_xslt)
        {
            transform_sequence.emplace_back(get_cpp2cpp_after_xslt_transform());
        }

        for (const auto& tf : post_language_transforms_.user_transforms.transforms | ranges::views::filter(galimatron_config::is_enabled))
        {
            transform_sequence.emplace_back(get_post_xml2post_xml_transform(tf));
        }
    }

    const tsb::builder_t::container_t state_transforms =
    {
        get_gal2cpp_transform(),// GaL -> CPP
        get_cpp2gal_transform(),// CPP -> GaL
        get_cpp2xml_transform(),// CPP -> XML
        get_xml2saxon_transform(),// XML -> SAXON, before a language xslt
        get_xml2cpp_transform(),// SAXON -> CPP
        get_cpp2post_cpp_transform(),// LANGUAGE:CPP -> POST_LANGUAGE:CPP
        get_post_cpp2post_xml_transform(),// POST_LANGUAGE:CPP -> POST_LANGUAGE:XML
        get_post_xml2saxon_transform()// POST_LANGUAGE:XML -> POST_LANGUAGE:SAXON, before a post xslt
    };

    complete_sequence(transform_sequence, state_transforms, true);
    take_sequence(transforms_, transform_sequence);
}

// the flagged steps are the seed, in translate_slice order.
// the catalog holds gal2cpp when that flag is on, so a later step that comes in from CPP
// can be reached after cpp2gal, and holds xml2saxon when xml2cpp is on.
// a flag that is off is not in the seed or the catalog, so the builder cannot insert it.
// the builder does not finish the sequence at post XML
//
void translator_t::prepare_testing_transforms()
{
    namespace tsb = transform_sequence_builder;
    const auto& testing = *args_.testing;

    tsb::builder_t::container_t transform_sequence;
    tsb::builder_t::container_t state_transforms;

    if (testing.gal2cpp)
    {
        const auto step = get_gal2cpp_transform(!testing.cpp2cpp_validate);
        transform_sequence.push_back(step);
        state_transforms.push_back(step);
    }

    if (testing.cpp2cpp_validate)
    {
        transform_sequence.push_back(get_cpp2cpp_validate_transform());
    }

    if (testing.cpp2cpp_wizard)
    {
        transform_sequence.push_back(get_cpp2cpp_wizard_transform());
    }

    if (testing.cpp2xml || testing.xml2cpp)
    {
        transform_sequence.push_back(get_cpp2xml_transform());
    }

    if (testing.xml2cpp)
    {
        state_transforms.push_back(get_xml2saxon_transform());
        transform_sequence.push_back(get_xml2cpp_transform());
    }

    if (testing.cpp2gal)
    {
        transform_sequence.push_back(get_cpp2gal_transform());
    }

    if (testing.cpp2post_cpp)
    {
        transform_sequence.push_back(get_cpp2post_cpp_transform());

        if (testing.post_cpp2post_xml)
        {
            transform_sequence.push_back(get_post_cpp2post_xml_transform());
        }
    }

    complete_sequence(transform_sequence, state_transforms, false);
    take_sequence(transforms_, transform_sequence);
}

}// namespace galimatron::translator::translator
