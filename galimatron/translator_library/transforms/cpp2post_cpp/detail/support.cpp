// galimatron
#include "galimatron/translator_library/transforms/cpp2post_cpp/detail/support.hpp"
#include "galimatron/common_library/exception_augment.hpp"


namespace galimatron::translator::transforms::cpp2post_cpp
{

using namespace galimatron::translator::state;

[[noreturn]] void throw_compiler(const std::string& what)
{
    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::cpp2post_cpp{},
        exception::e_what_info{ what }
    );
}

}// namespace galimatron::translator::transforms::cpp2post_cpp
