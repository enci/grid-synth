#include <random>
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include "grid_synth.hpp"

using namespace gs;

////////////////////////////////////////////////////////////////////////////////
////                                grid
////////////////////////////////////////////////////////////////////////////////

grid::grid(int width, int height, int default_value)
{
    resize(width, height, default_value);
}

void grid::resize(int new_width, int new_height, int default_value)
{
    m_width = new_width;
    m_height = new_height;
    m_data.resize(m_width * m_height, default_value);
}

void grid::clear(int value)
{
    std::fill(m_data.begin(), m_data.end(), value);
}

////////////////////////////////////////////////////////////////////////////////
////                            alphabet
////////////////////////////////////////////////////////////////////////////////

const symbol alphabet::empty_symbol = {0, "empty"};
const symbol alphabet::wildcard_symbol = {-1, "wildcard"};

alphabet::alphabet()
{
    // add_symbol(empty_symbol);
    // add_symbol(wildcard_symbol);
}

const std::vector<symbol>& alphabet::get_symbols() const
{
    static std::vector<symbol> symbols;
    symbols.clear();
    for (const auto& [id, s] : m_symbols)
        symbols.push_back(s);
    return symbols;
}

void alphabet::add_symbol(const symbol &s)
{
    if(m_symbols.find(s.id) == m_symbols.end())
        m_symbols[s.id] = s;
}

