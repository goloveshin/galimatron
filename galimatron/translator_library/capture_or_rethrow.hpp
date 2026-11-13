#pragma once

// Boost.LEAF
#include <boost/leaf/handle_errors.hpp>

// leaf_spy
#include "captured_leaf_exception.hpp"


namespace galimatron::translator
{

// a Taskflow thread has no leaf context.
// a throw inside fn is captured and rethrown as captured_leaf_exception.
// a success returns
//
// testing: no
//
template <typename Fn>
void capture_or_rethrow(Fn&& fn)
{
    auto captured = leaf::try_capture_all(std::forward<Fn>(fn));
    if (!captured)
    {
        throw ::leaf_spy::captured_leaf_exception{ std::move(captured) };
    }
}

}// namespace galimatron::translator
