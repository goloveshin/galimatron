// galimatron
#include "galimatron/common_library/forward.hpp"


namespace galimatron::runtime
{

// Original Character Escaped Form
// " (double quote)	    &quot;
// ' (single quote)	    &apos;
// < (less-than)	    &lt;
// > (greater-than)	    &gt;
// & (ampersand)	    &amp;
//
// Saxon: DOE works only in a couple of commands, and often does not work at all.
// use-character-maps does not work either.
//
content_t replace_escaped_form_to_original_symbols(const content_t&);
file_content_t replace_escaped_form_to_original_symbols(const file_content_t&);

}// namespace galimatron::runtime
