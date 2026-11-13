#pragma once

#if WIN32
#define _WIN32_WINNT 0x0602
// wingdi.h defines ERROR as 0, which breaks antlr4::tree::ParseTreeType
#define NOGDI
#endif

// std
#include <any>
#include <array>
#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <deque>
#include <format>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <regex>
#include <set>
#include <shared_mutex>
#include <stacktrace>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

// Boost
#include <boost/algorithm/string.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/icl/interval_set.hpp>
#include <boost/leaf.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

// range-v3
#include <range/v3/all.hpp>

// PugiXML
#include <pugixml.hpp>

// nlohmann_json
#include <nlohmann/json.hpp>

// magic_enum
#include <magic_enum/magic_enum.hpp>

// base64
#include "base64/base64.hpp"

// Windows and CRT macros that clash with ANTLR identifiers
#ifdef ERROR
#undef ERROR
#endif
#ifdef DELETE
#undef DELETE
#endif
#ifdef EOF
#undef EOF
#endif
