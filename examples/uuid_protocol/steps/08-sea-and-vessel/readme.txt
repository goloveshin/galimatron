// Fog gains a sea state, and a vessel appears.
//
// The client speaks version 2, then 3, then 4, then 5.
// Version 5 says hello, sends the lamp lit at 1500 candela,
// fog at 250 metres in a swell, a steamer at bearing 40, and bye.
// The bye text names version 5.
//
// The previous GaL published the lamp with candela and fog in metres.
// This GaL publishes version 5 and closes it with a hash. Earlier hashes stay.
// Two enums appear: sea state (calm, swell) and vessel kind (sail, steamer).
// Fog gains sea. A vessel carries kind and bearing.
// The lamp gains no field. Its uuid still advances, because the uuid tag asks for a new one.
// An ordering places the enum before the struct that uses it.
// The uuid xslt is unchanged.
// The generation xslt keeps recorded conversions and adds an empty conversion into version 5.
// The version 3 lamp conversion still declares apply, and its body stays empty:
// this step stores the incoming message, so that conversion has no lamp to fill.
//
// This shows an enum, an ordering, and a uuid that advances on a message whose fields stay the same.
//
