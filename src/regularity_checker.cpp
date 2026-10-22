#include "regularity_checker.hpp"

#include <algorithm>
#include <cstddef>

RegularityChecker::RegularityChecker(RegularityCriteria criteria)
    : m_criteria(criteria)
{
}

bool RegularityChecker::is_regular(const data_t &data) const
{
    return analyze(data).has_value();
}

std::optional<RegularityChecker::Analysis> RegularityChecker::analyze(
    const data_t &data) const
{
    if (data.empty())
    {
        // The empty set is periodic with any period starting from zero.
        Analysis result;
        result.period = 1;
        result.threshold = 0;
        return result;
    }

    const num_t max_value = *data.rbegin();
    const std::size_t max_value_size = static_cast<std::size_t>(max_value);

    // Cap the searchable period so that every candidate is checked over at
    // least one full period's worth of comparisons.
    const std::size_t half_range =
        std::max<std::size_t>(1, max_value_size / 2);
    const std::size_t p_upper = std::min(m_criteria.max_period, half_range);

    std::optional<Analysis> best;

    for (std::size_t p = 1; p <= p_upper; ++p)
    {
        // Smallest N such that for all n in [N, max - p]:
        //     (n in S) <=> (n + p in S).
        // Equivalently, N = (rightmost mismatch position) + 1, or 0 if there
        // are no mismatches.
        std::size_t threshold = 0;
        if (p <= max_value_size)
        {
            const num_t p_num = static_cast<num_t>(p);
            const num_t max_minus_p = max_value - p_num;
            for (const num_t s : data)
            {
                // Case 1: s is a member but s + p is not (s + p is in range).
                if (s <= max_minus_p && data.count(s + p_num) == 0)
                {
                    threshold = std::max(threshold,
                                         static_cast<std::size_t>(s) + 1);
                }
                // Case 2: s = n + p with n not a member.
                if (s >= p_num && data.count(s - p_num) == 0)
                {
                    threshold = std::max(
                        threshold,
                        static_cast<std::size_t>(s - p_num) + 1);
                }
            }
        }

        // Count the elements in the periodic tail.
        std::size_t periodic_count = 0;
        for (auto it = data.lower_bound(static_cast<num_t>(threshold));
             it != data.end(); ++it)
        {
            ++periodic_count;
        }
        if (periodic_count < m_criteria.min_periodic_count)
        {
            continue;
        }
        const std::size_t finite_count = data.size() - periodic_count;

        const double ratio = 100.0 * static_cast<double>(finite_count)
                             / static_cast<double>(periodic_count);
        if (ratio > m_criteria.max_finite_to_periodic_percent)
        {
            continue;
        }

        Analysis candidate;
        candidate.period = p;
        candidate.threshold = threshold;
        candidate.finite_count = finite_count;
        candidate.periodic_count = periodic_count;
        candidate.finite_to_periodic_percent = ratio;

        // Prefer the longest periodic tail; break ties by the shortest
        // period.
        if (!best
            || candidate.threshold < best->threshold
            || (candidate.threshold == best->threshold
                && candidate.period < best->period))
        {
            best = candidate;
        }
    }

    return best;
}