// galimatron
#include "galimatron/runtime_library/translate/detail/translate_impl.hpp"
#include "galimatron/runtime_library/crash/crash.hpp"
#include "galimatron/runtime_library/filesystem_decorator/filesystem_decorator.hpp"
#include "galimatron/runtime_library/filesystem/filesystem.hpp"
#include "galimatron/runtime_library/leaf_spy/leaf_spy.hpp"
#include "galimatron/runtime_library/translate_summary_xml/translate_summary_xml.hpp"
#include "galimatron/runtime_library/utils/omit_filesystem.hpp"
#include "galimatron/runtime_library/utils/replace_escaped.hpp"
#include "galimatron/translator_library/galimatron_config_declaration.hpp"
#include "galimatron/translator_library/parser/gal_version.hpp"
#include "galimatron/translator_library/translator/translator.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Boost.Units
#include <boost/units/quantity.hpp>
#include <boost/units/systems/information/byte.hpp>
#include <boost/units/systems/information/prefixes.hpp>

// Logme
#include <Logme/SID.h>


namespace galimatron::runtime
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::rt_trn);
}


namespace ex = galimatron::executable;
namespace nfsd = ex::filesystem_decorator;
namespace translator = translator::translator;

using namespace ranges;
using galimatron::translator::galimatron_config::is_enabled;

namespace
{

void add_directory_parameter(
    galimatron::translator::galimatron_config::detail::transform::input::principal_xsl_parameters_t& parameters,
    std::string name,
    const path_t& path
)
{
    parameters.push_back({
        .name = std::move(name),
        .value = path.generic_string()
    });
}

// the xslt receives the top-level directories gal.cfg already resolved, as absolute paths
//
void add_directory_parameters(
    galimatron::translator::galimatron_config::detail::transform::transforms_t& transforms,
    const galimatron_config::config_t& config
)
{
    for (auto& tf : transforms | views::filter(is_enabled))
    {
        auto& parameters = tf.input.principal_xsl.parameters;
        add_directory_parameter(parameters, "input_gal_dir", config.galimatron.input_gal_dir);
        add_directory_parameter(parameters, "output_gal_dir", config.galimatron.output_gal_dir);
        add_directory_parameter(parameters, "intermediate_xml_dir", config.galimatron.intermediate_xml_dir);
        add_directory_parameter(parameters, "intermediate_log_dir", config.galimatron.intermediate_log_dir);
        add_directory_parameter(parameters, "intermediate_summary_dir", config.galimatron.intermediate_summary_dir);
        add_directory_parameter(parameters, "saxon_working_dir", config.saxon_processor.working_dir);
    }
}

// one unit for the private-memory size: gigabytes from 1 GB, megabytes from 1 MB, otherwise bytes.
// the byte count follows in parentheses
//
std::string state_memory_text(const std::size_t bytes)
{
    namespace information = boost::units::information;
    const auto amount = static_cast<double>(bytes) * information::byte;
    const auto megabytes = boost::units::quantity<decltype(information::mebi * information::byte), double>(amount).value();
    const auto gigabytes = boost::units::quantity<decltype(information::gibi * information::byte), double>(amount).value();
    if (gigabytes >= 1.0)
    {
        return std::format("{:.1f} GB ({} bytes)", gigabytes, bytes);
    }
    if (megabytes >= 1.0)
    {
        return std::format("{:.1f} MB ({} bytes)", megabytes, bytes);
    }
    return std::format("{} bytes", bytes);
}

}// namespace

