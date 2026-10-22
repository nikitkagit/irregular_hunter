#ifndef REGULARITY_CRITERIA_HPP
#define REGULARITY_CRITERIA_HPP

#include "CU/types.hpp"

#include <cstddef>

/**
 * @brief Criteria used to decide whether a finite set of natural numbers is
 *        considered regular (eventually periodic).
 *
 * The check is heuristic: every finite set is eventually periodic in the
 * mathematical sense, so the criteria define when the detected periodicity
 * is "meaningful". In particular, the size of the unexplained finite prefix
 * must not exceed the size of the periodic tail by more than the configured
 * ratio.
 */
struct RegularityCriteria
{
    /// Maximum allowed ratio, in percent, of the number of elements in the
    /// finite (non-periodic) prefix to the number of elements in the
    /// periodic tail. A value of 0 requires a purely periodic set.
    double max_finite_to_periodic_percent = 10.0;

    /// Upper bound on the period length to try. The actual upper bound is
    /// additionally capped at half the range of the input data, so that
    /// every candidate is verified over at least one period's worth of
    /// comparisons.
    std::size_t max_period = 1024;

    /// Minimum number of elements in the periodic tail required for a
    /// candidate to be accepted. Guards against spurious periodicity of
    /// tiny tails.
    std::size_t min_periodic_count = 10;

    /// All set elements strictly below this value are discarded before the
    /// analysis. This lets the caller ignore a messy prefix: for example,
    /// with @c ignore_below = 10 the set @c {4, 6, 8, 13} is analyzed as
    /// @c {13}. A value of 0 (the default) keeps every element.
    num_t ignore_below = 100;
};

#endif // REGULARITY_CRITERIA_HPP