#pragma once

// galimatron
#include "galimatron/translator_library/forward.hpp"

// Antlr
#include <tree/ParseTreeListener.h>
#include <tree/ParseTreeWalker.h>
#include <DefaultErrorStrategy.h>


namespace galimatron::translator
{

using antlr_listener_t = antlr4::tree::ParseTreeListener;
using antlr_listener_sptr_t = std::shared_ptr<antlr_listener_t>;

using antlr_parse_tree_t = antlr4::tree::ParseTree;

inline void antlr_walk(antlr_listener_t* listener, antlr_parse_tree_t* tree)
{
    antlr4::tree::ParseTreeWalker::DEFAULT.walk(listener, tree);
}

}//namespace galimatron::translator
