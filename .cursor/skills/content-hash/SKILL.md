---
name: content-hash
description: >-
  Keep a Galimatron version hash on important-tag values only. Aggregate
  names, labels, and paths stay out. Use when adding or changing
  important_tags_hash, hash_of_hashes, a version hash, wizard create hash,
  wizard delete hash, or HASH_DOES_NOT_MATCH.
---

# Version hash

A version hash is the values of tags that carry `important`.

- Aggregate names stay out, including the root name.
- Labels stay out, in a version and on a tag. `important` only selects the tag. The tag name and the tag mode stay out.
- Paths stay out.

`important_tags_hash` hashes those values for one version definition. No definition hashes empty text. A label statement is a different node and is not hashed.

Each live aggregate stores that content hash on its declaration. The root declaration stores `hash_of_hashes` of those content hashes, including the root's own. A deleted declaration is left out. `hash_of_hashes` sorts the content hashes before combining them. Do not order them by aggregate index, aggregate name, or chunk path.

`write_version_hash` and `is_version_hashes_match` use that same computation. A version with any stored hash stores one on every aggregate that is not deleted. No stored hash means the version is not checked.

A user xslt does not rewrite a stored hash. After the xslt and the seal, the language is validated the same way as after gal2cpp. A hash that no longer matches fails that validation.

`wizard create hash <versions> force` stores the hash of the important tags as they are now. `wizard delete hash <versions> force` removes that hash. The validation before the wizard skips `hash does not match` and `hash is absent` for those versions. A version the command does not name is still checked. The validation after a user xslt checks the hash. The wizard statement is absent from xml, so that later check does not see `force`.

Changing what is written, or the sort, changes every stored root hash that combines more than one content hash. An aggregate content hash stays when its important-tag values stay.
