#pragma once

// galimatron
#include "galimatron/translator_library/transforms/cpp2cpp_validate/cpp2cpp_validate.hpp"
#include "galimatron/translator_library/state/holder/holder.hpp"
#include "galimatron/common_library/exception_declaration.hpp"
#include "galimatron/common_library/exception_augment.hpp"


namespace galimatron::translator::transforms::cpp2cpp_validate
{

using namespace galimatron::translator::state;

// a name already chosen in this list, compared without case
//
bool name_already_listed(const strings_t& names, const name_t& name);

std::string join_indices(const indices_t& indices);

bool index_is_listed(const indices_t& indices, index_t index);

// empty, or 0, 1, 2, ... with no gaps
//
bool indexes_stay_continuous(const indices_t& indices);

bool is_deleted(const version_status_oet& status);

// the definition consists only of delete; — a historical version deletion
//
bool definition_is_only_version_delete(
    const holder::language_part_t& language,
    vertex_descriptor_t vd_aggregate,
    index_t index
);

const wizard_statement_t& wizard_statement(const holder::language_part_t& language_part);

// stops the check. path, aggregate, and version ride on where
//
[[noreturn]] void fail(
    exception::details::validation_error_code_et code,
    std::string what,
    const exception::error_where_t& where = {}
);

}// namespace galimatron::translator::transforms::cpp2cpp_validate
