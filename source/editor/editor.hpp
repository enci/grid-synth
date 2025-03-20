#pragma once

#include "core/grid_synth.hpp"
#include <vector>

namespace gs
{

class editor
{
public:
    editor();
    ~editor() = default;
    void edit();

private:
    // UI components
    void edit_alphabet();
    void edit_transformation_stack();
    void edit_selected_transformation();
    void edit_synthesizer();

    // Specific transformation editors
    void edit_random_transformation(random_transformation* transform);
    void edit_rule_based_transformation(rule_based_transformation* transform);

    // File operations
    bool save_to_file(const std::string& filename);
    bool load_from_file(const std::string& filename);
    void show_save_dialog();
    void show_load_dialog();
    void show_file_dialog(bool isSave);

    // Core synthesizer data
    grid_synth m_synth;

    // UI state for transformations
    int m_selected_transform_index = -1;
    std::vector<int> m_transforms_to_remove;
    std::vector<int> m_symbols_to_remove;

    // Pattern editing state
    bool m_editing_pattern = false;
    bool m_editing_search = true;
    grid m_pattern_grid;  // Working copy for editing
    grid m_search_pattern; // Actual pattern for search
    grid m_replacement_pattern; // Actual pattern for replacement
    int m_selected_symbol_id = alphabet::wildcard_symbol.id;

    // Temporary reference to the current rule being edited
    rule_based_transformation* m_current_rule = nullptr;

    // File dialog
    bool m_show_save_dialog = false;
    bool m_show_load_dialog = false;
    char m_filename[256] = "";
};

}