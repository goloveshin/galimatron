// galimatron
#include "galimatron/translator_library/state/version_status.hpp"


namespace galimatron::translator::state
{

std::string to_string(const version_status_et& status)
{
    using enum version_status_et;

    switch (status)
    {
        case DELETED:
            return "deleted";
    }
}

version_status_et to_enum(g_parser::AggregateVersionStatusContext& ctx)
{
    using enum version_status_et;

    if (ctx.DELETED())
    {
        return DELETED;
    }

    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::wizard{},
        exception::e_what_info{ "no such version status" }
    );
}

version_status_oet to_enum(g_parser::AggregateVersionStatusContext* ctx)
{
    if (ctx)
    {
        return to_enum(*ctx);
    }
    return {};
}

}// namespace galimatron::translator::state
