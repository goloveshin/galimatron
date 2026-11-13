// galimatron
#include "galimatron/translator_library/translator/detail/translator_impl.hpp"
#include "galimatron/translator_library/translator/builtin_transform_name.hpp"
#include "galimatron/translator_library/translator/builtin_transform_xml.hpp"
#include "galimatron/translator_library/transforms/cpp2xml/cpp2xml.hpp"
#include "galimatron/translator_library/transforms/cpp2post_cpp/cpp2post_cpp.hpp"
#include "galimatron/translator_library/transforms/history/history.hpp"
#include "galimatron/translator_library/capture_or_rethrow.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>



namespace galimatron::translator::translator
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_tsp);
}


transform_t translator_t::get_cpp2xml_transform()
{
    using enum transform_sequence_builder::domain_et;
    using enum transform_sequence_builder::state_type_et;

    return {
        .incoming_domain_and_state = {
            .domain = LANGUAGE,
            .state_type = CPP
        },
        .outgoing_domain_and_state = {
            .domain = LANGUAGE,
            .state_type = XML
        },
        .config_transform = store_builtin_transform(
            builtin_transform_name::cpp2xml,
            path_t{ builtin_transform_xml::cpp2xml }
        ),
        .named = {},
        .job = [&](const transform_t&, tf::Subflow&) -> file_contents_with_paths_ot
        {
            return {};
        },
        .description = builtin_transform_xml::cpp2xml_description
    };
}

// compiles the current graph when the cache is empty.
// no root aggregate — nothing to compile
//
void translator_t::ensure_compiled()
{
    if (compiled_state_ || !language_state_ || !language_state_->aggregates().has_root())
    {
        return;
    }

    // empty result: try_capture_all for void does not drop the context on success
    capture_or_rethrow([&]() -> leaf::result<void>
    {
        compiled_state_ = transforms::cpp2post_cpp::translate(language_state_);
        return {};
    });
}

// a language document compiles for history when this run validates or compiles
//
bool translator_t::language_xml_compiles() const
{
    return ranges::any_of(transforms_, [](const transform_t& tf)
    {
        const auto& name = tf.config_transform.name;
        return name == name_t{ builtin_transform_name::cpp2cpp_validate }
            || name == name_t{ builtin_transform_name::cpp2post_cpp };
    });
}

// the language document, or the compiled document when as_compiled_document is set.
// <history> node is a child of <galimatron> node
//
pugi_document_with_meanful_node_t translator_t::document_cpp2xml(
    const bool as_compiled_document,
    const std::string_view description
)
{
    // a compiled document needs the compile. a language document compiles when this run validates or compiles
    if (as_compiled_document || language_xml_compiles())
    {
        ensure_compiled();
    }
    auto pugi_document = transforms::cpp2xml::translate({
        .language_state = language_state_,
        .compiled_state = as_compiled_document ? compiled_state_ : state::holder::compiled_part_sptr_ot{},
        .split_path_by_parts = args_.split_path_by_parts,
        .layout_table = &any_transform_states_.layout_table(),
        .description = description
    });
    append_history(*pugi_document.document);
    return pugi_document;
}

// prints document_cpp2xml
//
file_content_t translator_t::translate_cpp2xml(
    const bool as_compiled_document,
    const std::string_view description
)
{
    const auto pugi_document = document_cpp2xml(as_compiled_document, description);
    return galimatron::translator::printout(*pugi_document.document);
}

// places the <history> node under <galimatron> node
//
void translator_t::append_history(pugi::xml_document& language)
{
    if (!language_state_)
    {
        return;
    }

    auto root = language.document_element();
    if (!root)
    {
        return;
    }

    state::holder::compiled_part_t unused;
    auto& compiled = compiled_state_ ? **compiled_state_ : unused;
    transforms::history::document(
        root,
        *language_state_,
        compiled
    );
}

}// namespace galimatron::translator::translator
