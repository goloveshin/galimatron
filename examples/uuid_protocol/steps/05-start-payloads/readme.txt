// The first payload: a lamp.
//
// The client speaks version 2, then version 3.
// Version 2 has no payload. Version 3 says hello, sends the lamp lit, and bye.
// Each hello names its version.
//
// The previous GaL had system messages and an empty payload.
// This GaL publishes version 3 and closes it with a hash. Earlier hashes stay.
// The lamp is a payload message. Its field lit defaults to dark. The client sends it lit.
// Older lamp versions are removed, so the live lamp is this version.
// The uuid xslt is unchanged.
// The generation xslt writes the field and its default.
// Version 3 is the first version whose payload namespace holds the lamp.
//
// This shows a payload message, a field default, and a new published version beside the closed ones.
//
