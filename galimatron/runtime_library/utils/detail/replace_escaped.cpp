// galimatron
#include "galimatron/runtime_library/utils/replace_escaped.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::runtime
{

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::rt);
}


content_t replace_escaped_form_to_original_symbols(const content_t& incoming)
{
    // using namespace boost::iostreams;
    // regex_filter filter1(boost::regex("~\\+"), "Bobby");
    // regex_filter filter2(boost::regex("\\^\\*"), "Danny");
    // filtering_ostream out;
    // out.push(filter1);
    // out.push(filter2);
    // out.push(std::stringstream);
    // out << "Hi ~+ and ^*. Is ^* still flying around ~+?" << std::endl;

    auto tmp = boost::replace_all_copy(incoming, "&quot;", R"(")");
    boost::replace_all(tmp, "&apos;", R"(')");
    boost::replace_all(tmp, "&lt;", R"(<)");
    boost::replace_all(tmp, "&gt;", R"(>)");
    boost::replace_all(tmp, "&amp;", R"(&)");
    return tmp;
}

file_content_t replace_escaped_form_to_original_symbols(const file_content_t& incoming)
{
    return make_file_content(replace_escaped_form_to_original_symbols(*incoming));
}

}// namespace galimatron::runtime
