#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "irregular_system_writer.hpp"

namespace
{
bool contains(const std::string &haystack, const std::string &needle)
{
    return haystack.find(needle) != std::string::npos;
}

std::size_t count_nonempty_lines_between(const std::string &text,
                                         const std::string &begin,
                                         const std::string &end)
{
    const auto b = text.find(begin);
    if (b == std::string::npos)
    {
        return 0;
    }
    const auto content_start = b + begin.size();
    const auto e = text.find(end, content_start);
    if (e == std::string::npos)
    {
        return 0;
    }

    std::size_t count = 0;
    std::istringstream in(text.substr(content_start, e - content_start));
    std::string line;
    while (std::getline(in, line))
    {
        if (!line.empty())
        {
            ++count;
        }
    }
    return count;
}

std::size_t count_solution_rows(const std::string &text)
{
    const auto b = text.find("=== Solution ===");
    if (b == std::string::npos)
    {
        return 0;
    }
    const auto e = text.find("=== Summary ===", b);
    if (e == std::string::npos)
    {
        return 0;
    }

    std::size_t count = 0;
    std::istringstream in(text.substr(b, e - b));
    std::string line;
    while (std::getline(in, line))
    {
        if (line.find(" | ") != std::string::npos)
        {
            ++count;
        }
    }
    return count;
}

IrregularSystemWriterParameters make_params()
{
    IrregularSystemWriterParameters params;
    params.criteria.max_finite_to_periodic_percent = 10.0;
    params.criteria.max_period = 1024;
    params.criteria.min_periodic_count = 1;
    return params;
}
} // namespace

