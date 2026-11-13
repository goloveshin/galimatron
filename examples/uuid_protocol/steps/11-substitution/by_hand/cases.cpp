// uuid_protocol
#include "v2/v2.hpp"
#include "v3/v3.hpp"
#include "v4/v4.hpp"
#include "v5/v5.hpp"
#include "v6/v6.hpp"
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


namespace uuid_protocol::v5
{

// fog gains sea. vessel appears. lamp stays lit with candela
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
            []{ return prepare_case(fog_t{ .metres = 250, .sea = sea_state_e::swell }); },
            []{ return prepare_case(vessel_t{ .kind = vessel_kind_e::steamer, .bearing = 40 }); }
        }
    });
}

}// namespace uuid_protocol::v5


namespace uuid_protocol::v6
{

// lamp and fog travel inside watch. vessel gains a name
//
// testing: no
//
void registrate_test_cases()
{
    using namespace payload;

    test_cases().push_back({
        .version = version,
        .test_cases = {
            []
            {
                return prepare_case(
                    watch_t{
                        .day = "1912-03-14",
                        .lamp = { .lit = true, .candela = 1200 },
                        .fog = { .metres = 200, .sea = sea_state_e::gale }
                    }
                );
            },
            []
            {
                return prepare_case(
                    vessel_t{
                        .kind = vessel_kind_e::launch,
                        .bearing = 15,
                        .name = "North Light"
                    }
                );
            }
        }
    });
}

}// namespace uuid_protocol::v6


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
    v5::registrate_test_cases();
    v6::registrate_test_cases();
}

}// namespace uuid_protocol
