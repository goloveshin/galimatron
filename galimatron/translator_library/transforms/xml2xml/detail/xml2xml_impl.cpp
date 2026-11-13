// galimatron
#include "galimatron/translator_library/transforms/xml2xml/detail/xml2xml_impl.hpp"
#include "galimatron/translator_library/capture_or_rethrow.hpp"
#include "galimatron/common_library/exception_augment.hpp"

// Boost.DLL
#include <boost/dll/runtime_symbol_info.hpp>

// Boost.LEAF
#include <boost/leaf/handle_errors.hpp>

// Saxon
#include <saxonc/XdmAtomicValue.h>

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::xml2xml
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_xslt);
}

namespace
{

// newXslt30Processor must stay alive while this reads the exception
//
std::string printout_native_saxon_exception(SaxonApiException& e)
{
    std::ostringstream ss;

    if (const char* str = e.what())
    {
        ss << std::format("what: '{}'", str) << std::endl;
    }

    if (const char* str = e.getErrorCode())
    {
        ss << std::format("getErrorCode: '{}'", str) << std::endl;
    }

    if (const int line = e.getLineNumber(); line != -1)
    {
        ss << std::format("getLineNumber: {}", line) << std::endl;
    }

    if (const char* str = e.getMessage())
    {
        ss << std::format("getMessage: '{}'", str) << std::endl;
    }

    if (const char* str = e.getMessageWithErrorCode())
    {
        ss << std::format("getMessageWithErrorCode: '{}'", str) << std::endl;
    }

    ss << std::format("staticErrorCount: {}", e.staticErrorCount()) << std::endl;

    if (const char* str = e.getSystemId())
    {
        ss << std::format("getSystemId: '{}'", str) << std::endl;
    }

    return ss.str();
}

// reads the Saxon exception, then throws translator.
// the path is empty when the xslt has no file.
// the caller catches while newXslt30Processor is still alive.
//
[[noreturn]] void throw_translator(SaxonApiException& e, const path_ot& path)
{
    const auto what = printout_native_saxon_exception(e);
    capture_or_rethrow([&]
    {
        leaf::throw_exception(
            exception::domain::translator_library{},
            exception::subdomain::translator_library::translator{},
            exception::e_what_info{ what },
            exception::e_path_info_for(path)
        );
    });
}

// the directory that contains this process
//
path_t executable_directory()
{
    std::error_code error;
    const auto location = boost::dll::program_location(error);
    if (error)
    {
        return {};
    }
    return location.parent_path();
}

// maps https://github.com/goloveshin/galimatron/xslt/gal-functions.xsl to the installed file.
// a build that has not been installed has no catalog, and a local include still resolves.
// the debug log names the result. none means this process has no catalog.
// Saxon receives the absolute file when the catalog is present
//
void load_installed_catalog(SaxonProcessor& processor)
{
    const auto from_executable = path_t("..") / "share" / "galimatron" / "catalog.xml";
    const auto directory = executable_directory();
    if (directory.empty())
    {
        fLogmeD("xslt catalog: none");
        return;
    }

    const auto catalog = (directory / from_executable).lexically_normal();
    std::error_code error;
    if (!std::filesystem::exists(catalog, error) || error)
    {
        fLogmeD("xslt catalog: none");
        return;
    }

    const auto name = catalog.generic_string();
    const char* files[] = { name.c_str() };
    try
    {
        processor.setCatalogFiles(files, 1);
    }
    catch (SaxonApiException& e)
    {
        throw_translator(e, path_ot{ catalog });
    }
    fLogmeD("xslt catalog: {}", from_executable.generic_string());
}

}// namespace


saxon_session_t::saxon_session_t(const galimatron_config::detail::saxon_processor_t& config)
    : config_(config)
{
}

void saxon_session_t::prepare()
{
    if (processor_)
    {
        return;
    }

    processor_ = std::make_shared<SaxonProcessor>(config_.config_file.string().c_str());
    load_installed_catalog(*processor_);
    transformer_ = std::shared_ptr<Xslt30Processor>(processor_->newXslt30Processor());
    transformer_->setcwd(config_.working_dir.string().c_str());
    fLogmeI("xslt processor: {}", processor_->version());
}

