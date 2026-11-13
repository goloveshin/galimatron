// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_wizard/detail/cpp2cpp_wizard_impl.hpp"
#include "galimatron/translator_library/transforms/cpp2cpp_wizard/detail/support.hpp"
#include "galimatron/translator_library/state/builder/builder.hpp"
#include "galimatron/translator_library/state/content_hash.hpp"
#include "galimatron/translator_library/state/dynamic_label.hpp"
#include "galimatron/translator_library/state/entity/entity.hpp"
#include "galimatron/translator_library/state/gal_and_xml_tags.hpp"

// global
#include "global/common_library/utils.hpp"


namespace galimatron::translator::transforms::cpp2cpp_wizard
{

using namespace galimatron::translator::state;

namespace
{
// the open version the command writes into. the name selects that aggregate's last version
//
struct version_write_target_t
{
    vertex_descriptor_t vd_aggregate;
    vertex_descriptor_t vd_definition;
    index_t version_index;
};

version_write_target_t version_write_target(
    holder::language_part_t& language,
    const name_ot& aggregate_name
)
{
    const auto* record = language.aggregates().try_by_name(*aggregate_name);
    const auto declarations = language.versions().declarations_in_order(record->vd);
    const auto index = declarations.back().index;
    return {
        .vd_aggregate = record->vd,
        .vd_definition = *language.versions().try_definition(record->vd, index),
        .version_index = index
    };
}

// a string at the top of the tag is recorded for naming. a nested string is not
//
void write_tag_json(
    state::builder::builder_t& builder,
    holder::language_part_t& language,
    const vertex_descriptor_t vd_tag,
    const vertex_descriptor_t parent,
    const wizard::json_t& json,
    const bool zero_level_string,
    const index_t zero_index
)
{
    if (json.string)
    {
        const auto vd_string = builder.create_tag_string({
            .parent_node = parent,
            .value = *json.string,
            .delimiter = json.delimiter,
            .is_template = json.is_template
        });
        if (zero_level_string)
        {
            std::get<tag_statement_t>(language.graph()[vd_tag].entity).string_values_at_zero_level[zero_index] = vd_string;
        }
        return;
    }

    if (json.is_array)
    {
        const auto vd_array = builder.create_tag_array({
            .parent_node = parent
        });
        for (const auto& element : json.elements)
        {
            const auto vd_value = builder.create_tag_value({
                .parent_node = vd_array,
                .index = {}
            });
            write_tag_json(builder, language, vd_tag, vd_value, element, false, 0);
        }
        return;
    }

    const auto vd_record = builder.create_tag_record({
        .parent_node = parent
    });
    for (const auto& element : json.elements)
    {
        const auto vd_keyvalue = builder.create_tag_keyvalue({
            .parent_node = vd_record,
            .key = element.key,
            .key_delimiter = element.key_delimiter
        });
        write_tag_json(builder, language, vd_tag, vd_keyvalue, element, false, 0);
    }
}
}// namespace

// writes the label into the open version
//
void cpp2cpp_wizard_t::apply_wizard_create_label()
{
    validate_->may_execute_wizard_command();

    const auto& command = wizard_statement(*language_state_);
    if (command.type != wizard::command_et::CREATE_LABEL)
    {
        return;
    }

    const auto& data = std::get<wizard::label_data_t>(command.data);
    const auto target = version_write_target(*language_state_, data.aggregate);
    const auto& static_values = data.static_values;
    state::builder::builder_t builder{
        *language_state_,
        state::builder::builder_t::on_existing_state_t{}
    };
    const auto vd_label = builder.create_label_statement({
        .version_definition_node = target.vd_definition,
        .index = language_state_->next_version_statement_index(target.vd_aggregate, target.version_index),
        .name = data.name,
        .location = {}
    });
    if (data.is_dynamic)
    {
        builder.create_label_dynamic({
            .label_node = vd_label,
            .value = *data.dynamic_value
        });
        return;
    }

    builder.create_label_static({
        .label_node = vd_label,
        .values = static_values | ranges::to<strings_t>(),
        .tag_name_from = data.tag_name_from
    });
}

// writes the tag into the open version
//
void cpp2cpp_wizard_t::apply_wizard_create_tag()
{
    validate_->may_execute_wizard_command();

    const auto& command = wizard_statement(*language_state_);
    if (command.type != wizard::command_et::CREATE_TAG)
    {
        return;
    }

    const auto& data = std::get<wizard::create_tag_data_t>(command.data);
    const auto target = version_write_target(*language_state_, data.aggregate);
    const auto& label_names = data.labels;
    state::builder::builder_t builder{
        *language_state_,
        state::builder::builder_t::on_existing_state_t{}
    };
    const auto vd_tag = builder.create_tag_statement({
        .version_definition_node = target.vd_definition,
        .index_in_version = language_state_->next_version_statement_index(target.vd_aggregate, target.version_index),
        .name = data.name,
        .labels = label_names | ranges::to<std::deque<name_t>>(),
        .mode = data.mode,
        .location = {}
    });
    const auto vd_values = builder.create_tag_values({
        .parent_node = vd_tag
    });
    index_t index = 0;
    for (const auto& value : data.values)
    {
        const auto vd_value = builder.create_tag_value({
            .parent_node = vd_values,
            .index = index_ot{ index }
        });
        write_tag_json(
            builder,
            *language_state_,
            vd_tag,
            vd_value,
            value,
            value.string.has_value(),
            index
        );
        ++index;
    }
}

// writes delete <name>; into the open version
//
void cpp2cpp_wizard_t::apply_wizard_delete_tag()
{
    validate_->may_execute_wizard_command();

    const auto& command = wizard_statement(*language_state_);
    if (command.type != wizard::command_et::DELETE_TAG)
    {
        return;
    }

    const auto& data = std::get<wizard::delete_tag_data_t>(command.data);
    const auto target = version_write_target(*language_state_, data.aggregate);
    state::builder::builder_t builder{
        *language_state_,
        state::builder::builder_t::on_existing_state_t{}
    };
    builder.create_operation_statement({
        .version_definition_node = target.vd_definition,
        .index = language_state_->next_version_statement_index(target.vd_aggregate, target.version_index),
        .name = data.name,
        .is_version = false,
        .location = {}
    });
}

// writes the ordering into the open version of the root aggregate
//
void cpp2cpp_wizard_t::apply_wizard_create_ordering()
{
    validate_->may_execute_wizard_command();

    const auto& command = wizard_statement(*language_state_);
    if (command.type != wizard::command_et::CREATE_ORDERING)
    {
        return;
    }

    const auto& data = std::get<wizard::ordering_data_t>(command.data);
    const auto target = version_write_target(*language_state_, data.aggregate);
    const auto& source = data.parts;
    state::builder::builder_t builder{
        *language_state_,
        state::builder::builder_t::on_existing_state_t{}
    };
    builder.create_ordering_statement({
        .version_definition_node = target.vd_definition,
        .index = language_state_->next_version_statement_index(target.vd_aggregate, target.version_index),
        .name = data.name,
        .ordering_parts =
            source
            | ranges::views::transform([](const strings_t& part)
            {
                return part | ranges::to<std::deque<name_t>>();
            })
            | ranges::to<state::builder::builder_t::create_ordering_statement_t::ordering_parts_t>()
        ,
        .location = {}
    });
}

}// namespace galimatron::translator::transforms::cpp2cpp_wizard
