// uuid_protocol
#include "v2/v2.hpp"
#include "v3/v3.hpp"
#include "v4/v4.hpp"
#include "uuid_protocol/common/test_case_container.hpp"


namespace uuid_protocol::v2
{

// v2 has no payload yet. the session still sends the hello that names this version
//
// testing: no
//
void registrate_test_cases()
{
    test_cases().push_back({
        .version = version,
        .test_cases = {}
    });
}

}// namespace uuid_protocol::v2


namespace uuid_protocol::v3
{

// the first payload case. lamp has lit only. fog and candela are later versions
//
// testing: no
//
void registrate_test_cases()
{
    using namespace payload;

    test_cases().push_back({
        .version = version,
        .test_cases = {
            []{ return prepare_case(lamp_t{ .lit = true }); }
        }
    });
}

}// namespace uuid_protocol::v3


namespace uuid_protocol::v4
{

// lamp gains candela. fog appears, in metres
//
// testing: no
//
void registrate_test_cases()
{
    using namespace payload;

    test_cases().push_back({
        .version = version,
        .test_cases = {
            []{ return prepare_case(lamp_t{ .lit = true, .candela = 1500 }); },
            []{ return prepare_case(fog_t{ .metres = 250 }); }
        }
    });
}

}// namespace uuid_protocol::v4


namespace uuid_protocol
{

// the shared endpoint calls this
//
// testing: no
//
void registrate_test_cases()
{
    v2::registrate_test_cases();
    v3::registrate_test_cases();
    v4::registrate_test_cases();
}

}// namespace uuid_protocol
