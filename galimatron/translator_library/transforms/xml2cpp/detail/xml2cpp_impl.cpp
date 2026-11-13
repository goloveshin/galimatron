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


namespace doc = tags::document;
namespace file = galimatron::translator::xml_tags::file;

xml2cpp_t::xml2cpp_t(const translate_t& args)
    : args_(args)
    , state_(std::make_shared<holder::language_part_t>())
    , builder_(*state_)
{
}

holder::language_part_sptr_t xml2cpp_t::translate()
{
    const auto part = exception::augment_domain(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::xml2cpp{}
    );

    const auto node = source_node();
    read_string_delimiters(node);
    walk(node, builder_.root_node());
    state_->drop_detached_vertices();
    return state_;
}

pugi::xml_node xml2cpp_t::source_node() const
{
    if (args_.document.meanful_node)
    {
        return *args_.document.meanful_node;
    }

    const auto root = args_.document.document->document_element();
    if (root.name() == doc::language || root.name() == doc::compiled)
    {
        return root;
    }

    if (auto language = root.child(doc::language.data()))
    {
        return language;
    }
    if (auto compiled = root.child(doc::compiled.data()))
    {
        return compiled;
    }

    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::xml2cpp{},
        exception::e_what_info{ "xml has no language or compiled node" }
    );
}

location_ot xml2cpp_t::read_location(const pugi::xml_node& node) const
{
    const auto location_node = node.child(file::location.data());
    if (!location_node)
    {
        return {};
    }

    location_t location;

    const auto filesystem = location_node.child(file::filesystem.data());
    if (const auto path = filesystem.attribute(file::path.data()))
    {
        location.path = path_t(path.value());
    }

    return location;
}

string_ot xml2cpp_t::read_cdata(const pugi::xml_node& node) const
{
    string_ot pcdata;
    for (const auto child : node.children())
    {
        if (child.type() == pugi::node_cdata)
        {
            return std::string(child.value());
        }
        if (child.type() == pugi::node_pcdata && !pcdata)
        {
            pcdata = std::string(child.value());
        }
    }
    return pcdata;
}

string_ot xml2cpp_t::attribute_if(const pugi::xml_node& node, const std::string_view name) const
{
    if (const auto attr = node.attribute(name.data()))
    {
        return std::string(attr.value());
    }
    return {};
}

index_t xml2cpp_t::attribute_index(const pugi::xml_node& node) const
{
    const auto attr = node.attribute(tag_c::index.data());
    if (!attr)
    {
        leaf::throw_exception(
            exception::domain::translator_library{},
            exception::subdomain::translator_library::xml2cpp{},
            exception::e_what_info{ std::format("node '{}' has no index", node.name()) }
        );
    }
    return std::stoull(attr.value());
}

index_ot xml2cpp_t::attribute_index_if(const pugi::xml_node& node) const
{
    const auto attr = node.attribute(tag_c::index.data());
    if (!attr)
    {
        return {};
    }
    return index_t{ std::stoull(attr.value()) };
}

tag_mode_et xml2cpp_t::attribute_mode(const pugi::xml_node& node) const
{
    const auto attr = node.attribute(tag_c::mode.data());
    if (!attr)
    {
        return tag_mode_et::INHERIT;
    }
    const std::string_view text = attr.value();
    if (text == gal_and_xml_tags::tag_gal::once)
    {
        return tag_mode_et::ONCE;
    }
    if (text == gal_and_xml_tags::tag_gal::local)
    {
        return tag_mode_et::LOCAL;
    }
    if (text == gal_and_xml_tags::tag_gal::own)
    {
        return tag_mode_et::OWN;
    }
    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::xml2cpp{},
        exception::e_what_info{ std::format("unknown version mode '{}'", text) }
    );
}

version_status_oet xml2cpp_t::attribute_status(const pugi::xml_node& node) const
{
    const auto attr = node.attribute(tag_c::status.data());
    if (!attr)
    {
        return {};
    }

    const std::string_view status = attr.value();
    if (status == to_string(version_status_et::DELETED))
    {
        return version_status_et::DELETED;
    }

    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::xml2cpp{},
        exception::e_what_info{ std::format("unknown version status '{}'", status) }
    );
}

