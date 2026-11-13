// The lamp gains brightness, and fog appears.
//
// The client speaks version 2, then 3, then 4.
// Version 4 says hello, sends the lamp lit at 1500 candela, fog at 250 metres, and bye.
// The bye text names version 4. An error carries the text stored before the server sends it.
//
// The previous GaL published the lamp with lit.
// This GaL publishes version 4 and closes it with a hash. Earlier hashes stay.
// The lamp keeps lit and gains candela. Fog is new: how far you can see, in metres.
// Bye and error gain a message. Hello already has that field.
// The uuid xslt is unchanged.
// The generation xslt keeps every recorded conversion and adds an empty conversion
// from each older version into version 4.
// Version 3 has a lamp, so that conversion declares apply for the lamp.
// Versions 1 and 2 have no payload.
//
// This shows a new published version, a field added beside a closed one, and conversions that accumulate.
//
