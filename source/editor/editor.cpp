#include <algorithm>
#include "editor.hpp"
#include "imgui.h"

using namespace gs;
using namespace std;

namespace
{
    ImColor nice_color(int idx)
    {
        float hue = (float)idx / 16.0f;
        return ImColor::HSV(hue, 0.8f, 0.6f);
    }

    // Helper function to ensure no zero dimensions for ImGui elements
    ImVec2 safe_size(float width, float height, float min_size = 1.0f)
    {
        return ImVec2(
                std::max(min_size, width),
                std::max(min_size, height)
        );
    }
}

editor::editor() : m_synth(16, 16, alphabet::empty_symbol.id),
                   m_pattern_grid(3, 3, alphabet::wildcard_symbol.id),
                   m_search_pattern(3, 3, alphabet::wildcard_symbol.id),
                   m_replacement_pattern(3, 3, alphabet::wildcard_symbol.id)
{
    // Add some symbols to the alphabet
    m_synth.get_alphabet()->add_symbol({1, "F"});
    m_synth.get_alphabet()->add_symbol({2, "G"});

    // Add some transformations to the synthesizer
    m_synth.add_transformation(make_unique<random_transformation>("Random", m_synth.get_alphabet()));

    // Add a rule-based transformation to the synthesizer
    unique_ptr<transformation> rule = make_unique<rule_based_transformation>("Rule-based", m_synth.get_alphabet());
    auto search = grid(3, 3, alphabet::empty_symbol.id);
    search(0, 0) = alphabet::wildcard_symbol.id;
    search(0, 1) = 1;
    search(0, 2) = alphabet::wildcard_symbol.id;
    search(1, 0) = 1;
    search(1, 1) = 2;
    search(1, 2) = 1;
    search(2, 0) = alphabet::wildcard_symbol.id;
    search(2, 1) = 1;
    search(2, 2) = alphabet::wildcard_symbol.id;
    auto replacement = grid(3,3, alphabet::empty_symbol.id);
    replacement(0, 0) = alphabet::wildcard_symbol.id;
    replacement(0, 1) = 1;
    replacement(0, 2) = alphabet::wildcard_symbol.id;
    replacement(1, 0) = 1;
    replacement(1, 1) = 1;
    replacement(1, 2) = 1;
    replacement(2, 0) = alphabet::wildcard_symbol.id;
    replacement(2, 1) = 1;
    replacement(2, 2) = alphabet::wildcard_symbol.id;
    rule_based_transformation* rule_ptr = dynamic_cast<rule_based_transformation*>(rule.get());
    rule_ptr->set_search(search);
    rule_ptr->add_replacement(1.0f, replacement);
    m_synth.add_transformation(move(rule));
}

void editor::edit()
{
    edit_alphabet();
    edit_transformation_stack();
    edit_selected_transformation();
    edit_synthesizer();
}

