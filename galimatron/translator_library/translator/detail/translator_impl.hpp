#pragma once

// galimatron
#include "galimatron/translator_library/forward.hpp"
#include "galimatron/translator_library/galimatron_config_declaration.hpp"
#include "galimatron/translator_library/state/holder/holder.hpp"
#include "galimatron/translator_library/translator/translator.hpp"
#include "galimatron/translator_library/pugi_forward.hpp"
#include "galimatron/translator_library/translator/detail/any_states.hpp"
#include "galimatron/translator_library/translator/detail/transform_sequence_builder.hpp"
#include "galimatron/translator_library/transforms/xml2xml/xml2xml.hpp"

// Taskflow
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include <taskflow/taskflow.hpp>


namespace galimatron::translator::translator
{

struct transform_t
{
    transform_sequence_builder::domain_and_state_t incoming_domain_and_state;
    transform_sequence_builder::domain_and_state_t outgoing_domain_and_state;

    // generating transform
    const galimatron_config::transform_t& config_transform;

    // named transform, link to the config
    detail::named_oet named;

    using job_ft = std::function<file_contents_with_paths_ot (const transform_t&, tf::Subflow&)>;
    using is_skipping_ft = std::function<bool (const transform_t&)>;

    job_ft job;
    is_skipping_ft is_skipping;

    detail::origin_et origin = detail::origin_et::BUILD_IN;

    // post xml prints the compiled document. language xml stays the language document
    bool print_as_compiled_document = false;

    // the static description of the principal xml this step prints.
    // empty leaves <galimatron> node without the attribute
    std::string_view description = {};
};
using transforms_t = std::list<transform_t>;

namespace tfs_ns = galimatron_config::detail::transforms;


class translator_t
{
public:
    translator_t(const config_t&);
    ~translator_t();

    translator_t(const translator_t&) = delete;
    translator_t& operator=(const translator_t&) = delete;
    translator_t(translator_t&&) = delete;
    translator_t& operator=(translator_t&&) = delete;

public:
    void translate();

private:
    void translate_internal();
    void prepare_transforms();
    void prepare_testing_transforms();
    void prepare_prod_transforms();
    void execute_transform(index_t, const transform_t&, tf::Subflow&);

    // stores a builtin step as if it came from gal.cfg.
    // an empty principal_xml means the step prints no file.
    // the reference stays valid: config_transforms_ is a list
    //
    const galimatron_config::transform_t& store_builtin_transform(
        const std::string_view name,
        const path_ot principal_xml = {}
    );

    transform_t get_gal2cpp_transform(
        // production passes false: validation writes lang-parsed.xml after the compile
        const bool writes_parsed_xml = false
    );
    transform_t get_cpp2cpp_validate_transform();
    transform_t get_cpp2cpp_wizard_transform();
    transform_t get_cpp2cpp_before_xslt_transform();
    transform_t get_cpp2cpp_after_xslt_transform();
    transform_t get_cpp2xml_transform();
    transform_t get_xml2saxon_transform();
    transform_t get_xml2xml_transform(const galimatron_config::transform_t&);
    transform_t get_xml2cpp_transform();
    transform_t get_cpp2gal_transform();
    transform_t get_cpp2post_cpp_transform();
    transform_t get_post_cpp2post_xml_transform();
    transform_t get_post_xml2saxon_transform();
    transform_t get_post_xml2post_xml_transform(const galimatron_config::transform_t&);

    // compiles the current graph when the cache is empty.
    // no root aggregate — nothing to compile
    //
    void ensure_compiled();

    // a language document compiles for history when this run validates or compiles
    //
    bool language_xml_compiles() const;

    // the language document, or the compiled document when as_compiled_document is set.
    // <history> node is a child of <galimatron> node. description is the <galimatron> node attribute
    //
    pugi_document_with_meanful_node_t document_cpp2xml(
        const bool as_compiled_document,
        const std::string_view description
    );

    // prints document_cpp2xml
    //
    file_content_t translate_cpp2xml(
        const bool as_compiled_document,
        const std::string_view description
    );

    // places the <history> node under <galimatron> node
    //
    void append_history(pugi::xml_document& language);

private:
    const config_t args_;

    const tfs_ns::language_transforms_t& language_transforms_;
    const tfs_ns::post_language_transforms_t& post_language_transforms_;

    transforms_t transforms_;
    galimatron_config::transforms_t config_transforms_;

    state::holder::language_part_sptr_t language_state_;
    // language graph as it was before the language xslt. cpp2xml drops language_state_
    state::holder::language_part_sptr_t language_before_xslt_;
    // compile of language_state_. dropped when that graph is replaced or edited in place
    state::holder::compiled_part_sptr_ot compiled_state_;
    any_states_t any_transform_states_;

    tf::Taskflow taskflow_;

    // one executor for the process. each task attaches its worker to Saxon
    // and detaches that worker on the way out. the thread that created
    // the isolate stays attached. a worker left attached dies later
    // with StackOverflowError
    //
    static tf::Executor& executor();
};

}// namespace galimatron::translator::translator
