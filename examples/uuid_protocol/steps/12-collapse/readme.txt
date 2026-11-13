// Versions 0 through 5 collapse into the anonymous body owned by version 6.
// Version 6 is published. It is the only version the server speaks.
//
// The client speaks that version only: hello, the watch, the vessel, and bye.
// An earlier client's uuid is not registered. The server throws
// "no message with uuid" and the session fails.
//
// The generator writes v6 and no conversion sources. There is no older
// published version to convert from. conversion-state.xml still lists every
// pair carried in from earlier steps. Those pairs are not removed: a pair
// leaves the list only when this run emits that pair, and this run emits none.
// The conversion .cpp files are not rewritten. delete-if-not-rewrited is false,
// so a file left in generated_conversion would stay, but this step does not
// compile one. The types those conversions name are gone.
//
// The greeting from the substitution step stays. dictionary.md still shows
// North Light on watch.
//
