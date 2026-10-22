#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include "regularity_checker.hpp"
#include "CU/types.hpp"

namespace
{
data_t make_set(std::initializer_list<num_t> values)
{
    data_t s;
    for (num_t v : values)
    {
        s.insert(v);
    }
    return s;
}

// Every field is set explicitly so the tests never depend on defaults.
RegularityCriteria criteria(
    double max_finite_percent,
    std::size_t max_period,
    std::size_t min_periodic_count,
    num_t ignore_below,
    double tail_percent,
    std::size_t min_period_additions = 0)
{
    RegularityCriteria c;
    c.max_finite_to_periodic_percent = max_finite_percent;
    c.max_period = max_period;
    c.min_periodic_count = min_periodic_count;
    c.ignore_below = ignore_below;
    c.tail_percent = tail_percent;
    c.min_period_additions = min_period_additions;
    return c;
}

data_t evens_up_to(num_t max)
{
    data_t s;
    for (num_t i = 0; i <= max; i += 2)
    {
        s.insert(i);
    }
    return s;
}

data_t multiples_up_to(num_t step, num_t max)
{
    data_t s;
    for (num_t i = 0; i <= max; i += step)
    {
        s.insert(i);
    }
    return s;
}

// Clearly irregular fixtures, all with more than 10 elements.
data_t fibonacci()
{
    return make_set({1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233});
}

data_t powers_of_two()
{
    return make_set({1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024});
}

data_t block_with_outlier()
{
    return make_set({0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 1000});
}
} // namespace

