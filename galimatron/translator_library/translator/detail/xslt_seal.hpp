#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/holder.hpp"


namespace galimatron::translator::translator
{

// after the language xslt: a sealed version keeps the text from before the xslt.
// a tag whose effective value changed is written into the first version after the last hash.
// that version is added when the xslt did not add one.
// an xslt that left every sealed version unchanged leaves no new version.
// a wizard copy that uses a dynamic label stays in the sealed version, and that version's hash is refreshed.
//
// testing: no
//
state::holder::language_part_sptr_t reassign_sealed_edits(
    const state::holder::language_part_sptr_t& before,
    const state::holder::language_part_sptr_t& after
);

}// namespace galimatron::translator::translator
