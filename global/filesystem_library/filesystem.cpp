// global
#include "global/filesystem_library/filesystem.hpp"


namespace global::filesystem
{

std::ostream& operator<<(std::ostream& stream, const directory_compare_result_t& result)
{
    if (!result.diff.empty())
    {
        return stream << result.diff;
    }
    return stream << (result.same ? "same" : "different");
}

}// namespace global::filesystem
