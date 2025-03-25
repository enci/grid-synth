#pragma once

#include "core/grid_synth.hpp"
#include <vector>

namespace gs
{

/// @brief Editor class for Grid Synth
///
/// Provides interactive UI for editing the grid, alphabet, and transformations.
/// Handles visualization, user input, and file operations.
class editor
{
public:
    /// @brief Constructor
    /// Initializes the editor with a default grid and some sample transformations.
    editor();

    /// @brief Destructor
    ~editor() = default;

    /// @brief Main edit function
    /// Renders all editor components and processes user input.
    void edit();

private:
    // UI components
    /// @brief Edit the alphabet (symbols)
    void edit_alphabet();

    /// @brief Edit the transformation stack (add, remove, reorder transformations)
    void edit_transformation_stack();

    /// @brief Edit the currently selected transformation
    void edit_selected_transformation();

    /// @brief Edit and visualize the synthesizer
    void edit_synthesizer();

    // Specific transformation editors
    /// @brief Edit a random transformation
    /// @param transform Pointer to the random transformation to edit
    void edit_random_transformation(random_transformation* transform);

    /// @brief Edit a rule-based transformation
    /// @param transform Pointer to the rule-based transformation to edit
    void edit_rule_based_transformation(rule_based_transformation* transform);

    // File operations
    /// @brief Save grid synth to a JSON file
    /// @param filename Path to the file
    /// @return True if save was successful, false otherwise
    bool save_to_file(const std::string& filename);

    /// @brief Load grid synth from a JSON file
    /// @param filename Path to the file
    /// @return True if load was successful, false otherwise
    bool load_from_file(const std::string& filename);

    /// @brief Show file dialog
    /// @param isSave True for save dialog, false for load dialog
    void show_file_dialog(bool isSave);

    // Core synthesizer data
    /// @brief The main grid synthesis object
    grid_synth m_synth;

    // UI state for transformations
    /// @brief Index of the currently selected transformation
    int m_selected_transform_index = -1;

    /// @brief List of transformation indices to remove on next update
    std::vector<int> m_transforms_to_remove;

    /// @brief List of symbol IDs to remove on next update
    std::vector<int> m_symbols_to_remove;

    // Pattern editing state
    /// @brief Whether we're currently editing a pattern
    bool m_editing_pattern = false;

    /// @brief Whether we're editing a search pattern (true) or replacement pattern (false)
    bool m_editing_search = true;

    /// @brief Working copy of the pattern being edited
    grid m_pattern_grid;

    /// @brief Copy of the search pattern
    grid m_search_pattern;

    /// @brief Copy of the replacement pattern
    grid m_replacement_pattern;

    /// @brief Currently selected symbol ID for pattern editing
    int m_selected_symbol_id = alphabet::wildcard_symbol.id;

    // Temporary reference to the current rule being edited
    rule_based_transformation* m_current_rule = nullptr;

    // File dialog
    /// @brief Whether to show the save dialog
    bool m_show_save_dialog = false;

    /// @brief Whether to show the load dialog
    bool m_show_load_dialog = false;

    /// @brief Buffer for file name input
    char m_filename[256] = "";
};

}