// galimatron
#include "galimatron/translator_library/transforms/cpp2xml/detail/cpp2xml_impl.hpp"

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


// writes the delimiter list onto the galimatron node, in index order
//
void cpp2xml_t::write_string_delimiters()
{
    if (!language_state_ || !doc_)
    {
        return;
    }

    auto list = doc_->document_element().append_child(gal_and_xml_tags::document::string_delimiters.data());

    for (const auto& delimiter : language_state_->string_delimiters())
    {
        auto entry = list.append_child(gal_and_xml_tags::document::string_delimiter.data());
        {
            entry.append_attribute(tag_c::raw.data()) =
                delimiter.raw
                ? tag_c::bool_true.data()
                : tag_c::bool_false.data();
        }
        {
            const auto quote = std::string(1, delimiter.quote);
            entry.append_attribute(tag_c::quote.data()) = quote.c_str();
        }
        if (!delimiter.dashes.empty())
        {
            entry.append_attribute(tag_c::dashes.data()) = delimiter.dashes.c_str();
        }
    }
}

void cpp2xml_t::remember_layout(const vertex_t& vertex, pugi::xml_node node)
{
    if (!args_.layout_table)
    {
        return;
    }

    const bool has_formatting = vertex.user_formatting && !vertex.user_formatting->empty();
    const bool has_comments = !vertex.user_comments.empty();
    if (!has_formatting && !has_comments)
    {
        return;
    }

    const auto fingerprint = layout::fingerprint(node);
    const auto mark = ++args_.layout_table->next;
    const auto text = std::to_string(mark);
    node.append_attribute(tag_c::layout_mark) = text.c_str();
    args_.layout_table->entries.insert({
        mark,
        {
            .user_formatting = vertex.user_formatting,
            .user_comments = vertex.user_comments,
            .fingerprint = fingerprint
        }
    });
}

void cpp2xml_t::append_wizard_mark(pugi::xml_node node, const std::string_view value)
{
    if (compiled_state_)
    {
        return;
    }

    auto mark = node.append_child(gal_and_xml_tags::wizard_mark::dynamic);
    mark.append_attribute(gal_and_xml_tags::wizard_mark::wizard) = std::string{ value };
}

cpp2xml_t::cpp2xml_t(const translate_t& args)
    : args_(args)
    , compiled_state_(args_.compiled_state)
    , language_state_(args_.language_state)
{
}

pugi::xml_node cpp2xml_t::push_parent(const pugi::xml_node& node)
{
    parent_nodes_.push(node);
    return parent();
}

void cpp2xml_t::pop_parent()
{
    parent_nodes_.pop();
}

pugi::xml_node cpp2xml_t::parent() const
{
    return parent_nodes_.top();
}

const vertex_t& cpp2xml_t::vertex()
{
    if (vertex_)
    {
        return vertex_->get();
    }

    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::cpp2xml{},
        exception::e_what_info{ "no current vertex" }
    );
}

}// namespace galimatron::translator::transforms::cpp2xml
