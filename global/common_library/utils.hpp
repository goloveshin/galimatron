#pragma once

// from internet
//
template <typename ... Ts>
struct overloads_t : Ts... { using Ts::operator()...; };

template <typename It>
auto subrange(const std::pair<It, It>& pair)
{
    return ranges::subrange(pair.first, pair.second);
}


namespace global
{

// calls process once for each key, on the first element of the range with that key
// key_of(element) is the key. a repeated key is skipped
//
// testing: yes (unit)
//
template <typename Range, typename Process, typename KeyOf>
void find_and_process_unique(
    Range&& range,
    Process&& process,
    KeyOf&& key_of
)
{
    using key_t = std::remove_cvref_t<std::invoke_result_t<KeyOf&, ranges::range_reference_t<Range>>>;
    std::unordered_set<key_t> seen;
    for (const auto& element : range)
    {
        if (!seen.insert(key_of(element)).second)
        {
            continue;
        }
        process(element);
    }
}

}// namespace global
