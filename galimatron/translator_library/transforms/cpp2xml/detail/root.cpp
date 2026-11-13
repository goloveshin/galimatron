// galimatron
#include "galimatron/translator_library/state/gal_and_xml_tags.hpp"
#include "galimatron/translator_library/transforms/cpp2xml/detail/cpp2xml_impl.hpp"
#include "galimatron/translator_library/xml_tags.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::cpp2xml
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_s2x);
}


namespace tag = gal_and_xml_tags::document;

pugi::xml_node cpp2xml_t::create_root(const root_t&)
{
    doc_ = std::make_shared<pugi::xml_document>();

    auto decl = doc_->prepend_child(pugi::node_declaration);
    {
        decl.append_attribute(xml_tags::xml_document::version) = "1.0";
        decl.append_attribute(xml_tags::xml_document::encoding) = "UTF-8";
    }

    auto galimatron_node = doc_->append_child(tag::galimatron);
    if (!args_.description.empty())
    {
        const auto text = std::string{ args_.description };
        galimatron_node.append_attribute(tag::description.data()) = text.c_str();
    }

    mode_node_ = galimatron_node.append_child(
        compiled_state_
        ? tag::compiled
        : tag::language
    );

    return *mode_node_;
}

}// namespace galimatron::translator::transforms::cpp2xml
