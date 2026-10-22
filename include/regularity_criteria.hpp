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
    num_t ignore_below = 0;

    /// Percentage of the maximum value, counted from the top of the range,
    /// that is treated as an unchecked tail. For a set with maximum value
    /// M the last @c ceil(M * tail_percent / 100) positions are not used to
    /// verify periodicity: the checker does not test whether
    /// @c n in S  <=>  n + p in S for @c n such that @c n + p falls into
    /// this region, because the data near the end of the set is assumed
    /// incomplete and cannot be trusted. This is on top of the natural
    /// @c p-position tail that the algorithm always skips.
    ///
    /// Large values make the check more lenient (more sets look regular);
    /// a value of 0 (the default) keeps the standard behaviour.
    double tail_percent = 1.0;

    /// Minimum number of "period additions" the set must demonstrate for
    /// a candidate period @c p to be accepted. For a fixed element @c x,
    /// the sequence @c x, x+p, x+2p, ..., x+kp is the longest run of set
    /// elements that starts at @c x and increases by @c p at each step;
    /// @c k is the number of additions for that run. The value compared
    /// against this field is the maximum @c k over all elements of the
    /// (filtered) set. It measures how many times the periodic pattern
    /// actually repeats, independently of how many elements the tail
    /// contains.
    ///
    /// Examples:
    ///  - @c {0, 100, 200, 300} with @c p = 100 has one run of 4 elements,
    ///    so @c max_additions = 3.
    ///  - @c {0, 100, 500} with @c p = 100 has a run of only 2 elements,
    ///    so @c max_additions = 1.
    ///
    /// A value of 0 (the default) disables the check. Values of 1 or more
    /// force the periodic part to actually repeat.
    std::size_t min_period_additions = 10;
};

#endif // REGULARITY_CRITERIA_HPP