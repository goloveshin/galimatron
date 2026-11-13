// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_wizard/detail/support.hpp"


namespace galimatron::translator::transforms::cpp2cpp_wizard
{

using namespace galimatron::translator::state;

wizard_statement_t& wizard_statement(holder::language_part_t& state)
{
    const auto statements = state.wizard_statements();
    return std::get<wizard_statement_t>(state.graph()[statements.front()].entity);
}

}// namespace galimatron::translator::transforms::cpp2cpp_wizard