int main()
{
    // ===== Empty set =====
    {
        RegularityChecker checker(criteria(10.0, 1024, 1, 0, 0.0));
        data_t empty;
        assert(checker.is_regular(empty));
        auto a = checker.analyze(empty);
        assert(a.has_value());
        assert(a->period == 1);
        assert(a->min_periodic_element == 0);
        assert(a->finite_count == 0);
        assert(a->periodic_count == 0);
        assert(a->ignored_count == 0);
    }

    // ===== Purely periodic positive baseline =====
    // S = evens 0..30, 16 elements.
    {
        data_t s = evens_up_to(30);
        assert(s.size() == 16);
        RegularityChecker checker(criteria(0.0, 1024, 1, 0, 0.0));
        assert(checker.is_regular(s));
        auto a = checker.analyze(s);
        assert(a.has_value());
        assert(a->period == 2);
        assert(a->min_periodic_element == 0);
        assert(a->finite_count == 0);
        assert(a->periodic_count == 16);
        assert(a->finite_to_periodic_percent == 0.0);
    }

    // ===== max_finite_to_periodic_percent =====
    // S = evens 0..30 ∪ {15}, 17 elements, max = 30.
    // p = 2: finite = {0,2,4,6,8,10,12,14,15} (9), periodic = 8,
    // ratio = 112.5%.
    {
        data_t s = evens_up_to(30);
        s.insert(15);
        assert(s.size() == 17);

        RegularityChecker reject(criteria(100.0, 1024, 2, 0, 0.0));
        assert(!reject.is_regular(s));
        assert(!reject.analyze(s).has_value());

        RegularityChecker accept(criteria(200.0, 1024, 7, 0, 0.0));
        assert(accept.is_regular(s));
        auto a = accept.analyze(s);
        assert(a.has_value());
        assert(a->period == 2);
        assert(a->min_periodic_element == 16);
        assert(a->finite_count == 9);
        assert(a->periodic_count == 8);
        assert(a->finite_to_periodic_percent > 112.0);
        assert(a->finite_to_periodic_percent < 113.0);
    }

    // ===== max_period =====
    // S = multiples of 15 in [0, 150], 11 elements.
    // Only p = 15 gives a perfect fit.
    {
        data_t s = multiples_up_to(15, 150);
        assert(s.size() == 11);

        RegularityChecker tight(criteria(100.0, 14, 1, 0, 0.0));
        assert(!tight.is_regular(s));

        RegularityChecker ok(criteria(100.0, 15, 1, 0, 0.0));
        assert(ok.is_regular(s));
        auto a = ok.analyze(s);
        assert(a.has_value());
        assert(a->period == 15);
        assert(a->min_periodic_element == 0);
        assert(a->finite_count == 0);
    }

    // ===== min_periodic_count =====
    // Same set as max_finite test: p = 2 leaves a periodic tail of 8.
    {
        data_t s = evens_up_to(30);
        s.insert(15);

        RegularityChecker tight(criteria(200.0, 1024, 9, 0, 0.0));
        assert(!tight.is_regular(s));

        RegularityChecker ok(criteria(200.0, 1024, 8, 0, 0.0));
        assert(ok.is_regular(s));
        auto a = ok.analyze(s);
        assert(a.has_value());
        assert(a->periodic_count == 8);
    }

    // ===== ignore_below =====
    // S = evens 0..30 ∪ {3}, 17 elements. Element 3 is noise.
    {
        data_t s = evens_up_to(30);
        s.insert(3);
        assert(s.size() == 17);

        RegularityChecker baseline(criteria(10.0, 1024, 1, 0, 0.0));
        assert(!baseline.is_regular(s));

        RegularityChecker filtered(criteria(10.0, 1024, 1, 4, 0.0));
        assert(filtered.is_regular(s));
        auto a = filtered.analyze(s);
        assert(a.has_value());
        assert(a->ignored_count == 3);
        assert(a->period == 2);
        assert(a->finite_count == 0);
    }

    // ===== tail_percent =====
    // S = evens 0..30 ∪ {33}, 17 elements, max = 33.
    {
        data_t s = evens_up_to(30);
        s.insert(33);
        assert(s.size() == 17);

        RegularityChecker strict(criteria(100.0, 1024, 1, 0, 0.0));
        assert(!strict.is_regular(s));

        RegularityChecker loose(criteria(100.0, 1024, 1, 0, 10.0));
        assert(loose.is_regular(s));
        auto a = loose.analyze(s);
        assert(a.has_value());
        assert(a->period == 2);
        assert(a->finite_count == 0);
        assert(a->periodic_count == 17);
    }

    // ===== min_period_additions (small) =====
    // S = {0, 100, 200}, p = 100. Chain from 0 = 3 elements, 2 additions.
    {
        data_t s = make_set({0, 100, 200});
        assert(s.size() == 3);

        RegularityChecker ok(
            criteria(10.0, 1024, 1, 0, 0.0, 2));
        assert(ok.is_regular(s));
        auto a = ok.analyze(s);
        assert(a.has_value());
        assert(a->period == 100);
        assert(a->finite_count == 0);

        RegularityChecker tight(
            criteria(10.0, 1024, 1, 0, 0.0, 3));
        assert(!tight.is_regular(s));
    }

    // ===== min_period_additions (large set) =====
    // S = multiples of 100 up to 1000, 11 elements.
    // Chain from 0 = 11 elements, 10 additions.
    {
        data_t s = multiples_up_to(100, 1000);
        assert(s.size() == 11);

        RegularityChecker ok(
            criteria(10.0, 1024, 1, 0, 0.0, 10));
        assert(ok.is_regular(s));
        auto a = ok.analyze(s);
        assert(a.has_value());
        assert(a->period == 100);
        assert(a->finite_count == 0);
        assert(a->periodic_count == 11);

        RegularityChecker tight(
            criteria(10.0, 1024, 1, 0, 0.0, 11));
        assert(!tight.is_regular(s));
    }

    // ===== min_period_additions with a gap =====
    // S = multiples of 100 up to 500 ∪ {900, 1000}.
    // Longest chain from 0 is 6 elements (0..500), 5 additions.
    {
        data_t s = multiples_up_to(100, 500);
        s.insert(900);
        s.insert(1000);
        assert(s.size() == 8);

        RegularityChecker ok(
            criteria(500.0, 1024, 1, 0, 20.0, 5));
        assert(ok.is_regular(s));

        RegularityChecker tight(
            criteria(500.0, 1024, 1, 0, 20.0, 6));
        assert(!tight.is_regular(s));
    }

    // ===== All criteria together on a messy set =====
    // S = {1, 2, 4, 6, ..., 30} ∪ {33}.
    {
        data_t s = evens_up_to(30);
        s.erase(0);
        s.insert(1);
        s.insert(33);
        assert(s.size() == 17);

        RegularityChecker messy(criteria(5.0, 1024, 1, 0, 0.0, 0));
        assert(!messy.is_regular(s));

        RegularityChecker tailored(
            criteria(50.0, 1024, 10, 2, 10.0, 0));
        assert(tailored.is_regular(s));
        auto a = tailored.analyze(s);
        assert(a.has_value());
        assert(a->ignored_count == 1);
        assert(a->finite_count == 0);
    }

    // ===== Clearly irregular fixtures rejected under many criteria =====
    const std::vector<data_t> irregular = {
        fibonacci(),
        powers_of_two(),
        block_with_outlier(),
    };
    for (const auto &s : irregular)
    {
        assert(s.size() > 10);
    }

    {
        RegularityChecker strict(criteria(10.0, 1024, 1, 0, 0.0));
        for (const auto &s : irregular)
        {
            assert(!strict.is_regular(s));
            assert(!strict.analyze(s).has_value());
        }
    }
    {
        RegularityChecker moderate(criteria(50.0, 1024, 1, 0, 5.0));
        for (const auto &s : irregular)
        {
            assert(!moderate.is_regular(s));
            assert(!moderate.analyze(s).has_value());
        }
    }
    {
        RegularityChecker loose(criteria(500.0, 1024, 1, 0, 20.0, 3));
        assert(!loose.is_regular(fibonacci()));
        assert(!loose.is_regular(powers_of_two()));
        assert(!loose.is_regular(block_with_outlier()));
    }

    // ===== Accounting invariants across criteria and fixtures =====
    {
        const std::vector<data_t> fixtures = {
            evens_up_to(30),
            multiples_up_to(15, 150),
            fibonacci(),
            powers_of_two(),
            block_with_outlier(),
        };
        for (const auto &s : fixtures)
        {
            for (double max_finite : {10.0, 100.0, 1000.0})
            {
                for (double tail : {0.0, 5.0, 20.0})
                {
                    for (num_t below : {num_t{0}, num_t{5}, num_t{100}})
                    {
                        for (std::size_t add :
                             {std::size_t{0}, std::size_t{3}})
                        {
                            RegularityChecker checker(criteria(
                                max_finite, 1024, 1, below, tail, add));
                            const auto analysis = checker.analyze(s);
                            if (!analysis.has_value())
                            {
                                continue;
                            }
                            assert(analysis->finite_count +
                                       analysis->periodic_count +
                                       analysis->ignored_count ==
                                   s.size());
                            assert(analysis->ignored_count <= s.size());
                            assert(analysis->period >= 1);
                        }
                    }
                }
            }
        }
    }

    std::cout << "All RegularityChecker tests passed\n";
    return 0;
}