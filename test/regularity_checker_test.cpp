#include <cassert>
#include <iostream>
#include <vector>

#include "regularity_checker.hpp"
#include "CU/types.hpp"

int main()
{
    // --- Empty set ---
    {
        RegularityChecker checker;
        data_t empty;
        assert(checker.is_regular(empty));

        auto analysis = checker.analyze(empty);
        assert(analysis.has_value());
        assert(analysis->period == 1);
        assert(analysis->threshold == 0);
        assert(analysis->finite_count == 0);
        assert(analysis->periodic_count == 0);
    }

    // --- Singleton {0} ---
    {
        RegularityChecker checker;
        data_t data = {0};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->period == 1);
        assert(analysis->threshold == 0);
        assert(analysis->finite_count == 0);
        assert(analysis->periodic_count == 1);
    }

    // --- Singleton {1} ---
    {
        RegularityChecker checker;
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
        RegularityChecker checker;
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
        RegularityChecker checker;
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
        RegularityChecker checker;
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

        RegularityCriteria strict;
        strict.max_finite_to_periodic_percent = 10.0;
        assert(!RegularityChecker(strict).is_regular(data));

        RegularityCriteria loose;
        loose.max_finite_to_periodic_percent = 100.0;
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
        RegularityChecker checker;
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
        RegularityCriteria criteria;
        criteria.max_period = 50;
        RegularityChecker checker(criteria);
        data_t data = {100, 200, 300, 400};
        // p=50 yields threshold=301, ratio=300%.
        assert(!checker.is_regular(data));
    }

    // --- Two elements with a huge gap: {0, 1000} ---
    {
        RegularityChecker checker;
        data_t data = {0, 1000};
        assert(!checker.is_regular(data));
    }

    // --- Period 5: {0,5,10,15,20} ---
    {
        RegularityChecker checker;
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
        RegularityChecker checker;
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
        RegularityChecker checker;
        data_t data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        assert(checker.is_regular(data));

        auto analysis = checker.analyze(data);
        assert(analysis.has_value());
        assert(analysis->period == 1);
        assert(analysis->threshold == 1);
        assert(analysis->periodic_count == 10);
    }

    // --- min_periodic_count ---
    {
        data_t data = {2, 4, 6, 8};

        RegularityCriteria strict;
        strict.min_periodic_count = 5;
        assert(!RegularityChecker(strict).is_regular(data));

        RegularityCriteria ok;
        ok.min_periodic_count = 4;
        assert(RegularityChecker(ok).is_regular(data));
    }

    // --- max_finite_to_periodic_percent boundary ---
    {
        // {1,2,3,5,7,9}: period 2, finite=2, periodic=4, ratio=50%.
        data_t data = {1, 2, 3, 5, 7, 9};

        RegularityCriteria exact;
        exact.max_finite_to_periodic_percent = 50.0;
        assert(RegularityChecker(exact).is_regular(data));

        RegularityCriteria below;
        below.max_finite_to_periodic_percent = 49.0;
        assert(!RegularityChecker(below).is_regular(data));
    }

    // --- max_period = 1 forces period 1 on an even-period set ---
    {
        RegularityCriteria criteria;
        criteria.max_period = 1;
        criteria.max_finite_to_periodic_percent = 1000.0;
        RegularityChecker checker(criteria);

        data_t data = {2, 4, 6, 8};
        // p=1 yields threshold=8, periodic_count=1, ratio=300%.
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
        RegularityCriteria criteria;
        criteria.max_finite_to_periodic_percent = 100.0;
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
        RegularityCriteria criteria;
        criteria.max_finite_to_periodic_percent = 1000.0;
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
            assert(analysis->finite_count + analysis->periodic_count ==
                   s.size());
            assert(analysis->period >= 1);
        }
    }

    // ===== Negative tests: sets that should NOT pass =====

    // --- Negative: large finite prefix, default 10% ratio ---
    {
        // {1,2,3,100}: half_range caps p at 50, so the natural p=97 is not
        // tried. Every tried p leaves 100 alone in the tail:
        // finite=3, periodic=1 → ratio 300% exceeds the 10% default.
        RegularityChecker checker;
        data_t data = {1, 2, 3, 100};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Negative: default criteria rejects 50% finite/periodic ratio ---
    {
        // {1,2,3,5,7,9}: period 2, threshold 3, finite=2, periodic=4, 50%.
        RegularityChecker checker;
        data_t data = {1, 2, 3, 5, 7, 9};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Negative: min_periodic_count not met ---
    {
        RegularityCriteria criteria;
        criteria.min_periodic_count = 3;
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
        RegularityChecker default_checker;
        data_t data = {0, 100, 200, 300, 400};
        assert(default_checker.is_regular(data));

        // With max_period = 50, p=100 is not tried; every smaller p leaves
        // at least 1 element in the finite prefix (ratio >= 25%).
        RegularityCriteria small;
        small.max_period = 50;
        RegularityChecker small_checker(small);
        assert(!small_checker.is_regular(data));
        assert(!small_checker.analyze(data).has_value());
    }

    // --- Negative: default max_period=1024 is not enough for a 2000 gap ---
    {
        // {0, 2000, 4000}: half_range = 2000, p_upper = 1024. The natural
        // period 2000 is not tried, and every smaller period leaves a huge
        // finite prefix (ratio >= 200%).
        RegularityChecker checker;
        data_t data = {0, 2000, 4000};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Negative: zero ratio requires purely periodic set ---
    {
        RegularityCriteria criteria;
        criteria.max_finite_to_periodic_percent = 0.0;
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
        RegularityCriteria criteria;
        criteria.max_finite_to_periodic_percent = 10000.0;
        criteria.min_periodic_count = 10;
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
        RegularityChecker checker;
        data_t data = {1, 2, 3, 4, 5, 100};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Almost period 2: evens with one far outlier ---
    {
        // {2,4,6,8,10,100}: looks like evens, but 100 is isolated.
        // p=2 gives threshold=99, finite=5, periodic=1, ratio=500%.
        RegularityChecker checker;
        data_t data = {2, 4, 6, 8, 10, 100};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Almost period 3: skipped multiple of 3 ---
    {
        // {3,6,9,12,15,21}: 18 is missing, so p=3 breaks at the end.
        // p=3 gives threshold=19, finite=5, periodic=1, ratio=500%.
        RegularityChecker checker;
        data_t data = {3, 6, 9, 12, 15, 21};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Almost period 2: evens with one odd appended ---
    {
        // {0,2,4,6,8,11}: the extra 11 is off-parity with the rest.
        // Every p <= 5 leaves a large finite prefix (ratio >= 500%).
        RegularityChecker checker;
        data_t data = {0, 2, 4, 6, 8, 11};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Geometric progression: no usable period at all ---
    {
        // {1,10,100,1000}: gaps grow by 10x each time; no small period
        // covers the tail. p is capped at half_range = 500, and every
        // candidate leaves almost all elements in the finite prefix.
        RegularityChecker checker;
        data_t data = {1, 10, 100, 1000};
        assert(!checker.is_regular(data));
        assert(!checker.analyze(data).has_value());
    }

    // --- Blocks with a truncated tail: {5,6,7, 15,16,17, 25,27} ---
    {
        // Looks like period-10 blocks of 3, but the last block is wrong.
        RegularityChecker checker;
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

    std::cout << "All RegularityChecker tests passed\n";
    return 0;
}