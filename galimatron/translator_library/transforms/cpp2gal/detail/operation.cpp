// galimatron
#include "galimatron/translator_library/transforms/cpp2gal/detail/cpp2gal_impl.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::cpp2gal
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_s2g);
}


namespace tag_gal = tags::operation_gal;

void cpp2gal_t::emit_operation_statement(const vertex_descriptor_t vd, const operation_statement_t& args)
{
    append_user_comments(vd);
    if (append_user_formatting(vd))
    {
        return;
    }

    if (args.is_version)
    {
        append_line(tag_gal::delete_tag);
    }
    else
    {
        append_line(std::format("{} {}",
            tag_gal::delete_tag,
            args.name
        ));
    }
    append_back(GaL::SEMI);
}

}// namespace galimatron::translator::transforms::cpp2gal