void editor::edit_alphabet()
{
    ImGui::Begin("Alphabet");
    auto alphabet_ptr = m_synth.get_alphabet(); // Removed reference since it's already a shared_ptr

    // Display existing symbols
    ImGui::Text("Symbols");
    ImGui::Separator();

    // Table with symbol ID and name
    if (ImGui::BeginTable("##SymbolTable", 3)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableHeadersRow();

        const auto& symbols = alphabet_ptr->get_symbols();
        for (const auto& symbol : symbols) {
            ImGui::TableNextRow();

            // ID Column
            ImGui::TableNextColumn();
            ImGui::Text("%d", symbol.id);

            // Name Column
            ImGui::TableNextColumn();
            ImGui::Text("%s", symbol.name.c_str());

            // Actions Column
            ImGui::TableNextColumn();
            ImGui::PushID(symbol.id);
            if (ImGui::Button("Remove")) {
                m_symbols_to_remove.push_back(symbol.id);
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    // Process removals
    for (int id : m_symbols_to_remove) {
        alphabet_ptr->remove_symbol(id);
    }
    m_symbols_to_remove.clear();

    // Add new symbol
    ImGui::Separator();
    ImGui::Text("Add New Symbol");

    static int new_id = 3; // Start from 3 since 0, 1, 2 might be reserved
    static char new_name[64] = "";

    ImGui::InputInt("ID", &new_id);
    ImGui::InputText("Name", new_name, 64);

    // Use explicit button size only for full-width buttons
    if (ImGui::Button("Add Symbol", ImVec2(-1, 24))) {
        if (new_id != alphabet::empty_symbol.id &&
            new_id != alphabet::wildcard_symbol.id &&
            strlen(new_name) > 0) {
            alphabet_ptr->add_symbol({new_id, new_name});
            new_id++;
            new_name[0] = '\0';
        }
    }

    ImGui::End();
}

void editor::edit_transformation_stack()
{
    ImGui::Begin("Transformation Stack");

    auto& transformations = m_synth.get_transformations();

    // Table with transformations
    if (ImGui::BeginTable("##TransformationTable", 4)) {
        ImGui::TableSetupColumn("##Enabled");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Actions");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < transformations.size(); i++) {
            auto& transform = transformations[i];

            ImGui::TableNextRow();

            // Enabled Column
            ImGui::TableNextColumn();
            bool enabled = transform->enabled();
            if (ImGui::Checkbox(("##enabled" + std::to_string(i)).c_str(), &enabled)) {
                transform->set_enabled(enabled);
            }

            // Name Column
            ImGui::TableNextColumn();
            ImGui::Text("%s", transform->name().c_str());

            // Type Column
            ImGui::TableNextColumn();
            if (dynamic_cast<random_transformation*>(transform.get())) {
                ImGui::Text("Random");
            } else if (dynamic_cast<rule_based_transformation*>(transform.get())) {
                ImGui::Text("Rule-based");
            } else {
                ImGui::Text("Unknown");
            }

            // Actions Column
            ImGui::TableNextColumn();
            ImGui::PushID((int)i);

            if (ImGui::Button("Select")) {
                m_selected_transform_index = (int)i;
            }
            ImGui::SameLine();

            // Move up button (disabled for first element)
            if (i > 0) {
                if (ImGui::Button("↑")) {
                    // Swap with previous element
                    std::swap(transformations[i], transformations[i-1]);
                    if (m_selected_transform_index == (int)i) {
                        m_selected_transform_index = (int)i - 1;
                    } else if (m_selected_transform_index == (int)i - 1) {
                        m_selected_transform_index = (int)i;
                    }
                }
                ImGui::SameLine();
            } else {
                ImGui::InvisibleButton("##placeholder", ImVec2(23, 19)); // Use minimum size for invisible buttons
                ImGui::SameLine();
            }

            // Move down button (disabled for last element)
            if (i < transformations.size() - 1) {
                if (ImGui::Button("↓")) {
                    // Swap with next element
                    std::swap(transformations[i], transformations[i+1]);
                    if (m_selected_transform_index == (int)i) {
                        m_selected_transform_index = (int)i + 1;
                    } else if (m_selected_transform_index == (int)i + 1) {
                        m_selected_transform_index = (int)i;
                    }
                }
                ImGui::SameLine();
            } else {
                ImGui::InvisibleButton("##placeholder2", ImVec2(23, 19)); // Use minimum size for invisible buttons
                ImGui::SameLine();
            }

            if (ImGui::Button("Remove")) {
                m_transforms_to_remove.push_back((int)i);
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    // Process removals (in reverse order to maintain indices)
    std::sort(m_transforms_to_remove.begin(), m_transforms_to_remove.end(), std::greater<int>());
    for (int idx : m_transforms_to_remove) {
        transformations.erase(transformations.begin() + idx);
        if (m_selected_transform_index == idx) {
            m_selected_transform_index = -1;
        } else if (m_selected_transform_index > idx) {
            m_selected_transform_index--;
        }
    }
    m_transforms_to_remove.clear();

    // Add new transformation
    ImGui::Separator();
    ImGui::Text("Add New Transformation");

    static int transform_type = 0;
    static char transform_name[64] = "";

    const char* types[] = { "Random", "Rule-based" };
    ImGui::Combo("Type", &transform_type, types, IM_ARRAYSIZE(types));
    ImGui::InputText("Name", transform_name, 64);

    if (ImGui::Button("Add Transformation")) {
        if (strlen(transform_name) > 0) {
            if (transform_type == 0) { // Random
                m_synth.add_transformation(make_unique<random_transformation>(transform_name, m_synth.get_alphabet()));
            } else { // Rule-based
                unique_ptr<transformation> rule = make_unique<rule_based_transformation>(transform_name, m_synth.get_alphabet());
                auto* rule_ptr = dynamic_cast<rule_based_transformation*>(rule.get());

                // Initialize with default patterns
                auto search = grid(3, 3, alphabet::wildcard_symbol.id);
                auto replacement = grid(3, 3, alphabet::wildcard_symbol.id);
                rule_ptr->set_search(search);
                rule_ptr->add_replacement(1.0f, replacement);

                m_synth.add_transformation(move(rule));
            }
            transform_name[0] = '\0';
            m_selected_transform_index = (int)transformations.size() - 1;
        }
    }

    ImGui::End();
}

void editor::edit_selected_transformation()
{
    ImGui::Begin("Transformation Editor");

    auto& transformations = m_synth.get_transformations();

    if (m_selected_transform_index >= 0 && m_selected_transform_index < (int)transformations.size()) {
        auto& transform = transformations[m_selected_transform_index];

        // Display name and allow editing
        static char name_buffer[64];
        strncpy(name_buffer, transform->name().c_str(), sizeof(name_buffer));
        if (ImGui::InputText("Name", name_buffer, sizeof(name_buffer))) {
            // Update the name using our new setter
            std::string new_name = name_buffer;
            if (!new_name.empty()) {
                transform->set_name(new_name);
            }
        }

        // Display transformation specific settings
        if (auto* random_transform = dynamic_cast<random_transformation*>(transform.get())) {
            edit_random_transformation(random_transform);
        } else if (auto* rule_transform = dynamic_cast<rule_based_transformation*>(transform.get())) {
            edit_rule_based_transformation(rule_transform);
        }
    } else {
        ImGui::TextDisabled("No transformation selected");
    }

    ImGui::End();
}

void editor::edit_random_transformation(random_transformation* transform)
{
    ImGui::Text("Random Transformation");
    ImGui::Text("This transformation fills the grid with random symbols from the alphabet.");

    // No specific settings for random transformation
}

void editor::edit_rule_based_transformation(rule_based_transformation* transform)
{
    // Store reference to current rule for pattern editing
    m_current_rule = transform;

    ImGui::Text("Rule-based Transformation");

    // Button to edit search pattern
    if (ImGui::Button("Edit Search Pattern")) {
        m_editing_pattern = true;
        m_editing_search = true;

        // Get the current search pattern using our new getter
        m_pattern_grid = transform->get_search();
    }

    // Button to edit replacement pattern
    if (ImGui::Button("Edit Replacement Pattern")) {
        m_editing_pattern = true;
        m_editing_search = false;

        // Get the first replacement pattern if one exists
        if (transform->replacement_count() > 0) {
            m_pattern_grid = transform->get_replacement_grid(0);
        } else {
            // No replacement patterns yet, initialize with same size as search
            const grid& search = transform->get_search();
            m_pattern_grid = grid(search.width(), search.height(), alphabet::wildcard_symbol.id);
        }
    }

    // Pattern Editor Popup
    if (m_editing_pattern) {
        ImGui::OpenPopup("Pattern Editor");

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Pattern Editor", &m_editing_pattern, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Editing %s Pattern", m_editing_search ? "Search" : "Replacement");

            // Pattern dimensions
            static int pattern_width = 3;
            static int pattern_height = 3;

            if (ImGui::SliderInt("Width", &pattern_width, 1, 8)) {
                // Resize the pattern grid
                grid new_grid(pattern_width, pattern_height, alphabet::wildcard_symbol.id);

                // Copy existing values where possible
                for (int y = 0; y < std::min(m_pattern_grid.height(), pattern_height); y++) {
                    for (int x = 0; x < std::min(m_pattern_grid.width(), pattern_width); x++) {
                        new_grid(x, y) = m_pattern_grid(x, y);
                    }
                }

                m_pattern_grid = new_grid;
            }

            if (ImGui::SliderInt("Height", &pattern_height, 1, 8)) {
                // Resize the pattern grid
                grid new_grid(pattern_width, pattern_height, alphabet::wildcard_symbol.id);

                // Copy existing values where possible
                for (int y = 0; y < std::min(m_pattern_grid.height(), pattern_height); y++) {
                    for (int x = 0; x < std::min(m_pattern_grid.width(), pattern_width); x++) {
                        new_grid(x, y) = m_pattern_grid(x, y);
                    }
                }

                m_pattern_grid = new_grid;
            }

            ImGui::Separator();

            // Symbol selector
            ImGui::Text("Select Symbol");
            auto alphabet_ptr = m_synth.get_alphabet(); // Removed reference

            // Wildcard & Empty symbols
            if (ImGui::Button("Wildcard")) {
                m_selected_symbol_id = alphabet::wildcard_symbol.id;
            }
            ImGui::SameLine();

            if (ImGui::Button("Empty")) {
                m_selected_symbol_id = alphabet::empty_symbol.id;
            }

            // Other symbols
            const auto& symbols = alphabet_ptr->get_symbols();
            for (const auto& symbol : symbols) {
                ImGui::SameLine();
                if (ImGui::Button(symbol.name.c_str())) {
                    m_selected_symbol_id = symbol.id;
                }
            }

            ImGui::Text("Selected: %d", m_selected_symbol_id);
            ImGui::Separator();

            // Grid editor
            ImGui::Text("Pattern Grid");

            float cell_size = 30.0f;
            float grid_width = m_pattern_grid.width() * cell_size;
            float grid_height = m_pattern_grid.height() * cell_size;

            ImVec2 cursor = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            // Draw grid background
            draw_list->AddRectFilled(cursor,
                                     ImVec2(cursor.x + grid_width, cursor.y + grid_height),
                                     IM_COL32(50, 50, 50, 255));

            // Draw grid cells
            for (int y = 0; y < m_pattern_grid.height(); y++) {
                for (int x = 0; x < m_pattern_grid.width(); x++) {
                    ImVec2 cell_min(cursor.x + x * cell_size, cursor.y + y * cell_size);
                    ImVec2 cell_max(cell_min.x + cell_size, cell_min.y + cell_size);

                    int symbol_id = m_pattern_grid(x, y);
                    ImColor cell_color = nice_color(symbol_id);

                    // Draw cell
                    draw_list->AddRectFilled(cell_min, cell_max, cell_color);
                    draw_list->AddRect(cell_min, cell_max, IM_COL32(200, 200, 200, 255));

                    // Draw symbol text in cell
                    std::string symbol_text;
                    if (symbol_id == alphabet::wildcard_symbol.id) {
                        symbol_text = "*";
                    } else if (symbol_id == alphabet::empty_symbol.id) {
                        symbol_text = " ";
                    } else if (alphabet_ptr->has_symbol(symbol_id)) {
                        symbol_text = alphabet_ptr->get_symbol(symbol_id).name;
                    } else {
                        symbol_text = "?";
                    }

                    ImVec2 text_size = ImGui::CalcTextSize(symbol_text.c_str());
                    ImVec2 text_pos(
                            cell_min.x + (cell_size - text_size.x) * 0.5f,
                            cell_min.y + (cell_size - text_size.y) * 0.5f
                    );

                    draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), symbol_text.c_str());

                    // Handle cell clicks
                    if (ImGui::IsMouseHoveringRect(cell_min, cell_max) && ImGui::IsMouseClicked(0)) {
                        m_pattern_grid(x, y) = m_selected_symbol_id;
                    }
                }
            }

            // Add spacing for the grid - ensure minimum dimensions
            ImGui::InvisibleButton("##grid", ImVec2(
                    std::max(1.0f, grid_width),
                    std::max(1.0f, grid_height)
            ));

            ImGui::Separator();

            // Apply changes button
            if (ImGui::Button("Apply")) {
                // Apply the pattern to the transformation
                if (m_current_rule) {
                    if (m_editing_search) {
                        m_current_rule->set_search(m_pattern_grid);
                        // Store a copy for next time
                        m_search_pattern = m_pattern_grid;
                    } else {
                        // Check if we need to update or add a replacement pattern
                        if (m_current_rule->replacement_count() > 0) {
                            // Update the first replacement
                            m_current_rule->update_replacement(0, 1.0f, m_pattern_grid);
                        } else {
                            // Add a new replacement
                            m_current_rule->add_replacement(1.0f, m_pattern_grid);
                        }
                        // Store a copy for next time
                        m_replacement_pattern = m_pattern_grid;
                    }
                }
                m_editing_pattern = false;
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel")) {
                m_editing_pattern = false;
            }

            ImGui::EndPopup();
        }
    }
}

void editor::edit_synthesizer()
{
    ImGui::Begin("Synthesizer");

    if (ImGui::Button("Synthesize", ImVec2(100, 24))) {
        m_synth.synthesize();
    }

    ImGui::SameLine();

    // Grid size controls
    static int grid_width = m_synth.get_grid().width();
    static int grid_height = m_synth.get_grid().height();

    ImGui::PushItemWidth(60);
    if (ImGui::InputInt("Width", &grid_width)) {
        if (grid_width < 1) grid_width = 1;
        if (grid_width > 256) grid_width = 256;
    }

    ImGui::SameLine();

    if (ImGui::InputInt("Height", &grid_height)) {
        if (grid_height < 1) grid_height = 1;
        if (grid_height > 256) grid_height = 256;
    }
    ImGui::PopItemWidth();

    ImGui::SameLine();

    if (ImGui::Button("Resize", ImVec2(60, 24))) {
        m_synth.get_grid().resize(grid_width, grid_height);
    }

    ImGui::SameLine();

    if (ImGui::Button("Clear", ImVec2(60, 24))) {
        m_synth.get_grid().clear(alphabet::empty_symbol.id);
    }

    // Visualize the grid
    auto& grid = m_synth.get_grid();
    auto alphabet_ptr = m_synth.get_alphabet(); // Removed reference

    // Calculate grid visualization parameters
    float available_width = ImGui::GetContentRegionAvail().x;
    float available_height = ImGui::GetContentRegionAvail().y;

    // Ensure we're not dividing by zero
    int safe_grid_width = std::max(1, grid.width());
    int safe_grid_height = std::max(1, grid.height());

    float cell_size = std::min(
            available_width / safe_grid_width,
            available_height / safe_grid_height
    );

    cell_size = std::max(4.0f, std::min(32.0f, cell_size));

    float grid_width_px = safe_grid_width * cell_size;
    float grid_height_px = safe_grid_height * cell_size;

    // Draw the grid
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size(grid_width_px, grid_height_px);

    // Background
    draw_list->AddRectFilled(
            canvas_pos,
            ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
            IM_COL32(50, 50, 50, 255)
    );

    // Grid cells
    for (int y = 0; y < grid.height(); ++y) {
        for (int x = 0; x < grid.width(); ++x) {
            ImVec2 cell_min = ImVec2(
                    canvas_pos.x + x * cell_size,
                    canvas_pos.y + y * cell_size
            );
            ImVec2 cell_max = ImVec2(
                    cell_min.x + cell_size,
                    cell_min.y + cell_size
            );

            int symbol_id = grid(x, y);
            ImColor color = nice_color(symbol_id);

            draw_list->AddRectFilled(cell_min, cell_max, color);

            // Add symbol text if cells are large enough
            if (cell_size >= 12.0f) {
                std::string symbol_text;
                if (symbol_id == alphabet::wildcard_symbol.id) {
                    symbol_text = "*";
                } else if (symbol_id == alphabet::empty_symbol.id) {
                    symbol_text = " ";
                } else if (alphabet_ptr->has_symbol(symbol_id)) {
                    symbol_text = alphabet_ptr->get_symbol(symbol_id).name;
                } else {
                    symbol_text = "?";
                }

                ImVec2 text_size = ImGui::CalcTextSize(symbol_text.c_str());
                ImVec2 text_pos(
                        cell_min.x + (cell_size - text_size.x) * 0.5f,
                        cell_min.y + (cell_size - text_size.y) * 0.5f
                );

                draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), symbol_text.c_str());
            }
        }
    }

    // Allow interaction with the grid canvas
    // Make sure the canvas size is never zero
    ImVec2 safe_canvas_size = ImVec2(
            std::max(1.0f, canvas_size.x),
            std::max(1.0f, canvas_size.y)
    );
    ImGui::InvisibleButton("canvas", safe_canvas_size);

    // Handle grid cell clicks (optional)
    if (ImGui::IsItemHovered() && ImGui::IsMouseDown(0)) {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        int grid_x = (int)((mouse_pos.x - canvas_pos.x) / cell_size);
        int grid_y = (int)((mouse_pos.y - canvas_pos.y) / cell_size);

        if (grid.in_bounds(grid_x, grid_y)) {
            // Set cell to currently selected symbol
            grid(grid_x, grid_y) = m_selected_symbol_id;
        }
    }

    ImGui::End();
}

