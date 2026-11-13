// uuid_protocol
#include "v6/v6.hpp"
#include "uuid_protocol/common/test_case_container.hpp"


namespace uuid_protocol::v6
{

// the only published version. lamp and fog travel inside watch
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
    v6::registrate_test_cases();
}

}// namespace uuid_protocol
