
#pragma once

#include "uuid_protocol/common/forward.hpp"

namespace uuid_protocol::v3
{

constexpr version_t version = 3;



namespace system
{

struct bye_t
{
};
uuid_t get_uuid(std::type_identity <bye_t>);
any_state_t create(std::type_identity <bye_t>, const any_state_t &);
void handle_client(const bye_t&, const any_state_t &);
void handle_server(const bye_t&, const any_state_t &);
void log(const bye_t&);

struct error_t
{
};
uuid_t get_uuid(std::type_identity <error_t>);
any_state_t create(std::type_identity <error_t>, const any_state_t &);
void handle_client(const error_t&, const any_state_t &);
void handle_server(const error_t&, const any_state_t &);
void log(const error_t&);

struct hello_t
{
    std::string message;
};
uuid_t get_uuid(std::type_identity <hello_t>);
any_state_t create(std::type_identity <hello_t>, const any_state_t &);
void handle_client(const hello_t&, const any_state_t &);
void handle_server(const hello_t&, const any_state_t &);
void log(const hello_t&);

struct ok_t
{
};
uuid_t get_uuid(std::type_identity <ok_t>);
any_state_t create(std::type_identity <ok_t>, const any_state_t &);
void handle_client(const ok_t&, const any_state_t &);
void handle_server(const ok_t&, const any_state_t &);
void log(const ok_t&);

}// namespace system



namespace payload
{

struct lamp_t
{
    bool lit = false;
};
uuid_t get_uuid(std::type_identity <lamp_t>);
any_state_t create(std::type_identity <lamp_t>, const any_state_t &);
void handle_client(const lamp_t&, const any_state_t &);
void handle_server(const lamp_t&, const any_state_t &);
void log(const lamp_t&);

}// namespace payload


void registrate();

}// namespace uuid_protocol::v3