#pragma once

// galimatron
#include "galimatron/translator_library/state/wizard/context.hpp"


namespace galimatron::translator::state::wizard
{

// a language entity whose lifetime the commands implement
//
enum class entity_et
{
    ROOT_AGGREGATE,
    AGGREGATE,
    VERSION,
    TAG,
    LABEL,
    ORDERING
};

enum class action_et
{
    C,
    U,
    D
};

// none is zero commands, one is a single command, many is at least one
//
enum class cardinality_et
{
    NONE,
    ONE,
    MANY
};

// physical changes the graph: a file, an aggregate, a version, a statement, or a hash.
// historical writes delete into a version. the history stays, and compilation drops what was deleted.
// none is a batch: it runs other commands
//
enum class edit_et
{
    NONE,
    PHYSICAL,
    HISTORICAL
};

namespace ent
{
using enum entity_et;
}// namespace galimatron::translator::state::wizard::ent

namespace car
{
using enum cardinality_et;
}// namespace galimatron::translator::state::wizard::car

namespace ac
{
using enum action_et;
}// namespace galimatron::translator::state::wizard::ac

namespace ed
{
using enum edit_et;
}// namespace galimatron::translator::state::wizard::ed

enum class command_et
{
    // wizard create root aggregate <name>
    //   mode: none
    //   context: chunk
    //     wizard create root aggregate protocol;
    //   writes protocol.gal next to the file that runs the command.
    //   that file is not filled, and it cannot be protocol.gal
    CREATE_ROOT_AGGREGATE,

    // wizard create aggregate [once|local|own] <name> [, <name>]*
    //   mode: once, local, own
    //   context: chunk
    //     wizard create aggregate ping, pong;
    //     wizard create aggregate local dictionary;
    //     wizard create aggregate own dictionary;
    //   writes ping.gal and pong.gal next to the file that runs the command.
    //   that file is not filled, and it cannot be one of those names.
    //   once, local, and own mark the last version of each new aggregate.
    //   the names differ, ignoring case
    CREATE_AGGREGATE,

    // wizard copy aggregate <source> to <name> [, <name>]* [in <directory>]
    //   mode: none
    //   context: chunk
    //     wizard copy aggregate ping to pong;
    //     wizard copy aggregate ping to pong, pang in payload;
    //   the source already exists. each name is a new aggregate.
    //   the new aggregate receives the source declarations and bodies, including a deleted version and a stored hash.
    //   the source stays. ordering is left alone.
    //   a source that is still present in a closed version is refused: the copy would change that version's root hash.
    //   a deleted declaration in a closed version is copied as deleted.
    //   the names differ, ignoring case, and differ from the source.
    //   the command file is not one of the new names. the root aggregate is neither source nor target
    COPY_AGGREGATE,

    // wizard continue aggregate <source> to <name> [, <name>]* [in <directory>]
    //   mode: none
    //   context: chunk
    //     wizard continue aggregate ping to pong;
    //     wizard continue aggregate ping to pong, pang in payload;
    //   adds the next version on the root and on every aggregate, the same way wizard create version does.
    //   the source keeps its versions. that new version of the source is delete;.
    //   each new aggregate is deleted in the earlier versions and live in the new one.
    //   the new version holds the statements of the source's last version, and the dynamic tags create version carries forward.
    //   a closed version of the source stays. the new aggregate is deleted there, so the root hash stays.
    //   the source's last version is live. the names differ, ignoring case, and differ from the source.
    //   the command file is not one of the new names. the root aggregate is neither source nor target.
    //   an ordering that names the source stays
    CONTINUE_AGGREGATE,

    // wizard delete aggregate <name> [, <name>]* [physically]
    //   mode: physically
    //   context: chunk
    //     wizard delete aggregate ping, pong;
    //     wizard delete aggregate ping physically;
    //   writes delete; into the last version of each named aggregate.
    //   that version is then historically deleted.
    //   the last version is open: a root version that stores a hash is closed, and so is every version up to that index.
    //   physically removes the named chunks. the name can be created again.
    //   an aggregate present in a closed version is refused.
    //   a version whose ordering names the aggregate is refused.
    //   no name removes every aggregate whose versions are already all deleted, and takes no mode.
    //   the names differ, ignoring case. the root aggregate is not a target
    DELETE_AGGREGATE,

