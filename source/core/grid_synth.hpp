#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <memory>
#include <fstream>
#include <nlohmann/json.hpp>

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

        // For serialization
        const std::vector<int>& data() const { return m_data; }

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

        // For serialization
        const std::map<int, symbol>& symbols() const { return m_symbols; }

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
        void set_name(const std::string& name) { m_name = name; }
        bool enabled() const { return m_enabled; }
        void set_enabled(bool enabled) { m_enabled = enabled; }

        // Type identification for serialization
        enum class Type {
            RANDOM,
            RULE_BASED
        };

        virtual Type type() const = 0;

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

        // Type identification
        Type type() const override { return Type::RANDOM; }
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

        // Pattern management
        void set_search(const grid& search) { m_search = search; }
        const grid& get_search() const { return m_search; }

        // Replacement management
        void add_replacement(float probability, const grid& replacement)
        { m_replacement.push_back({probability, replacement}); }

        // Get replacement patterns
        int replacement_count() const { return m_replacement.size(); }

        // Get specific replacement
        float get_replacement_probability(int index) const {
            return (index >= 0 && index < (int)m_replacement.size()) ?
                   m_replacement[index].probability : 0.0f;
        }

        const grid& get_replacement_grid(int index) const {
            static grid empty_grid;
            return (index >= 0 && index < (int)m_replacement.size()) ?
                   m_replacement[index].replacement : empty_grid;
        }

        // Clear all replacements
        void clear_replacements() { m_replacement.clear(); }

        // Update an existing replacement
        void update_replacement(int index, float probability, const grid& replacement) {
            if (index >= 0 && index < (int)m_replacement.size()) {
                m_replacement[index].probability = probability;
                m_replacement[index].replacement = replacement;
            }
        }

        void apply(grid &g) override;

        // Type identification
        Type type() const override { return Type::RULE_BASED; }

        // For serialization
        struct replacement_entry {
            float probability;
            grid replacement;
        };
        const std::vector<replacement_entry>& replacements() const { return m_replacement; }

    private:
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
        ~grid_synth() = default;
        std::shared_ptr<alphabet> get_alphabet() { return m_alphabet; }
        grid& get_grid() { return m_grid; }
        void add_transformation(std::unique_ptr<transformation> t)
        { m_transformations.push_back(std::move(t)); }
        const std::vector<std::unique_ptr<transformation>>& get_transformations() const
        { return m_transformations; }
        std::vector<std::unique_ptr<transformation>>& get_transformations()
        { return m_transformations; }
        void synthesize();

        // JSON serialization - separated from file I/O
        nlohmann::json to_json() const;
        static grid_synth from_json(const nlohmann::json& j);

    protected:
        grid m_grid;
        std::shared_ptr<alphabet> m_alphabet = std::make_shared<alphabet>();
        std::vector<std::unique_ptr<transformation>> m_transformations;
    };

}