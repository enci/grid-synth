#include <random>
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