    // wizard create version [once|local|own]
    //   mode: once, local, own
    //   context: an empty chunk, with no aggregates in that file
    //     wizard create version;
    //     wizard create version once;
    //     wizard create version local;
    //     wizard create version own;
    //   plain keeps local on an aggregate whose previous version is local and copies that body.
    //   plain keeps own and does not copy the body: the next version sees the earlier one.
    //   once does not carry forward: the next version is unmarked.
    //   once writes version once on the root and on every live aggregate.
    //   local writes version local on every live nested aggregate and version once on the root.
    //   own writes version own on every live nested aggregate and leaves the root unmarked.
    //   a local body copies the previous version's ordinary tags, labels, and orderings
    CREATE_VERSION,

    // wizard delete version [<version> [, <version>]*]
    //   mode: none
    //   context: an empty chunk, with no aggregates in that file
    //   no version means the last version of the root aggregate
    //     wizard delete version;
    //     wizard delete version 10, 11;
    //   removes those versions from every aggregate. each version is open.
    //   the versions that remain run from 0 without gaps.
    //   a root version that stores a hash is closed, and so is every version up to that index
    DELETE_VERSION,

    // wizard collapse version <first>-<last>
    //   mode: none
    //   context: an empty chunk, with no aggregates in that file
    //     wizard collapse version 1-4;
    //   one inclusive range. when a version follows the range, that version owns
    //   version before <owner>, and the statements move there.
    //   when the range ends on the last version, a new version is created and the
    //   statements move there. an aggregate whose last version is already deleted
    //   gets a deleted declaration and no body.
    //   marks the range deleted, drops its hashes, and deletes its bodies,
    //   on the root and on every aggregate, including a closed version
    COLLAPSE_VERSION,

    // wizard create label [aggregate] <name> = static|dynamic ...
    //   mode: none
    //   context: an empty chunk, with no aggregates in that file
    //     wizard create label protocol field = static { type };
    //   the command names the aggregate. the version is that aggregate's last version, and it is open
    CREATE_LABEL,

    // wizard create tag [aggregate] [name =] <labels> { <values> }
    //   mode: none
    //   context: an empty chunk, with no aggregates in that file
    //     wizard create tag protocol speed = field { "int" };
    //     wizard create tag hello field important { "std::string", "message" };
    //     wizard create tag dictionary entry { "uuid", "0", "stable id of a message" };
    //   the command names the aggregate. the version is that aggregate's last version, and it is open
    CREATE_TAG,

    // wizard delete tag [aggregate] <name>
    //   mode: none
    //   context: an empty chunk, with no aggregates in that file
    //     wizard delete tag item speed;
    //   writes delete <name>; into the open version. the tag stays absent there and in later versions. it does not delete the version
    DELETE_TAG,

    // wizard create ordering [aggregate] <name> = { <name>, <name> } [, { ... }]*
    //   mode: none
    //   context: an empty chunk, with no aggregates in that file
    //     wizard create ordering protocol precedes = { item, pong };
    //   the aggregate is the root aggregate. the version is its last version, and it is open
    CREATE_ORDERING,

    // wizard create hash [<version> [, <version>]*] [force]
    //   mode: force
    //   context: an empty chunk, with no aggregates in that file
    //   no version means the last version of the root aggregate
    //     wizard create hash;
    //     wizard create hash 10, 11;
    //     wizard create hash 10 force;
    //   force stores the hash of the important tags as they are now.
    //   the validation before the wizard does not check the hash of those versions.
    //   a version the command does not name is still checked.
    //   the validation after a user xslt checks the hash the command stored
    CREATE_HASH,

    // wizard delete hash [<version> [, <version>]*] [force]
    //   mode: force
    //   context: an empty chunk, with no aggregates in that file
    //   no version means the last version of the root aggregate
    //     wizard delete hash;
    //     wizard delete hash 10, 11;
    //     wizard delete hash 10 force;
    //   force removes the hash when the stored hash does not match.
    //   the validation before the wizard does not check the hash of those versions.
    //   a version the command does not name is still checked
    DELETE_HASH,

