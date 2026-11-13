// galimatron
#include "galimatron/translator_library/parser/gal_version.hpp"
#include "galimatron/common_library/exception_augment.hpp"

// global
#include "global/filesystem_library/filesystem.hpp"

// Boost.LEAF
#include <boost/leaf/exception.hpp>
#include <boost/leaf/on_error.hpp>


namespace galimatron::translator::parser::gal_version
{

namespace
{

// gal.version text is not a version number
//
[[noreturn]] void bad_content()
{
    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::parser{},
        exception::e_what_info{ "gal.version must contain a version number" }
    );
}

bool is_space(const char symbol)
{
    return std::isspace(static_cast<unsigned char>(symbol)) != 0;
}

bool is_digit(const char symbol)
{
    return std::isdigit(static_cast<unsigned char>(symbol)) != 0;
}

}// namespace

index_t from_content(const content_t& text)
{
    std::size_t index = 0;
    while (index < text.size() && is_space(text[index]))
    {
        ++index;
    }

    if (index >= text.size() || !is_digit(text[index]))
    {
        bad_content();
    }

    index_t version = 0;
    while (index < text.size() && is_digit(text[index]))
    {
        const auto digit = static_cast<index_t>(text[index] - '0');
        if (version > (std::numeric_limits<index_t>::max() - digit) / 10)
        {
            bad_content();
        }
        version = version * 10 + digit;
        ++index;
    }

    while (index < text.size() && is_space(text[index]))
    {
        ++index;
    }

    if (index != text.size() || version == 0)
    {
        bad_content();
    }

    return version;
}

index_t from_input_dir(
    const gf::abstract::filesystem_t& filesystem,
    const path_t& input_gal_dir
)
{
    const auto path = input_gal_dir / std::string{ file_name };
    if (!filesystem.exist(path))
    {
        return current;
    }

    auto load = leaf::on_error(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::parser{},
        exception::e_path_info_for(path)
    );

    const auto content = filesystem.read(path);
    return from_content(*content);
}

void require_supported(const index_t gal_version)
{
    if (gal_version == current)
    {
        return;
    }

    leaf::throw_exception(
        exception::domain::translator_library{},
        exception::subdomain::translator_library::parser{},
        exception::e_what_info{ std::format("unknown GaL version: {}", gal_version) }
    );
}

}// namespace galimatron::translator::parser::gal_version
