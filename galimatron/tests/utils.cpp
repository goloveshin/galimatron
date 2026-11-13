// galimatron
#include "galimatron/tests/utils.hpp"
#include "galimatron/runtime_library/translate/translate.hpp"
#include "galimatron/translator_library/pugi_forward.hpp"


namespace galimatron
{

namespace
{

// the slice flags, in the order of testing_t
//
translator::translator::detail::testing_t testing_flags(const translate_slice_t& flags)
{
    return {
        .gal2cpp = flags.gal2cpp,
        .cpp2cpp_validate = flags.cpp2cpp_validate,
        .cpp2cpp_wizard = flags.cpp2cpp_wizard,
        .cpp2xml = flags.cpp2xml,
        .xml2cpp = flags.xml2cpp,
        .cpp2gal = flags.cpp2gal,
        .cpp2post_cpp = flags.cpp2post_cpp,
        .post_cpp2post_xml = flags.post_cpp2post_xml,
        .drop_tag_user_formatting = flags.drop_tag_user_formatting
    };
}

// .xml goes through pugixml and back on read and on write, so the bytes match printout.
// other files stay as they are
//
file_content_t xml_through_pugixml(
    const path_t& path,
    const file_content_t& content,
    const gf::abstract::content_filter_direction_et direction
)
{
    switch (direction)
    {
        case gf::abstract::content_filter_direction_et::READ:
        case gf::abstract::content_filter_direction_et::WRITE:
            break;
    }

    if (!content || path.extension() != ".xml")
    {
        return content;
    }

    pugi::xml_document document;
    const auto loaded = document.load_buffer(
        content->data(),
        content->size(),
        pugi::parse_default | pugi::parse_declaration | pugi::parse_comments | pugi::parse_pi
    );
    if (!loaded)
    {
        return content;
    }
    return translator::printout(document);
}

// log.cfg and gal.cfg from the case directory
// no slice: the production pipeline
//
void translate_case(
    const path_t& case_root,
    const std::optional<translate_slice_t>& slice
)
{
    runtime::translate(
    {
        .get_log_config = [&]
        {
            return runtime::translate_t::get_log_config_from_file(case_root / "log.cfg");
        },
        .get_gal_config = [&]
        {
            return runtime::translate_t::get_gal_config_from_file(case_root / "gal.cfg");
        },
        .is_testing_mode = true,
        .testing = slice.transform(testing_flags),
        .content_filter = xml_through_pugixml
    });
}

}// namespace

void translate_production(const path_t& case_root)
{
    translate_case(case_root, {});
}

void translate_slice(const path_t& case_root, const translate_slice_t& args)
{
    translate_case(case_root, args);
}

void stage_file(const path_t& source, const path_t& destination)
{
    fs::create_directories(destination.parent_path());
    fs::copy_file(
        source,
        destination,
        fs::copy_options::overwrite_existing
    );
}

void stage_directory(const path_t& source, const path_t& destination)
{
    std::error_code error;
    fs::remove_all(destination, error);
    CHECK_FALSE(error);
    fs::copy(
        source,
        destination,
        fs::copy_options::recursive
    );
}

}// namespace galimatron
