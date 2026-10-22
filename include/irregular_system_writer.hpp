#ifndef IRREGULAR_SYSTEM_WRITER_HPP
#define IRREGULAR_SYSTEM_WRITER_HPP

#include "irregular_system_writer_parameters.hpp"

#include <string>

/**
 * @brief Generates a random equation system, solves it, and writes a
 *        readable report.
 *
 * The class bundles the whole "hunt for irregular sets" pipeline:
 *
 *  1. Randomly generate an equation system with RandomSystemGenerator
 *     configured by IrregularSystemWriterParameters::generator.
 *  2. Parse and iteratively solve it with EquationCalculator, stopping
 *     either on convergence (two consecutive states with identical values
 *     of every variable) or when max_iterations() is reached.
 *  3. Analyze every variable value with a RegularityChecker constructed
 *     from IrregularSystemWriterParameters::criteria.
 *  4. If require_irregular() is true, retry steps 1-3 with the next seed
 *     until a system with at least one irregular variable is found or the
 *     attempt budget is exhausted.
 *  5. Write the system, its solution, and the regularity verdicts to a
 *     text report.
 *
 * When IrregularSystemWriterParameters::generator.seed is non-zero, the
 * attempt sequence is reproducible: attempt i uses seed + i. When the seed
 * is zero, every attempt draws a fresh random seed, and the exact seed is
 * not recoverable from the report.
 */
class IrregularSystemWriter
{
public:
    /**
     * @brief Constructs the writer.
     * @param params Pipeline parameters.
     */
    explicit IrregularSystemWriter(IrregularSystemWriterParameters params = {});

    /**
     * @brief Returns the current parameters.
     */
    const IrregularSystemWriterParameters &parameters() const;

    /**
     * @brief Replaces the parameters.
     * @param params New parameters.
     */
    void set_parameters(IrregularSystemWriterParameters params);

    /**
     * @brief Runs the pipeline and writes the report to @p path.
     * @param path Destination file path.
     * @return True on success, false if the file could not be opened or
     *         written.
     */
    bool write_to_file(const std::string &path) const;

    /**
     * @brief Runs the pipeline and returns the formatted report.
     * @return Multi-line report text.
     */
    std::string format() const;

private:
    IrregularSystemWriterParameters m_params;
};

#endif // IRREGULAR_SYSTEM_WRITER_HPP