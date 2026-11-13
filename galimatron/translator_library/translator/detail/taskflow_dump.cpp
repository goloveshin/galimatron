// galimatron
#include "galimatron/translator_library/translator/detail/taskflow_dump.hpp"


namespace galimatron::translator::translator
{

std::string readable_taskflow_dump(const std::string& dump)
{
    // p and at least eight hex digits: a pointer printed by Taskflow::dump
    static const std::regex pointer_id("p[0-9A-Fa-f]{8,}");

    std::map<std::string, std::string> names;
    std::string result;
    auto unread = dump.begin();

    for (
        std::sregex_iterator match(dump.begin(), dump.end(), pointer_id), end;
        match != end;
        ++match
    )
    {
        result.append(unread, match->prefix().second);
        const auto id = match->str();
        const auto found = names.find(id);
        if (found == names.end())
        {
            const auto inserted = names.emplace(
                id,
                std::format("n{}", names.size() + 1)
            );
            result += inserted.first->second;
        }
        else
        {
            result += found->second;
        }
        unread = match->suffix().first;
    }

    result.append(unread, dump.end());
    return result;
}

}// namespace galimatron::translator::translator
