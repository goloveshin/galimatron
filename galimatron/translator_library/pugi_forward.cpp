// galimatron
#include "galimatron/translator_library/pugi_forward.hpp"
#include "galimatron/translator_library/xml_tags.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr);
}


namespace tag = galimatron::translator::xml_tags::file;

void append_location(const append_location_t& args)
{
    if (args.location)
    {
        const auto& location = *args.location;

        auto node = args.parent_node.append_child(tag::location);

        if (location.path)
        {
            auto filesystem_node = node.append_child(tag::filesystem);
            filesystem_node.append_attribute(tag::path) = location.path->string();

            append_path_by_parts({
                .parent_node = filesystem_node,
                .parts = args.split_path_by_parts(*location.path)
            });
        }
    }
}

void append_path_by_parts(const append_path_by_parts_t& args)
{
    auto parent_node = args.parent_node;

    if (args.parts)
    {
        const auto& parts = *args.parts;

        if (parts.root)
        {
            parent_node.append_attribute(tag::root) = parts.root->string();
        }

        if (parts.relative)
        {
            parent_node.append_attribute(tag::relative) = parts.relative->string();
        }

        if (parts.relative_and_filename)
        {
            parent_node.append_attribute(tag::relative_and_filename) = parts.relative_and_filename->string();
        }

        if (parts.filename.name)
        {
            parent_node.append_attribute(tag::stem) = parts.filename.name->string();
        }

        if (parts.filename.extension)
        {
            parent_node.append_attribute(tag::extension) = parts.filename.extension->string();
        }
    }
}

file_content_t printout(const pugi::xml_document& document)
{
    std::ostringstream stream;
    document.save(stream, std::string(4, ' ').c_str());
    return make_file_content(stream.str());
}

}//namespace galimatron::translator
