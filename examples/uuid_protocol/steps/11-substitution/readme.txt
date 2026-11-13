// The published protocol stays version 6. A greeting substitutes a station name.
//
// The client still speaks version 2, then 3, then 4, then 5, then 6.
// Version 6 is unchanged: hello, the watch, and bye.
//
// This GaL does not open a version and does not hash version 6.
// Versions 0 through 5 stay closed. The dictionary stays in the open version.
// station holds North Light. The greeting entry is a template, marked _te.
// The gal file keeps {{station}}. Compilation renders that string.
// The dictionary xslt reads the compiled document, so dictionary.md shows
// North Light on watch.
// A name already on the dictionary stays. The fill xslt adds station,
// because that tag is new and has no entry of its own.
//
// This shows a template string rendered from another tag in the same version.
//
