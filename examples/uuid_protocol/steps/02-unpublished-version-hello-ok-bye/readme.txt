// An unpublished version, and a uuid that advances.
//
// The session is still the published empty messages.
//
// The previous GaL is closed. This GaL opens version 1.
// A dynamic label marks each uuid. A label stays outside the hash, so the version 0 hashes stay.
// Create version copies a labeled tag. The copy still holds the previous uuid.
// The xslt is new. It keeps the message id and adds one to the version counter.
// A further version adds one again. The same GaL writes the same text on a second run.
// Version 1 is unpublished. The generation xslt is unchanged and still writes a published version.
//
// This shows an unpublished version kept in the GaL, and a dynamic label that asks the xslt for the next uuid.
//
