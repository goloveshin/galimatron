#pragma once

// galimatron
#include "galimatron/common_library/forward.hpp"


namespace galimatron::runtime::crash
{

// installs the crash-termination handler once
//
// testing: no
//
void install();

// opens the file next to FileBackend for Logme::CrashWrite
// FileBackend holds the main log with write denied to others
//
// testing: yes (unit)
//
path_ot arm_log();

// remembers the galimatron_summary.xml path for writing from the handler
//
// testing: no
//
void arm_summary(const path_t& summary_path);

}// namespace galimatron::runtime::crash
