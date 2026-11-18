// galimatron
#include "galimatron/translator_library/state/statement_facts.hpp"


namespace galimatron::translator::state
{

namespace
{

template <typename Statement>
constexpr bool is_version_statement_v =
    std::same_as<Statement, tag_statement_t>
    || std::same_as<Statement, label_statement_t>
    || std::same_as<Statement, ordering_statement_t>
    || std::same_as<Statement, operation_statement_t>;

template <typename Statement>
bool names_a_tag(const Statement& statement)
{
    if constexpr (std::same_as<Statement, tag_statement_t>)
    {
        return statement.name.has_value();
    }
    else if constexpr (std::same_as<Statement, operation_statement_t>)
    {
        return !statement.is_version;
    }
    else
    {
        return false;
    }
}

}// namespace

statement_facts_ot statement_facts(const entity_t& entity)
{
    return std::visit(
        [](const auto& statement) -> statement_facts_ot
        {
            using statement_type = std::decay_t<decltype(statement)>;
            if constexpr (is_version_statement_v<statement_type>)
            {
                return statement_facts_t{
                    .name = statement.name,
                    .index_in_version = statement.index_in_version,
                    .names_tag = names_a_tag(statement)
                };
            }
            else
            {
                return std::nullopt;
            }
        },
        entity
    );
}

bool set_index_in_version(entity_t& entity, const index_t index)
{
    return std::visit(
        [&](auto& statement) -> bool
        {
            using statement_type = std::decay_t<decltype(statement)>;
            if constexpr (is_version_statement_v<statement_type>)
            {
                statement.index_in_version = index;
                return true;
            }
            else
            {
                return false;
            }
        },
        entity
    );
}

}// namespace galimatron::translator::state
