// galimatron
#include "galimatron/runtime_library/utils/omit_filesystem.hpp"
#include "galimatron/translator_library/xml_tags.hpp"


namespace galimatron::runtime
{

namespace tag_f = galimatron::translator::xml_tags::file;

file_content_t omit_filesystem_from_printout(const file_content_t& content)
{
    // [\s\S] is the body: std::regex has no flag that lets '.' match a newline
    static const std::regex element(
        std::string(R"([ \t]*<)")
        + std::string(tag_f::filesystem)
        + R"((?=[\s/>])(?:[^>]*/>|[^>]*>[\s\S]*?</)"
        + std::string(tag_f::filesystem)
        + R"(>)\r?\n?)"
    );

    return make_file_content(std::regex_replace(*content, element, ""));
}

}// namespace galimatron::runtime
