// galimatron
#include "galimatron/translator_library/transforms/xml2cpp_saxon/detail/xml2cpp_saxon.hpp"
#include "galimatron/common_library/exception_augment.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::xml2cpp_saxon
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_x2s);
}


namespace doc = tags::document;
namespace file = galimatron::translator::xml_tags::file;

xml2cpp_saxon_t::xml2cpp_saxon_t(
    node_t root,
    layout::table_t* layout_table
)
    : layout_table_(layout_table)
    , state_(std::make_shared<holder::language_part_t>())
    , builder_(*state_)
{
    bind(std::move(root));
}

holder::language_part_sptr_t xml2cpp_saxon_t::translate()
{
    const auto part = exception::augment_domain(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::xml2cpp{}
    );

    read_string_delimiters(source_);
    walk(source_, builder_.root_node());
    state_->drop_detached_vertices();
    return state_;
}

// the language or compiled element. a document node yields its root element first.
// throws translator_library::xml2cpp when neither element is present
//
void xml2cpp_saxon_t::bind(node_t root)
{
    if (!root.is_element())
    {
        kept_document_ = std::move(root);
        for (auto& child : kept_document_.children())
        {
            if (child.is_element())
            {
                root = std::move(child);
                break;
            }
        }
    }

    if (root.local_name() == doc::language || root.local_name() == doc::compiled)
    {
        source_ = std::move(root);
        return;
    }

    if (auto language = root.child(doc::language))
    {
        anchor_ = std::move(root);
        source_ = std::move(language);
        return;
    }
    if (auto compiled = root.child(doc::compiled))
    {
        anchor_ = std::move(root);
        source_ = std::move(compiled);
        return;
    }

    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::xml2cpp{},
        exception::e_what_info{ "xml has no language or compiled node" }
    );
}

location_ot xml2cpp_saxon_t::read_location(const node_t& node) const
{
    const auto location_node = node.child(file::location);
    if (!location_node)
    {
        return {};
    }

    location_t location;

    const auto filesystem = location_node.child(file::filesystem);
    if (const auto path = filesystem.attribute(file::path))
    {
        location.path = path_t(path.value());
    }

    return location;
}

string_ot xml2cpp_saxon_t::read_cdata(const node_t& node) const
{
    for (const auto& child : node.children())
    {
        if (child.is_text())
        {
            return child.text();
        }
    }
    return {};
}

string_ot xml2cpp_saxon_t::attribute_if(const node_t& node, const std::string_view name) const
{
    if (const auto attr = node.attribute(name))
    {
        return std::string{ attr.value() };
    }
    return {};
}

index_t xml2cpp_saxon_t::attribute_index(const node_t& node) const
{
    const auto attr = node.attribute(tag_c::index);
    if (!attr)
    {
        leaf::throw_exception(
            exception::domain::translator_library{},
            exception::subdomain::translator_library::xml2cpp{},
            exception::e_what_info{ std::format("node '{}' has no index", node.local_name()) }
        );
    }
    return std::stoull(attr.value());
}

index_ot xml2cpp_saxon_t::attribute_index_if(const node_t& node) const
{
    const auto attr = node.attribute(tag_c::index);
    if (!attr)
    {
        return {};
    }
    return index_t{ std::stoull(attr.value()) };
}

tag_mode_et xml2cpp_saxon_t::attribute_mode(const node_t& node) const
{
    const auto attr = node.attribute(tag_c::mode);
    if (!attr)
    {
        return tag_mode_et::INHERIT;
    }
    const std::string_view text = attr.value();
    if (text == tags::tag_gal::once)
    {
        return tag_mode_et::ONCE;
    }
    if (text == tags::tag_gal::local)
    {
        return tag_mode_et::LOCAL;
    }
    if (text == tags::tag_gal::own)
    {
        return tag_mode_et::OWN;
    }
    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::xml2cpp{},
        exception::e_what_info{ std::format("unknown version mode '{}'", text) }
    );
}

