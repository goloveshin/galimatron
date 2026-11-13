#pragma once

// galimatron
#include "galimatron/translator_library/forward.hpp"


namespace galimatron::translator::translator::transform_sequence_builder
{

// transform domains
enum class domain_et
{
    LANGUAGE,       // Language domain (Galimatron Language)
    POST_LANGUAGE   // Post-Language domain (Post-Galiatron Language)
};

// state representation
enum class state_type_et
{
    GaL, // as .gal files
    CPP, // C++/boost.graph
    XML, // XML written by pugixml
    SAXON // that document held for an xslt
};

struct domain_and_state_t
{
    domain_et domain;
    state_type_et state_type;
};

// Takes a set of state transforms and a seed of the transform sequence.
// Completes the transform sequence.
//
// check_state_transforms() checks that state_transforms contains the transitions:
// LANGUAGE GaL -> LANGUAGE CPP, LANGUAGE CPP -> LANGUAGE GaL,
// LANGUAGE CPP -> LANGUAGE XML, LANGUAGE XML -> LANGUAGE SAXON, LANGUAGE SAXON -> LANGUAGE CPP,
// LANGUAGE CPP -> POST_LANGUAGE CPP, POST_LANGUAGE CPP -> POST_LANGUAGE XML.
// If a transition is missing — exception::domain::translator_library, exception::subdomain::translator_library::translator.
//
// check_transform_sequence() checks transform_sequence: first a contiguous
// LANGUAGE-domain set, then a contiguous POST_LANGUAGE-domain set.
// Either set may be absent. The LANGUAGE -> POST_LANGUAGE transition is allowed
// once between the sets. A return to LANGUAGE — exception::domain::translator_library, exception::subdomain::translator_library::translator.
//
// check_transform_sequence_state_types() checks that each transform
// in transform_sequence has matching incoming and outgoing state_type.
// Domain is ignored. If they do not match — exception::domain::translator_library, exception::subdomain::translator_library::translator.
//
// complete_transform_sequence() completes transform_sequence by inserting from state_transforms.
// The original transforms are not removed and do not change order.
// finish_sequence: the result has LANGUAGE and POST_LANGUAGE domains, the outgoing state_type matches
// the next incoming one, the first transform comes in from GaL, and the last one goes out to XML.
// finish_sequence false: the seed stays the whole sequence. an edge is inserted only where two
// seed steps do not meet, and only from the catalog that was passed.
// When finish_sequence: a language xslt comes in from SAXON. The adopt step LANGUAGE XML -> LANGUAGE SAXON is inserted before it,
// and xml2cpp brings SAXON back to CPP. No language xslt means that step is absent.
// A post xslt comes in from SAXON. The adopt step POST XML -> POST SAXON is inserted before it,
// and the sequence then ends in SAXON. No post xslt means that step is absent.
// Before the transition to POST_LANGUAGE, the LANGUAGE section ends with CPP -> GaL.
// If it cannot be completed — exception::domain::translator_library, exception::subdomain::translator_library::translator.
//
// testing: yes (unit)
//
class builder_t
{
public:
    using container_type_t = std::any;
    using container_t = std::list<container_type_t>;

public:
    struct builder_args_t
    {
        using get_domain_and_state_ft = std::function<domain_and_state_t (const container_type_t&)>;

        const container_t& state_transforms;
        get_domain_and_state_ft incoming_domain_and_state;
        get_domain_and_state_ft outgoing_domain_and_state;
        container_t& transform_sequence;

        // production finishes at post-language XML and requires the full catalog.
        // false keeps the seed. a catalog edge is inserted only between two seed steps
        // that do not meet, so a step that is not in the catalog cannot be inserted
        bool finish_sequence = true;
    };
    builder_t(const builder_args_t& args);

public:
    static void check_state_transforms(const builder_args_t&);
    static void check_transform_sequence(const builder_args_t&);
    static void check_transform_sequence_state_types(const builder_args_t&);
    static void complete_transform_sequence(const builder_args_t&);
};

}// namespace galimatron::translator::translator::transform_sequence_builder
