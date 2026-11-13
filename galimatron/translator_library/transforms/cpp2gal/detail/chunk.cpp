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


void cpp2gal_t::emit_chunk(const vertex_descriptor_t vd, const chunk_t&)
{
    append_user_comments(vd);
    walk_children({ .vd = vd });

    auto content = make_file_content();
    {
        content->reserve(size_of_lines(lines_) + 256);
        join_lines(lines_, *content);
        *content += GaL::NEWLINE;
    }

    const auto& vertex = graph_[vd];
    if (!vertex.location || !vertex.location->path)
    {
        leaf::throw_exception(
            exception::domain::translator_library{},
            exception::subdomain::translator_library::cpp2gal{},
            exception::e_what_info{ "chunk has no path" }
        );
    }

    contents_.push_back({
        .content = content,
        .path = *vertex.location->path
    });

    lines_.clear();
}

}// namespace galimatron::translator::transforms::cpp2gal
