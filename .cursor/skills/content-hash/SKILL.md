---
name: content-hash
description: >-
  Keep a Galimatron version hash on important-tag values only. Use when changing
  important_tags_hash, hash_of_hashes, a version hash, wizard create/delete hash,
  or HASH_DOES_NOT_MATCH.
---

# Version hash

A version hash is the values of tags that carry `important`. Left out: aggregate names (the root too), labels (`important` only selects the tag; the tag name and mode stay out too), and paths.

`important_tags_hash` hashes those values for one version definition. No definition hashes empty text. A label statement is another node and is not hashed.

Each live aggregate stores that content hash on its declaration. The root declaration stores `hash_of_hashes` of those hashes, the root's own included. A deleted declaration is left out. `hash_of_hashes` sorts the content hashes before combining them: never order them by aggregate index, name, or chunk path.

`write_version_hash` and `is_version_hashes_match` use the same computation. A version with any stored hash has one on every aggregate that is not deleted. No stored hash: the version is not checked.

A user xslt does not rewrite a stored hash. After the xslt and the seal, the language is validated as after gal2cpp, and a hash that no longer matches fails there.

`wizard create hash <versions> force` stores the hash of the important tags as they are now. `wizard delete hash <versions> force` removes it. The validation before the wizard skips `hash does not match` and `hash is absent` for the named versions only. The validation after a user xslt still checks the hash, and it does not see `force`, because the wizard statement is absent from xml.

Changing what is written, or the sort, changes every stored root hash that combines more than one content hash. An aggregate content hash stays while its important-tag values stay.
