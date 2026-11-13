#pragma once

// galimatron
#include "galimatron/translator_library/forward.hpp"
#include "galimatron/translator_library/galimatron_config_declaration.hpp"
#include "galimatron/translator_library/pugi_forward.hpp"

// Saxon
#include <saxonc/SaxonProcessor.h>
#include <saxonc/XdmNode.h>
#include <saxonc/XdmValue.h>
#include <saxonc/Xslt30Processor.h>
#include <saxonc/XsltExecutable.h>


namespace galimatron::translator::transforms::xml2xml
{

// one Saxon processor and its document for the whole translation.
// apply() replaces the document, and the next apply() uses that result.
//
// testing: no
//
class saxon_session_t
{
public:
    explicit saxon_session_t(const galimatron_config::detail::saxon_processor_t& config);

    saxon_session_t(const saxon_session_t&) = delete;
    saxon_session_t& operator=(const saxon_session_t&) = delete;
    saxon_session_t(saxon_session_t&&) = delete;
    saxon_session_t& operator=(saxon_session_t&&) = delete;

public:
    void adopt(const file_content_t& xml);
    bool has_document() const;
    file_content_t printout() const;
    pugi_document_with_meanful_node_t to_pugi() const;

    // the node getHead returned. owned means the caller deletes it
    //
    struct taken_node_t
    {
        XdmNode* node = nullptr;
        bool owned = false;
    };

    // getHead hands the node to the caller when the document holds it.
    // owned is false when the document is that node and the session still deletes it
    //
    taken_node_t take_document_node();

    file_contents_with_paths_ot apply(const galimatron_config::transform_t& config);
    file_content_t apply_primary(const std::string& xslt);

private:
    void prepare();

    // prints with a newline, Saxon's 3-space indent, and an XML declaration.
    // Saxon-HE has no indent-spaces. toString omits the declaration
    //
    file_content_t serialize(XdmValue* node) const;

    // compiles the xslt and replaces the document with the primary result.
    // parameters are set before compile. side results are captured when asked
    //
    struct compile_and_apply_t
    {
        const std::string& xslt;
        const galimatron_config::detail::transform::input::principal_xsl_parameters_t* parameters = nullptr;
        bool capture_side_results = false;
    };
    void compile_and_apply(const compile_and_apply_t&);

private:
    const galimatron_config::detail::saxon_processor_t& config_;

    std::shared_ptr<SaxonProcessor> processor_;
    std::shared_ptr<Xslt30Processor> transformer_;
    std::shared_ptr<XsltExecutable> executable_;
    std::shared_ptr<XdmValue> document_;
};

}// namespace galimatron::translator::transforms::xml2xml