////////////////////////////////////////////////////////////////////////////////
////                    rule_based_transformation
////////////////////////////////////////////////////////////////////////////////
void rule_based_transformation::apply(grid &g)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    for(int i = 0; i < g.width(); ++i) {
        for (int j = 0; j < g.height(); ++j) {
            if (!g.in_bounds(i + m_search.width() - 1, j + m_search.height() - 1))
                continue;

            bool match = true;
            for (int x = 0; x < m_search.width(); ++x) {
                for (int y = 0; y < m_search.height(); ++y) {
                    if (m_search(x, y) == alphabet::wildcard_symbol.id)
                        continue;
                    if (g(i + x, j + y) != m_search(x, y)) {
                        match = false;
                        break;
                    }
                }
            }

            if (match) {
                float r = dis(gen);
                float acc = 0.0f;
                for (const auto &[p, replacement]: m_replacement) {
                    acc += p;
                    if (r <= acc) {
                        for (int x = 0; x < replacement.width(); ++x)
                            for (int y = 0; y < replacement.height(); ++y) {
                                if (replacement(x, y) == alphabet::wildcard_symbol.id)
                                    continue;
                                g(i + x, j + y) = replacement(x, y);
                            }
                        break;
                    }
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
////                         random_transformation
////////////////////////////////////////////////////////////////////////////////
void random_transformation::apply(grid &g)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, m_alphabet->get_symbols().size() - 1);

    for(int i = 0; i < g.width(); ++i)
        for(int j = 0; j < g.height(); ++j)
            g(i, j) = m_alphabet->get_symbols()[dis(gen)].id;
}

////////////////////////////////////////////////////////////////////////////////
////                             grid_synth
////////////////////////////////////////////////////////////////////////////////
void grid_synth::synthesize()
{
    for(auto& t : m_transformations)
    {
        if (t->enabled())
        {
            t->apply(m_grid);
        }
    }
}

/*
nlohmann::json grid_synth::to_json() const
{
    nlohmann::json j;

    // Store version info
    j["version"] = 1;

    // Serialize grid
    j["grid"] = {
            {"width", m_grid.width()},
            {"height", m_grid.height()},
            {"data", m_grid.data()}
    };

    // Serialize alphabet
    j["alphabet"] = {{"symbols", nlohmann::json::array()}};
    for (const auto& [id, s] : m_alphabet->symbols()) {
        j["alphabet"]["symbols"].push_back({
                                                   {"id", s.id},
                                                   {"name", s.name}
                                           });
    }

    // Serialize transformations
    j["transformations"] = nlohmann::json::array();
    for (const auto& t : m_transformations) {
        nlohmann::json t_json;

        // Common transformation properties
        t_json["name"] = t->name();
        t_json["enabled"] = t->enabled();

        if (t->type() == transformation::Type::RANDOM) {
            t_json["type"] = "random";
        }
        else if (t->type() == transformation::Type::RULE_BASED) {
            auto* rule_t = static_cast<const rule_based_transformation*>(t.get());
            t_json["type"] = "rule_based";

            // Serialize search pattern
            const grid& search = rule_t->get_search();
            t_json["search"] = {
                    {"width", search.width()},
                    {"height", search.height()},
                    {"data", search.data()}
            };

            // Serialize replacements
            t_json["replacements"] = nlohmann::json::array();
            for (const auto& repl : rule_t->replacements()) {
                const grid& repl_grid = repl.replacement;
                t_json["replacements"].push_back({
                                                         {"probability", repl.probability},
                                                         {"grid", {
                                                                                 {"width", repl_grid.width()},
                                                                                 {"height", repl_grid.height()},
                                                                                 {"data", repl_grid.data()}
                                                                         }}
                                                 });
            }
        }

        j["transformations"].push_back(t_json);
    }

    return j;
}

grid_synth grid_synth::from_json(const nlohmann::json& j)
{
    try {
        // Check version
        int version = j["version"];
        if (version != 1) {
            throw std::runtime_error("Unsupported grid_synth file version: " + std::to_string(version));
        }

        // Parse grid
        int grid_width = j["grid"]["width"];
        int grid_height = j["grid"]["height"];
        std::vector<int> grid_data = j["grid"]["data"];

        // Create grid_synth with correct dimensions
        grid_synth synth(grid_width, grid_height);
        for (int i = 0; i < grid_width * grid_height; ++i) {
            if (i < (int)grid_data.size()) {
                synth.m_grid.data()[i] = grid_data[i];
            }
        }

        // Parse alphabet
        for (const auto& symbol_json : j["alphabet"]["symbols"]) {
            symbol s;
            s.id = symbol_json["id"];
            s.name = symbol_json["name"];
            synth.m_alphabet->add_symbol(s);
        }

        // Parse transformations
        for (const auto& t_json : j["transformations"]) {
            std::string type = t_json["type"];
            std::string name = t_json["name"];
            bool enabled = t_json["enabled"];

            if (type == "random") {
                auto t = std::make_unique<random_transformation>(name, synth.m_alphabet);
                t->set_enabled(enabled);
                synth.add_transformation(std::move(t));
            }
            else if (type == "rule_based") {
                auto t = std::make_unique<rule_based_transformation>(name, synth.m_alphabet);
                t->set_enabled(enabled);

                // Parse search pattern
                int search_width = t_json["search"]["width"];
                int search_height = t_json["search"]["height"];
                std::vector<int> search_data = t_json["search"]["data"];

                grid search_grid(search_width, search_height);
                for (int i = 0; i < search_width * search_height; ++i) {
                    if (i < (int)search_data.size()) {
                        search_grid.data()[i] = search_data[i];
                    }
                }
                t->set_search(search_grid);

                // Parse replacements
                for (const auto& repl_json : t_json["replacements"]) {
                    float probability = repl_json["probability"];

                    int repl_width = repl_json["grid"]["width"];
                    int repl_height = repl_json["grid"]["height"];
                    std::vector<int> repl_data = repl_json["grid"]["data"];

                    grid repl_grid(repl_width, repl_height);
                    for (int i = 0; i < repl_width * repl_height; ++i) {
                        if (i < (int)repl_data.size()) {
                            repl_grid.data()[i] = repl_data[i];
                        }
                    }

                    t->add_replacement(probability, repl_grid);
                }

                synth.add_transformation(std::move(t));
            }
        }

        return synth;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse grid_synth from JSON: " + std::string(e.what()));
    }
}
 */