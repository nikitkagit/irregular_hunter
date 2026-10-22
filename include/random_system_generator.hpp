#ifndef RANDOM_SYSTEM_GENERATOR_HPP
#define RANDOM_SYSTEM_GENERATOR_HPP

#include "random_system_generator_parameters.hpp"

#include "CU/types.hpp"

#include <cstddef>
#include <random>
#include <string>
#include <vector>

class Parser;

/**
 * @brief Randomly generates systems of equations over sets of natural
 *        numbers.
 *
 * The generator uses a deterministic RNG seeded from
 * GenerationParameters::seed. Generated equations are strings compatible
 * with Parser::parse_equation(), so they can be fed back into the
 * conjunctive_unary library for further analysis.
 *
 * @note The RNG state is advanced by every call to generate_equations()
 *       (and, transitively, generate_into()). Two consecutive calls with
 *       the same parameters produce different systems. Call reseed() or
 *       construct a new generator with the same seed to obtain identical
 *       output.
 */
class RandomSystemGenerator
{
public:
    /**
     * @brief Constructs a generator with the given parameters.
     * @param parameters Generation parameters.
     */
    explicit RandomSystemGenerator(GenerationParameters parameters = {});

    /**
     * @brief Returns the current generation parameters.
     */
    const GenerationParameters &parameters() const;

    /**
     * @brief Replaces the generation parameters and reseeds the RNG.
     * @param parameters New generation parameters.
     */
    void set_parameters(GenerationParameters parameters);

    /**
     * @brief Reseeds the RNG from parameters().seed.
     *
     * Useful to reproduce the sequence of generated systems.
     */
    void reseed();

    /**
     * @brief Generates a fresh system of equations.
     * @return One equation per element, in definition order.
     */
    std::vector<std::string> generate_equations() const;

    /**
     * @brief Generates a system and parses it into the given parser.
     *
     * @param parser Parser bound to the target EquationCalculator.
     * @throws std::runtime_error if a generated equation fails to parse.
     */
    void generate_into(Parser &parser) const;

private:
    std::string generate_expression(std::size_t depth,
                                    std::size_t defined_count) const;
    std::string generate_fixed_set() const;
    std::string generate_variable_reference(std::size_t defined_count) const;
    std::string variable_name(std::size_t index) const;
    char pick_operator() const;
    std::size_t uniform_size(std::size_t lo, std::size_t hi) const;
    bool roll(unsigned percent) const;

    GenerationParameters m_parameters;
    mutable std::mt19937_64 m_rng;
};

#endif // RANDOM_SYSTEM_GENERATOR_HPP