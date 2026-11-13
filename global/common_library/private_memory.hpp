#pragma once


namespace global
{

// private bytes of this process
// Windows reports PrivateUsage, Linux reports RssAnon
// empty when the operating system does not report it
//
// testing: yes (unit)
//
std::optional<std::size_t> private_memory_bytes();

}// namespace global
