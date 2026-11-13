// galimatron
#include "galimatron/common_library/exception_augment.hpp"
#include "galimatron/common_library/path_report.hpp"


namespace exception
{

namespace
{

// copies the run's path report. empty when this run spells a path as stored
//
void copy_current_path_report(
    galimatron::bool_ot& committed_report,
    galimatron::path_ot& config_dir
)
{
    const auto& slot = galimatron::current_path_report();
    if (!slot.committed_report || slot.config_dir.empty())
    {
        return;
    }
    committed_report = true;
    config_dir = slot.config_dir;
}

}


std::unique_ptr<detail::path_holder_t> augment_path(const galimatron::path_ot& path)
{
    std::unique_ptr<detail::path_holder_t> holder;
    if (path)
    {
        detail::path_to_load_t loaded{ .path = *path };
        copy_current_path_report(loaded.committed_report, loaded.config_dir);
        holder = std::make_unique<detail::path_holder_t>(loaded, detail::on_error_path);
    }
    return holder;
}

e_path_info e_path_info_for(const galimatron::path_t& path)
{
    e_path_info info{ .path = path };
    copy_current_path_report(info.committed_report, info.config_dir);
    return info;
}

e_path_info e_path_info_for(const galimatron::path_ot& path)
{
    return path.transform([](const galimatron::path_t& value)
    {
        return e_path_info_for(value);
    }).value_or(e_path_info{});
}

error_where_load_t augment_where(const error_where_t& where)
{
    error_where_load_t load;
    load.path = augment_path(where.path);
    if (where.aggregate)
    {
        load.aggregate = std::make_unique<error_load_t<
            error_where_load_t::aggregate_load_t,
            galimatron::name_t,
            decltype(&augment_aggregate)
        >>(
            *where.aggregate,
            augment_aggregate
        );
    }
    if (where.version)
    {
        load.version = std::make_unique<error_load_t<
            error_where_load_t::version_load_t,
            galimatron::index_t,
            decltype(&augment_version)
        >>(
            *where.version,
            augment_version
        );
    }
    return load;
}

}// namespace exception
