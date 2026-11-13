// One watch carries the lamp and the fog. The server keeps the last version.
//
// The client speaks version 2, then 3, then 4, then 5, then 6.
// Version 6 says hello, then a watch for 14 March 1912:
// the lamp lit at 1200 candela, fog at 200 metres in a gale,
// and a launch named North Light at bearing 15, then bye.
// The bye text names version 6.
//
// The previous GaL sent the lamp, the fog, and the vessel as their own messages.
// This GaL publishes version 6 and closes it with a hash. Earlier hashes stay.
// After the uuid is copied, lamp and fog drop it. They stay types.
// The watch is the message that carries the day, the lamp, and the fog.
// Sea gains gale. Vessel kind gains launch. The vessel gains a name, and its uuid still advances.
// The uuid xslt is unchanged.
// The generation xslt writes a struct with no uuid as a type and skips its registration.
// Recorded conversions stay. Conversions into version 6 are left empty.
// You write those bodies. Version 3 lamp fills the watch lamp.
// Version 4 adds candela and metres. Version 5 adds the sea and the vessel.
// A version 6 watch or vessel is stored as it arrives.
// Apply is one function per source type, so the body you write is the one that runs.
//
// This shows a nested payload, a type that is not a message,
// and a conversion body that generation leaves for you to write.
//
