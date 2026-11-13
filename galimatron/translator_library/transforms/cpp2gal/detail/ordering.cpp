// galimatron
#include "galimatron/translator_library/transforms/cpp2gal/detail/cpp2gal_impl.hpp"
#include "galimatron/translator_library/transforms/cpp2gal/detail/ordering_printer.hpp"

// global
#include "global/common_library/log_subsystems.hpp"

// Logme
#include <Logme/SID.h>


namespace galimatron::translator::transforms::cpp2gal
{

using namespace galimatron::translator::state;

namespace
{
Logme::SID SUBSID = Logme::SID::Build(global::log_subsystems::tr_s2g);
}

// stored source text is not kept. one line when the statement fits in 100 columns
//
void cpp2gal_t::emit_ordering_statement(const vertex_descriptor_t vd, const ordering_statement_t& args)
{
    append_user_comments(vd);

    const ordering_printer_t printer;
    const auto printed = printer.print(tab_count_.back(), args);
    for (const auto& line : printed)
    {
        tab_count_.push_back(line.tab);
        append_line(line.content);
        tab_count_.pop_back();
    }

    append_statement_comment(vd);
}

}// namespace galimatron::translator::transforms::cpp2gal
