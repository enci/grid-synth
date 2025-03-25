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
/// @brief A 2D grid of integer values
///
/// The grid class represents a rectangular grid of integer values.
/// It provides methods for accessing and modifying grid cells, resizing,
/// and clearing the grid. This is the fundamental data structure for
/// representing levels and patterns in the grid synthesis system.
class grid
{
public:
    /// @brief Constructs a grid with the specified dimensions and default value
    /// @param width The width of the grid
    /// @param height The height of the grid
    /// @param default_value The default value to fill the grid with
    explicit grid(int width = 10, int height = 10, int default_value = 0);

    /// @brief Destructor
    ~grid() = default;

    /// @brief Copy constructor
    /// @param other The grid to copy from
    grid(const grid& other)
        : m_width(other.m_width), m_height(other.m_height), m_data(other.m_data) {}

    /// @brief Assignment operator
    /// @param other The grid to copy from
    /// @return Reference to this grid
    grid& operator=(const grid& other) {
        if (this != &other) {
            m_width = other.m_width;
            m_height = other.m_height;
            m_data = other.m_data;
        }
        return *this;
    }

    /// @brief Get the value at a specific position
    /// @param x The x coordinate
    /// @param y The y coordinate
    /// @return The value at the specified position
    int get(int x, int y) const { return m_data[y * m_width + x]; }

    /// @brief Set the value at a specific position
    /// @param x The x coordinate
    /// @param y The y coordinate
    /// @param value The value to set
    void set(int x, int y, int value) { m_data[y * m_width + x] = value; }

    /// @brief Check if the coordinates are within the grid bounds
    /// @param x The x coordinate
    /// @param y The y coordinate
    /// @return True if the coordinates are within bounds, false otherwise
    bool in_bounds(int x, int y) const { return x >= 0 && x < m_width && y >= 0 && y < m_height; }

    /// @brief Access operator - allows modification
    /// @param x The x coordinate
    /// @param y The y coordinate
    /// @return Reference to the value at the specified position
    int& operator()(int x, int y) { return m_data[y * m_width + x]; }

    /// @brief Access operator - const version
    /// @param x The x coordinate
    /// @param y The y coordinate
    /// @return Value at the specified position
    int operator()(int x, int y) const { return m_data[y * m_width + x]; }

    /// @brief Get the width of the grid
    /// @return The width
    int width() const { return m_width; }

    /// @brief Get the height of the grid
    /// @return The height
    int height() const { return m_height; }

    /// @brief Resize the grid to new dimensions
    /// @param new_width The new width
    /// @param new_height The new height
    /// @param default_value The value to fill new cells with
    void resize(int new_width, int new_height, int default_value = 0);

    /// @brief Clear the grid with a specific value
    /// @param value The value to fill the grid with
    void clear(int value = 0);

    /// @brief Get a copy of the grid data
    /// @return A copy of the internal data vector
    std::vector<int> data() const { return m_data; }

private:
    int m_width;
    int m_height;
    std::vector<int> m_data;
};


////////////////////////////////////////////////////////////////////////////////
////                                symbol
////////////////////////////////////////////////////////////////////////////////
/// @brief Represents a symbol in the grid alphabet
///
/// A symbol has a unique identifier and a human-readable name.
/// Symbols are used to give meaning to the integer values in the grid.
struct symbol
{
    int id;                 ///< Unique identifier for the symbol
    std::string name;       ///< Human-readable name for the symbol
};

////////////////////////////////////////////////////////////////////////////////
////                                alphabet
////////////////////////////////////////////////////////////////////////////////
/// @brief A collection of symbols that gives meaning to grid values
///
/// The alphabet defines the set of valid symbols that can appear in a grid.
/// It maintains special symbols like empty and wildcard, and provides methods
/// for adding, removing, and accessing symbols by their ID.
class alphabet
{
public:
    /// @brief Constructor
    alphabet();

    /// @brief Destructor
    ~alphabet() = default;

    /// @brief Add a symbol to the alphabet
    /// @param s The symbol to add
    void add_symbol(const symbol& s);

    /// @brief Check if a symbol with the specified ID exists
    /// @param id The symbol ID to check
    /// @return True if the symbol exists, false otherwise
    bool has_symbol(int id) const { return m_symbols.find(id) != m_symbols.end(); }

    /// @brief Remove a symbol from the alphabet
    /// @param id The ID of the symbol to remove
    void remove_symbol(int id) { m_symbols.erase(id); }

    /// @brief Get a symbol by its ID
    /// @param id The ID of the symbol to retrieve
    /// @return The symbol
    /// @throws std::out_of_range if the symbol doesn't exist
    const symbol& get_symbol(int id) const { return m_symbols.at(id); }

