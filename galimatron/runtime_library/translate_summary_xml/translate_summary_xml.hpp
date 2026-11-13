// galimatron
#include "galimatron/common_library/forward.hpp"
#include "galimatron/translator_library/translator/forward.hpp"


namespace galimatron::runtime::translate_summary_xml
{

// file name in the intermediate files directory
inline constexpr std::string_view file_name = "galimatron_summary.xml";

class builder_t
{
public:
    builder_t();

    struct append_transform_t
    {
        index_t sequential_number;
        std::string name;
        translator::translator::detail::origin_et origin;
        translator::translator::detail::status_et status;
        path_ot produced_xml_path;
        paths_ot produced_files;
        content_ot user_data;
    };
    void append_transform(const append_transform_t&);

    void set_result(const bool);
    void set_state_bytes(const std::size_t bytes);
    void set_error(const std::string&);

    file_content_t printout() const;

private:
    pugi::xml_document doc;
    pugi::xml_node root_node;
};

}// namespace galimatron::runtime::translate_summary_xml
