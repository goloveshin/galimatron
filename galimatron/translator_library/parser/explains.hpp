#pragma once

// galimatron
#include "galimatron/translator_library/forward.hpp"


namespace galimatron::translator::parser::explains
{

// the diagnosis for a known bad parse, from the rule that failed and the antlr sentence.
// empty when this failure has no diagnosis.
//
string_ot explain(const strings_t& stack, const std::string& message);

}// namespace galimatron::translator::parser::explains