version_status_oet xml2cpp_saxon_t::attribute_status(const node_t& node) const
{
    const auto attr = node.attribute(tag_c::status);
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

void xml2cpp_saxon_t::read_string_delimiters(const node_t& language)
{
    const auto list = language.parent().child(tags::document::string_delimiters);
    if (!list)
    {
        return;
    }

    std::vector<string_delimiter_t> delimiters;
    for (const auto& entry : list.children(tags::document::string_delimiter))
    {
        string_delimiter_t delimiter;
        if (const auto raw = entry.attribute(tag_c::raw))
        {
            delimiter.raw = std::string_view{ raw.value() } == tag_c::bool_true;
        }
        if (const auto quote = entry.attribute(tag_c::quote))
        {
            const std::string_view text = quote.value();
            if (!text.empty())
            {
                delimiter.quote = text.front();
            }
        }
        if (const auto dashes = entry.attribute(tag_c::dashes))
        {
            delimiter.dashes = dashes.value();
        }
        delimiters.push_back(std::move(delimiter));
    }
    state_->replace_string_delimiters(std::move(delimiters));
}

index_t xml2cpp_saxon_t::attribute_delimiter(const node_t& node) const
{
    const auto attr = node.attribute(tag_c::delimiter);
    if (!attr)
    {
        return 0;
    }
    return std::stoull(attr.value());
}

void xml2cpp_saxon_t::restore_layout(const node_t& node, const vertex_descriptor_t vd)
{
    if (!layout_table_)
    {
        return;
    }

    const auto attribute = node.attribute(tag_c::layout_mark);
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

    const auto found = layout_table_->entries.find(mark);
    if (found == layout_table_->entries.end())
    {
        return;
    }

    auto& vertex = builder_.graph()[vd];
    vertex.user_comments = found->second.user_comments;
    if (fingerprint(node) == found->second.fingerprint)
    {
        vertex.user_formatting = found->second.user_formatting;
    }
}

void xml2cpp_saxon_t::unexpected(const node_t& node) const
{
    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::xml2cpp{},
        exception::e_what_info{ std::format("unexpected xml node '{}'", node.local_name()) }
    );
}

void xml2cpp_saxon_t::walk(const node_t& node, const vertex_descriptor_t parent)
{
    for (const auto& child : node.children())
    {
        if (!child.is_element())
        {
            continue;
        }

        const std::string_view name = child.local_name();

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

namespace tag = tags::tag;

void xml2cpp_saxon_t::read_tag_statement(const node_t& node, const vertex_descriptor_t parent)
{
    std::deque<name_t> labels;
    for (const auto& ref : node.child(tag::label_refs).child(tag::label_static).children(tag::label_ref))
    {
        if (const auto name = ref.attribute(tag_c::name))
        {
            labels.push_back(name.value());
        }
    }

    const auto mode = [&]() -> tag_mode_et
    {
        const auto text = attribute_if(node, tag_c::mode);
        if (!text)
        {
            return tag_mode_et::INHERIT;
        }
        if (*text == tags::tag_gal::once)
        {
            return tag_mode_et::ONCE;
        }
        if (*text == tags::tag_gal::local)
        {
            return tag_mode_et::LOCAL;
        }
        leaf::throw_exception(
            exception::domain::translator_library{},
            exception::subdomain::translator_library::xml2cpp{},
            exception::e_what_info{ std::format("unknown tag mode '{}'", *text) }
        );
    }();

    const auto vd = builder_.create_tag_statement({
        .version_definition_node = parent,
        .index_in_version = attribute_index(node),
        .name = attribute_if(node, tag_c::name),
        .labels = labels,
        .mode = mode,
        .location = read_location(node)
    });
    restore_layout(node, vd);

    read_tag_values(node.child(tag::values), vd, vd);
}

void xml2cpp_saxon_t::read_tag_values(
    const node_t& node,
    const vertex_descriptor_t parent,
    const vertex_descriptor_t vd_tag
)
{
    const auto vd = builder_.create_tag_values({
        .parent_node = parent
    });

    if (!node)
    {
        return;
    }

    for (const auto& child : node.children(tag::value))
    {
        read_tag_value(child, vd, vd_tag);
    }
}

void xml2cpp_saxon_t::read_tag_value(
    const node_t& node,
    const vertex_descriptor_t parent,
    const vertex_descriptor_t vd_tag
)
{
    const auto index = [&] -> index_ot
    {
        if (const auto attr = node.attribute(tag_c::index))
        {
            return std::stoull(attr.value());
        }
        return {};
    }();

    const auto vd = builder_.create_tag_value({
        .parent_node = parent,
        .index = index
    });

    for (const auto& child : node.children())
    {
        if (!child.is_element())
        {
            continue;
        }

        const std::string_view name = child.local_name();
        if (name == tag::record)
        {
            read_tag_record(child, vd, vd_tag);
        }
        else if (name == tag::array)
        {
            read_tag_array(child, vd, vd_tag);
        }
        else if (name == tag_c::string)
        {
            read_tag_string(child, vd, vd_tag, index);
        }
        else
        {
            unexpected(child);
        }
    }
}

void xml2cpp_saxon_t::read_tag_record(
    const node_t& node,
    const vertex_descriptor_t parent,
    const vertex_descriptor_t vd_tag
)
{
    const auto vd = builder_.create_tag_record({
        .parent_node = parent
    });

    for (const auto& child : node.children(tag::key))
    {
        read_tag_keyvalue(child, vd, vd_tag);
    }
}

void xml2cpp_saxon_t::read_tag_keyvalue(
    const node_t& node,
    const vertex_descriptor_t parent,
    const vertex_descriptor_t vd_tag
)
{
    const auto vd = builder_.create_tag_keyvalue({
        .parent_node = parent,
        .key = read_cdata(node),
        .key_delimiter = attribute_delimiter(node)
    });

    for (const auto& child : node.children(tag::value))
    {
        read_tag_value(child, vd, vd_tag);
    }
}

void xml2cpp_saxon_t::read_tag_array(
    const node_t& node,
    const vertex_descriptor_t parent,
    const vertex_descriptor_t vd_tag
)
{
    const auto vd = builder_.create_tag_array({
        .parent_node = parent
    });

    for (const auto& child : node.children(tag::value))
    {
        read_tag_value(child, vd, vd_tag);
    }
}

void xml2cpp_saxon_t::read_tag_string(
    const node_t& node,
    const vertex_descriptor_t parent,
    const vertex_descriptor_t vd_tag,
    const index_ot value_index
)
{
    const auto mark = attribute_if(node, tag_c::template_engine);
    const auto vd = builder_.create_tag_string({
        .parent_node = parent,
        .value = read_cdata(node).value_or(""),
        .delimiter = attribute_delimiter(node),
        .is_template = mark && *mark == tag_c::bool_true
    });

    if (value_index)
    {
        auto& tag_statement = get_entity<tag_statement_t>(builder_.graph()[vd_tag]);
        tag_statement.string_values_at_zero_level[*value_index] = vd;
    }
}

namespace aggregate = tags::aggregate;

void xml2cpp_saxon_t::read_aggregate(const node_t& node, const vertex_descriptor_t parent)
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
            exception::e_what_info{ "aggregate has no name" }
        );
    }

    const auto index = [&]() -> index_t
    {
        if (const auto attr = node.attribute(tag_c::index))
        {
            const index_t value = std::stoull(attr.value());
            next_aggregate_index_ = std::max(next_aggregate_index_, value + 1);
            return value;
        }
        return next_aggregate_index_++;
    }();

    const auto vd = builder_.create_aggregate({
        .chunk_node = parent,
        .name = *name,
        .index = index,
        .location = read_location(node)
    });
    restore_layout(node, vd);

    walk(node, vd);
}

