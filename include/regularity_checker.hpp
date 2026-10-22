#ifndef REGULARITY_CHECKER_HPP
#define REGULARITY_CHECKER_HPP

#include "regularity_criteria.hpp"
#include "CU/types.hpp"

#include <cstddef>
#include <optional>

/**
 * @brief Determines whether a finite set of natural numbers is consistent
 *        with an eventually periodic set, i.e. a regular set over a unary
 *        alphabet.
 *
 * Before analysis, every element strictly below
 * RegularityCriteria::ignore_below is discarded. The remaining set S is
 * then searched for a period p and a threshold N such that for all
 * n in [N, max(S) - p]:
 * @code
 *     n in S  <=>  n + p in S
 * @endcode
 * The candidate is accepted if the finite prefix [0, N) is not too large
 * compared to the periodic tail [N, max(S)], according to
 * RegularityCriteria.
 *
 * Note: every finite set is trivially eventually periodic in the
 * mathematical sense, so this class implements a practical "looks regular"
 * test, not a formal language-membership decision. The criteria parameter
 * controls how strict the test is.
 */
class RegularityChecker
{
public:
    /// Detailed result of a successful check.
    struct Analysis
    {
        std::size_t period = 0;             ///< Detected period.
        std::size_t threshold = 0;          ///< Start of the periodic tail.
        std::size_t finite_count = 0;       ///< Elements before threshold.
        std::size_t periodic_count = 0;     ///< Elements at/after threshold.
        std::size_t ignored_count = 0;      ///< Elements below ignore_below.
        double finite_to_periodic_percent = 0.0;
    };

    /**
     * @brief Constructs a checker with the given criteria.
     * @param criteria Criteria to use.
     */
    explicit RegularityChecker(RegularityCriteria criteria = {});

    /**
     * @brief Returns true if the set passes the regularity check.
     * @param data Set of natural numbers to check.
     */
    bool is_regular(const data_t &data) const;

    /**
     * @brief Returns a detailed analysis if the set passes the check, or
     *        std::nullopt otherwise.
     * @param data Set of natural numbers to check.
     */
    std::optional<Analysis> analyze(const data_t &data) const;

private:
    RegularityCriteria m_criteria;
};

#endif // REGULARITY_CHECKER_HPP