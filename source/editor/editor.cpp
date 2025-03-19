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
}

editor::editor() : m_synth(16, 16, alphabet::empty_symbol.id)
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


void editor::update()
{
    ImGui::Begin("Editor");

    if(ImGui::Button("Synthesize"))
    {
        m_synth.synthesize();
    }


    auto& grid = m_synth.get_grid();

    // Display the grid in the editor with small squares for each cell using a draw list
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float square_size = 2.0f;

    for(int y = 0; y < grid.height(); ++y)
    {
        for(int x = 0; x < grid.width(); ++x)
        {
            ImVec2 top_left = ImVec2(p.x + x * square_size, p.y + y * square_size);
            ImVec2 bottom_right = ImVec2(top_left.x + square_size, top_left.y + square_size);
            ImColor color = nice_color(grid(x, y));
            draw_list->AddRectFilled(top_left, bottom_right, color);
        }
    }

    ImGui::End();
}

