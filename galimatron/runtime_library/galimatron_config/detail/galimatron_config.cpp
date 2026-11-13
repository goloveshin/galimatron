// galimatron
#include "galimatron/runtime_library/filesystem/filesystem.hpp"
#include "galimatron/runtime_library/galimatron_config/detail/galimatron_config_tags.hpp"
#include "galimatron/translator_library/galimatron_config_declaration.hpp"
#include "galimatron/runtime_library/galimatron_config/galimatron_config.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::rt_cfg);
}


// modify paths when reading from json
using replace_path_ft = std::function<void (path_t&)>;
replace_path_ft replace_path_func;
replace_path_ft replace_path_xsl_func;
replace_path_ft replace_path_xml_func;
// deferred reading
global::filesystem::abstract::read_ft read_file_func;

}// namespace

using nlohmann::json;

namespace galimatron::translator::galimatron_config
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::rt_cfg);
}


namespace detail
{

void from_json(const json& j, galimatron_t& obj)
{
    using namespace galimatron_ns;
    using obj_t = galimatron_t;
    {
        j.at(tag_input_gal_dir).get_to(obj.input_gal_dir); replace_path_func(obj.input_gal_dir);
        j.at(tag_output_gal_dir).get_to(obj.output_gal_dir); replace_path_func(obj.output_gal_dir);
        j.at(tag_intermediate_xml_dir).get_to(obj.intermediate_xml_dir); replace_path_func(obj.intermediate_xml_dir);
        j.at(tag_intermediate_log_dir).get_to(obj.intermediate_log_dir); replace_path_func(obj.intermediate_log_dir);
        j.at(tag_intermediate_summary_dir).get_to(obj.intermediate_summary_dir); replace_path_func(obj.intermediate_summary_dir);
        if (j.contains(tag_committed_report))
        {
            j.at(tag_committed_report).get_to(obj.committed_report);
        }
    }
}
void to_json(json& j, const galimatron_t& obj)
{
    using namespace galimatron_ns;
    using obj_t = galimatron_t;
    {
        j = json{
            {tag_input_gal_dir, obj.input_gal_dir},
            {tag_output_gal_dir, obj.output_gal_dir},
            {tag_intermediate_xml_dir, obj.intermediate_xml_dir},
            {tag_intermediate_log_dir, obj.intermediate_log_dir},
            {tag_intermediate_summary_dir, obj.intermediate_summary_dir},
            {tag_committed_report, obj.committed_report}
        };
    }
}

void from_json(const json& j, saxon_processor_t& obj)
{
    using namespace saxon_processor_ns;
    using obj_t = saxon_processor_t;
    {
        j.at(tag_working_dir).get_to(obj.working_dir); replace_path_func(obj.working_dir);
        j.at(tag_config_file).get_to(obj.config_file); replace_path_xsl_func(obj.config_file);
    }
}
void to_json(json& j, const saxon_processor_t& obj)
{
    using namespace saxon_processor_ns;
    using obj_t = saxon_processor_t;
    {
        j = json{
            {tag_working_dir, obj.working_dir},
            {tag_config_file, obj.config_file}
        };
    }
}

namespace transform
{

namespace input
{

void from_json(const json& j, principal_xsl_parameter_t& obj)
{
    using namespace principal_xsl_ns::xsl_parameters_ns;
    using obj_t = principal_xsl_parameter_t;
    {
        j.at(tag_name).get_to(obj.name);
        j.at(tag_value).get_to(obj.value);
    }
}
void to_json(json& j, const principal_xsl_parameter_t& obj)
{
    using namespace principal_xsl_ns::xsl_parameters_ns;
    using obj_t = principal_xsl_parameter_t;
    {
        j = json{
            {tag_name, obj.name},
            {tag_value, obj.value}
        };
    }
}

void from_json(const json& j, principal_xsl_t& obj)
{
    using namespace principal_xsl_ns;
    using obj_t = principal_xsl_t;
    {
        auto file = j.at(tag_file).get<path_t>(); replace_path_xsl_func(file);
        obj.file = {
            .deferred_content = [=, f = read_file_func]{ return f(file); },
            .path = file
        };
        j.at(tag_parameters).get_to(obj.parameters);
    }
}
void to_json(json& j, const principal_xsl_t& obj)
{
    using namespace principal_xsl_ns;
    using obj_t = principal_xsl_t;
    {
        j = json{
            {tag_file, obj.file.path},
            {tag_parameters, obj.parameters}
        };
    }
}

void from_json(const json& j, input_t& obj)
{
    using namespace input_ns;
    using obj_t = input_t;
    {
        j.at(tag_principal_xsl).get_to(obj.principal_xsl);
    }
}
void to_json(json& j, const input_t& obj)
{
    using namespace input_ns;
    using obj_t = input_t;
    {
        j = json{
            {tag_principal_xsl, obj.principal_xsl}
        };
    }
}

}// namespace input

namespace output
{

void from_json(const json& j, principal_xml_t& obj)
{
    using namespace principal_xml_ns;
    using obj_t = principal_xml_t;
    {
        if (j.count(tag_file))
        {
            auto path = j.at(tag_file).get<path_t>();
            replace_path_xml_func(path);
            obj.file = path;
        }
    }
}
void to_json(json& j, const principal_xml_t& obj)
{
    using namespace principal_xml_ns;
    using obj_t = principal_xml_t;
    {
        if (obj.file)
        {
            j[tag_file] = obj.file.value();
        }
    }
}

void from_json(const json& j, other_files_t& obj)
{
    using namespace other_files_ns;
    using obj_t = other_files_t;
    {
        j.at(tag_store_dir).get_to(obj.store_dir); replace_path_func(obj.store_dir);
        j.at(tag_store_dir_string_alias).get_to(obj.store_dir_string_alias);
        j.at(tag_extentions).get_to(obj.extentions);
        j.at(tag_replace_escaped_form_to_original_symbols).get_to(obj.replace_escaped_form_to_original_symbols);
        if (j.contains(tag_delete_if_not_rewrited))
        {
            j.at(tag_delete_if_not_rewrited).get_to(obj.delete_if_not_rewrited);
        }
    }
}
void to_json(json& j, const other_files_t& obj)
{
    using namespace other_files_ns;
    using obj_t = other_files_t;
    {
        j = json{
            {tag_store_dir, obj.store_dir},
            {tag_store_dir_string_alias, obj.store_dir_string_alias},
            {tag_extentions, obj.extentions},
            {tag_replace_escaped_form_to_original_symbols, obj.replace_escaped_form_to_original_symbols},
            {tag_delete_if_not_rewrited, obj.delete_if_not_rewrited},
        };
    }
}

void from_json(const json& j, output_t& obj)
{
    using namespace output_ns;
    using obj_t = output_t;
    {
        j.at(tag_principal_xml).get_to(obj.principal_xml);
        if (j.count(tag_other_files))
        {
            obj.other_files = j.at(tag_other_files).get<other_files_list_t>();
        }
    }
}
void to_json(json& j, const output_t& obj)
{
    using namespace output_ns;
    using obj_t = output_t;
    {
        j = json{
            {tag_principal_xml, obj.principal_xml}
        };
        if (!obj.other_files.empty())
        {
            j[tag_other_files] = obj.other_files;
        }
    }
}

}// namespace output

void from_json(const json& j, transform_t& obj)
{
    using namespace transform_ns;
    using obj_t = transform_t;
    {
        if (j.contains(tag_enable))
        {
            j.at(tag_enable).get_to(obj.enable);
        }
        j.at(tag_name).get_to(obj.name);
        j.at(tag_input).get_to(obj.input);
        j.at(tag_output).get_to(obj.output);
        if (j.count(tag_user_data_for_info))
        {
            obj.user_data_for_info = j.at(tag_user_data_for_info).get<content_t>();
        }
    }
}
void to_json(json& j, const transform_t& obj)
{
    using namespace transform_ns;
    using obj_t = transform_t;
    {
        j = json{
            {tag_enable, obj.enable},
            {tag_name, obj.name},
            {tag_input, obj.input},
            {tag_output, obj.output}
        };
        if (obj.user_data_for_info)
        {
            j[tag_user_data_for_info] = obj.user_data_for_info.value();
        }
    }
}

}// namespace transform

namespace transforms
{

void from_json(const json&, buildin_transforms_t&)
{
}
void to_json(json& j, const buildin_transforms_t&)
{
    j = json::object();
}

void from_json(const json& j, user_transforms_t& obj)
{
    using namespace user_transforms_ns;
    using obj_t = user_transforms_t;
    {
        j.at(tag_transforms).get_to(obj.transforms);
    }
}
void to_json(json& j, const user_transforms_t& obj)
{
    using namespace user_transforms_ns;
    using obj_t = user_transforms_t;
    {
        j = json{
            {tag_transforms, obj.transforms}
        };
    }
}

void from_json(const json& j, language_transforms_t& obj)
{
    using namespace language_transforms_ns;
    using obj_t = language_transforms_t;
    {
        j.at(tag_buildin_transforms).get_to(obj.buildin_transforms);
        j.at(tag_user_transforms).get_to(obj.user_transforms);
    }
}
void to_json(json& j, const language_transforms_t& obj)
{
    using namespace language_transforms_ns;
    using obj_t = language_transforms_t;
    {
        j = json{
            {tag_buildin_transforms, obj.buildin_transforms},
            {tag_user_transforms, obj.user_transforms}
        };
    }
}

void from_json(const json& j, post_language_transforms_t& obj)
{
    using namespace post_language_transforms_ns;
    using obj_t = post_language_transforms_t;
    {
        j.at(tag_user_transforms).get_to(obj.user_transforms);
    }
}
void to_json(json& j, const post_language_transforms_t& obj)
{
    using namespace post_language_transforms_ns;
    using obj_t = post_language_transforms_t;
    {
        j = json{
            {tag_user_transforms, obj.user_transforms}
        };
    }
}

void from_json(const json& j, transforms_t& obj)
{
    using namespace transforms_ns;
    using obj_t = transforms_t;
    {
        j.at(tag_language_transforms).get_to(obj.language_transforms);
        j.at(tag_post_language_transforms).get_to(obj.post_language_transforms);
    }
}
void to_json(json& j, const transforms_t& obj)
{
    using namespace transforms_ns;
    using obj_t = transforms_t;
    {
        j = json{
            {tag_language_transforms, obj.language_transforms},
            {tag_post_language_transforms, obj.post_language_transforms}
        };
    }
}

}// namespace transforms

}// namespace detail

