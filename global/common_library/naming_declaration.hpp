#pragma once

/*

A name says what the thing is for. The suffix says only its C++ shape.
The words do that job: ordering_rank_container, refresh_live_and_hashed.
A second of something is not a _2.

Repeated spellings live in type_declaration.hpp (index_t, name_t, path_t, and their
optional and plural forms). A galimatron namespace sees them through using namespace global.

Plural is a word, not a letter: index_t, indices_t, indices_ot.
A deque of reference_wrappers is a plural _t. optional of that deque is _ot.
vertex_ocrt is one const reference, optional. It is not a container.

Suffixes, from the outside in:

    _t        a type
    _ot       std::optional of it
    _ft       std::function
    _et       an enum class; its values are SCREAMING_SNAKE
    _at       std::any
    _rt       std::reference_wrapper
    _crt      std::reference_wrapper of const
    _ort      std::optional of _rt
    _ocrt     std::optional of _crt
    _sptr_t   std::shared_ptr
    _sptr_ot  std::optional of that shared_ptr

A data member ends with _. A vertex_descriptor parameter or member starts with vd_.

A Boost.LEAF exception struct has no _t (e_what_info, domain::translator_library).
A named string constant is inline constexpr std::string_view.

*/
