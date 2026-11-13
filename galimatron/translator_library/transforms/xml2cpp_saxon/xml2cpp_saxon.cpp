// galimatron
#include "galimatron/translator_library/transforms/xml2cpp_saxon/xml2cpp_saxon.hpp"
#include "galimatron/translator_library/transforms/xml2cpp_saxon/detail/xml2cpp_saxon.hpp"
#include "galimatron/translator_library/transforms/xml2xml/detail/xml2xml_impl.hpp"
#include "galimatron/common_library/exception_augment.hpp"


namespace galimatron::translator::transforms::xml2cpp_saxon
{

holder::language_part_sptr_t translate(
    xml2xml::session_state_t& session,
    layout::table_t* layout_table
)
{
    auto& saxon = *std::any_cast<std::shared_ptr<xml2xml::saxon_session_t>>(session);
    const auto taken = saxon.take_document_node();
    if (!taken.node)
    {
        leaf::throw_exception(
            exception::domain::translator_library{},
            exception::subdomain::translator_library::xml2cpp{},
            exception::e_what_info{ "Saxon document has no node" }
        );
    }

    xml2cpp_saxon_t walker{
        node_t::adopt(taken.node, taken.owned),
        layout_table
    };
    return walker.translate();
}

}// namespace galimatron::translator::transforms::xml2cpp_saxon
