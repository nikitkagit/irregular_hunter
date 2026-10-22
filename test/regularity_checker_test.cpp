#include <cassert>
#include <iostream>
#include <vector>

#include "regularity_checker.hpp"
#include "CU/types.hpp"

namespace
{
RegularityCriteria make_criteria(
    double max_finite_percent = 10.0,
    std::size_t max_period = 1024,
    std::size_t min_periodic = 1,
    num_t ignore_below = 0)
{
    RegularityCriteria criteria;
    criteria.max_finite_to_periodic_percent = max_finite_percent;
    criteria.max_period = max_period;
    criteria.min_periodic_count = min_periodic;
    criteria.ignore_below = ignore_below;
    return criteria;
}
} // namespace

int main()
{
    // ===== Basic checks with explicit criteria (min_periodic = 1) =====

    // --- Empty set ---
    {
        RegularityChecker checker(make_criteria());
        data_t empty;
        assert(checker.is_regular(empty));

        auto analysis = checker.analyze(empty);
        assert(analysis.has_value());
        assert(analysis->period == 1);
        assert(analysis->threshold == 0);
        assert(analysis->finite_count == 0);
        assert(analysis->periodic_count == 0);
        assert(analysis->ignored_count == 0);
    }

    // --- Singleton {0} ---
    {
        RegularityChecker checker(make_criteria());
        data_t data = {0};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->period == 1);
        assert(analysis->threshold == 0);
        assert(analysis->finite_count == 0);
        assert(analysis->periodic_count == 1);
        assert(analysis->ignored_count == 0);
    }

    // --- Singleton {1} ---
    {
        RegularityChecker checker(make_criteria());
        data_t data = {1};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->period == 1);
        assert(analysis->threshold == 1);
        assert(analysis->finite_count == 0);
        assert(analysis->periodic_count == 1);
    }

    // --- Two consecutive numbers {1, 2} ---
    {
        RegularityChecker checker(make_criteria());
        data_t data = {1, 2};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->period == 1);
        assert(analysis->threshold == 1);
        assert(analysis->finite_count == 0);
        assert(analysis->periodic_count == 2);
    }

    // --- All odd numbers 1..9 ---
    {
        RegularityChecker checker(make_criteria());
        data_t data = {1, 3, 5, 7, 9};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->period == 2);
        assert(analysis->threshold == 0);
        assert(analysis->finite_count == 0);
        assert(analysis->periodic_count == 5);
    }

    // --- All even numbers 0..8 ---
    {
        RegularityChecker checker(make_criteria());
        data_t data = {0, 2, 4, 6, 8};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->period == 2);
        assert(analysis->threshold == 0);
        assert(analysis->finite_count == 0);
        assert(analysis->periodic_count == 5);
    }

    // --- Eventually periodic: {1,2,3,5,7,9} ---
    // Period 2, threshold 3, finite={1,2}, periodic={3,5,7,9}, ratio=50%.
    {
        data_t data = {1, 2, 3, 5, 7, 9};

        auto strict = make_criteria(10.0);
        assert(!RegularityChecker(strict).is_regular(data));

        auto loose = make_criteria(100.0);
        RegularityChecker loose_checker(loose);
        assert(loose_checker.is_regular(data));

        auto analysis = loose_checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->period == 2);
        assert(analysis->threshold == 3);
        assert(analysis->finite_count == 2);
        assert(analysis->periodic_count == 4);
        assert(analysis->finite_to_periodic_percent > 49.0);
        assert(analysis->finite_to_periodic_percent < 51.0);
        assert(analysis->finite_count + analysis->periodic_count ==
               data.size());
    }

    // --- Period 100: {100,200,300,400} ---
    {
        RegularityChecker checker(make_criteria());
        data_t data = {100, 200, 300, 400};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->period == 100);
        assert(analysis->threshold == 1);
        assert(analysis->periodic_count == 4);
    }

    // --- Same set with max_period too small ---
    {
        auto criteria = make_criteria();
        criteria.max_period = 50;
        RegularityChecker checker(criteria);
        data_t data = {100, 200, 300, 400};
        assert(!checker.is_regular(data));
    }

    // --- Two elements with a huge gap: {0, 1000} ---
    {
        RegularityChecker checker(make_criteria());
        data_t data = {0, 1000};
        assert(!checker.is_regular(data));
    }

    // --- Period 5: {0,5,10,15,20} ---
    {
        RegularityChecker checker(make_criteria());
        data_t data = {0, 5, 10, 15, 20};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->period == 5);
        assert(analysis->threshold == 0);
        assert(analysis->periodic_count == 5);
    }

    // --- Period 3: {0,3,6,9,12,15} ---
    {
        RegularityChecker checker(make_criteria());
        data_t data = {0, 3, 6, 9, 12, 15};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->period == 3);
        assert(analysis->threshold == 0);
        assert(analysis->periodic_count == 6);
    }

    // --- Period 1: consecutive numbers 1..10 ---
    {
        RegularityChecker checker(make_criteria());
        data_t data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->period == 1);
        assert(analysis->threshold == 1);
        assert(analysis->periodic_count == 10);
    }

    // --- max_finite_to_periodic_percent boundary ---
    {
        data_t data = {1, 2, 3, 5, 7, 9};

        auto exact = make_criteria(50.0);
        assert(RegularityChecker(exact).is_regular(data));

        auto below = make_criteria(49.0);
        assert(!RegularityChecker(below).is_regular(data));
    }

    // --- max_period = 1 forces period 1 on an even-period set ---
    {
        auto criteria = make_criteria(1000.0, 1);
        RegularityChecker checker(criteria);

        data_t data = {2, 4, 6, 8};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->period == 1);
        assert(analysis->threshold == 8);
        assert(analysis->finite_count == 3);
        assert(analysis->periodic_count == 1);
    }

    // --- Threshold in the middle: {1,2,3,10,12,14,16,18} ---
    // Period 2, threshold 9, finite=3, periodic=5, ratio=60%.
    {
        auto criteria = make_criteria(100.0);
        RegularityChecker checker(criteria);

        data_t data = {1, 2, 3, 10, 12, 14, 16, 18};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->period == 2);
        assert(analysis->threshold == 9);
        assert(analysis->finite_count == 3);
        assert(analysis->periodic_count == 5);
        assert(analysis->finite_count + analysis->periodic_count ==
               data.size());
    }

    // --- Analysis invariants: counts sum to |data| ---
    {
        auto criteria = make_criteria(1000.0);
        RegularityChecker checker(criteria);

        const std::vector<data_t> sets = {
            {},
            {0},
            {1},
            {1, 2},
            {1, 3, 5, 7, 9},
            {0, 2, 4, 6, 8},
            {0, 5, 10, 15, 20},
            {0, 3, 6, 9, 12, 15},
            {100, 200, 300, 400},
        };
        for (const data_t &s : sets)
        {
            auto analysis = checker.analyze(s);
            assert(analysis.has_value());
            assert(analysis->finite_count + analysis->periodic_count +
                       analysis->ignored_count ==
                   s.size());
            assert(analysis->period >= 1);
        }
    }

    // ===== Negative tests: sets that should NOT pass (min_periodic = 1) =====

    // --- Negative: large finite prefix, default 10% ratio ---
    {
        // {1,2,3,100}: half_range caps p at 50, so the natural p=97 is not
        // tried. Every tried p leaves 100 alone in the tail:
        // finite=3, periodic=1 → ratio 300% exceeds the 10% default.
        RegularityChecker checker(make_criteria());
        data_t data = {1, 2, 3, 100};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Negative: default criteria rejects 50% finite/periodic ratio ---
    {
        // {1,2,3,5,7,9}: period 2, threshold 3, finite=2, periodic=4, 50%.
        RegularityChecker checker(make_criteria());
        data_t data = {1, 2, 3, 5, 7, 9};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Negative: min_periodic_count not met ---
    {
        auto criteria = make_criteria(10.0, 1024, 3);
        RegularityChecker checker(criteria);

        // p=10 fits perfectly but leaves only 2 elements in the tail.
        data_t data = {0, 10};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Negative: max_period below the natural period ---
    {
        // With the default cap (half_range = 200), p=100 is tried and the
        // set is purely periodic.
        RegularityChecker default_checker(make_criteria());
        data_t data = {0, 100, 200, 300, 400};
        assert(default_checker.is_regular(data));

        // With max_period = 50, p=100 is not tried; every smaller p leaves
        // at least 1 element in the finite prefix (ratio >= 25%).
        auto small = make_criteria(10.0, 50);
        RegularityChecker small_checker(small);
        assert(!small_checker.is_regular(data));
        assert(!small_checker.analyze(data).has_value());
    }

    // --- Negative: default max_period=1024 is not enough for a 2000 gap ---
    {
        // {0, 2000, 4000}: half_range = 2000, p_upper = 1024. The natural
        // period 2000 is not tried, and every smaller period leaves a huge
        // finite prefix (ratio >= 200%).
        RegularityChecker checker(make_criteria());
        data_t data = {0, 2000, 4000};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Negative: zero ratio requires purely periodic set ---
    {
        auto criteria = make_criteria(0.0);
        RegularityChecker checker(criteria);

        // {0,1,5}: half_range = 2, p_upper = 2. No tried p yields
        // threshold 0, so the finite prefix is non-empty → rejected.
        data_t data = {0, 1, 5};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());

        // {0,2,4,6,8}: purely periodic with p=2, finite=0 → accepted.
        data_t pure = {0, 2, 4, 6, 8};
        assert(checker.is_regular(pure));
    }

    // --- Negative: even with generous ratio, tiny tails are rejected ---
    {
        auto criteria = make_criteria(10000.0, 1024, 10);
        RegularityChecker checker(criteria);

        // p=5 fits perfectly but leaves only 4 elements in the tail,
        // and no p can yield ≥ 10 periodic elements from a 4-element set.
        data_t data = {0, 5, 10, 15};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Almost period 1: long block, then a jump ---
    {
        // {1,2,3,4,5,100}: the run 1..5 suggests p=1, but 100 breaks it.
        // p=1 gives threshold=100, finite=5, periodic=1, ratio=500%.
        RegularityChecker checker(make_criteria());
        data_t data = {1, 2, 3, 4, 5, 100};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Almost period 2: evens with one far outlier ---
    {
        // {2,4,6,8,10,100}: looks like evens, but 100 is isolated.
        // p=2 gives threshold=99, finite=5, periodic=1, ratio=500%.
        RegularityChecker checker(make_criteria());
        data_t data = {2, 4, 6, 8, 10, 100};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Almost period 3: skipped multiple of 3 ---
    {
        // {3,6,9,12,15,21}: 18 is missing, so p=3 breaks at the end.
        // p=3 gives threshold=19, finite=5, periodic=1, ratio=500%.
        RegularityChecker checker(make_criteria());
        data_t data = {3, 6, 9, 12, 15, 21};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Almost period 2: evens with one odd appended ---
    {
        // {0,2,4,6,8,11}: the extra 11 is off-parity with the rest.
        // Every p <= 5 leaves a large finite prefix (ratio >= 500%).
        RegularityChecker checker(make_criteria());
        data_t data = {0, 2, 4, 6, 8, 11};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Geometric progression: no usable period at all ---
    {
        // {1,10,100,1000}: gaps grow by 10x each time; no small period
        // covers the tail. p is capped at half_range = 500, and every
        // candidate leaves almost all elements in the finite prefix.
        RegularityChecker checker(make_criteria());
        data_t data = {1, 10, 100, 1000};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Blocks with a truncated tail: {5,6,7, 15,16,17, 25,27} ---
    {
        // Looks like period-10 blocks of 3, but the last block is wrong.
        RegularityChecker checker(make_criteria());
        // the truth period-10 set:
        data_t data = {5, 6, 7, 15, 16, 17, 25, 26};
        assert(checker.is_regular(data));
        auto analysis = checker.analyze(data);

        assert(analysis.has_value());
        assert(analysis->period == 10);
        assert(analysis->threshold == 0);
        assert(analysis->finite_count == 0);
        assert(analysis->periodic_count == 8);

        data = {5, 6, 7, 15, 16, 17, 25, 27};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // ===== min_periodic_count = 2 =====

    {
        auto criteria = make_criteria(1000.0, 1024, 2);
        RegularityChecker checker(criteria);

        // {0,10,20}: p=10, threshold=0, tail=3 → accepted.
        assert(checker.is_regular({0, 10, 20}));

        // {0,10}: no period with tail ≥ 2 in the searchable range
        // (p_upper = 5) → rejected.
        assert(!checker.is_regular({0, 10}));

        // {2,4,6,8}: p=2, tail=4 → accepted.
        assert(checker.is_regular({2, 4, 6, 8}));
    }

    // ===== min_periodic_count = 10 =====

    // Sets with a too-short periodic tail are rejected.
    {
        auto criteria = make_criteria(1000.0, 1024, 10);
        RegularityChecker checker(criteria);

        assert(!checker.is_regular({0}));
        assert(!checker.is_regular({1}));
        assert(!checker.is_regular({1, 2}));
        assert(!checker.is_regular({0, 2, 4, 6, 8}));
        assert(!checker.is_regular({1, 3, 5, 7, 9}));
        assert(!checker.is_regular({0, 5, 10, 15, 20}));
    }

    // Sets whose periodic tail has exactly 10 elements are accepted.
    {
        auto criteria = make_criteria(1000.0, 1024, 10);
        RegularityChecker checker(criteria);

        // {0,2,...,18}: 10 evens, p=2, threshold=0, tail=10.
        data_t evens;
        for (int i = 0; i < 10; ++i)
        {
            evens.insert(static_cast<num_t>(2 * i));
        }
        assert(checker.is_regular(evens));

        auto analysis = checker.analyze(evens);
        assert(analysis.has_value());
        assert(analysis->period == 2);
        assert(analysis->threshold == 0);
        assert(analysis->periodic_count == 10);

        // {1,2,...,10}: 10 consecutive, p=1, threshold=1, tail=10.
        assert(checker.is_regular({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));

        // {0,3,...,27}: 10 multiples of 3, p=3, threshold=0, tail=10.
        data_t multiples;
        for (int i = 0; i < 10; ++i)
        {
            multiples.insert(static_cast<num_t>(3 * i));
        }
        assert(checker.is_regular(multiples));
    }

    // Boundary: exactly min_periodic_count passes; min_periodic_count + 1
    // fails on the same set.
    {
        data_t evens;
        for (int i = 0; i < 10; ++i)
        {
            evens.insert(static_cast<num_t>(2 * i));
        }

        auto ok = make_criteria(1000.0, 1024, 10);
        assert(RegularityChecker(ok).is_regular(evens));

        auto too_strict = make_criteria(1000.0, 1024, 11);
        assert(!RegularityChecker(too_strict).is_regular(evens));
    }

    // Set with a finite prefix and a tail of size 10.
    {
        // p=2, threshold=9 (because 8∉S and 10∈S), finite={0,1,2},
        // periodic tail = {10,12,...,28} (10 elems).
        data_t data = {0, 1, 2, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28};

        auto ok = make_criteria(100.0, 1024, 10);
        RegularityChecker ok_checker(ok);
        assert(ok_checker.is_regular(data));

        auto analysis = ok_checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->period == 2);
        assert(analysis->threshold == 9);
        assert(analysis->finite_count == 3);
        assert(analysis->periodic_count == 10);

        auto too_strict = make_criteria(100.0, 1024, 11);
        assert(!RegularityChecker(too_strict).is_regular(data));
    }

    // A set with a long tail is still rejected by a very high
    // min_periodic_count.
    {
        data_t evens;
        for (int i = 0; i < 20; ++i)
        {
            evens.insert(static_cast<num_t>(2 * i));
        }

        auto ok = make_criteria(1000.0, 1024, 10);
        assert(RegularityChecker(ok).is_regular(evens));

        auto ok20 = make_criteria(1000.0, 1024, 20);
        assert(RegularityChecker(ok20).is_regular(evens));

        auto too_strict = make_criteria(1000.0, 1024, 21);
        assert(!RegularityChecker(too_strict).is_regular(evens));
    }

    // min_periodic_count scales with tail size on the block-pattern set.
    {
        data_t data = {5, 6, 7, 15, 16, 17, 25, 26};  // tail=8

        auto ok = make_criteria(1000.0, 1024, 8);
        assert(RegularityChecker(ok).is_regular(data));

        auto too_strict = make_criteria(1000.0, 1024, 9);
        assert(!RegularityChecker(too_strict).is_regular(data));
    }

    // min_periodic_count does not change the verdict when the set has no
    // periodicity at all.
    {
        auto loose = make_criteria(100.0, 1024, 1);
        auto strict = make_criteria(100.0, 1024, 10);

        data_t data = {1, 2, 3, 4, 5, 100};
        assert(!RegularityChecker(loose).is_regular(data));
        assert(!RegularityChecker(strict).is_regular(data));
    }

    // Large min_periodic_count on small non-empty sets never passes.
    // (The empty set is special-cased as regular and is not affected by
    // min_periodic_count.)
    {
        auto criteria = make_criteria(1000.0, 1024, 100);
        RegularityChecker checker(criteria);

        assert(!checker.is_regular({0, 2, 4, 6, 8}));
        assert(!checker.is_regular({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
    }

    // ===== ignore_below =====

    // --- Motivating example: {4,6,8,13} ---

    // Without ignore_below, the isolated 13 destroys periodicity.
    {
        RegularityChecker checker(make_criteria());
        data_t data = {4, 6, 8, 13};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // ignore_below = 14 removes every element.
    {
        RegularityChecker checker(make_criteria(10.0, 1024, 1, 14));
        data_t data = {4, 6, 8, 13};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->ignored_count == 4);
        assert(analysis->finite_count == 0);
        assert(analysis->periodic_count == 0);
        assert(analysis->period == 1);
        assert(analysis->threshold == 0);
    }

    // ignore_below = 0 behaves identically to no filtering.
    {
        RegularityChecker with_zero(make_criteria(10.0, 1024, 1, 0));
        RegularityChecker no_explicit(make_criteria(10.0, 1024, 1));

        data_t data = {4, 6, 8, 13};
        assert(with_zero.is_regular(data) ==
               no_explicit.is_regular(data));

        data_t evens = {0, 2, 4, 6, 8};
        assert(with_zero.is_regular(evens));
        assert(no_explicit.is_regular(evens));
    }

    // ignore_below that equals the smallest member keeps that member
    // (the filter is strict: v >= ignore_below is kept).
    {
        RegularityChecker checker(make_criteria(10.0, 1024, 1, 2));
        data_t data = {0, 1, 2, 3, 4, 5};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->ignored_count == 2);  // 0 and 1 are dropped
        assert(analysis->finite_count + analysis->periodic_count == 4);
    }

    // ignore_below can rescue a set whose noisy prefix would otherwise
    // break the ratio.
    {
        // {1,2,3,5,7,9}: period 2, threshold 3, ratio 50%.
        // With ignore_below = 3 only {3,5,7,9} remain, threshold 0,
        // ratio 0% → accepted under the default 10%.
        RegularityChecker checker(make_criteria(10.0, 1024, 1, 3));
        data_t data = {1, 2, 3, 5, 7, 9};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->period == 2);
        assert(analysis->threshold == 2);
        assert(analysis->finite_count == 0);
        assert(analysis->periodic_count == 4);
        assert(analysis->ignored_count == 2);
    }

    // ignore_below does not change the verdict when the filtered set is
    // already irregular for intrinsic reasons.
    {
        RegularityChecker checker(make_criteria(10.0, 1024, 1, 0));
        data_t data = {1, 10, 100, 1000};
        assert(!checker.is_regular(data));

        // Filtering below 2 does not help: 10,100,1000 is still
        // non-periodic in the searchable range.
        RegularityChecker filtered(make_criteria(10.0, 1024, 1, 2));
        assert(!filtered.is_regular(data));
    }

    // ignore_below combines with min_periodic_count.
    {
        // {4,6,8,13} with ignore_below = 4 → {4,6,8,13}.
        // p=2 gives tail={13} (1 element) which is below min_periodic=2.
        RegularityChecker strict(make_criteria(50.0, 1024, 2, 4));
        assert(!strict.is_regular({4, 6, 8, 13}));

        // ignore_below = 10 → {13}: still 1 element, still below 2.
        RegularityChecker filtered(make_criteria(50.0, 1024, 2, 10));
        assert(!filtered.is_regular({4, 6, 8, 13}));
    }

    // ignore_below combines with max_finite_to_periodic_percent.
    {
        // {0,1,2,10,12,14,16,18}: p=2, threshold=3, finite=3, tail=5,
        // ratio=60% → rejected by default 10%.
        RegularityChecker base(make_criteria(10.0));
        data_t data = {0, 1, 2, 10, 12, 14, 16, 18};
        assert(!base.is_regular(data));

        // ignore_below = 3 drops {0,1,2}: finite=0, ratio=0% → accepted.
        RegularityChecker filtered(make_criteria(10.0, 1024, 1, 3));
        assert(filtered.is_regular(data));

        auto analysis = filtered.analyze(data);
        assert(analysis.has_value());
        assert(analysis->ignored_count == 3);
        assert(analysis->period == 2);
        assert(analysis->threshold == 9);
        assert(analysis->finite_count == 0);
        assert(analysis->periodic_count == 5);
    }

    // ignore_below on a set where everything is filtered: regular, empty.
    {
        RegularityChecker checker(make_criteria(10.0, 1024, 1, 100));
        data_t data = {0, 2, 4, 6, 8};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->ignored_count == 5);
        assert(analysis->finite_count == 0);
        assert(analysis->periodic_count == 0);
    }

    // ignore_below is not applied to already-empty input.
    {
        RegularityChecker checker(make_criteria(10.0, 1024, 1, 100));
        data_t data;
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->ignored_count == 0);
    }

    // Analysis invariants with ignore_below: counts sum to |data|.
    {
        const std::vector<data_t> sets = {
            {},
            {0},
            {1},
            {1, 2},
            {1, 3, 5, 7, 9},
            {0, 2, 4, 6, 8},
            {4, 6, 8, 13},
            {100, 200, 300, 400},
        };
        for (const data_t &s : sets)
        {
            for (num_t ib : {num_t{0}, num_t{3}, num_t{10}, num_t{100}})
            {
                RegularityChecker checker(
                    make_criteria(1000.0, 1024, 1, ib));
                auto analysis = checker.analyze(s);
                if (!analysis.has_value())
                {
                    continue;
                }
                assert(analysis->finite_count + analysis->periodic_count +
                           analysis->ignored_count ==
                       s.size());
                assert(analysis->ignored_count <= s.size());
            }
        }
    }

    std::cout << "All RegularityChecker tests passed\n";
    return 0;
}