    /// @brief Get all symbols in the alphabet
    /// @return A vector containing all symbols
    const std::vector<symbol>& get_symbols() const;

    /// @brief The empty symbol (ID = 0)
    const static symbol empty_symbol;

    /// @brief The wildcard symbol (ID = -1)
    const static symbol wildcard_symbol;

    /// @brief Get the internal symbols map for serialization
    /// @return The map of symbols
    const std::map<int, symbol>& symbols() const { return m_symbols; }

private:
    std::map<int, symbol> m_symbols;
};


////////////////////////////////////////////////////////////////////////////////
////                            transformation
////////////////////////////////////////////////////////////////////////////////
/// @brief Abstract base class for all grid transformations
///
/// A transformation applies changes to a grid according to specific rules.
/// This is the base class for all transformation types and defines the common
/// interface and properties like name and enabled state.
class transformation
{
public:
    /// @brief Constructor
    /// @param name The name of the transformation
    /// @param alphabet The alphabet to use
    transformation(std::string name, std::shared_ptr<alphabet> alphabet)
    : m_name(std::move(name)), m_alphabet(std::move(alphabet)) {}

    /// @brief Virtual destructor
    virtual ~transformation() = default;

    /// @brief Apply the transformation to a grid
    /// @param input The input grid to transform
    /// @param output The output grid to store the result
    /// @note The output grid will be resized to match the input grid if needed
    virtual void apply(const grid& input, grid& output) = 0;

    /// @brief Get the name of the transformation
    /// @return The name
    const std::string& name() const { return m_name; }

    /// @brief Set the name of the transformation
    /// @param name The new name
    void set_name(const std::string& name) { m_name = name; }

    /// @brief Check if the transformation is enabled
    /// @return True if enabled, false otherwise
    bool enabled() const { return m_enabled; }

    /// @brief Enable or disable the transformation
    /// @param enabled The new enabled state
    void set_enabled(bool enabled) { m_enabled = enabled; }

    /// @brief Transformation types for serialization
    enum class Type {
        RANDOM,
        RULE_BASED
    };

    /// @brief Get the type of the transformation
    /// @return The transformation type
    virtual Type type() const = 0;

protected:
    std::string m_name;
    bool m_enabled = true;
    std::shared_ptr<alphabet> m_alphabet;
};

////////////////////////////////////////////////////////////////////////////////
////                        random_transformation
////////////////////////////////////////////////////////////////////////////////
/// @brief A transformation that fills the grid with random symbols
///
/// This transformation fills each cell of the grid with a randomly selected
/// symbol from the alphabet. It's useful for generating initial noise or
/// randomized starting points for patterns.
class random_transformation : public transformation
{
public:
    /// @brief Constructor
    /// @param name The name of the transformation
    /// @param alphabet The alphabet to use
    explicit random_transformation(std::string name, std::shared_ptr<alphabet> alphabet)
    : transformation(std::move(name), std::move(alphabet)) {}

    /// @brief Destructor
    ~random_transformation() override = default;

    /// @brief Apply random values to the grid
    /// @param input The input grid
    /// @param output The output grid to fill with random values
    void apply(const grid& input, grid& output) override;

    /// @brief Get the type of transformation
    /// @return Type::RANDOM
    Type type() const override { return Type::RANDOM; }
};

////////////////////////////////////////////////////////////////////////////////
////                    rule_based_transformation
////////////////////////////////////////////////////////////////////////////////
/// @brief A transformation that applies pattern matching and replacement rules
///
/// This transformation searches for specific patterns in the grid and replaces
/// them with new patterns according to defined rules. It supports wildcards
/// in both search and replacement patterns, and can apply multiple replacements
/// with different probabilities.
class rule_based_transformation : public transformation
{
public:
    /// @brief Constructor
    /// @param name The name of the transformation
    /// @param alphabet The alphabet to use
    explicit rule_based_transformation(std::string name, std::shared_ptr<alphabet> alphabet)
    : transformation(std::move(name), std::move(alphabet)) {}

    /// @brief Destructor
    ~rule_based_transformation() override = default;

    /// @brief Set the search pattern
    /// @param search The pattern to search for
    void set_search(const grid& search) { m_search = search; }

    /// @brief Get the search pattern
    /// @return The search pattern
    const grid& get_search() const { return m_search; }

    /// @brief Add a replacement pattern with a probability
    /// @param probability The probability of this replacement (0.0-1.0)
    /// @param replacement The replacement pattern
    void add_replacement(float probability, const grid& replacement)
    { m_replacement.push_back({probability, replacement}); }

    /// @brief Get the number of replacement patterns
    /// @return The number of replacements
    int replacement_count() const { return m_replacement.size(); }

