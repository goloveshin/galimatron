// galimatron
#include "galimatron/runtime_library/translate_summary_xml/translate_summary_xml.hpp"
#include "galimatron/runtime_library/translate_summary_xml/detail/xml_tags.hpp"
#include "galimatron/translator_library/xml_tags.hpp"
#include "galimatron/translator_library/pugi_forward.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::runtime::translate_summary_xml
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::rt_sum);
}


using namespace ranges;

namespace tag_doc = translator::xml_tags::xml_document;
namespace tag = galimatron::runtime::transformation_summary_xml::xml_tags;

builder_t::builder_t()
{
    auto decl = doc.prepend_child(pugi::node_declaration);
    {
        decl.append_attribute(tag_doc::version) = "1.0";
        decl.append_attribute(tag_doc::encoding) = "UTF-8";
    }
    root_node = doc.append_child(tag::transformations);
}

void builder_t::append_transform(const append_transform_t& args)
{
    auto tf_node = root_node.append_child(tag::transformation);
    {
        tf_node.append_attribute(tag::index) = std::to_string(args.sequential_number);
        tf_node.append_attribute(tag::name) = args.name;
        tf_node.append_attribute(tag::from) = std::string(
            args.origin == translator::translator::detail::origin_et::USER
                ? tag::user
                : tag::build_in
        );
        tf_node.append_attribute(tag::status) = std::string(magic_enum::enum_name(args.status));
    }
    if (args.user_data)
    {
        tf_node
            .append_child(tag::user_data)
            .append_child(pugi::node_cdata)
            .set_value(*args.user_data);
    }
    // an absolute path keeps the native spelling. a relative path is a committed report and uses '/'
    const auto path_text = [](const path_t& path)
    {
        return path.is_absolute() ? path.string() : path.generic_string();
    };
    if (args.produced_xml_path)
    {
        auto xml_node = tf_node.append_child(tag::produced_xml);
        {
            xml_node.append_attribute(tag::produced_xml_path) = path_text(*args.produced_xml_path).c_str();
        }
    }
    if (args.produced_files && !args.produced_files->empty())
    {
        using namespace ranges;

        auto files_node = tf_node.append_child(tag::produced_files);
        {
            files_node.append_attribute(tag::count) = std::to_string(args.produced_files->size());
        }
        for (const auto& [index, path] : *args.produced_files | views::enumerate)
        {
            auto file_node = files_node.append_child(tag::produced_file);
            file_node.append_attribute(tag::index) = index;
            file_node.append_attribute(tag::produced_path) = path_text(path).c_str();
        }
    }
}

void builder_t::set_result(const bool value)
{
    root_node.remove_attribute(tag::status);
    root_node.append_attribute(tag::status) = value ? tag::success : tag::fail;
}

// private memory added while the translator state was alive, in bytes
//
void builder_t::set_state_bytes(const std::size_t bytes)
{
    root_node.remove_attribute(tag::state_bytes);
    root_node.append_attribute(tag::state_bytes) = std::format("{}", bytes).c_str();
}

void builder_t::set_error(const std::string& text)
{
    root_node
        .append_child(tag::error)
        .append_child(pugi::node_cdata)
        .set_value(text);
}

file_content_t builder_t::printout() const
{
    return galimatron::translator::printout(doc);
}

}// namespace galimatron::runtime::translate_summary_xml