int main()
{
    // 1. Explicit parameter round-trip through parameters().
    {
        auto params = make_params();
        params.require_irregular = true;
        params.max_iterations = 1000;
        params.max_attempts = 100;

        IrregularSystemWriter writer(params);
        assert(writer.parameters().require_irregular == true);
        assert(writer.parameters().max_iterations == 1000);
        assert(writer.parameters().max_attempts == 100);
        assert(writer.parameters().criteria.max_finite_to_periodic_percent
               == 10.0);
        assert(writer.parameters().criteria.max_period == 1024);
        assert(writer.parameters().criteria.min_periodic_count == 1);
    }

    // 2. set_parameters() / parameters().
    {
        auto params = make_params();
        params.generator.seed = 7;
        params.generator.num_equations = 3;
        params.require_irregular = false;
        params.max_iterations = 42;
        params.max_attempts = 9;

        IrregularSystemWriter writer(make_params());
        writer.set_parameters(params);

        assert(writer.parameters().generator.seed == 7);
        assert(writer.parameters().generator.num_equations == 3);
        assert(writer.parameters().require_irregular == false);
        assert(writer.parameters().max_iterations == 42);
        assert(writer.parameters().max_attempts == 9);
    }

    // 3. format() returns text with all sections.
    {
        auto params = make_params();
        params.generator.seed = 1;
        params.generator.num_equations = 3;
        params.require_irregular = false;

        IrregularSystemWriter writer(params);
        const std::string report = writer.format();

        assert(!report.empty());
        assert(contains(report, "=== Generation ==="));
        assert(contains(report, "=== System ==="));
        assert(contains(report, "=== Solution ==="));
        assert(contains(report, "=== Summary ==="));
    }

    // 4. write_to_file writes the same content as format().
    {
        const char *path = "test_isw_output.txt";
        std::remove(path);

        auto params = make_params();
        params.generator.seed = 2;
        params.generator.num_equations = 4;
        params.require_irregular = false;

        IrregularSystemWriter writer(params);
        assert(writer.write_to_file(path));

        std::ifstream in(path);
        std::ostringstream ss;
        ss << in.rdbuf();
        std::remove(path);

        assert(ss.str() == writer.format());
    }

    // 5. write_to_file to an invalid path returns false.
    {
        auto params = make_params();
        params.generator.seed = 3;
        params.generator.num_equations = 2;
        params.require_irregular = false;

        IrregularSystemWriter writer(params);
        assert(!writer.write_to_file("no_such_dir_xyz/out.txt"));
    }

    // 6. require_irregular = false: exactly one attempt is made.
    {
        auto params = make_params();
        params.generator.seed = 5;
        params.generator.num_equations = 3;
        params.require_irregular = false;
        params.max_attempts = 100;

        IrregularSystemWriter writer(params);
        const std::string report = writer.format();
        assert(contains(report, "attempts_used: 1"));
        assert(!contains(report, "NOTE:"));
    }

    // 7. Fixed seed is reflected in the report.
    {
        auto params = make_params();
        params.generator.seed = 42;
        params.generator.num_equations = 2;
        params.require_irregular = false;

        IrregularSystemWriter writer(params);
        assert(contains(writer.format(), "seed: 42"));
    }

    // 8. seed = 0 is reported as random.
    {
        auto params = make_params();
        params.generator.seed = 0;
        params.generator.num_equations = 2;
        params.require_irregular = false;

        IrregularSystemWriter writer(params);
        assert(contains(writer.format(), "seed: 0 (random)"));
    }

    // 9. Determinism with a fixed seed.
    {
        auto params = make_params();
        params.generator.seed = 12345;
        params.generator.num_equations = 4;
        params.require_irregular = false;

        IrregularSystemWriter writer(params);
        assert(writer.format() == writer.format());
    }

    // 10. require_irregular = true with parameters that produce irregular
    //     values with overwhelming probability.
    {
        auto params = make_params();
        params.generator.seed = 1;
        params.generator.num_equations = 5;
        params.generator.max_depth = 1;
        params.generator.leaf_variable_percent = 0;
        params.generator.min_set_size = 2;
        params.generator.max_set_size = 2;
        params.generator.max_constant_value = 100;
        params.require_irregular = true;
        params.max_attempts = 50;

        IrregularSystemWriter writer(params);
        const std::string report = writer.format();
        assert(!contains(report, "NOTE:"));
        assert(contains(report, "IRREGULAR"));
    }

    // 11. require_irregular = true with parameters that produce only
    //     regular values: emits NOTE and reports the attempt budget.
    {
        auto params = make_params();
        params.generator.seed = 1;
        params.generator.num_equations = 3;
        params.generator.max_depth = 1;
        params.generator.leaf_variable_percent = 0;
        params.generator.min_set_size = 1;
        params.generator.max_set_size = 1;
        params.generator.max_constant_value = 5;
        params.require_irregular = true;
        params.max_attempts = 3;

        IrregularSystemWriter writer(params);
        const std::string report = writer.format();
        assert(contains(report, "NOTE:"));
        assert(contains(report, "attempts_used: 3"));
    }

    // 12. max_iterations = 0: no iterations, not converged.
    {
        auto params = make_params();
        params.generator.seed = 4;
        params.generator.num_equations = 2;
        params.require_irregular = false;
        params.max_iterations = 0;

        IrregularSystemWriter writer(params);
        const std::string report = writer.format();
        assert(contains(report, "iterations_done: 0"));
        assert(contains(report, "converged: no"));
    }

    // 13. max_depth = 1 with constants only: converges within a few
    //     iterations.
    {
        auto params = make_params();
        params.generator.seed = 5;
        params.generator.num_equations = 3;
        params.generator.max_depth = 1;
        params.generator.leaf_variable_percent = 0;
        params.require_irregular = false;
        params.max_iterations = 1000;

        IrregularSystemWriter writer(params);
        const std::string report = writer.format();
        assert(contains(report, "converged: yes"));
    }

    // 14. Number of System lines equals num_equations.
    {
        auto params = make_params();
        params.generator.seed = 6;
        params.generator.num_equations = 5;
        params.require_irregular = false;

        IrregularSystemWriter writer(params);
        const std::string report = writer.format();

        const std::size_t lines = count_nonempty_lines_between(
            report, "=== System ===\n", "=== Solution ===");
        assert(lines == 5);
    }

    // 15. Number of Solution rows equals num_equations + 1 (header).
    {
        auto params = make_params();
        params.generator.seed = 7;
        params.generator.num_equations = 4;
        params.require_irregular = false;

        IrregularSystemWriter writer(params);
        const std::string report = writer.format();

        assert(count_solution_rows(report) == 5);
    }

    // 16. Report mentions every variable name.
    {
        auto params = make_params();
        params.generator.seed = 8;
        params.generator.num_equations = 3;
        params.require_irregular = false;
        params.max_iterations = 10;

        IrregularSystemWriter writer(params);
        const std::string report = writer.format();
        assert(contains(report, "V1"));
        assert(contains(report, "V2"));
        assert(contains(report, "V3"));
    }

    // 17. max_attempts = 0 is clamped to 1.
    {
        auto params = make_params();
        params.generator.seed = 9;
        params.generator.num_equations = 2;
        params.require_irregular = true;
        params.max_attempts = 0;

        IrregularSystemWriter writer(params);
        const std::string report = writer.format();
        assert(contains(report, "attempts_used: 1"));
    }

    // 18. Different seeds yield different reports.
    {
        auto a = make_params();
        a.generator.seed = 100;
        a.generator.num_equations = 5;
        a.require_irregular = false;

        auto b = a;
        b.generator.seed = 200;

        IrregularSystemWriter wa(a);
        IrregularSystemWriter wb(b);
        assert(wa.format() != wb.format());
    }

    // 19. Summary line contains the irregular/total counter.
    {
        auto params = make_params();
        params.generator.seed = 11;
        params.generator.num_equations = 3;
        params.require_irregular = false;

        IrregularSystemWriter writer(params);
        const std::string report = writer.format();
        assert(contains(report, "irregular variables: "));
        assert(contains(report, " / 3"));
    }

    std::cout << "All IrregularSystemWriter tests passed\n";
    return 0;
}