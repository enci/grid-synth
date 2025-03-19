#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <memory>

namespace gs
{

////////////////////////////////////////////////////////////////////////////////
////                                grid
////////////////////////////////////////////////////////////////////////////////
class grid
{
public:
    explicit grid(int width = 10, int height = 10, int default_value = 0);
    ~grid() = default;
    int get(int x, int y) const { return m_data[y * m_width + x]; }
    void set(int x, int y, int value) { m_data[y * m_width + x] = value; }
    bool in_bounds(int x, int y) const { return x >= 0 && x < m_width && y >= 0 && y < m_height; }
    int& operator()(int x, int y) { return m_data[y * m_width + x]; }
    int operator()(int x, int y) const { return m_data[y * m_width + x]; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    void resize(int new_width, int new_height, int default_value = 0);
    void clear(int value = 0);
private:
    int m_width;
    int m_height;
    std::vector<int> m_data;
};


////////////////////////////////////////////////////////////////////////////////
////                                symbol
////////////////////////////////////////////////////////////////////////////////
struct symbol
{
    int id;
    std::string name;
};

////////////////////////////////////////////////////////////////////////////////
////                                alphabet
////////////////////////////////////////////////////////////////////////////////
class alphabet
{
public:
    alphabet();
    ~alphabet() = default;
    void add_symbol(const symbol& s);
    bool has_symbol(int id) const { return m_symbols.find(id) != m_symbols.end(); }
    void remove_symbol(int id) { m_symbols.erase(id); }
    const symbol& get_symbol(int id) const { return m_symbols.at(id); }
    const std::vector<symbol>& get_symbols() const;
    const static symbol empty_symbol;
    const static symbol wildcard_symbol;
private:
    std::map<int, symbol> m_symbols;
};


////////////////////////////////////////////////////////////////////////////////
////                            transformation
////////////////////////////////////////////////////////////////////////////////
class transformation
{
public:
    transformation(std::string name, std::shared_ptr<alphabet> alphabet)
    : m_name(std::move(name)), m_alphabet(std::move(alphabet)) {}
    virtual ~transformation() = default;
    virtual void apply(grid& g) = 0;
    const std::string& name() const { return m_name; }
    bool enabled() const { return m_enabled; }
    void set_enabled(bool enabled) { m_enabled = enabled; }
protected:
    std::string m_name;
    bool m_enabled = true;
    std::shared_ptr<alphabet> m_alphabet;
};

////////////////////////////////////////////////////////////////////////////////
////                        random_transformation
////////////////////////////////////////////////////////////////////////////////
class random_transformation : public transformation
{
public:
    explicit random_transformation(std::string name, std::shared_ptr<alphabet> alphabet)
    : transformation(std::move(name), std::move(alphabet)) {}
    ~random_transformation() override = default;
    void apply(grid& g) override;
};

////////////////////////////////////////////////////////////////////////////////
////                    rule_based_transformation
////////////////////////////////////////////////////////////////////////////////
class rule_based_transformation : public transformation
{
public:
    explicit rule_based_transformation(std::string name, std::shared_ptr<alphabet> alphabet)
    : transformation(std::move(name), std::move(alphabet)) {}
    ~rule_based_transformation() override = default;
    void set_search(const grid& search) { m_search = search; }
    void add_replacement(float probability, const grid& replacement)
    { m_replacement.push_back({probability, replacement}); }
    void apply(grid &g) override;
private:
    struct replacement_entry {
        float probability;
        grid replacement;
    };
    grid m_search;
    std::vector<replacement_entry> m_replacement;
};

////////////////////////////////////////////////////////////////////////////////
////                            grid_synth
////////////////////////////////////////////////////////////////////////////////
class grid_synth
{
public:
    grid_synth(int width = 32, int height = 32, int default_value = 0)
        : m_grid(width, height, default_value) {}
    // grid_synth() = default;
    ~grid_synth() = default;
    std::shared_ptr<alphabet> get_alphabet() { return m_alphabet; }
    grid& get_grid() { return m_grid; }
    void add_transformation(std::unique_ptr<transformation> t)
    { m_transformations.push_back(std::move(t)); }
    const std::vector<std::unique_ptr<transformation>>& get_transformations() const
    { return m_transformations; }
    void synthesize();
protected:
    grid m_grid;
    std::shared_ptr<alphabet> m_alphabet = std::make_shared<alphabet>();
    std::vector<std::unique_ptr<transformation>> m_transformations;
};


}

