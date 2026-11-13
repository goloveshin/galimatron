#pragma once

// doctest
#include <doctest/doctest.h>


// the boundary a suite calls
// doctest::description stores the pointer, so each view is a string literal
//
namespace galimatron::test_axes
{

// Product: full integration test
// gal.cfg, log.cfg, and translate_production, on real files
//
inline constexpr std::string_view product = "Product";

// Translator slice: partial integration test
// a chosen run of translator steps, without gal.cfg, on real files
//
inline constexpr std::string_view translator_slice = "Translator slice";

// the .gal grammar and the version rule
// parses text or reads a version, without the production translate
//
inline constexpr std::string_view parser = "Parser";

// one helper
// values, tags, digests, or argv, with no translate and no grammar
//
inline constexpr std::string_view component = "Component";

// the crash handler in a child process
// the parent checks the log and the summary
//
inline constexpr std::string_view process = "Process";

// attaches the entry to the suite; the suite path stays the feature path
//
inline doctest::description mark(const std::string_view entry)
{
    return doctest::description(entry.data());
}

}// namespace galimatron::test_axes