void saxon_session_t::adopt(const file_content_t& xml)
{
    if (document_)
    {
        return;
    }

    prepare();
    document_ = std::shared_ptr<XdmValue>(processor_->parseXmlFromString(xml->c_str()));
}

bool saxon_session_t::has_document() const
{
    return static_cast<bool>(document_);
}

file_content_t saxon_session_t::serialize(XdmValue* node) const
{
    if (!node)
    {
        return make_file_content("");
    }
    const char* xml = node->toString();
    if (!xml)
    {
        return make_file_content("");
    }
    const std::string text{ xml };
    operator delete(const_cast<char*>(xml));
    return make_file_content(text);
}

file_content_t saxon_session_t::printout() const
{
    return serialize(document_.get());
}

saxon_session_t::taken_node_t saxon_session_t::take_document_node()
{
    if (!document_)
    {
        return {};
    }

    auto* const head = dynamic_cast<XdmNode*>(document_->getHead());
    if (!head)
    {
        return {};
    }

    const auto* const document_node = dynamic_cast<XdmNode*>(document_.get());
    return {
        .node = head,
        .owned = head != document_node
    };
}

pugi_document_with_meanful_node_t saxon_session_t::to_pugi() const
{
    auto doc = std::make_shared<pugi::xml_document>();

    // XdmValue::size() is the item count. The serialized document is longer than that.
    const char* xml = document_->toString();
    if (auto result = doc->load_buffer(xml, std::strlen(xml)))
    {
        return {
            .document = doc
        };
    }
    else
    {
        leaf::throw_exception(
            exception::domain::translator_library{},
            exception::subdomain::translator_library::translator{},
            exception::e_what_info{
                std::format("xml parse error: {} at offset {}", result.description(), result.offset)
            }
        );
    }
}

void saxon_session_t::compile_and_apply(const compile_and_apply_t& args)
{
    prepare();
    executable_.reset();

    if (args.parameters)
    {
        for (const auto& parameter : *args.parameters)
        {
            auto* value = processor_->makeStringValue(parameter.value);
            transformer_->setParameter(parameter.name.c_str(), value);
        }
    }

    executable_ = std::shared_ptr<XsltExecutable>(transformer_->compileFromString(args.xslt.c_str()));
    executable_->setGlobalContextItem((XdmItem*)(document_.get()));
    executable_->setInitialMatchSelection(document_.get());
    if (args.capture_side_results)
    {
        executable_->setCaptureResultDocuments(true);// TODO raw
        executable_->setSaveXslMessage(true);
    }
    document_ = std::shared_ptr<XdmValue>(executable_->applyTemplatesReturningValue());
}

file_contents_with_paths_ot saxon_session_t::apply(const galimatron_config::transform_t& config)
{
    try
    {
        const auto& input = config.input;
        const auto xslt = input.principal_xsl.file.deferred_content();
        compile_and_apply({
            .xslt = *xslt,
            .parameters = &input.principal_xsl.parameters,
            .capture_side_results = true
        });

        if (XdmValue* messages = executable_->getXslMessages())
        {
            fLogmeI("begin messages from Saxon");
            for (const int index : std::views::iota(0, messages->size()))
            {
                fLogmeI("{}", messages->itemAt(index)->toString());
            }
            fLogmeI("end messages from Saxon");
        }

        const auto& result_docs_nodes = executable_->getResultDocuments();

        return
            result_docs_nodes
            | ranges::views::transform([&](const auto& pair) -> file_content_with_path_t
                {
                    const auto& [path, node] = pair;
                    // the result document already has its method. the XML serializer would prefix a text file with an XML declaration
                    const auto content = make_file_content(node->toString());
                    delete node;
                    return {
                        .content = content,
                        .path = path
                    };
                }
            )
            | ranges::to<std::list>
            ;
    }
    catch (SaxonApiException& e)
    {
        throw_translator(e, config.input.principal_xsl.file.path);
    }
    return {};
}

file_content_t saxon_session_t::apply_primary(const std::string& xslt)
{
    try
    {
        compile_and_apply({
            .xslt = xslt
        });
        return printout();
    }
    catch (SaxonApiException& e)
    {
        throw_translator(e, {});
    }
    return {};
}

}// namespace galimatron::translator::transforms::xml2xml