// loads the delimiter list from the galimatron node. a missing list keeps the plain double quote
//
void xml2cpp_t::read_string_delimiters(const pugi::xml_node& language)
{
    const auto list = language.parent().child(tags::document::string_delimiters.data());
    if (!list)
    {
        return;
    }

    std::vector<string_delimiter_t> delimiters;
    for (const auto entry : list.children(tags::document::string_delimiter.data()))
    {
        string_delimiter_t delimiter;
        if (const auto raw = entry.attribute(tag_c::raw.data()))
        {
            delimiter.raw = std::string_view{ raw.value() } == tag_c::bool_true;
        }
        if (const auto quote = entry.attribute(tag_c::quote.data()))
        {
            const std::string_view text = quote.value();
            if (!text.empty())
            {
                delimiter.quote = text.front();
            }
        }
        if (const auto dashes = entry.attribute(tag_c::dashes.data()))
        {
            delimiter.dashes = dashes.value();
        }
        delimiters.push_back(std::move(delimiter));
    }
    state_->replace_string_delimiters(std::move(delimiters));
}

index_t xml2cpp_t::attribute_delimiter(const pugi::xml_node& node) const
{
    const auto attr = node.attribute(tag_c::delimiter.data());
    if (!attr)
    {
        return 0;
    }
    return std::stoull(attr.value());
}

void xml2cpp_t::restore_layout(const pugi::xml_node& node, const vertex_descriptor_t vd)
{
    if (!args_.layout_table)
    {
        return;
    }

    const auto attribute = node.attribute(tag_c::layout_mark.data());
    if (!attribute)
    {
        return;
    }

    char* end = nullptr;
    const auto mark = std::strtoull(attribute.value(), &end, 10);
    if (!end || *end != '\0' || mark == 0)
    {
        return;
    }

    const auto found = args_.layout_table->entries.find(mark);
    if (found == args_.layout_table->entries.end())
    {
        return;
    }

    auto& vertex = builder_.graph()[vd];
    vertex.user_comments = found->second.user_comments;
    if (layout::fingerprint(node) == found->second.fingerprint)
    {
        vertex.user_formatting = found->second.user_formatting;
    }
}

void xml2cpp_t::unexpected(const pugi::xml_node& node) const
{
    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::xml2cpp{},
        exception::e_what_info{ std::format("unexpected xml node '{}'", node.name()) }
    );
}

void xml2cpp_t::walk(const pugi::xml_node& node, const vertex_descriptor_t parent)
{
    for (const auto child : node.children())
    {
        if (child.type() != pugi::node_element)
        {
            continue;
        }

        const std::string_view name = child.name();

        // the wizard mark is for a language transform; xml2cpp does not keep it
        if (name == file::location || name == tags::aggregate::orderings || name == tags::wizard_mark::dynamic)
        {
            continue;
        }
        if (name == file::chunk)
        {
            read_chunk(child);
        }
        else if (name == tags::root_aggregate::root_aggregate)
        {
            read_root_aggregate(child, parent);
        }
        else if (name == tags::aggregate::aggregate)
        {
            read_aggregate(child, parent);
        }
        else if (name == tags::root_aggregate::root_aggregate_version_declarations)
        {
            read_root_aggregate_version_declarations(child, parent);
        }
        else if (name == tags::root_aggregate::root_aggregate_version_definitions)
        {
            read_root_aggregate_version_definitions(child, parent);
        }
        else if (name == tags::aggregate::aggregate_version_declarations)
        {
            read_aggregate_version_declarations(child, parent);
        }
        else if (name == tags::aggregate::aggregate_version_definitions)
        {
            read_aggregate_version_definitions(child, parent);
        }
        else if (name == tags::tag::tag)
        {
            read_tag_statement(child, parent);
        }
        else if (name == tags::label::label)
        {
            read_label_statement(child, parent);
        }
        else if (name == tags::ordering::ordering)
        {
            read_ordering_statement(child, parent);
        }
        else if (name == tags::operation::delete_tag)
        {
            read_operation_statement(child, parent);
        }
        else
        {
            unexpected(child);
        }
    }
}

}// namespace galimatron::translator::transforms::xml2cpp