void xml2cpp_saxon_t::read_aggregate_version_declarations(const node_t& node, const vertex_descriptor_t parent)
{
    const auto vd = builder_.create_aggregate_version_declarations({
        .aggregate_node = parent
    });
    restore_layout(node, vd);

    for (const auto& child : node.children(aggregate::aggregate_version_declaration))
    {
        read_aggregate_version_declaration(child, vd);
    }
}

void xml2cpp_saxon_t::read_aggregate_version_declaration(const node_t& node, const vertex_descriptor_t parent)
{
    const auto vd = builder_.create_aggregate_version_declaration({
        .versions_node = parent,
        .index = attribute_index(node),
        .status = attribute_status(node),
        .comment = attribute_if(node, tag_c::comment),
        .hash = attribute_if(node, tag_c::hash)
    });
    restore_layout(node, vd);
}

void xml2cpp_saxon_t::read_aggregate_version_definitions(const node_t& node, const vertex_descriptor_t parent)
{
    const auto vd = builder_.create_aggregate_version_definitions({
        .aggregate_node = parent
    });
    restore_layout(node, vd);

    for (const auto& child : node.children(aggregate::aggregate_version_definition))
    {
        read_aggregate_version_definition(child, vd);
    }
    builder_.state().bind_anonymous_owners(vd);
}

void xml2cpp_saxon_t::read_aggregate_version_definition(const node_t& node, const vertex_descriptor_t parent)
{
    index_ot owner;
    if (const auto text = attribute_if(node, tag_c::owner))
    {
        owner = index_t{ std::stoull(*text) };
    }
    const auto vd = builder_.create_aggregate_version_definition({
        .versions_node = parent,
        .index = attribute_index_if(node),
        .owner = owner,
        .mode = attribute_mode(node)
    });
    restore_layout(node, vd);

    walk(node, vd);
    builder_.apply_aggregate_version_delete(vd);
}

namespace root = tags::root_aggregate;

void xml2cpp_saxon_t::read_root_aggregate(const node_t& node, const vertex_descriptor_t parent)
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
            exception::e_what_info{ "root aggregate has no name" }
        );
    }

    const auto vd = builder_.create_root_aggregate({
        .chunk_node = parent,
        .name = *name,
        .location = read_location(node)
    });
    restore_layout(node, vd);

    walk(node, vd);
}

