#pragma once

// uuid_protocol
#include "uuid_protocol/common/forward.hpp"


namespace uuid_protocol
{

// testing: no
//
inline version_test_cases_t& test_cases()
{
    static version_test_cases_t test_cases;
    return test_cases;
}

// the uuid comes from the generated get_uuid of Obj
//
// testing: no
//
template <typename Obj>
version_test_case_t::test_case_t prepare_case(const Obj& obj)
{
    return {
        .uuid = get_uuid(std::type_identity<Obj>{}),
        .any_message = obj
    };
}

}// namespace uuid_protocol
