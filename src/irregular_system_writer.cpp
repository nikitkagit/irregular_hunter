#include "irregular_system_writer.hpp"

#include "random_system_generator.hpp"
#include "regularity_checker.hpp"

#include "CU/equation/equation_calculator.hpp"
#include "CU/equation/parser.hpp"
#include "CU/equation/variable_dictionary.hpp"
#include "CU/expression/variable_expression.hpp"

#include <algorithm>
#include <fstream>
#include <optional>
#include <sstream>
#include <utility>
#include <vector>

namespace
{
std::string format_set(const data_t &set)
{
    std::ostringstream out;
    out << '{';
    bool first = true;
    for (const num_t &v : set)
    {
        if (!first)
        {
            out << ", ";
        }
        out << v;
        first = false;
    }
    out << '}';
    return out.str();
}

std::string pad_right(std::string s, std::size_t width)
{
    if (s.size() < width)
    {
        s.append(width - s.size(), ' ');
    }
    return s;
}

std::string snapshot(const VariableDictionary &dictionary)
{
    std::ostringstream out;
    for (const auto &entry : dictionary.variables())
    {
        out << format_set(entry.second->data()) << ';';
    }
    return out.str();
}

struct VariableResult
{
    std::string name;
    data_t value;
    std::optional<RegularityChecker::Analysis> analysis;
};

struct SystemResult
{
    std::uint64_t seed_used = 0;
    std::size_t iterations_done = 0;
    bool converged = false;
    std::vector<std::string> equations;
    std::vector<VariableResult> variables;

    std::size_t irregular_count() const
    {
        std::size_t count = 0;
        for (const auto &v : variables)
        {
            if (!v.analysis.has_value())
            {
                ++count;
            }
        }
        return count;
    }
};

SystemResult run_attempt(const GenerationParameters &gen_params,
                         const RegularityChecker &checker,
                         std::size_t max_iterations,
                         std::uint64_t seed_override)
{
    GenerationParameters p = gen_params;
    p.seed = seed_override;

    RandomSystemGenerator generator(p);

    SystemResult result;
    result.seed_used = seed_override;
    result.equations = generator.generate_equations();

    EquationCalculator calculator;
    Parser parser(calculator);
    for (const std::string &equation : result.equations)
    {
        parser.parse_equation(equation);
    }

    std::string previous_state = snapshot(parser.variables());
    for (std::size_t i = 0; i < max_iterations; ++i)
    {
        calculator.iterate();
        ++result.iterations_done;
        std::string current_state = snapshot(parser.variables());
        if (current_state == previous_state)
        {
            result.converged = true;
            break;
        }
        previous_state = std::move(current_state);
    }

    for (const auto &entry : parser.variables().variables())
    {
        VariableResult vr;
        vr.name = entry.first;
        vr.value = entry.second->data();
        vr.analysis = checker.analyze(vr.value);
        result.variables.push_back(std::move(vr));
    }

    return result;
}
} // namespace

IrregularSystemWriter::IrregularSystemWriter(
    IrregularSystemWriterParameters params)
    : m_params(std::move(params))
{
}

const IrregularSystemWriterParameters &IrregularSystemWriter::parameters() const
{
    return m_params;
}

void IrregularSystemWriter::set_parameters(IrregularSystemWriterParameters params)
{
    m_params = std::move(params);
}

bool IrregularSystemWriter::write_to_file(const std::string &path) const
{
    std::ofstream file(path);
    if (!file)
    {
        return false;
    }
    file << format();
    return static_cast<bool>(file);
}

std::string IrregularSystemWriter::format() const
{
    const RegularityChecker checker(m_params.criteria);

    std::size_t attempts =
        m_params.require_irregular ? m_params.max_attempts : 1;
    if (attempts == 0)
    {
        attempts = 1;
    }

    SystemResult selected;
    bool found = false;
    std::size_t attempts_taken = 0;

    for (std::size_t i = 0; i < attempts; ++i)
    {
        ++attempts_taken;

        std::uint64_t seed_override;
        if (m_params.generator.seed == 0)
        {
            // Fresh random seed for every attempt.
            seed_override = 0;
        }
        else
        {
            // Deterministic sequence: seed, seed+1, seed+2, ...
            seed_override = m_params.generator.seed + i;
        }

        selected = run_attempt(m_params.generator, checker,
                               m_params.max_iterations, seed_override);

        if (!m_params.require_irregular || selected.irregular_count() > 0)
        {
            found = true;
            break;
        }
    }

    std::ostringstream out;

    if (m_params.require_irregular && !found)
    {
        out << "NOTE: no system with an irregular variable was found after "
            << attempts_taken << " attempt(s). "
            << "Showing the last attempt.\n\n";
    }

    out << "=== Generation ===\n"
        << "seed: " << selected.seed_used
        << (selected.seed_used == 0 ? " (random)" : "") << '\n'
        << "num_equations: " << selected.equations.size() << '\n'
        << "max_iterations: " << m_params.max_iterations << '\n'
        << "iterations_done: " << selected.iterations_done << '\n'
        << "converged: " << (selected.converged ? "yes" : "no") << '\n'
        << "require_irregular: "
        << (m_params.require_irregular ? "yes" : "no") << '\n'
        << "attempts_used: " << attempts_taken << '\n';

    out << "\n=== System ===\n";
    for (const std::string &equation : selected.equations)
    {
        out << equation << '\n';
    }

    std::size_t name_width = 4;   // "Name"
    std::size_t value_width = 5;  // "Value"
    for (const auto &v : selected.variables)
    {
        name_width = std::max(name_width, v.name.size());
        value_width = std::max(value_width, format_set(v.value).size());
    }

    out << "\n=== Solution ===\n";
    out << pad_right("Name", name_width) << " | "
        << pad_right("Value", value_width) << " | "
        << "Regularity\n";
    out << std::string(name_width, '-') << "-+-"
        << std::string(value_width, '-') << "-+-"
        << "----------\n";

    for (const auto &v : selected.variables)
    {
        out << pad_right(v.name, name_width) << " | "
            << pad_right(format_set(v.value), value_width) << " | ";
        if (v.analysis.has_value())
        {
            out << "regular (period=" << v.analysis->period
                << ", min_periodic_element="
                << v.analysis->min_periodic_element << ')';
        }
        else
        {
            out << "IRREGULAR";
        }
        out << '\n';
    }

    out << "\n=== Summary ===\n"
        << "irregular variables: " << selected.irregular_count()
        << " / " << selected.variables.size() << '\n';

    return out.str();
}