void xml2cpp_saxon_t::read_root_aggregate_version_declarations(const node_t& node, const vertex_descriptor_t parent)
{
    const auto vd = builder_.create_root_aggregate_version_declarations({
        .root_aggregate_node = parent
    });
    restore_layout(node, vd);

    for (const auto& child : node.children(root::root_aggregate_version_declaration))
    {
        read_root_aggregate_version_declaration(child, vd);
    }
}

void xml2cpp_saxon_t::read_root_aggregate_version_declaration(const node_t& node, const vertex_descriptor_t parent)
{
    const auto vd = builder_.create_root_aggregate_version_declaration({
        .versions_node = parent,
        .index = attribute_index(node),
        .status = attribute_status(node),
        .comment = attribute_if(node, tag_c::comment),
        .hash = attribute_if(node, tag_c::hash)
    });
    restore_layout(node, vd);
}

void xml2cpp_saxon_t::read_root_aggregate_version_definitions(const node_t& node, const vertex_descriptor_t parent)
{
    const auto vd = builder_.create_root_aggregate_version_definitions({
        .root_aggregate_node = parent
    });
    restore_layout(node, vd);

    for (const auto& child : node.children(root::root_aggregate_version_definition))
    {
        read_root_aggregate_version_definition(child, vd);
    }
    builder_.state().bind_anonymous_owners(vd);
}

void xml2cpp_saxon_t::read_root_aggregate_version_definition(const node_t& node, const vertex_descriptor_t parent)
{
    index_ot owner;
    if (const auto text = attribute_if(node, tag_c::owner))
    {
        owner = index_t{ std::stoull(*text) };
    }
    const auto vd = builder_.create_root_aggregate_version_definition({
        .versions_node = parent,
        .index = attribute_index_if(node),
        .owner = owner,
        .mode = attribute_mode(node)
    });
    restore_layout(node, vd);

    walk(node, vd);
}

namespace label = tags::label;

void xml2cpp_saxon_t::read_label_statement(const node_t& node, const vertex_descriptor_t parent)
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

    if (const auto static_node = node.child(label::static_highlight))
    {
        strings_t values;
        for (const auto& value_node : static_node.child(label::values).children(label::value))
        {
            if (const auto value = value_node.attribute(label::value))
            {
                values.push_back(value.value());
            }
        }

        builder_.create_label_static({
            .label_node = vd,
            .values = values,
            .tag_name_from = [&]() -> string_ot
            {
                const auto from = static_node.child(label::tag_name_from);
                if (const auto value = from.attribute(label::value))
                {
                    return std::string{ value.value() };
                }
                return {};
            }()
        });
    }

    if (const auto dynamic_node = node.child(label::dynamic_highlight))
    {
        builder_.create_label_dynamic({
            .label_node = vd,
            .value = attribute_if(dynamic_node, tag_c::value).value_or("")
        });
    }

    restore_layout(node, vd);
}

namespace ordering = tags::ordering;

void xml2cpp_saxon_t::read_ordering_statement(const node_t& node, const vertex_descriptor_t parent)
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
            exception::e_what_info{ "ordering has no name" }
        );
    }

    builder::builder_t::create_ordering_statement_t::ordering_parts_t parts;
    for (const auto& part_node : node.child(ordering::ordering_parts).children(ordering::ordering_part))
    {
        builder::builder_t::create_ordering_statement_t::ordering_part_t part;
        for (const auto& ref : part_node.children(ordering::aggregate_ref))
        {
            if (const auto ref_name = ref.attribute(ordering::name))
            {
                part.push_back(ref_name.value());
            }
        }
        parts.push_back(std::move(part));
    }

    const auto vd = builder_.create_ordering_statement({
        .version_definition_node = parent,
        .index = attribute_index(node),
        .name = *name,
        .ordering_parts = std::move(parts),
        .location = read_location(node)
    });
    restore_layout(node, vd);
}

void xml2cpp_saxon_t::read_operation_statement(const node_t& node, const vertex_descriptor_t parent)
{
    const auto name = attribute_if(node, tag_c::name);

    const auto vd = builder_.create_operation_statement({
        .version_definition_node = parent,
        .index = attribute_index(node),
        .name = name.value_or(""),
        .is_version = !name,
        .location = read_location(node)
    });
    restore_layout(node, vd);
}

void xml2cpp_saxon_t::read_chunk(const node_t& node)
{
    const auto vd = builder_.create_chunk({
        .root_node = builder_.root_node(),
        .location = read_location(node)
    });
    restore_layout(node, vd);

    walk(node, vd);
}

}// namespace galimatron::translator::transforms::xml2cpp_saxon
