#pragma once

// galimatron
#include "galimatron/translator_library/forward.hpp"
#include "galimatron/common_library/path_report.hpp"


namespace galimatron::translator::galimatron_config
{

// directory paths are absolute or relative to the config (unless stated otherwise)

namespace detail
{

// settings that did not fall into other groups
struct galimatron_t
{
    // input folder with .gal files
    path_t input_gal_dir;

    // output folder with .gal files
    // usually equal to input_gal_dir (tests usually set a different one)
    path_t output_gal_dir;

    // intermediate xml goes here (principal xml, ...)
    path_t intermediate_xml_dir;

    // intermediate logs go here
    path_t intermediate_log_dir;

    // galimatron_summary.xml goes here
    path_t intermediate_summary_dir;

    // the summary, the info log, and exception text spell paths relative to the gal.cfg directory
    // and omit the state size and the run time. absent means false
    bool committed_report = false;
};

struct saxon_processor_t
{
    // xsl of the transforms and their accompanying xml are placed here
    path_t working_dir;

    // relative to working_dir
    path_t config_file;
};

namespace transform
{

namespace input
{

struct principal_xsl_parameter_t
{
    std::string name;
    std::string value;
};
using principal_xsl_parameters_t = std::list<principal_xsl_parameter_t>;

struct principal_xsl_t
{
    deferred_file_content_with_path_t file;// if relative, then relative to saxon_processor_t::working_dir
    principal_xsl_parameters_t parameters;// <xsl:param name="name" select="'value'"/>
};

struct input_t
{
    principal_xsl_t principal_xsl;
};

}// namespace input

namespace output
{

struct principal_xml_t
{
    path_ot file;// relative only, relative to galimatron_t::intermediate_xml_dir
};

struct other_files_t
{
    path_t store_dir;
    std::string store_dir_string_alias;
    std::set<path_t> extentions;
    bool replace_escaped_form_to_original_symbols = false;
    // files of this store that the transform did not rewrite are removed
    bool delete_if_not_rewrited = true;
    // the filesystem place for this store, set when the run starts. gal.cfg does not carry it
    index_ot place_id;
};
using other_files_list_t = std::list<other_files_t>;

struct output_t
{
    principal_xml_t principal_xml;
    other_files_list_t other_files;
};

}// namespace output

struct transform_t
{
    // absent in gal.cfg means the transform runs
    bool enable = true;

    std::string name;

    // any data; will be placed in transforms_info.xml
    content_ot user_data_for_info;

    input::input_t input;
    output::output_t output;
};
using transforms_t = std::list<transform_t>;

}// namespace transform

namespace transforms
{

struct buildin_transforms_t
{
};

struct user_transforms_t
{
    transform::transforms_t transforms;
};

struct language_transforms_t
{
    buildin_transforms_t buildin_transforms;
    user_transforms_t user_transforms;
};

struct post_language_transforms_t
{
    user_transforms_t user_transforms;
};

struct transforms_t
{
    language_transforms_t language_transforms;
    post_language_transforms_t post_language_transforms;
};

}// namespace transforms

}// namespace detail

struct config_t
{
    // general settings
    detail::galimatron_t galimatron;

    // Saxon processor settings
    detail::saxon_processor_t saxon_processor;

    // transforms
    detail::transforms::transforms_t transforms;

    // directory of the gal.cfg this object was read from. empty when built by hand
    path_t config_dir;
};

// path text for the summary, the info log, and exception text.
// committed-report spells it relative to the gal.cfg directory, with '/'
//
// testing: yes (unit)
//
inline std::string report_path(
    const config_t& config,
    const path_t& path
)
{
    if (!config.galimatron.committed_report || config.config_dir.empty())
    {
        return path.string();
    }

    const auto directory = [](path_t value)
    {
        value = value.lexically_normal();
        if (value.has_relative_path() && !value.has_filename())
        {
            value = value.parent_path();
        }
        return value;
    };

    return directory(path).lexically_relative(directory(config.config_dir)).generic_string();
}

// path text from the copies an exception or a caller already holds.
// committed_report false or an empty config_dir spells the path as stored
//
// testing: yes (unit)
//
inline std::string report_path(
    const path_t& path,
    const bool committed_report,
    const path_t& config_dir
)
{
    config_t config;
    config.galimatron.committed_report = committed_report;
    config.config_dir = config_dir;
    return report_path(config, path);
}

// path text for the current run. the catch does not call this; it uses the copies on e_path_info
//
// testing: yes (unit)
//
inline std::string report_path(const path_t& path)
{
    const auto& slot = galimatron::current_path_report();
    return report_path(path, slot.committed_report, slot.config_dir);
}

using transform_t = detail::transform::transform_t;
using transforms_t = detail::transform::transforms_t;

// the transform is part of the run. enable false in gal.cfg leaves it out
//
inline bool is_enabled(const transform_t& transform)
{
    return transform.enable;
}

}// namespace galimatron::translator::galimatron_config
