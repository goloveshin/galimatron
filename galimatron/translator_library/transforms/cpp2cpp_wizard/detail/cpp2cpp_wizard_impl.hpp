#pragma once

// galimatron
#include "galimatron/translator_library/state/holder/holder.hpp"
#include "galimatron/translator_library/transforms/cpp2cpp_validate/cpp2cpp_validate.hpp"


namespace galimatron::translator::transforms::cpp2cpp_wizard
{

using namespace galimatron::translator::state;

class cpp2cpp_wizard_t
{
public:
    explicit cpp2cpp_wizard_t(holder::language_part_t&);

public:
    void translate();

private:
    // the executed command stays on its chunk as // comments
    //
    void comment_executed_command();

    void apply_wizard_batch();
    void apply_one(wizard::command_et);
    void apply_wizard_create_root_aggregate();
    void apply_wizard_create_aggregate();
    void apply_wizard_copy_aggregate();
    void apply_wizard_continue_aggregate();
    void apply_wizard_delete_aggregate();
    void apply_wizard_create_version();
    void apply_wizard_delete_version();
    void apply_wizard_collapse_version();
    void apply_wizard_create_label();
    void apply_wizard_create_tag();
    void apply_wizard_delete_tag();
    void apply_wizard_create_ordering();
    void apply_wizard_create_hash();
    void apply_wizard_delete_hash();

    // the checks snapshot the graph. the next command must see what the previous command added
    //
    void refresh_checks();

    holder::language_part_t* language_state_;
    std::unique_ptr<cpp2cpp_validate::validate_memo_t> validate_memo_;
    std::unique_ptr<cpp2cpp_validate::cpp2cpp_validate_t> validate_;
};

}// namespace galimatron::translator::transforms::cpp2cpp_wizard
