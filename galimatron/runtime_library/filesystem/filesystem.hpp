#pragma once

// galimatron
#include "galimatron/common_library/forward.hpp"

// global
#include "global/filesystem_library/filesystem.hpp"


namespace galimatron::executable::filesystem::physically
{

// here we combine the functions from global::filesystem::physically with our logger and exceptions.
// an empty content filter leaves read and write bytes as they are
//
gf::abstract::filesystem_t make_filesystem(const gf::abstract::content_filter_ft& content_filter = {});

}// namespace galimatron::executable::filesystem::physically

inline auto make_runtime_filesystem()
{
    return galimatron::executable::filesystem::physically::make_filesystem();
}
