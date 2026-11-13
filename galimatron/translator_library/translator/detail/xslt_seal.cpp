// galimatron
#include "galimatron/translator_library/translator/detail/xslt_seal.hpp"
#include "galimatron/translator_library/translator/detail/xslt_seal_detail.hpp"
#include "galimatron/translator_library/transforms/cpp2cpp_wizard/detail/support.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>

namespace galimatron::translator::translator
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_tsp);
}

holder::language_part_sptr_t reassign_sealed_edits(
    const holder::language_part_sptr_t& before,
    const holder::language_part_sptr_t& after
)
{
    if (!before || !after || !before->aggregates().has_root() || !after->aggregates().has_root())
    {
        return after;
    }

    before->refresh_version_records();
    after->refresh_version_records();

    const auto last_sealed = last_sealed_index(*before);
    if (!last_sealed)
    {
        return after;
    }

    reject_sealed_shape(*before, *after, *last_sealed);
    if (!sealed_history_differs(*before, *after, *last_sealed))
    {
        return after;
    }

    const auto edits = effective_edits(before, after, *last_sealed);
    if (only_wizard_dynamic_copies(*before, edits, *last_sealed))
    {
        transforms::cpp2cpp_wizard::write_version_hash(*after, *last_sealed);
        fLogmeI("xslt finished wizard copies; refreshed the hash of version {}", *last_sealed);
        return after;
    }

    restore_sealed_history(*before, *after, *last_sealed);
    if (edits.empty())
    {
        fLogmeI("xslt changed a sealed version; restored the sealed text");
        after->drop_detached_vertices();
        return after;
    }

    const auto version_index = destination_version(*after, *last_sealed);
    write_edits(*after, version_index, edits);
    after->drop_detached_vertices();
    return after;
}

}// namespace galimatron::translator::translator
