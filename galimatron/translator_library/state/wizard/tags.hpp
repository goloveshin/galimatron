#pragma once


// testing: yes (unit)
//
namespace galimatron::translator::state::wizard::tags
{

namespace command
{

static constexpr std::string_view must_be_one = "wizard command must be one";

}// namespace command

namespace create_root_aggregate
{

static constexpr std::string_view command = "wizard create root aggregate";
static constexpr std::string_view context_must_be_chunk = "may be executed in chunk context only";
static constexpr std::string_view command_file_is_the_aggregate = "cannot create the aggregate in the command file";
static constexpr std::string_view chunk_already_exists = "chunk already exists";
static constexpr std::string_view root_aggregate_already_exists = "root aggregate already exists";

}// namespace create_root_aggregate

namespace create_version
{

static constexpr std::string_view command = "wizard create version";
static constexpr std::string_view context_must_be_chunk = "may be executed in chunk context only";
static constexpr std::string_view root_aggregate_must_exist = "root aggregate must exist";

}// namespace create_version

namespace delete_version
{

static constexpr std::string_view command = "wizard delete version";
static constexpr std::string_view context_must_be_chunk = "may be executed in chunk context only";
static constexpr std::string_view root_aggregate_must_exist = "root aggregate must exist";
static constexpr std::string_view last_version_must_exist = "last version must exist";
static constexpr std::string_view version_must_exist = "version must exist";
static constexpr std::string_view cannot_delete_closed_version = "cannot delete a closed version";
static constexpr std::string_view versions_must_stay_continuous = "versions must stay continuous";

}// namespace delete_version

namespace collapse_version
{

static constexpr std::string_view command = "wizard collapse version";
static constexpr std::string_view context_must_be_chunk = "may be executed in chunk context only";
static constexpr std::string_view root_aggregate_must_exist = "root aggregate must exist";
static constexpr std::string_view version_must_exist = "version must exist";
static constexpr std::string_view owner_must_be_present = "owner version must be present";
static constexpr std::string_view range_must_ascend = "version range must ascend";

}// namespace collapse_version

namespace create_hash
{

static constexpr std::string_view command = "wizard create hash";
static constexpr std::string_view context_must_be_chunk = "may be executed in chunk context only";
static constexpr std::string_view version_must_exist = "version must exist";
static constexpr std::string_view last_version_must_exist = "last version must exist";

}// namespace create_hash

namespace delete_hash
{

static constexpr std::string_view command = "wizard delete hash";
static constexpr std::string_view context_must_be_chunk = "may be executed in chunk context only";
static constexpr std::string_view version_must_exist = "version must exist";
static constexpr std::string_view last_version_must_exist = "last version must exist";

}// namespace delete_hash

namespace create_aggregate
{

static constexpr std::string_view command = "wizard create aggregate";
static constexpr std::string_view context_must_be_chunk = "may be executed in chunk context only";
static constexpr std::string_view root_aggregate_must_exist = "root aggregate must exist";
static constexpr std::string_view names_must_be_unique = "names must be unique";
static constexpr std::string_view command_file_is_the_aggregate = "cannot create the aggregate in the command file";
static constexpr std::string_view chunk_already_exists = "chunk already exists";
static constexpr std::string_view last_version_must_be_open = "cannot create an aggregate when the last version is closed";

}// namespace create_aggregate

namespace copy_aggregate
{

static constexpr std::string_view command = "wizard copy aggregate";
static constexpr std::string_view context_must_be_chunk = "may be executed in chunk context only";
static constexpr std::string_view root_aggregate_must_exist = "root aggregate must exist";
static constexpr std::string_view source_must_exist = "source must exist";
static constexpr std::string_view cannot_copy_root_aggregate = "cannot copy the root aggregate";
static constexpr std::string_view names_must_be_unique = "names must be unique";
static constexpr std::string_view aggregate_must_be_absent = "aggregate must be absent";
static constexpr std::string_view command_file_is_the_aggregate = "cannot copy the aggregate onto the command file";
static constexpr std::string_view chunk_already_exists = "chunk already exists";
static constexpr std::string_view source_in_closed_version = "source is present in a closed version";

}// namespace copy_aggregate

namespace continue_aggregate
{

static constexpr std::string_view command = "wizard continue aggregate";
static constexpr std::string_view context_must_be_chunk = "may be executed in chunk context only";
static constexpr std::string_view root_aggregate_must_exist = "root aggregate must exist";
static constexpr std::string_view source_must_exist = "source must exist";
static constexpr std::string_view cannot_continue_root_aggregate = "cannot continue the root aggregate";
static constexpr std::string_view names_must_be_unique = "names must be unique";
static constexpr std::string_view aggregate_must_be_absent = "aggregate must be absent";
static constexpr std::string_view command_file_is_the_aggregate = "cannot continue the aggregate onto the command file";
static constexpr std::string_view chunk_already_exists = "chunk already exists";
static constexpr std::string_view last_version_must_exist = "last version must exist";
static constexpr std::string_view last_version_is_already_deleted = "last version is already deleted";

}// namespace continue_aggregate

namespace version_statement
{

static constexpr std::string_view context_must_be_chunk = "may be executed in chunk context only";
static constexpr std::string_view aggregate_must_be_named = "aggregate must be named";
static constexpr std::string_view aggregate_must_be_root = "aggregate must be the root aggregate";
static constexpr std::string_view version_must_be_open = "version must be the open version";
static constexpr std::string_view version_is_deleted = "version is deleted";

}// namespace version_statement

namespace create_label
{

static constexpr std::string_view command = "wizard create label";

}// namespace create_label

namespace create_tag
{

static constexpr std::string_view command = "wizard create tag";

}// namespace create_tag

namespace delete_tag
{

static constexpr std::string_view command = "wizard delete tag";

}// namespace delete_tag

namespace create_ordering
{

static constexpr std::string_view command = "wizard create ordering";

}// namespace create_ordering

namespace delete_aggregate
{

static constexpr std::string_view command = "wizard delete aggregate";
static constexpr std::string_view context_must_be_chunk = "may be executed in chunk context only";
static constexpr std::string_view root_aggregate_must_exist = "root aggregate must exist";
static constexpr std::string_view names_must_be_unique = "names must be unique";
static constexpr std::string_view aggregate_must_exist = "aggregate must exist";
static constexpr std::string_view cannot_delete_root_aggregate = "cannot delete the root aggregate";
static constexpr std::string_view last_version_must_exist = "last version must exist";
static constexpr std::string_view last_version_is_already_deleted = "last version is already deleted";
static constexpr std::string_view cannot_delete_closed_version = "cannot delete a closed version";
static constexpr std::string_view version_definition_must_exist = "version definition must exist";
static constexpr std::string_view ordering_names_the_aggregate = "ordering names the aggregate";

}// namespace delete_aggregate

namespace batch
{

static constexpr std::string_view command = "wizard batch";
static constexpr std::string_view context_must_be_chunk = "may be executed in chunk context only";
static constexpr std::string_view command_must_be_allowed = "executes create commands, delete tag, delete version, and collapse version only";

}// namespace batch

}// namespace galimatron::translator::state::wizard::tags
