#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/graph/graph.hpp"


namespace galimatron::translator::content_hash
{

// SHA-256 of the text, a lowercase hexadecimal string
//
// testing: yes (unit)
//
std::string sha256_hex(std::string_view bytes);

// hash of the contents of the version's tags that carry the important label. the name and the labels are not part of the hash.
// no definition means the hash of empty text
//
// testing: no
//
std::string important_tags_hash(
    const state::holder::vertex_graph::graph_t& graph,
    state::vertex_descriptor_ot vd_definition
);

// hash of the content hashes. they are sorted first, so the file path and the aggregate order are not part of it
//
// testing: no
//
std::string hash_of_hashes(const std::vector<std::string>& hashes);

}// namespace galimatron::translator::content_hash
