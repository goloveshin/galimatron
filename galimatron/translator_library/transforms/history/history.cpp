// galimatron
#include "galimatron/translator_library/transforms/history/history.hpp"
#include "galimatron/translator_library/transforms/history/detail/history_impl.hpp"


namespace galimatron::translator::transforms::history
{

using namespace galimatron::translator::state;

void document(
    pugi::xml_node parent,
    const holder::language_part_t& language,
    holder::compiled_part_t& compiled
)
{
    detail::build(parent, language, compiled);
}

}// namespace galimatron::translator::transforms::history
