#ifndef IRREGULAR_SYSTEM_WRITER_PARAMETERS_HPP
#define IRREGULAR_SYSTEM_WRITER_PARAMETERS_HPP

#include "random_system_generator_parameters.hpp"
#include "regularity_criteria.hpp"

#include <cstddef>

/**
 * @brief Parameters for IrregularSystemWriter.
 *
 * Bundles the generator parameters, the regularity criteria, and the
 * pipeline-level controls (iteration cap, irregular filter, retry budget).
 */
struct IrregularSystemWriterParameters
{
    /// Parameters for the random system generator.
    GenerationParameters generator;

    /// Criteria used by the regularity checker applied to every value.
    RegularityCriteria criteria;

    /// Upper bound on the number of solver iterations per system.
    std::size_t max_iterations = 1000;

    /// If true, keep generating systems until one has at least one irregular
    /// variable. If false, write the first generated system regardless of
    /// its regularity.
    bool require_irregular = true;

    /// Maximum number of systems to try when require_irregular is true.
    /// Ignored when require_irregular is false. Values below 1 are clamped
    /// to 1.
    std::size_t max_attempts = 100;
};

#endif // IRREGULAR_SYSTEM_WRITER_PARAMETERS_HPP