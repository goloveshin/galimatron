#pragma once


// INHERIT is kept by later versions and copied from the root into nested aggregates.
// ONCE stays in this version and is still copied from the root into nested aggregates of this version.
// LOCAL stays in this version of this aggregate.
//
// The same words on a version cut what that version receives. own is a version word only.
// once drops earlier versions of this aggregate and still receives the root version.
// local drops earlier versions of this aggregate and does not receive the root.
// own receives earlier versions of this aggregate and does not receive the root.
// local and own on the root are errors.
//
// testing: yes (unit)
//
namespace galimatron::translator::state
{

enum class tag_mode_et
{
    INHERIT,
    ONCE,
    LOCAL,
    OWN
};

}// namespace galimatron::translator::state