void translate_impl_t::translate(const galimatron_config::config_t& config)
{
    auto filesystem = ex::filesystem::physically::make_filesystem(args_.content_filter);

    auto fsd = nfsd::filesystem_decorator_t(
    {
        .filesystem = filesystem
    });

    const bool is_normal_mode = (config.galimatron.input_gal_dir == config.galimatron.output_gal_dir);

    if (is_normal_mode)
    {
        fLogmeI("normal mode - unparsing will be overwrite incoming .gal");
    }
    else
    {
        fLogmeI(
            "separate directories - {} is only read. Those .gal files stay on disk as they were.",
            galimatron::translator::galimatron_config::report_path(config, config.galimatron.input_gal_dir)
        );
        fLogmeI(
            "separate directories - {} receives the unparsed .gal. At the start every .gal already in that folder is deleted. Each file this run produces is written there immediately, once.",
            galimatron::translator::galimatron_config::report_path(config, config.galimatron.output_gal_dir)
        );
    }

    // places of incoming/outgoing .gal files
    // in tests these places usually differ, as does the behavior
    const auto [ input_gal_place, output_gal_place ] = [&] -> std::pair<nfsd::place_id_t, nfsd::place_id_t>
    {
        if (is_normal_mode)
        {
            const auto input = fsd.add_place({
                .name = "input/output .gal files",
                .root = config.galimatron.input_gal_dir,
                .extentions = {".gal"},
                .read = {
                    .cache = true
                },
                .write = {
                    .delayed_to_demand = true,
                    .if_content_differ = true,
                    .once = true
                },
                .remove = {
                    .at_begin = false,
                    .not_rewrited_at_end = true
                }
            });
            return { input, input };
        }
        else // the directories differ
        {
            const auto input = fsd.add_place({
                .name = "input .gal files",
                .root = config.galimatron.input_gal_dir,
                .extentions = {".gal"},
                .read = {
                    .cache = false
                },
                .write = {
                    .delayed_to_demand = false,
                    .if_content_differ = false,
                    .once = false
                },
                .remove = {
                    .at_begin = false,
                    .not_rewrited_at_end = false
                }
            });
            const auto output = fsd.add_place({
                .name = "output .gal files",
                .root = config.galimatron.output_gal_dir,
                .extentions = {".gal"},
                .read = {
                    .cache = false
                },
                .write = {
                    .delayed_to_demand = false,
                    .if_content_differ = false,
                    .once = true
                },
                .remove = {
                    .at_begin = true,
                    .not_rewrited_at_end = false
                }
            });
            return { input, output };
        }
    }();

    // place for intermediate .xml files
    // logs go to galimatron.intermediate_log_dir and are opened from log.cfg
    const auto intermediate_place = fsd.add_place({
        .name = "intermediate xml files",
        .root = config.galimatron.intermediate_xml_dir,
        .extentions = {".xml", ".html"},
        .read = {
            .cache = false
        },
        .write = {
            .delayed_to_demand = false,
            .if_content_differ = false,
            .once = true
        },
        .remove = {
            .at_begin = true,
            .not_rewrited_at_end = false
        }
    });

    // create places for generated files of the transforms that run.
    // a store-dir-string-alias is unique inside one transform. another transform may reuse it.
    auto run_transforms = config.transforms;
    const auto user_transform_lists =
    {
        &run_transforms.language_transforms.user_transforms.transforms,
        &run_transforms.post_language_transforms.user_transforms.transforms
    };
    for (const auto list : user_transform_lists)
    {
        for (auto& tf : *list | views::filter(is_enabled))
        {
            for (auto it = tf.output.other_files.begin(); it != tf.output.other_files.end(); ++it)
            {
                const auto& alias = it->store_dir_string_alias;
                const auto earlier = ranges::subrange(tf.output.other_files.begin(), it);
                if (ranges::any_of(earlier, [&](const auto& other)
                {
                    return other.store_dir_string_alias == alias;
                }))
                {
                    leaf::throw_exception(
                        exception::domain::runtime_library{},
                        exception::subdomain::runtime_library::translate{},
                        exception::e_what_info{ std::format(
                            "transform '{}': store-dir-string-alias '{}' is not unique",
                            tf.name,
                            alias
                        ) }
                    );
                }

                it->place_id = fsd.add_place({
                    .name = std::format("{} ({})", tf.name, alias),
                    .string_alias = alias,
                    .root = it->store_dir,
                    .extentions = it->extentions,
                    .read = {
                        .cache = false
                    },
                    .write = {
                        .delayed_to_demand = false,
                        .if_content_differ = true,
                        .once = true
                    },
                    .remove = {
                        .at_begin = false,
                        .not_rewrited_at_end = it->delete_if_not_rewrited
                    }
                });
            }
        }
    }

    auto summary_xml = translate_summary_xml::builder_t();
    const auto summary_file = config.galimatron.intermediate_summary_dir / path_t{ translate_summary_xml::file_name };

    auto guard = scope_guard_t(
        [&]{
            // failure by default
            summary_xml.set_result(false);
        },
        [&]{
            // if success, it is reported to summary_xml before this point
            filesystem.write(
                summary_file,
                summary_xml.printout()
            );
        }
    );

    auto process_result = [&](const translator::detail::translate_result_t& args)
    {
        const auto& [
            sequential_number,
            name,
            origin,
            named,
            status,
            produced_xml,
            produced_files,
            config_transform
        ] = args;

        const auto replace_escaped = [&](const std::string& alias) -> bool
        {
            const auto& stores = config_transform.output.other_files;
            const auto found = ranges::find_if(stores, [&](const auto& other_files)
            {
                return other_files.store_dir_string_alias == alias;
            });
            if (found == ranges::end(stores))
            {
                return false;
            }
            return found->replace_escaped_form_to_original_symbols;
        };

        path_ot produced_xml_path;

        if (produced_xml)
        {
            const auto content = is_normal_mode
                ? produced_xml->content
                : omit_filesystem_from_printout(produced_xml->content);

            produced_xml_path = fsd.printout_to_place(
                intermediate_place,
                produced_xml->path,
                content
            ).finally_path;
        }

        paths_ot produced_files_paths;

        if (produced_files)
        {
            produced_files_paths = paths_t{};

            if (named == translator::detail::named_et::CPP2GAL)
            {
                for (const auto& [content, path] : *produced_files)
                {
                    produced_files_paths->push_back(fsd.printout_to_place({
                        .place = output_gal_place,
                        .path = {
                            .path = path
                        },
                        .replace_by = {},
                        .content = content
                    }).finally_path);
                }
            }
            else
            {
                for (const auto& [content, mangled_uri] : *produced_files)
                {
                    const auto unmangled = fsd.unmangled_saxon_uri(mangled_uri.string());
                    const auto* parsed = std::get_if<nfsd::filesystem_decorator_t::unmangle_saxon_uri_t>(&unmangled);
                    const auto is_replace_escape = parsed
                        ? replace_escaped(parsed->string_alias)
                        : ranges::any_of(config_transform.output.other_files, [](const auto& other_files)
                        {
                            return other_files.replace_escaped_form_to_original_symbols;
                        });
                    auto content_to_write = is_replace_escape
                        ? replace_escaped_form_to_original_symbols(content)
                        : content;

                    if (!is_normal_mode && mangled_uri.extension() == ".xml")
                    {
                        content_to_write = omit_filesystem_from_printout(content_to_write);
                    }

                    if (parsed)
                    {
                        const auto& stores = config_transform.output.other_files;
                        const auto store = ranges::find_if(stores, [&](const auto& other_files)
                        {
                            return other_files.store_dir_string_alias == parsed->string_alias;
                        });
                        if (store == ranges::end(stores) || !store->place_id)
                        {
                            leaf::throw_exception(
                                exception::domain::runtime_library{},
                                exception::subdomain::runtime_library::translate{},
                                exception::e_what_info{ std::format(
                                    "transform '{}': no store-dir-string-alias '{}'",
                                    config_transform.name,
                                    parsed->string_alias
                                ) }
                            );
                        }

                        produced_files_paths->push_back(fsd.printout_to_place(
                            *store->place_id,
                            parsed->relative_path,
                            content_to_write
                        ).finally_path);
                    }
                    else
                    {
                        const auto& path = std::get<path_t>(unmangled);
                        // a relative path is written into intermediate-xml-dir
                        if (path.is_relative())
                        {
                            produced_files_paths->push_back(fsd.printout_to_place(
                                intermediate_place,
                                path,
                                content_to_write
                            ).finally_path);
                        }
                        else
                        {
                            filesystem.write(path, content_to_write);
                            produced_files_paths->push_back(path);
                        }
                    }
                }
            }
        }

        const auto summary_path = [&](const path_t& path)
        {
            if (!config.galimatron.committed_report)
            {
                return path;
            }
            return path_t{
                galimatron::translator::galimatron_config::report_path(config, path)
            };
        };
        const auto summary_xml_path = produced_xml_path
            ? path_ot{ summary_path(*produced_xml_path) }
            : path_ot{};
        const auto summary_files = [&]() -> paths_ot
        {
            if (!produced_files_paths)
            {
                return {};
            }
            auto paths = *produced_files_paths;
            return paths_ot{
                paths
                | views::transform(summary_path)
                | ranges::to<paths_t>()
            };
        }();

        summary_xml.append_transform({
            .sequential_number = sequential_number,
            .name = name,
            .origin = origin,
            .status = status,
            .produced_xml_path = summary_xml_path,
            .produced_files = summary_files,
            .user_data = config_transform.user_data_for_info
        });
    };

    // perform the translation
    // the exception text gets into the summary before the file is written in the guard destructor
    try_catch({
        .global_args = {
            .job_that_may_throw = [&]
            {
                fsd.do_jobs_at_begin();
                crash::arm_summary(summary_file);

                add_directory_parameters(run_transforms.language_transforms.user_transforms.transforms, config);
                add_directory_parameters(run_transforms.post_language_transforms.user_transforms.transforms, config);

                translator::translate({
                    .galimatron = {
                        .intermediate_summary_dir = config.galimatron.intermediate_summary_dir,
                        .committed_report = config.galimatron.committed_report
                    },
                    .saxon_processor = config.saxon_processor,
                    .transforms = run_transforms,
                    .gal_state = fsd.get_place_content(input_gal_place),
                    .gal_version = galimatron::translator::parser::gal_version::from_input_dir(
                        filesystem,
                        config.galimatron.input_gal_dir
                    ),
                    .split_path_by_parts = [&](const path_t& path)
                    {
                        return filesystem.split_path_by_parts({
                            .path_root = path,
                            .path = path
                        });
                    },
                    .testing = args_.testing,
                    .notify_transform_result = process_result,
                    .notify_taskflow_graph = [&](const content_t& graph)
                    {
                        const auto path = config.galimatron.intermediate_summary_dir
                            / path_t{ translator::taskflow_graph_file_name };
                        filesystem.write(path, make_file_content(graph));
                    },
                    .notify_state_memory = [&](const std::size_t bytes)
                    {
                        if (config.galimatron.committed_report)
                        {
                            return;
                        }
                        summary_xml.set_state_bytes(bytes);
                        fLogmeI("all state: {}", state_memory_text(bytes));
                    }
                });

                // reached here -> there were no exceptions and everything succeeded
                summary_xml.set_result(true);

                fsd.do_jobs_at_end();
            },
            .notify_that_job_executed_with_throw = []{},
            .notify_that_job_executed_without_throw = []{},
            .is_throw_out = true
        },
        .notify_printed = [&](const std::string& info)
        {
            summary_xml.set_error(info);
        }
    });
}

}// namespace galimatron::runtime