    // wizard { <create command>; ... }
    //   mode: none
    //   context: an empty chunk, with no aggregates in that file
    //     wizard { create root aggregate protocol; create version; create hash 0; }
    //   the commands run one by one: create commands, including create hash, delete tag, delete version, and collapse version
    BATCH
};

namespace cmd
{
using enum command_et;
}// namespace galimatron::translator::state::wizard::cmd

template <entity_et Entity, action_et Action>
struct act_t
{
    static constexpr entity_et entity = Entity;
    static constexpr action_et action = Action;
};

template <cardinality_et C, cardinality_et U, cardinality_et D>
struct lifetime_t
{
    static constexpr cardinality_et c = C;
    static constexpr cardinality_et u = U;
    static constexpr cardinality_et d = D;
};

template <entity_et Entity, typename Lifetime>
struct entity_declaration_t
{
    static constexpr entity_et entity = Entity;
    using lifetime_t = Lifetime;
};

template <command_et Command, edit_et Edit, typename ... Act>
struct command_declaration_t
{
    static constexpr command_et command = Command;
    static constexpr edit_et edit = Edit;
};

using root_aggregate_lifetime_t = entity_declaration_t<
    ent::ROOT_AGGREGATE,
    lifetime_t<car::ONE, car::MANY, car::NONE>
>;
using aggregate_lifetime_t = entity_declaration_t<
    ent::AGGREGATE,
    lifetime_t<car::MANY, car::MANY, car::MANY>
>;
using version_lifetime_t = entity_declaration_t<
    ent::VERSION,
    lifetime_t<car::MANY, car::MANY, car::MANY>
>;
using tag_lifetime_t = entity_declaration_t<
    ent::TAG,
    lifetime_t<car::MANY, car::MANY, car::MANY>
>;
using label_lifetime_t = entity_declaration_t<
    ent::LABEL,
    lifetime_t<car::MANY, car::MANY, car::NONE>
>;
using ordering_lifetime_t = entity_declaration_t<
    ent::ORDERING,
    lifetime_t<car::MANY, car::MANY, car::NONE>
>;

using entities_t = std::tuple<
    root_aggregate_lifetime_t,
    aggregate_lifetime_t,
    version_lifetime_t,
    tag_lifetime_t,
    label_lifetime_t,
    ordering_lifetime_t
>;

// copy aggregate is C of an aggregate.
// continue aggregate is C of an aggregate, D of the source, and the same version C and updates as create version.
// create version is C of a version and U of the root and of an aggregate.
// create tag, create label, and create ordering are C and U: a later statement of the same name is the update.
// create hash and delete hash are U of a version.
// delete version and collapse version are D of a version.
//
using commands_t = std::tuple<
    command_declaration_t<cmd::CREATE_ROOT_AGGREGATE, ed::PHYSICAL,
        act_t<ent::ROOT_AGGREGATE, ac::C>>,
    command_declaration_t<cmd::CREATE_AGGREGATE, ed::PHYSICAL,
        act_t<ent::AGGREGATE, ac::C>>,
    command_declaration_t<cmd::COPY_AGGREGATE, ed::PHYSICAL,
        act_t<ent::AGGREGATE, ac::C>>,
    command_declaration_t<cmd::CONTINUE_AGGREGATE, ed::PHYSICAL,
        act_t<ent::AGGREGATE, ac::C>,
        act_t<ent::AGGREGATE, ac::U>,
        act_t<ent::AGGREGATE, ac::D>,
        act_t<ent::VERSION, ac::C>,
        act_t<ent::ROOT_AGGREGATE, ac::U>>,
    command_declaration_t<cmd::DELETE_AGGREGATE, ed::HISTORICAL,
        act_t<ent::AGGREGATE, ac::D>>,
    command_declaration_t<cmd::CREATE_VERSION, ed::PHYSICAL,
        act_t<ent::VERSION, ac::C>,
        act_t<ent::ROOT_AGGREGATE, ac::U>,
        act_t<ent::AGGREGATE, ac::U>>,
    command_declaration_t<cmd::DELETE_VERSION, ed::PHYSICAL,
        act_t<ent::VERSION, ac::D>>,
    command_declaration_t<cmd::COLLAPSE_VERSION, ed::PHYSICAL,
        act_t<ent::VERSION, ac::D>>,
    command_declaration_t<cmd::CREATE_HASH, ed::PHYSICAL,
        act_t<ent::VERSION, ac::U>>,
    command_declaration_t<cmd::DELETE_HASH, ed::PHYSICAL,
        act_t<ent::VERSION, ac::U>>,
    command_declaration_t<cmd::CREATE_TAG, ed::PHYSICAL,
        act_t<ent::TAG, ac::C>,
        act_t<ent::TAG, ac::U>>,
    command_declaration_t<cmd::DELETE_TAG, ed::HISTORICAL,
        act_t<ent::TAG, ac::D>>,
    command_declaration_t<cmd::CREATE_LABEL, ed::PHYSICAL,
        act_t<ent::LABEL, ac::C>,
        act_t<ent::LABEL, ac::U>>,
    command_declaration_t<cmd::CREATE_ORDERING, ed::PHYSICAL,
        act_t<ent::ORDERING, ac::C>,
        act_t<ent::ORDERING, ac::U>>,
    command_declaration_t<cmd::BATCH, ed::NONE>
>;

template <entity_et Entity, action_et Action, command_et Command, edit_et Edit, typename ... Act>
constexpr std::size_t lists_action(command_declaration_t<Command, Edit, Act...>)
{
    return ((Act::entity == Entity && Act::action == Action) || ... || false) ? std::size_t{ 1 } : std::size_t{ 0 };
}

// how many commands in the tuple list this entity and action
//
template <entity_et Entity, action_et Action, typename ... Declaration>
constexpr std::size_t command_count(std::tuple<Declaration...>)
{
    return (lists_action<Entity, Action>(Declaration{}) + ... + std::size_t{ 0 });
}

// none is zero commands, one is a single command, many is at least one
//
constexpr bool cardinality_fits(
    const cardinality_et cardinality,
    const std::size_t count
)
{
    switch (cardinality)
    {
    case cardinality_et::NONE:
        return count == 0;
    case cardinality_et::ONE:
        return count == 1;
    case cardinality_et::MANY:
        return count >= 1;
    }
    return false;
}

// the commands that list this entity provide its {C, U, D}
//
template <typename Entity>
constexpr bool entity_fits()
{
    using life = typename Entity::lifetime_t;
    return cardinality_fits(life::c, command_count<Entity::entity, action_et::C>(commands_t{}))
        && cardinality_fits(life::u, command_count<Entity::entity, action_et::U>(commands_t{}))
        && cardinality_fits(life::d, command_count<Entity::entity, action_et::D>(commands_t{}));
}

static_assert(entity_fits<root_aggregate_lifetime_t>(), "root aggregate lifetime does not match its commands");
static_assert(entity_fits<aggregate_lifetime_t>(), "aggregate lifetime does not match its commands");
static_assert(entity_fits<version_lifetime_t>(), "version lifetime does not match its commands");
static_assert(entity_fits<tag_lifetime_t>(), "tag lifetime does not match its commands");
static_assert(entity_fits<label_lifetime_t>(), "label lifetime does not match its commands");
static_assert(entity_fits<ordering_lifetime_t>(), "ordering lifetime does not match its commands");

// historical writes a delete, so the command lists D. a batch is the command with edit none and no action
//
template <command_et Command, edit_et Edit, typename ... Act>
constexpr bool declaration_edit_fits()
{
    constexpr auto deletes =
        ((Act::action == action_et::D ? std::size_t{ 1 } : std::size_t{ 0 }) + ... + std::size_t{ 0 });
    if constexpr (Command == command_et::BATCH)
    {
        return Edit == edit_et::NONE && sizeof...(Act) == 0;
    }
    else if constexpr (Edit == edit_et::HISTORICAL)
    {
        return deletes > 0;
    }
    else if constexpr (Edit == edit_et::PHYSICAL)
    {
        return sizeof...(Act) > 0;
    }
    else
    {
        return false;
    }
}

template <typename Declaration>
inline constexpr bool declaration_fits = false;

template <command_et Command, edit_et Edit, typename ... Act>
inline constexpr bool declaration_fits<command_declaration_t<Command, Edit, Act...>> =
    declaration_edit_fits<Command, Edit, Act...>();

// historical lists D. physical lists an action. a batch has edit none and no action
//
template <typename ... Declaration>
constexpr bool command_edits_fit(std::tuple<Declaration...>)
{
    return (declaration_fits<Declaration> && ...);
}

static_assert(command_edits_fit(commands_t{}), "command edit kind does not match its actions");

// each enumerator has one declaration
//
template <typename ... Declaration>
constexpr std::size_t declarations_of(
    std::tuple<Declaration...>,
    const command_et command
)
{
    return ((Declaration::command == command ? std::size_t{ 1 } : std::size_t{ 0 }) + ... + std::size_t{ 0 });
}

template <typename Commands, std::size_t ... Index>
constexpr bool commands_cover_enum(std::index_sequence<Index...>)
{
    return ((declarations_of(Commands{}, static_cast<command_et>(Index)) == 1) && ...);
}

static_assert(
    commands_cover_enum<commands_t>(std::make_index_sequence<magic_enum::enum_count<command_et>()>{}),
    "each command has one declaration"
);

std::string to_string(const command_et&);

}// namespace galimatron::translator::state::wizard
