// galimatron
#include "galimatron/translator_library/state/content_hash.hpp"

// galimatron
#include "galimatron/tests/forward.hpp"


namespace galimatron
{

TEST_SUITE("galimatron/translator_library/content_hash" * galimatron::test_axes::mark(galimatron::test_axes::component))
{

TEST_CASE("sha256")
{
    CHECK(translator::content_hash::sha256_hex("") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    CHECK(translator::content_hash::sha256_hex("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

}

}// namespace galimatron
