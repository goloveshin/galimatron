// galimatron
#include "galimatron/translator_library/translator/detail/translator_impl.hpp"

// Saxon
#include <saxonc/SaxonProcessor.h>

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


void translator_t::execute_transform(const index_t index, const transform_t& tf, tf::Subflow& subflow)
{
    // attach records this worker, so detach can release it when the task returns.
    // detachCurrentThread leaves the worker in the isolate while the recorded
    // thread is still the one that created it. that creator stays attached.
    auto detach_saxon_worker = scope_guard_t(
        []
        {
            if (SaxonProcessor::sxn_environ)
            {
                SaxonProcessor::attachCurrentThread();
            }
        },
        []
        {
            if (SaxonProcessor::sxn_environ)
            {
                SaxonProcessor::detachCurrentThread();
            }
        }
    );

    const auto& config_tf = tf.config_transform;

    auto print_transform_beginend = scope_guard_t(
        [&]
        {
            fLogmeI("begin of transformation: {}, '{}'", index, config_tf.name);
        },
        [&](scope_guard_t& print_transform_beginend)
        {
            const auto suffix = [&] () -> std::string
            {
                return args_.galimatron.committed_report
                    ? ""
                    : std::format(": {}", print_transform_beginend.duration().text());
            }();
            fLogmeI(
                "end   of transformation: {}, '{}'{}",
                index,
                config_tf.name,
                suffix
            );
        }
    );

    file_contents_with_paths_ot produced_files;
    file_content_with_path_ot produced_xml;
    auto status = detail::status_et::SUCCESS;
    const int uncaught_on_enter = std::uncaught_exceptions();

    auto notify_transform_result = scope_guard_t(
        []{},
        [&]
        {
            if (std::uncaught_exceptions() > uncaught_on_enter)
            {
                status = detail::status_et::FAILED;
            }

            args_.notify_transform_result({
                .sequential_number = index,
                .name = config_tf.name,
                .origin = tf.origin,
                .named = tf.named,
                .status = status,
                .produced_xml = produced_xml,
                .produced_files = produced_files,
                .config_transform = tf.config_transform
            });
        }
    );

    const bool is_skipping = tf.is_skipping && tf.is_skipping(tf);

    // executor does the job
    produced_files = [&] -> file_contents_with_paths_ot
    {
        if (is_skipping)
        {
            fLogmeI("skip transformation");
            status = detail::status_et::SKIPPED;
            return {};
        }

        return tf.job(tf, subflow);
    }();

    produced_xml = [&] -> file_content_with_path_ot
    {
        if (is_skipping)
        {
            return {};
        }

        if (!config_tf.output.principal_xml.file)
        {
            return {};
        }

        const file_content_ot content = [&] -> file_content_ot
        {
            auto& saxon = any_transform_states_.state_slot(state_slot_et::SAXON_SESSION);

            if (transforms::xml2xml::operation_t::has_document(saxon))
            {
                return transforms::xml2xml::operation_t::printout(saxon);
            }
            else if (language_state_)
            {
                return translate_cpp2xml(
                    tf.print_as_compiled_document,
                    tf.description
                );
            }
            return {};
        }();

        return content.transform([&](const file_content_t& file_content)
        {
            return file_content_with_path_t{
                .content = file_content,
                .path = *config_tf.output.principal_xml.file
            };
        });
    }();
}

}// namespace galimatron::translator::translator
