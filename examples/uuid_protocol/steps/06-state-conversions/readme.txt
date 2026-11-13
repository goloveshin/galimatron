// Conversions appear. The protocol stays the version 3 lamp.
//
// The session is the same as the previous step: version 2 hello, then version 3 hello and the lamp lit.
//
// The GaL is unchanged.
// The uuid xslt is unchanged.
// The generation xslt now writes an empty conversion from each older version into version 3.
// Versions 1 and 2 have no payload, so those conversions have nothing to apply.
// The first run records each pair. A later run keeps a pair that is already recorded.
//
// This shows a generated conversion template, and a recorded pair that a later run keeps.
//
