#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "CU/equation/equation_calculator.hpp"
#include "CU/equation/parser.hpp"
#include "CU/equation/variable_dictionary.hpp"

#include "random_system_generator.hpp"

namespace
{
std::size_t count_operators(const std::string &s)
{
    std::size_t count = 0;
    for (char c : s)
    {
        if (c == '|' || c == '+' || c == '&')
        {
            ++count;
        }
    }
    return count;
}

std::string lhs_of(const std::string &equation)
{
    const auto pos = equation.find(" = ");
    return pos == std::string::npos ? equation : equation.substr(0, pos);
}
} // namespace

int main()
{
    // 1. num_equations = 0 produces empty output.
    {
        GenerationParameters p;
        p.seed = 1;
        p.num_equations = 0;
        RandomSystemGenerator gen(p);
        assert(gen.generate_equations().empty());
    }

    // 2. num_equations = 1 produces exactly one equation.
    {
        GenerationParameters p;
        p.seed = 1;
        p.num_equations = 1;
        RandomSystemGenerator gen(p);
        assert(gen.generate_equations().size() == 1);
    }

    // 3. Variable names follow the prefix and are 1-based.
    {
        GenerationParameters p;
        p.seed = 7;
        p.num_equations = 5;
        p.variable_prefix = "X";
        RandomSystemGenerator gen(p);
        const auto eqs = gen.generate_equations();
        assert(eqs.size() == 5);
        for (std::size_t i = 0; i < eqs.size(); ++i)
        {
            const std::string expected = "X" + std::to_string(i + 1);
            assert(lhs_of(eqs[i]) == expected);
        }
    }

    // 4. Same seed -> same output across generator instances.
    {
        GenerationParameters p;
        p.seed = 12345;
        p.num_equations = 10;
        RandomSystemGenerator a(p);
        RandomSystemGenerator b(p);
        assert(a.generate_equations() == b.generate_equations());
    }

    // 5. Different seeds -> different output.
    {
        GenerationParameters p1;
        p1.seed = 1;
        p1.num_equations = 10;

        GenerationParameters p2 = p1;
        p2.seed = 2;

        RandomSystemGenerator g1(p1);
        RandomSystemGenerator g2(p2);
        assert(g1.generate_equations() != g2.generate_equations());
    }

    // 6. Calling generate_equations() twice advances the RNG; reseed()
    //    reproduces the first sequence.
    {
        GenerationParameters p;
        p.seed = 777;
        p.num_equations = 8;
        RandomSystemGenerator gen(p);

        const auto first = gen.generate_equations();
        const auto second = gen.generate_equations();
        assert(first != second);

        gen.reseed();
        assert(gen.generate_equations() == first);
    }

    // 7. set_parameters() reseeds, matching a freshly constructed generator.
    {
        GenerationParameters p;
        p.seed = 42;
        p.num_equations = 6;

        RandomSystemGenerator fresh(p);

        RandomSystemGenerator reused;
        reused.set_parameters(p);

        assert(fresh.generate_equations() == reused.generate_equations());
    }

    // 8. Generated equations parse successfully.
    {
        GenerationParameters p;
        p.seed = 2024;
        p.num_equations = 20;
        RandomSystemGenerator gen(p);
        const auto eqs = gen.generate_equations();

        EquationCalculator calc;
        Parser parser(calc);
        for (const auto &eq : eqs)
        {
            parser.parse_equation(eq);
        }
        assert(calc.equations().size() == 20);
    }

    // 9. generate_into() populates the calculator and the dictionary.
    {
        GenerationParameters p;
        p.seed = 99;
        p.num_equations = 5;
        RandomSystemGenerator gen(p);

        EquationCalculator calc;
        Parser parser(calc);
        gen.generate_into(parser);

        assert(calc.equations().size() == 5);
        assert(parser.variables().variables().size() == 5);
    }

    // 10. Fixed sets respect max_constant_value and max_set_size.
    {
        GenerationParameters p;
        p.seed = 3;
        p.num_equations = 10;
        p.max_depth = 1;              // only leaves
        p.leaf_variable_percent = 0;  // all leaves are fixed sets
        p.max_constant_value = 7;
        p.min_set_size = 1;
        p.max_set_size = 4;
        RandomSystemGenerator gen(p);

        for (const auto &eq : gen.generate_equations())
        {
            const auto open = eq.find('{');
            const auto close = eq.find('}');
            assert(open != std::string::npos);
            assert(close != std::string::npos);

            const std::string body = eq.substr(open + 1, close - open - 1);
            if (body.empty())
            {
                continue;
            }

            std::size_t start = 0;
            std::size_t elements = 0;
            while (start < body.size())
            {
                const auto comma = body.find(',', start);
                const std::string token = comma == std::string::npos
                    ? body.substr(start)
                    : body.substr(start, comma - start);
                start = comma == std::string::npos
                    ? body.size()
                    : comma + 2; // ", "

                const auto b = token.find_first_not_of(' ');
                const auto e = token.find_last_not_of(' ');
                if (b == std::string::npos)
                {
                    continue;
                }
                const std::string trimmed = token.substr(b, e - b + 1);
                const num_t v = static_cast<num_t>(std::stoull(trimmed));
                assert(v <= p.max_constant_value);
                ++elements;
            }
            assert(elements <= p.max_set_size);
        }
    }

    // 11. All declared variables appear in the dictionary.
    {
        GenerationParameters p;
        p.seed = 5;
        p.num_equations = 6;
        p.max_depth = 2;
        p.internal_node_percent = 100;
        p.min_operands = 2;
        p.max_operands = 2;
        p.leaf_variable_percent = 100;
        p.leaf_unknown_variable_percent = 100;
        RandomSystemGenerator gen(p);

        EquationCalculator calc;
        Parser parser(calc);
        gen.generate_into(parser);

        const auto &vars = parser.variables().variables();
        for (std::size_t i = 1; i <= p.num_equations; ++i)
        {
            assert(vars.count("V" + std::to_string(i)) == 1);
        }
    }

    // 12. Forward references are used when leaf_unknown_variable_percent is
    //     forced, and the resulting system still parses.
    {
        GenerationParameters p;
        p.seed = 21;
        p.num_equations = 6;
        p.max_depth = 2;
        p.internal_node_percent = 100;
        p.min_operands = 2;
        p.max_operands = 2;
        p.leaf_variable_percent = 100;
        p.leaf_unknown_variable_percent = 100;
        RandomSystemGenerator gen(p);

        EquationCalculator calc;
        Parser parser(calc);
        for (const auto &eq : gen.generate_equations())
        {
            parser.parse_equation(eq);
        }
        assert(calc.equations().size() == 6);
    }

    // 13. Internal nodes appear when forced.
    {
        GenerationParameters p;
        p.seed = 11;
        p.num_equations = 10;
        p.max_depth = 2;
        p.internal_node_percent = 100;
        RandomSystemGenerator gen(p);

        std::size_t total_ops = 0;
        for (const auto &eq : gen.generate_equations())
        {
            total_ops += count_operators(eq);
        }
        assert(total_ops >= p.num_equations);
    }

    // 14. max_depth = 1 produces leaves only.
    {
        GenerationParameters p;
        p.seed = 12;
        p.num_equations = 10;
        p.max_depth = 1;
        RandomSystemGenerator gen(p);

        for (const auto &eq : gen.generate_equations())
        {
            assert(count_operators(eq) == 0);
        }
    }

    // 15. Union-only operators.
    {
        GenerationParameters p;
        p.seed = 13;
        p.num_equations = 5;
        p.max_depth = 2;
        p.internal_node_percent = 100;
        p.operator_union_percent = 100;
        p.operator_sum_percent = 0;
        p.operator_intersection_percent = 0;
        RandomSystemGenerator gen(p);

        for (const auto &eq : gen.generate_equations())
        {
            assert(eq.find('|') != std::string::npos);
            assert(eq.find('+') == std::string::npos);
            assert(eq.find('&') == std::string::npos);
        }
    }

    // 16. Sum-only operators.
    {
        GenerationParameters p;
        p.seed = 14;
        p.num_equations = 5;
        p.max_depth = 2;
        p.internal_node_percent = 100;
        p.operator_union_percent = 0;
        p.operator_sum_percent = 100;
        p.operator_intersection_percent = 0;
        RandomSystemGenerator gen(p);

        for (const auto &eq : gen.generate_equations())
        {
            assert(eq.find('+') != std::string::npos);
            assert(eq.find('|') == std::string::npos);
            assert(eq.find('&') == std::string::npos);
        }
    }

    // 17. Intersection-only operators.
    {
        GenerationParameters p;
        p.seed = 15;
        p.num_equations = 5;
        p.max_depth = 2;
        p.internal_node_percent = 100;
        p.operator_union_percent = 0;
        p.operator_sum_percent = 0;
        p.operator_intersection_percent = 100;
        RandomSystemGenerator gen(p);

        for (const auto &eq : gen.generate_equations())
        {
            assert(eq.find('&') != std::string::npos);
            assert(eq.find('|') == std::string::npos);
            assert(eq.find('+') == std::string::npos);
        }
    }

    // 18. Generated system iterates without throwing.
    {
        GenerationParameters p;
        p.seed = 16;
        p.num_equations = 10;
        p.max_depth = 3;
        p.max_constant_value = 10;
        RandomSystemGenerator gen(p);

        EquationCalculator calc;
        Parser parser(calc);
        gen.generate_into(parser);

        for (int i = 0; i < 10; ++i)
        {
            calc.iterate();
        }
        assert(calc.equations().size() == 10);
    }

    // 19. parameters() reflects construction values.
    {
        GenerationParameters p;
        p.seed = 123;
        p.num_equations = 7;
        p.variable_prefix = "Q";
        RandomSystemGenerator gen(p);
        assert(gen.parameters().seed == 123);
        assert(gen.parameters().num_equations == 7);
        assert(gen.parameters().variable_prefix == "Q");
    }

    // 20. min_operands == max_operands forces the operand count.
    {
        GenerationParameters p;
        p.seed = 17;
        p.num_equations = 5;
        p.max_depth = 2;
        p.internal_node_percent = 100;
        p.min_operands = 3;
        p.max_operands = 3;
        p.operator_union_percent = 100;
        p.operator_sum_percent = 0;
        p.operator_intersection_percent = 0;
        p.leaf_variable_percent = 0;
        RandomSystemGenerator gen(p);

        // 3 operands -> exactly 2 top-level operators per equation.
        for (const auto &eq : gen.generate_equations())
        {
            assert(count_operators(eq) == 2);
        }
    }

    // 21. Deeper trees produce more operators in total.
    {
        GenerationParameters p;
        p.seed = 18;
        p.num_equations = 30;
        p.max_depth = 3;
        p.internal_node_percent = 100;
        RandomSystemGenerator gen(p);

        std::size_t total = 0;
        for (const auto &eq : gen.generate_equations())
        {
            total += count_operators(eq);
        }
        assert(total > p.num_equations);
    }

    // 22. Balanced parentheses in generated expressions.
    {
        GenerationParameters p;
        p.seed = 19;
        p.num_equations = 20;
        p.max_depth = 3;
        p.internal_node_percent = 100;
        RandomSystemGenerator gen(p);

        for (const auto &eq : gen.generate_equations())
        {
            int depth = 0;
            int min_depth = 0;
            for (char c : eq)
            {
                if (c == '(') { ++depth; }
                if (c == ')') { --depth; min_depth = std::min(min_depth, depth); }
            }
            assert(depth == 0);
            assert(min_depth == 0);
        }
    }

    std::cout << "All RandomSystemGenerator tests passed\n";
    return 0;
}