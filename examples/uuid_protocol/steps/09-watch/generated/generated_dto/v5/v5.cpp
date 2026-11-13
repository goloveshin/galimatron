
#include "v5.hpp"
#include "uuid_protocol/common/protocol.hpp"
#include "uuid_protocol/common/protocol_forward.hpp"
#include "uuid_protocol/common/registrate.hpp"

namespace uuid_protocol::v5
{


namespace system
{

uuid_t get_uuid(std::type_identity <bye_t>)
{
    return uuid_gen("6f1e0002-0000-4000-8000-000000000005");
}


uuid_t get_uuid(std::type_identity <error_t>)
{
    return uuid_gen("6f1e0004-0000-4000-8000-000000000005");
}


uuid_t get_uuid(std::type_identity <hello_t>)
{
    return uuid_gen("6f1e0001-0000-4000-8000-000000000005");
}


uuid_t get_uuid(std::type_identity <ok_t>)
{
    return uuid_gen("6f1e0003-0000-4000-8000-000000000005");
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

    }
}


uuid_t get_uuid(std::type_identity <fog_t>)
{
    return uuid_gen("6f1e0006-0000-4000-8000-000000000002");
}


std::string print(const vessel_kind_e& value)
{
    using enum vessel_kind_e;
    switch (value)
    {

    case sail: return "sail";

    case steamer: return "steamer";

    }
}


uuid_t get_uuid(std::type_identity <vessel_t>)
{
    return uuid_gen("6f1e0007-0000-4000-8000-000000000001");
}


uuid_t get_uuid(std::type_identity <lamp_t>)
{
    return uuid_gen("6f1e0005-0000-4000-8000-000000000003");
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

        registrate_t<fog_t>{version, no_group};


        registrate_t<lamp_t>{version, no_group};


        registrate_t<vessel_t>{version, no_group};


    }


}

}// namespace uuid_protocol::v5