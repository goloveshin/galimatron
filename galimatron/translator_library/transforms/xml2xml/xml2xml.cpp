// galimatron
#include "galimatron/translator_library/transforms/xml2xml/xml2xml.hpp"
#include "galimatron/translator_library/transforms/xml2xml/detail/xml2xml_impl.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>
#include <memory>


namespace galimatron::translator::transforms::xml2xml
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_xslt);
}

namespace
{
    saxon_session_t& cast(session_state_t& state)
    {
        return *std::any_cast<std::shared_ptr<saxon_session_t>>(state);
    }
}// namespace

session_state_t prepare(const prepare_t& config)
{
    return std::make_shared<saxon_session_t>(config.config);
}


void operation_t::adopt(session_state_t& state, const file_content_t& xml)
{
    cast(state).adopt(xml);
}

bool operation_t::has_document(session_state_t& state)
{
    if (!state.has_value())
    {
        return false;
    }
    return cast(state).has_document();
}

file_content_t operation_t::printout(session_state_t& state)
{
    return cast(state).printout();
}

pugi_document_with_meanful_node_t operation_t::to_pugi(session_state_t& state)
{
    return cast(state).to_pugi();
}

file_contents_with_paths_ot operation_t::apply(session_state_t& state, const galimatron_config::transform_t& config)
{
    return cast(state).apply(config);
}

file_content_t apply_primary(
    const galimatron_config::detail::saxon_processor_t& config,
    const file_content_t& document,
    const std::string& xslt
)
{
    saxon_session_t saxon{ config };
    saxon.adopt(document);
    return saxon.apply_primary(xslt);
}

}// namespace galimatron::translator::transforms::xml2xml
