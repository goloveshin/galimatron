#pragma once

// galimatron
#include "galimatron/translator_library/state/entity/entity.hpp"


namespace galimatron::translator::state
{

struct ordering_rank_problem_t
{
    bool cycle = false;
    name_t first;
    name_t second;
};

struct ordering_ranks_t
{
    std::map<name_t, index_t> rank;
    std::optional<ordering_rank_problem_t> problem;
};

// the rank of each aggregate the parts name. problem is set when the parts are not a strict weak order
//
ordering_ranks_t ranks_of_ordering(const ordering_statement_t::ordering_parts_t& parts);

}// namespace galimatron::translator::state
