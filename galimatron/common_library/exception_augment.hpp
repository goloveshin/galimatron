#pragma once

// galimatron
#include "galimatron/common_library/exception_declaration.hpp"

// Boost.LEAF
#include <boost/leaf/on_error.hpp>


namespace exception
{

namespace detail
{

// the path plus the copies the catch uses to spell it
//
struct path_to_load_t
{
    galimatron::path_t path;
    galimatron::bool_ot committed_report;
    galimatron::path_ot config_dir;
};

// on_error's result has no move constructor. augment_path builds it in place
//
[[nodiscard]] inline auto on_error_path(const path_to_load_t& arg)
{
    return boost::leaf::on_error(e_path_info{
        .path = arg.path,
        .committed_report = arg.committed_report,
        .config_dir = arg.config_dir
    });
}

}// namespace detail

// aggregate name for exceptions in this scope
//
[[nodiscard]] inline auto augment_aggregate(const galimatron::name_t& name)
{
    return boost::leaf::on_error(e_aggregate_info{ name });
}

// version number for exceptions in this scope
//
[[nodiscard]] inline auto augment_version(const galimatron::index_t version)
{
    return boost::leaf::on_error(e_version_info{ version });
}

// domain and subdomain of the call site: an exception from a callback is marked with them
//
template <typename Domain, typename Subdomain>
[[nodiscard]] inline auto augment_domain(
    const Domain& domain,
    const Subdomain& subdomain
)
{
    return boost::leaf::on_error(domain, subdomain);
}

// known path, aggregate, and version. an empty field does not go into the exception
//
struct error_where_t
{
    galimatron::path_ot path;
    galimatron::name_ot aggregate;
    galimatron::index_ot version;
};

// on_error's result has no move constructor. it is built into the member
//
template <typename Load, typename Arg, typename Make>
struct error_load_t
{
    Load load;

    explicit error_load_t(
        const Arg& arg,
        Make make
    )
        : load(make(arg))
    {
    }
};

namespace detail
{

using path_holder_t = error_load_t<
    decltype(on_error_path(path_to_load_t{})),
    path_to_load_t,
    decltype(&on_error_path)
>;

}// namespace detail

// path for exceptions in this scope. an empty path is not a payload.
// committed_report and config_dir are copied from the run slot onto e_path_info
//
[[nodiscard]] std::unique_ptr<detail::path_holder_t> augment_path(const galimatron::path_ot&);

// the same copies augment_path stores, for a throw that builds e_path_info itself
//
e_path_info e_path_info_for(const galimatron::path_t&);

// an empty path stays empty and loads no report copies
//
e_path_info e_path_info_for(const galimatron::path_ot&);

// holds the path, aggregate, and version until the end of the scope
//
struct error_where_load_t
{
    using path_load_t = decltype(augment_path(galimatron::path_ot{}));
    using aggregate_load_t = decltype(augment_aggregate(galimatron::name_t{}));
    using version_load_t = decltype(augment_version(galimatron::index_t{}));

    path_load_t path;
    std::unique_ptr<error_load_t<aggregate_load_t, galimatron::name_t, decltype(&augment_aggregate)>> aggregate;
    std::unique_ptr<error_load_t<version_load_t, galimatron::index_t, decltype(&augment_version)>> version;
};

[[nodiscard]] error_where_load_t augment_where(const error_where_t&);

}// namespace exception
