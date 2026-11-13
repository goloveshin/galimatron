
#include "v6.hpp"
#include "uuid_protocol/common/protocol.hpp"
#include "uuid_protocol/common/protocol_forward.hpp"
#include "uuid_protocol/common/registrate.hpp"

namespace uuid_protocol::v6
{


namespace system
{

uuid_t get_uuid(std::type_identity <bye_t>)
{
    return uuid_gen("6f1e0002-0000-4000-8000-000000000006");
}


uuid_t get_uuid(std::type_identity <error_t>)
{
    return uuid_gen("6f1e0004-0000-4000-8000-000000000006");
}


uuid_t get_uuid(std::type_identity <hello_t>)
{
    return uuid_gen("6f1e0001-0000-4000-8000-000000000006");
}


uuid_t get_uuid(std::type_identity <ok_t>)
{
    return uuid_gen("6f1e0003-0000-4000-8000-000000000006");
}


}// namespace system



namespace payload
{

std::string print(const sea_state_e& value)
{
    using enum sea_state_e;
    switch (value)
    {

    case calm: return "calm";

    case swell: return "swell";

    case gale: return "gale";

    }
}




std::string print(const vessel_kind_e& value)
{
    using enum vessel_kind_e;
    switch (value)
    {

    case sail: return "sail";

    case steamer: return "steamer";

    case launch: return "launch";

    }
}


uuid_t get_uuid(std::type_identity <vessel_t>)
{
    return uuid_gen("6f1e0007-0000-4000-8000-000000000002");
}




uuid_t get_uuid(std::type_identity <watch_t>)
{
    return uuid_gen("6f1e0008-0000-4000-8000-000000000001");
}


}// namespace payload


void registrate()
{


    {
        using namespace system;

        registrate_t<bye_t>{version, bye_group};


        registrate_t<error_t>{version, error_group};


        registrate_t<hello_t>{version, hello_group};


        registrate_t<ok_t>{version, ok_group};


    }



    {
        using namespace payload;





        registrate_t<vessel_t>{version, no_group};


        registrate_t<watch_t>{version, no_group};


    }


}

}// namespace uuid_protocol::v6