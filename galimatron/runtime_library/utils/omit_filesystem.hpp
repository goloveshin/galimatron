#pragma once

// galimatron
#include "galimatron/common_library/forward.hpp"


namespace galimatron::runtime
{

// drops every <filesystem> node from xml written to the folder.
// the in-memory document still has the node, so a later xml2cpp keeps the path.
//
// testing: no
//
file_content_t omit_filesystem_from_printout(const file_content_t&);

}// namespace galimatron::runtime
