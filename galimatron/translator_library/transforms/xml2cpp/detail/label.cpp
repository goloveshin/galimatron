// galimatron
#include "galimatron/translator_library/transforms/xml2cpp/detail/xml2cpp_impl.hpp"
#include "galimatron/common_library/exception_augment.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::xml2cpp
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_x2s);
}


namespace tag = tags::label;

void xml2cpp_t::read_label_statement(const pugi::xml_node& node, const vertex_descriptor_t parent)
{
    const auto name = attribute_if(node, tag_c::name);
    if (!name)
    {
        const auto location = read_location(node);
        const auto where = exception::augment_where({
            .path = location ? location->path : path_ot{}
        });
        leaf::throw_exception(
            exception::domain::translator_library{},
            exception::subdomain::translator_library::xml2cpp{},
            exception::e_what_info{ "label has no name" }
        );
    }

    const auto vd = builder_.create_label_statement({
        .version_definition_node = parent,
        .index = attribute_index(node),
        .name = *name,
        .location = read_location(node)
    });

    if (const auto static_node = node.child(tag::static_highlight.data()))
    {
        strings_t values;
        for (const auto value_node : static_node.child(tag::values.data()).children(tag::value.data()))
        {
            if (const auto value = value_node.attribute(tag::value.data()))
            {
                values.push_back(value.value());
            }
        }

        builder_.create_label_static({
            .label_node = vd,
            .values = values,
            .tag_name_from = [&] -> string_ot
            {
                const auto from = static_node.child(tag::tag_name_from.data());
                if (const auto value = from.attribute(tag::value.data()))
                {
                    return std::string(value.value());
                }
                return {};
            }()
        });
    }

    if (const auto dynamic_node = node.child(tag::dynamic_highlight.data()))
    {
        builder_.create_label_dynamic({
            .label_node = vd,
            .value = attribute_if(dynamic_node, tag_c::value).value_or("")
        });
    }

    restore_layout(node, vd);
}

}// namespace galimatron::translator::transforms::xml2cpp
