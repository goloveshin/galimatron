// The published protocol stays version 6. A dictionary names every tag.
//
// The client still speaks version 2, then 3, then 4, then 5, then 6.
// Version 6 is unchanged: hello, the watch, and bye.
//
// This GaL opens version 6 and does not hash it. published is once, so version 6 stays unpublished.
// Versions 0 through 5 stay closed. generate-uuid.xsl runs first and advances
// a copied uuid in version 6. The dictionary aggregate is deleted in the closed versions
// and live in version 6.
// The language document carries history beside language. The xslt reads that
// history and adds an entry for each tag name at the version where the name first appears.
// gal:dictionary-entry writes the tag. A name already on the dictionary stays,
// including the text a wizard create tag wrote.
// The Markdown xslt copies the compiled document and writes dictionary.md.
// It runs before the source xslt, which replaces that document.
//
// This shows a project dictionary made of ordinary tags.
//
