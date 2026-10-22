#include "random_system_generator.hpp"

#include "CU/equation/parser.hpp"

#include <algorithm>
#include <sstream>
#include <utility>

RandomSystemGenerator::RandomSystemGenerator(GenerationParameters parameters)
    : m_parameters(std::move(parameters))
{
    reseed();
}

const GenerationParameters &RandomSystemGenerator::parameters() const
{
    return m_parameters;
}

void RandomSystemGenerator::set_parameters(GenerationParameters parameters)
{
    m_parameters = std::move(parameters);
    reseed();
}

void RandomSystemGenerator::reseed()
{
    if (m_parameters.seed == 0)
    {
        std::random_device rd;
        std::seed_seq seq{rd(), rd(), rd(), rd()};
        m_rng.seed(seq);
    }
    else
    {
        m_rng.seed(m_parameters.seed);
    }
}

std::vector<std::string> RandomSystemGenerator::generate_equations() const
{
    std::vector<std::string> result;
    result.reserve(m_parameters.num_equations);

    for (std::size_t i = 0; i < m_parameters.num_equations; ++i)
    {
        const std::string name = variable_name(i + 1);
        const std::string expr = generate_expression(1, i + 1);
        result.push_back(name + " = " + expr);
    }
    return result;
}

void RandomSystemGenerator::generate_into(Parser &parser) const
{
    for (const std::string &equation : generate_equations())
    {
        parser.parse_equation(equation);
    }
}

std::string RandomSystemGenerator::generate_expression(
    std::size_t depth, std::size_t defined_count) const
{
    const bool can_expand = depth < m_parameters.max_depth;
    const bool expand = can_expand && roll(m_parameters.internal_node_percent);

    if (!expand)
    {
        if (roll(m_parameters.leaf_variable_percent))
        {
            return generate_variable_reference(defined_count);
        }
        return generate_fixed_set();
    }

    const char op = pick_operator();

    const std::size_t lo =
        std::max<std::size_t>(std::size_t{2}, m_parameters.min_operands);
    const std::size_t hi =
        std::max<std::size_t>(lo, m_parameters.max_operands);
    const std::size_t n = uniform_size(lo, hi);

    std::ostringstream out;
    out << '(';
    for (std::size_t i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            out << ' ' << op << ' ';
        }
        out << generate_expression(depth + 1, defined_count);
    }
    out << ')';
    return out.str();
}

std::string RandomSystemGenerator::generate_fixed_set() const
{
    std::size_t n = uniform_size(
        m_parameters.min_set_size, m_parameters.max_set_size);

    const std::size_t cap =
        static_cast<std::size_t>(m_parameters.max_constant_value) + 1;
    if (n > cap)
    {
        n = cap;
    }

    std::vector<num_t> pool(cap);
    for (std::size_t i = 0; i < cap; ++i)
    {
        pool[i] = static_cast<num_t>(i);
    }
    std::shuffle(pool.begin(), pool.end(), m_rng);
    pool.resize(n);
    std::sort(pool.begin(), pool.end());

    std::ostringstream out;
    out << '{';
    for (std::size_t i = 0; i < pool.size(); ++i)
    {
        if (i > 0)
        {
            out << ", ";
        }
        out << pool[i];
    }
    out << '}';
    return out.str();
}

std::string RandomSystemGenerator::generate_variable_reference(
    std::size_t defined_count) const
{
    const bool has_defined = defined_count > 0;
    const bool has_unknown = m_parameters.num_equations > defined_count;

    bool use_unknown = false;
    if (has_defined && has_unknown)
    {
        use_unknown = roll(m_parameters.leaf_unknown_variable_percent);
    }
    else if (!has_defined && has_unknown)
    {
        use_unknown = true;
    }

    std::size_t index = 1;
    if (use_unknown && has_unknown)
    {
        std::uniform_int_distribution<std::size_t> dist(
            defined_count + 1, m_parameters.num_equations);
        index = dist(m_rng);
    }
    else if (has_defined)
    {
        std::uniform_int_distribution<std::size_t> dist(1, defined_count);
        index = dist(m_rng);
    }
    return variable_name(index);
}

std::string RandomSystemGenerator::variable_name(std::size_t index) const
{
    return m_parameters.variable_prefix + std::to_string(index);
}

char RandomSystemGenerator::pick_operator() const
{
    const unsigned u = m_parameters.operator_union_percent;
    const unsigned s = m_parameters.operator_sum_percent;
    const unsigned i = m_parameters.operator_intersection_percent;
    const unsigned total = u + s + i;

    if (total == 0)
    {
        return '|';
    }

    std::uniform_int_distribution<unsigned> dist(1, total);
    const unsigned r = dist(m_rng);
    if (r <= u)
    {
        return '|';
    }
    if (r <= u + s)
    {
        return '+';
    }
    return '&';
}

std::size_t RandomSystemGenerator::uniform_size(
    std::size_t lo, std::size_t hi) const
{
    if (lo > hi)
    {
        std::swap(lo, hi);
    }
    std::uniform_int_distribution<std::size_t> dist(lo, hi);
    return dist(m_rng);
}

bool RandomSystemGenerator::roll(unsigned percent) const
{
    if (percent >= 100)
    {
        return true;
    }
    if (percent == 0)
    {
        return false;
    }
    std::uniform_int_distribution<unsigned> dist(1, 100);
    return dist(m_rng) <= percent;
}