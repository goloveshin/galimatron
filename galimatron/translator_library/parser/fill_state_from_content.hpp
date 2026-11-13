#pragma once

// galimatron
#include "galimatron/translator_library/antlr_forward.hpp"


namespace galimatron::translator::parser
{

/*

Builds the parsing result (state) for many contents in a "general form" (anonymous states)
- asks the user to create a shared result state (global_parse_state_at)
- iterates the contents, and for each one
    - asks the user to create a listener (get_listener_ft) to walk the parse result (single_parse_state_at)
        accumulating into the shared state (global_parse_state_at)
    - asks the user to parse (parse_content_ft) and return its result (single_parse_state_at)
    - asks the user to walk single_parse_state_at with the created listener
- returns the shared result (global_parse_state_at)

The user does not have to use global_parse_state_at; having it is a requirement of the "general form" implementation.

*/

// state-result of parsing one content
using single_parse_state_at = std::any;

// state-result of parsing many contents
using global_parse_state_at = std::any;

struct fill_state_from_content_t
{
    using get_global_state_ft = std::function<global_parse_state_at ()>;
    using parse_content_ft = std::function<single_parse_state_at (const file_content_t&)>;
    using get_listener_ft = std::function<antlr_listener_sptr_t (global_parse_state_at&, const path_t&)>;
    using walk_ft = std::function<void (single_parse_state_at&, antlr_listener_t*)>;

    // this is a set of contents - paths to files with their contents
    deferred_file_contents_with_paths_t view_deferred_file_content_with_path;

    // we ask the user to create the global state where we accumulate parse results of all contents
    get_global_state_ft get_global_state;

    // we ask the user to parse one content and return its result
    parse_content_ft parse_content;

    // we get something castable to ParseTreeListener, with the global state and the content path attached to it
    get_listener_ft get_listener;

    // we walk the listener over one parse result and fill the global state
    walk_ft walk;
};
global_parse_state_at fill_state_from_content(const fill_state_from_content_t&);

}// namespace galimatron::translator::parser
