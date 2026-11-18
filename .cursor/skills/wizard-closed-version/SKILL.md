---
name: wizard-closed-version
description: >-
  Check how a wizard command interacts with a closed version (a root version at or
  before the last hashed declaration; the xslt seal calls it sealed). Use when
  changing a wizard command (aggregate, version, hash, tag, label, ordering) or a
  version hash.
---

# Closed versions and wizard commands

A closed version is a root-aggregate version at or before the last declaration that stores a hash (`versions().last_hashed_index` on the root). The xslt seal calls that history sealed. A later version is open, and with no hash every version is open. Every aggregate declares the same version indexes as the root, so an aggregate's last version is the root's last version.

A command that writes into a version refuses a closed version.

- `wizard create hash` closes the listed versions by storing the hash. No version means the last version, resolved when the command runs. `force` stores the hash when the stored one does not match. The validation before the wizard skips the hash checks for the named versions. The validation after a user xslt still checks the stored hash.
- `wizard delete hash` removes the hash and opens those versions. No version means the last version. It is the command that changes a closed version. `force` removes the hash when the stored one does not match. The validation before the wizard skips the hash checks for the named versions.
- `wizard create version` adds an open version after the closed ones and changes no closed version. `once`, `local`, or `own` may follow `version`.
    - Plain keeps `local` on an aggregate whose previous version is `local` and copies that body. It keeps `own` and does not copy the body.
    - `once` writes `version once` on the root and on every live aggregate.
    - `local` writes `version local` on every live nested aggregate and `version once` on the root.
    - `own` writes `version own` on every live nested aggregate and leaves the root unmarked.
    - `wizard create aggregate local` and `own` mark the last version of each new aggregate.
- `wizard delete version` removes the listed versions from every aggregate. No version means the last version. Each listed version must be open (a closed one is refused), and the versions that remain run from 0 without gaps.
- `wizard collapse version <first>-<last>` is one inclusive range.
    - When a version follows the range, that owner gets a new anonymous body, `version before <owner> { }`, placed right before it. The statements of the range move into that body, on the root and on every aggregate.
    - A later statement of the same name replaces the earlier one, so the body keeps the last variant as inheritance would. An unnamed tag takes its name from a label's tag-name-from, so two field rows with different names both stay. A delete replaces the tag of that name, and a later tag replaces the delete.
    - Refused when the root owner is deleted: the body would have no live owner. An aggregate whose owner is already deleted gets no parked body, and its range is marked deleted, so it stays deleted.
    - When the range ends on the last version, a new version is created and the statements are parked there instead of in `version before`. An aggregate whose last version is already deleted gets a deleted declaration and no body.
    - The range is marked deleted, its hashes are removed, and its bodies are deleted. A closed version in the range is changed. The new version is open. An anonymous body is deleted when its owner is deleted.
- `wizard delete aggregate` writes `delete;` into the last version while the aggregate has a live version. That write is refused when the last version is already deleted, and `is_wizard_delete_aggregate_last_version_open` refuses it when the last version is closed. When every version is already deleted, the command deletes the chunk and writes into no version, a closed one included.
    - No name deletes every aggregate whose versions are all deleted and leaves an aggregate with a live version. The no-name form has no mode.
    - `physically` removes the named chunks and does not write `delete;`. It is refused when the aggregate is present in a closed version, and when an ordering of a version names it.
- `wizard create tag`, `delete tag`, `create label`, and `create ordering` are written in an empty chunk and name the aggregate. They write into its last version, and `is_wizard_version_statement` refuses the command when that version is closed or deleted. Ordering writes only into the root.
    - `wizard delete tag` writes `delete <name>;`. The tag stays absent in that version and the later ones. `wizard create version` does not copy it and does not change the closed version that holds the delete.
- `wizard create aggregate` writes a live definition of the last version. `is_wizard_create_aggregate_last_version_open` refuses it when that version is closed.
- `wizard copy aggregate` writes a new aggregate and copies the source declarations and bodies, including a deleted version and a stored hash. The source stays. A closed version of the source is copied into the new file and not edited. Refused when the source is present in a closed version, because the new live aggregate would change that version's root hash. A deleted declaration in a closed version is copied as deleted, and the root hash stays.
- `wizard continue aggregate` adds the next version on the root and on every aggregate, as `wizard create version` does. The source keeps its versions, and its new version is `delete;`. Each new aggregate is deleted in the earlier versions and live in the new one.
    - The new version holds the statements in effect on the source's last version: a later statement of the same name replaces the earlier one, and a delete removes that tag. The dynamic tags `wizard create version` carries forward are written too. A local or once last version does not keep earlier statements.
    - A closed version of the source stays, and the new aggregate is deleted there, so the root hash stays. The source's last version is live. An ordering that names the source stays.
- `wizard create hash`, `wizard delete hash`, and `wizard create version` are written in an empty chunk: the file has no aggregates.