void from_json(const json& j, config_t& obj)
{
    using namespace config_ns;
    using obj_t = config_t;
    {
        j.at(tag_galimatron).get_to(obj.galimatron);
        j.at(tag_saxon_processor).get_to(obj.saxon_processor);
        j.at(tag_transforms).get_to(obj.transforms);
    }
};
void to_json(json& j, const config_t& obj)
{
    using namespace config_ns;
    using obj_t = config_t;
    {
        j = json{
            {tag_galimatron, obj.galimatron},
            {tag_saxon_processor, obj.saxon_processor},
            {tag_transforms, obj.transforms}
        };
    }
}

}// namespace galimatron::translator::galimatron_config

namespace galimatron::runtime::galimatron_config
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::rt_cfg);
}


config_t read_from_json(
    const file_content_with_path_t& file_content_with_path,
    const gf::abstract::read_ft& read_f
)
{
    auto guard = scope_guard_t(
        [&]{
            replace_path_func = [](auto&){};
            replace_path_xsl_func = [](auto&){};
            replace_path_xml_func = [](auto&){};
            read_file_func = {};
        },
        []{
            replace_path_func = [](auto&){};
            replace_path_xsl_func = [](auto&){};
            replace_path_xml_func = [](auto&){};
            read_file_func = {};
        }
    );

    const auto& [file_content, to_config_path] = file_content_with_path;

    const auto relative_to_config = path_t{to_config_path}.remove_filename();

    const auto j = json::parse(
        *file_content,
        nullptr,// callback
        true,// allow exceptions
        true// ignore_comments
    );

    // read the first time - as is
    const auto config_as_is = [&]
    {
        return j.get<config_t>();
    }();
    fLogmeD("Galimatron config 'as-is':\n{}", write_to_json(config_as_is));

    // read the second time - correct paths relative to the config
    const auto config_relative_corrected = [&]
    {
        replace_path_func = [&](path_t& path)
        {
            if (!path.is_absolute())
            {
                path = relative_to_config / path;
            }
        };
        replace_path_xsl_func = [](auto&){};
        replace_path_xml_func = [](auto&){};

        return j.get<config_t>();
    }();
    fLogmeD("Galimatron config after relative to config path normalizing:\n{}", write_to_json(config_relative_corrected));

    // read the third time - perform the combined (relative and absolute) path correction
    const auto config_absolute_corrected = [&]
    {
        const auto relative_to_saxon_dir = config_relative_corrected.saxon_processor.working_dir;
        const auto relative_to_intermediate_xml_dir = config_relative_corrected.galimatron.intermediate_xml_dir;

        replace_path_func = [&](path_t& path)
        {
            if (!path.is_absolute())
            {
                path = make_runtime_filesystem().to_absolute(relative_to_config / path);
            }
        };
        replace_path_xsl_func = [&](path_t& path)
        {
            if (!path.is_absolute())
            {
                path = make_runtime_filesystem().to_absolute(relative_to_saxon_dir / path);
            }
        };
        replace_path_xml_func = [&](path_t& path)
        {
            if (!path.is_absolute())
            {
                path = make_runtime_filesystem().to_absolute(relative_to_intermediate_xml_dir / path);
            }
        };
        read_file_func = read_f;

        return j.get<config_t>();
    }();
    fLogmeD("Galimatron config after absolute path normalizing:\n{}", write_to_json(config_absolute_corrected));

    auto result = config_absolute_corrected;
    result.config_dir = make_runtime_filesystem().to_absolute(relative_to_config);
    return result;
}

content_t write_to_json(const config_t& config)
{
    return json(config).dump(4);
}

}// namespace galimatron::runtime::galimatron_config
