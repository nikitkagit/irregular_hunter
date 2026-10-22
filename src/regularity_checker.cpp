#include "regularity_checker.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <unordered_map>

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
    // Discard elements strictly below the configured threshold.
    data_t filtered;
    if (m_criteria.ignore_below == 0)
    {
        filtered = data;
    }
    else
    {
        for (const num_t &v : data)
        {
            if (v >= m_criteria.ignore_below)
            {
                filtered.insert(v);
            }
        }
    }
    const std::size_t ignored_count = data.size() - filtered.size();

    if (filtered.empty())
    {
        Analysis result;
        result.period = 1;
        result.min_periodic_element = 0;
        result.ignored_count = ignored_count;
        return result;
    }

    const num_t max_value = *filtered.rbegin();
    const std::size_t max_value_size = static_cast<std::size_t>(max_value);

    // Extra unchecked tail on top of the natural p-position tail, driven by
    // the configurable percentage of the maximum value.
    const std::size_t extra_tail_size =
        static_cast<std::size_t>(std::ceil(
            static_cast<double>(max_value_size)
            * m_criteria.tail_percent / 100.0));

    // Cap the searchable period so that every candidate is verified over at
    // least one period's worth of comparisons.
    const std::size_t checkable_range =
        (max_value_size > extra_tail_size)
        ? max_value_size - extra_tail_size
        : 0;
    const std::size_t half_range =
        std::max<std::size_t>(1, checkable_range / 2);
    const std::size_t p_upper = std::min(m_criteria.max_period, half_range);

    std::optional<Analysis> best;

    for (std::size_t p = 1; p <= p_upper; ++p)
    {
        const num_t p_num = static_cast<num_t>(p);

        // Smallest boundary N such that for all n in [N, max - p -
        // extra_tail_size]:
        //     (n in S) <=> (n + p in S).
        // Positions n such that n + p falls into the extra tail are not
        // checked, because the data near the end of the set is considered
        // incomplete.
        std::size_t boundary = 0;
        std::size_t max_n_size = 0;

        if (p + extra_tail_size < max_value_size)
        {
            max_n_size = max_value_size - p - extra_tail_size;
            const num_t max_n = static_cast<num_t>(max_n_size);

            for (const num_t s : filtered)
            {
                // Case 1: n = s (s in S but s + p not in S).
                if (s <= max_n && filtered.count(s + p_num) == 0)
                {
                    boundary = std::max(
                        boundary, static_cast<std::size_t>(s) + 1);
                }
                // Case 2: n = s - p (s in S but s - p not in S).
                if (s >= p_num)
                {
                    const num_t n = s - p_num;
                    if (n <= max_n && filtered.count(n) == 0)
                    {
                        boundary = std::max(
                            boundary, static_cast<std::size_t>(n) + 1);
                    }
                }
            }
        }

        // Require at least one full period's worth of verified positions in
        // [boundary, max_n]. Otherwise the "periodicity" would be vacuously
        // true at a single boundary point (e.g. period 15 on
        // {0, 2, ..., 30} ∪ {15} would only check n = 15), which is not a
        // meaningful periodic pattern.
        if (max_n_size < boundary)
        {
            continue;
        }
        if (max_n_size - boundary < p)
        {
            continue;
        }

        // Smallest element actually present in the periodic tail.
        const auto it = filtered.lower_bound(static_cast<num_t>(boundary));
        if (it == filtered.end())
        {
            continue;
        }
        const std::size_t min_periodic_element =
            static_cast<std::size_t>(*it);
        const std::size_t periodic_count =
            static_cast<std::size_t>(std::distance(it, filtered.end()));

        if (periodic_count < m_criteria.min_periodic_count)
        {
            continue;
        }
        const std::size_t finite_count = filtered.size() - periodic_count;

        const double ratio = 100.0 * static_cast<double>(finite_count)
                             / static_cast<double>(periodic_count);
        if (ratio > m_criteria.max_finite_to_periodic_percent)
        {
            continue;
        }

        // Longest chain of consecutive elements with step p. For each
        // element x, chain[x] = 1 + chain[x + p] when x + p is in the set,
        // otherwise 1. Since we iterate in reverse (descending) order,
        // x + p is processed before x, so its chain length is already
        // known.
        if (m_criteria.min_period_additions > 0)
        {
            std::unordered_map<num_t, std::size_t> chain;
            chain.reserve(filtered.size());
            std::size_t max_additions = 0;

            for (auto rit = filtered.rbegin(); rit != filtered.rend(); ++rit)
            {
                const num_t x = *rit;
                std::size_t c = 1;
                const auto found = chain.find(x + p_num);
                if (found != chain.end())
                {
                    c = found->second + 1;
                }
                chain.emplace(x, c);
                if (c - 1 > max_additions)
                {
                    max_additions = c - 1;
                }
            }

            if (max_additions < m_criteria.min_period_additions)
            {
                continue;
            }
        }

        Analysis candidate;
        candidate.period = p;
        candidate.min_periodic_element = min_periodic_element;
        candidate.finite_count = finite_count;
        candidate.periodic_count = periodic_count;
        candidate.ignored_count = ignored_count;
        candidate.finite_to_periodic_percent = ratio;

        // Prefer the longest periodic tail; break ties by the shortest
        // period.
        if (!best
            || candidate.min_periodic_element < best->min_periodic_element
            || (candidate.min_periodic_element ==
                    best->min_periodic_element
                && candidate.period < best->period))
        {
            best = candidate;
        }
    }

    return best;
}