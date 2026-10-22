#ifndef RANDOM_SYSTEM_GENERATOR_PARAMETERS_HPP
#define RANDOM_SYSTEM_GENERATOR_PARAMETERS_HPP

#include "CU/types.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

/**
 * @brief Parameters controlling random equation-system generation.
 *
 * All percentage fields are interpreted in the range [0, 100]. When the
 * operator percentages do not sum to 100, they are normalized internally.
 */
struct GenerationParameters
{
    /// RNG seed. @c 0 means "use std::random_device".
    std::uint64_t seed = 0;

    /// Number of equations (and therefore variables) to generate.
    std::size_t num_equations = 5;

    /// Prefix prepended to every variable name: "V" gives "V1", "V2", ...
    std::string variable_prefix = "V";

    /// Maximum numeric value inside a fixed set.
    num_t max_constant_value = 20;

    /// Minimum and maximum number of elements in a generated fixed set.
    std::size_t min_set_size = 1;
    std::size_t max_set_size = 4;

    /// Maximum depth of an expression tree (leaves have depth 1).
    std::size_t max_depth = 3;

    /// At depth < max_depth, probability that a node becomes an operator
    /// rather than a leaf.
    unsigned internal_node_percent = 60;

    /// Among leaves, probability that the leaf is a variable rather than a
    /// fixed set.
    unsigned leaf_variable_percent = 60;

    /// Among variable leaves, probability that the reference points to a
    /// not-yet-defined variable (a forward reference).
    unsigned leaf_unknown_variable_percent = 20;

    /// Probability of each operator at an internal node.
    unsigned operator_union_percent = 34;
    unsigned operator_sum_percent = 33;
    unsigned operator_intersection_percent = 33;

    /// Minimum number of operands an n-ary operator may have.
    std::size_t min_operands = 2;

    /// Maximum number of operands an n-ary operator may have.
    std::size_t max_operands = 3;
};

#endif // RANDOM_SYSTEM_GENERATOR_PARAMETERS_HPP