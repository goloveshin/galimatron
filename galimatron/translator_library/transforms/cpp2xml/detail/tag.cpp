// galimatron
#include "galimatron/translator_library/transforms/cpp2xml/detail/cpp2xml_impl.hpp"
#include "galimatron/translator_library/state/dynamic_label.hpp"

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


namespace tag = gal_and_xml_tags::tag;

pugi::xml_node cpp2xml_t::create_tag_statement(const tag_statement_t& args)
{
    const auto& name = [&] -> name_ot
    {
        if (compiled_state_)
        {
            return compiled_state_.value()->get_name_for_tag({
                .vd_tag = vertex_.value().get().vd_self,
                .name = args.name
            });
        }
        return args.name;
    }();

    auto node = parent().append_child(tag::tag);
    {
        node.append_attribute(tag_c::index) = args.index_in_version;

        if (name)
        {
            node.append_attribute(tag_c::name) = *name;
        }
        if (args.mode == tag_mode_et::ONCE)
        {
            node.append_attribute(tag_c::mode.data()) = gal_and_xml_tags::tag_gal::once.data();
        }
        else if (args.mode == tag_mode_et::LOCAL)
        {
            node.append_attribute(tag_c::mode.data()) = gal_and_xml_tags::tag_gal::local.data();
        }
    }

    // post-language printout ignores labels
    if (!compiled_state_)
    {
        const auto& labels = args.labels;

        // a wizard copy: the dynamic ids are a direct child node, same place as the version mark
        if (args.created_by_wizard && language_state_)
        {
            pugi::xml_node dynamic_node;
            for (const auto& label : labels)
            {
                const auto value = dynamic_value_of_label(*language_state_, vertex().vd_self, label);
                if (!value)
                {
                    continue;
                }
                if (!dynamic_node)
                {
                    dynamic_node = node.append_child(tag::label_dynamic);
                }
                dynamic_node.append_attribute(label.c_str()) = value->c_str();
            }
        }

        auto label_refs_node = node.append_child(tag::label_refs);

        if (!labels.empty())
        {
            auto label_static_node = label_refs_node.append_child(tag::label_static);

            for (const auto& [index, label] : ranges::views::enumerate(labels))
            {
                auto label_node = label_static_node.append_child(tag::label_ref);
                {
                    label_node.append_attribute(tag_c::index) = index;
                    label_node.append_attribute(tag_c::name) = label;
                }
            }
        }
    }

    if (compiled_state_)
    {
        static_labels_ = compiled_state_.value()->get_static_labels_for_tag(vertex().vd_self);
    }

    return node;
}

pugi::xml_node cpp2xml_t::create_tag_values(const tag_values_t& args)
{
    auto node = parent().append_child(tag::values);

    return node;
}

pugi::xml_node cpp2xml_t::create_tag_value(const tag_value_t& args)
{
    auto node = parent().append_child(tag::value);
    {
        // first level of values; they have an index
        if (args.index)
        {
            const auto index = args.index.value();

            node.append_attribute(tag_c::index) = index;

            // mark them with the values of static labels
            if (static_labels_)
            {
                for (const auto& aggregated_label_static : *static_labels_)
                {
                    const auto& label = aggregated_label_static.get();
                    if (index >= label.label_static.values.size())
                    {
                        continue;
                    }
                    node.append_attribute(label.name) = label.label_static.values[index];
                }
            }
        }
    }

    return node;
}

pugi::xml_node cpp2xml_t::create_tag_record(const tag_record_t& args)
{
    auto node = parent().append_child(tag::record);

    return node;
}

pugi::xml_node cpp2xml_t::create_tag_keyvalue(const tag_keyvalue_t& args)
{
    auto node = parent().append_child(tag::key);
    {
        if (args.key)
        {
            const auto delimiter = std::to_string(args.key_delimiter);
            node.append_attribute(tag_c::delimiter.data()) = delimiter.c_str();
            node.append_child(pugi::node_cdata).set_value(*args.key);
        }
    }

    return node;
}

pugi::xml_node cpp2xml_t::create_tag_array(const tag_array_t& args)
{
    auto node = parent().append_child(tag::array);

    return node;
}

pugi::xml_node cpp2xml_t::create_tag_string(const tag_string_t& args)
{
    auto element = parent().append_child(tag_c::string);
    {
        const auto delimiter = std::to_string(args.delimiter);
        element.append_attribute(tag_c::delimiter.data()) = delimiter.c_str();
    }

    auto node = element.append_child(pugi::node_cdata);
    {
        const bool compiled = compiled_state_ && vd_compiled_version_;
        const auto* rendered = (compiled && args.is_template)
            ? compiled_state_.value()->rendered_string(*vd_compiled_version_, vertex().vd_self)
            : nullptr;
        // the language document keeps the mark so a user xslt round-trip still renders
        if (args.is_template && !compiled)
        {
            element.append_attribute(tag_c::template_engine.data()) = tag_c::bool_true.data();
        }
        node.set_value(rendered ? *rendered : args.value);
    }

    return node;
}

}// namespace galimatron::translator::transforms::cpp2xml
