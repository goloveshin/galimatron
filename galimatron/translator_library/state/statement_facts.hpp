#pragma once

// galimatron
#include "galimatron/translator_library/state/entity/entity.hpp"


namespace galimatron::translator::state
{

// what the four statements of a version body have in common:
// tag, label, ordering, and operation (delete)
//
struct statement_facts_t
{
    name_ot name;
    index_t index_in_version = 0;
    // a tag with a name, or a delete of a tag. a label, an ordering, and delete; do not name a tag
    bool names_tag = false;
};
using statement_facts_ot = std::optional<statement_facts_t>;

// absent when the entity is not a tag, label, ordering, or operation statement
//
// testing: yes (unit)
//
statement_facts_ot statement_facts(const entity_t&);

// moves the statement inside its version body.
// false when the entity is not a tag, label, ordering, or operation statement
//
// testing: yes (unit)
//
bool set_index_in_version(entity_t&, index_t);

}// namespace galimatron::translator::state
