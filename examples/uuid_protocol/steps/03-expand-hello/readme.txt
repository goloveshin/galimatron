// Hello gains a text, and version 2 is published.
//
// The client hello says "hello client version v2!". Bye has no fields.
//
// The previous GaL left version 1 open, only to prove the next uuid.
// This GaL deletes that version and opens it again, so the uuid advances,
// then publishes version 2 and closes it with a hash.
// Galimatron refreshes that hash after the uuid xslt, so the hash covers the generated uuid.
// The version 0 hashes stay.
// Hello gains a string field named message.
// The uuid xslt is unchanged.
// The generation xslt writes the message field, and it writes both published versions.
//
// This shows a field on a system message, and a hash taken after the uuid xslt.
//
