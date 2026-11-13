#pragma once

// galimatron
#include "galimatron/translator_library/forward.hpp"
#include "galimatron/translator_library/galimatron_config_declaration.hpp"
#include "galimatron/translator_library/pugi_forward.hpp"


namespace galimatron::translator::transforms::xml2xml
{

using session_state_t = std::any;

struct operation_t
{
    static void adopt(session_state_t&, const file_content_t& xml);
    static bool has_document(session_state_t&);
    static file_content_t printout(session_state_t&);
    static pugi_document_with_meanful_node_t to_pugi(session_state_t&);
    static file_contents_with_paths_ot apply(session_state_t&, const galimatron_config::transform_t& config);
};

// one document through one xslt. the primary result, not result-document files
//
// testing: no
//
file_content_t apply_primary(
    const galimatron_config::detail::saxon_processor_t& config,
    const file_content_t& document,
    const std::string& xslt
);

struct prepare_t
{
    const galimatron_config::detail::saxon_processor_t& config;
};
session_state_t prepare(const prepare_t&);

}// namespace galimatron::translator::transforms::xml2xml