    /// @brief Get the probability of a specific replacement
    /// @param index The index of the replacement
    /// @return The probability or 0.0 if index is invalid
    float get_replacement_probability(int index) const {
        return (index >= 0 && index < (int)m_replacement.size()) ?
            m_replacement[index].probability : 0.0f;
    }

    /// @brief Get a specific replacement pattern
    /// @param index The index of the replacement
    /// @return The replacement grid or an empty grid if index is invalid
    const grid& get_replacement_grid(int index) const {
        static grid empty_grid;
        return (index >= 0 && index < (int)m_replacement.size()) ?
            m_replacement[index].replacement : empty_grid;
    }

    /// @brief Clear all replacement patterns
    void clear_replacements() { m_replacement.clear(); }

    /// @brief Update an existing replacement pattern
    /// @param index The index of the replacement to update
    /// @param probability The new probability
    /// @param replacement The new replacement pattern
    void update_replacement(int index, float probability, const grid& replacement) {
        if (index >= 0 && index < (int)m_replacement.size()) {
            m_replacement[index].probability = probability;
            m_replacement[index].replacement = replacement;
        }
    }

    /// @brief Apply the rule-based transformation to a grid
    /// @param input The input grid
    /// @param output The output grid where matches will be replaced
    void apply(const grid& input, grid& output) override;

    /// @brief Get the type of transformation
    /// @return Type::RULE_BASED
    Type type() const override { return Type::RULE_BASED; }

    /// @brief Replacement entry structure
    struct replacement_entry {
        float probability;   ///< Probability of this replacement (0.0-1.0)
        grid replacement;    ///< Replacement pattern
    };

    /// @brief Get all replacement entries for serialization
    /// @return Vector of replacement entries
    const std::vector<replacement_entry>& replacements() const { return m_replacement; }

private:
    grid m_search;
    std::vector<replacement_entry> m_replacement;
};

////////////////////////////////////////////////////////////////////////////////
////                            grid_synth
////////////////////////////////////////////////////////////////////////////////
/// @brief Main class for the grid synthesis system
///
/// The grid_synth class manages the entire grid synthesis process. It holds
/// the grid, alphabet, and a pipeline of transformations. It provides methods
/// for adding transformations and synthesizing the grid by applying all
/// enabled transformations in sequence.
class grid_synth
{
public:
    /// @brief Constructor
    /// @param width Initial grid width
    /// @param height Initial grid height
    /// @param default_value Default value for grid cells
    grid_synth(int width = 32, int height = 32, int default_value = 0)
        : m_grid(width, height, default_value) {}

    /// @brief Destructor
    ~grid_synth() = default;

    /// @brief Move constructor
    /// @param other The grid_synth to move from
    grid_synth(grid_synth&& other) noexcept
        : m_grid(std::move(other.m_grid)),
          m_alphabet(std::move(other.m_alphabet)),
          m_transformations(std::move(other.m_transformations))
    {}

    /// @brief Move assignment operator
    /// @param other The grid_synth to move from
    /// @return Reference to this object
    grid_synth& operator=(grid_synth&& other) noexcept
    {
        if (this != &other) {
            m_grid = std::move(other.m_grid);
            m_alphabet = std::move(other.m_alphabet);
            m_transformations = std::move(other.m_transformations);
        }
        return *this;
    }

    // Disable copy operations because of unique_ptr members
    grid_synth(const grid_synth&) = delete;
    grid_synth& operator=(const grid_synth&) = delete;

    /// @brief Get the alphabet
    /// @return Shared pointer to the alphabet
    std::shared_ptr<alphabet> get_alphabet() { return m_alphabet; }

    /// @brief Get the grid
    /// @return Reference to the grid
    grid& get_grid() { return m_grid; }

    /// @brief Add a transformation to the pipeline
    /// @param t The transformation to add
    void add_transformation(std::unique_ptr<transformation> t)
    { m_transformations.push_back(std::move(t)); }

    /// @brief Get all transformations (const)
    /// @return Vector of transformations
    const std::vector<std::unique_ptr<transformation>>& get_transformations() const
    { return m_transformations; }

    /// @brief Get all transformations (non-const)
    /// @return Vector of transformations
    std::vector<std::unique_ptr<transformation>>& get_transformations()
    { return m_transformations; }

    /// @brief Apply all enabled transformations to the grid
    void synthesize();

    /// @brief Convert to JSON
    /// @return JSON representation of the grid_synth
    nlohmann::json to_json() const;

    /// @brief Create a grid_synth from JSON
    /// @param j The JSON to parse
    /// @return A new grid_synth instance
    static grid_synth from_json(const nlohmann::json& j);

protected:
    grid m_grid;
    std::shared_ptr<alphabet> m_alphabet = std::make_shared<alphabet>();
    std::vector<std::unique_ptr<transformation>> m_transformations;
};

}