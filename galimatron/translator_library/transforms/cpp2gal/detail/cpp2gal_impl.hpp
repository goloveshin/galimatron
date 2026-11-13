#pragma once

// galimatron
#include "galimatron/translator_library/xml_tags.hpp"
#include "galimatron/translator_library/state/gal_and_xml_tags.hpp"
#include "galimatron/translator_library/transforms/cpp2gal/cpp2gal.hpp"


namespace galimatron::translator::transforms::cpp2gal
{

using namespace galimatron::translator::state;

namespace tags = gal_and_xml_tags;
namespace GaL = tags::GaL;

class cpp2gal_t
{
public:
    static constexpr std::size_t tab_size = 4;

    struct line_t
    {
        int tab_count;
        content_t content;

        std::size_t size() const
        {
            const auto tabs = tab_count > 0 ? static_cast<std::size_t>(tab_count) : 0;
            return tabs + content.size();
        }
    };
    using lines_t = std::deque<line_t>;

public:
    cpp2gal_t() = delete;

    explicit cpp2gal_t(const translate_t&);

    cpp2gal_t(const cpp2gal_t&) = delete;
    cpp2gal_t& operator=(const cpp2gal_t&) = delete;
    cpp2gal_t(cpp2gal_t&&) = delete;
    cpp2gal_t& operator=(cpp2gal_t&&) = delete;

public:
    file_contents_with_paths_t translate();

private:
    struct walk_children_t
    {
        vertex_descriptor_t vd;
        bool tabulate_on_enter = true;
        bool tabulate_on_exit = true;
        // this child is not printed during the walk and is returned to the caller
        std::optional<entity_et> skip = {};
    };
    vertex_descriptor_ot walk_children(const walk_children_t&);
    void emit(vertex_descriptor_t);

    void emit_chunk(vertex_descriptor_t, const chunk_t&);

    void emit_root_aggregate(vertex_descriptor_t, const root_aggregate_t&);
    void emit_root_aggregate_version_declarations(vertex_descriptor_t, const root_aggregate_version_declarations_t&);
    void emit_root_aggregate_version_declaration(vertex_descriptor_t, const root_aggregate_version_declaration_t&);
    void emit_root_aggregate_version_definitions(vertex_descriptor_t, const root_aggregate_version_definitions_t&);
    void emit_root_aggregate_version_definition(vertex_descriptor_t, const root_aggregate_version_definition_t&);

    void emit_aggregate(vertex_descriptor_t, const aggregate_t&);
    void emit_aggregate_version_declarations(vertex_descriptor_t, const aggregate_version_declarations_t&);
    void emit_aggregate_version_declaration(vertex_descriptor_t, const aggregate_version_declaration_t&);
    void emit_aggregate_version_definitions(vertex_descriptor_t, const aggregate_version_definitions_t&);
    void emit_aggregate_version_definition(vertex_descriptor_t, const aggregate_version_definition_t&);

    void emit_tag_statement(vertex_descriptor_t, const tag_statement_t&);
    void emit_tag_value(vertex_descriptor_t, const tag_value_t&);
    void emit_tag_record(vertex_descriptor_t, const tag_record_t&);
    void emit_tag_keyvalue(vertex_descriptor_t, const tag_keyvalue_t&);
    void emit_tag_array(vertex_descriptor_t, const tag_array_t&);
    void emit_tag_string(const tag_string_t&);

    void emit_label_statement(vertex_descriptor_t, const label_statement_t&);
    void emit_ordering_statement(vertex_descriptor_t, const ordering_statement_t&);
    void emit_operation_statement(vertex_descriptor_t, const operation_statement_t&);
    void emit_wizard_statement(vertex_descriptor_t, const wizard_statement_t&);

private:
    void append_user_comments(vertex_descriptor_t);
    // the comment after the statement semicolon, including the spaces before it
    //
    void append_statement_comment(vertex_descriptor_t);
    // parsed source text, including its spaces and line breaks
    // a wizard-created vertex has none
    //
    bool append_user_formatting(vertex_descriptor_t);
    void append_line(const std::string&);
    void append_line(const std::string_view&);
    void append_line(const char*);
    void append_empty_line();
    void append_back(const std::string_view&);

    static std::size_t size_of_lines(const lines_t&);
    static void join_lines(const lines_t&, content_t&);

    void trim_comma_back();

    vertex_descriptor_ot find_child(vertex_descriptor_t, entity_et) const;

private:
    const translate_t args_;
    const holder::language_part_t::graph_t& graph_;

    file_contents_with_paths_t contents_;
    lines_t lines_;
    std::deque<int> tab_count_;
};

}// namespace galimatron::translator::transforms::cpp2gal
