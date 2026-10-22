#include "application/application.hpp"

#include "application/command_parameters_parser.hpp"
#include "irregular_system_writer.hpp"

#include <chrono>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <string>

Application::Application(std::ostream &out, std::ostream &err)
    : m_out(out)
    , m_err(err)
{
}

std::string Application::generate_output_filename(std::uint64_t seed) const
{
    const auto now = std::chrono::system_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        now.time_since_epoch())
                        .count();

    std::ostringstream out;
    out << "irregular_system_";
    if (seed == 0)
    {
        out << "random";
    }
    else
    {
        out << "seed" << seed;
    }
    out << '_' << ms << ".txt";
    return out.str();
}

int Application::run(int argc, char **argv)
{
    CommandParametersParser cli;
    if (!cli.parse(argc, argv, m_err))
    {
        cli.print_usage(argv[0], m_err);
        return 1;
    }

    const CommandParserOptions &opts = cli.options();

    if (opts.help_requested)
    {
        cli.print_usage(argv[0], m_out);
        return 0;
    }

    IrregularSystemWriterParameters params;

    if (opts.iterations_override.has_value())
    {
        params.max_iterations =
            static_cast<std::size_t>(*opts.iterations_override);
    }
    if (opts.seed_override.has_value())
    {
        params.generator.seed = *opts.seed_override;
    }
    if (opts.equations_override.has_value())
    {
        params.generator.num_equations = *opts.equations_override;
    }
    if (opts.depth_override.has_value())
    {
        params.generator.max_depth = *opts.depth_override;
    }
    if (opts.max_value_override.has_value())
    {
        params.generator.max_constant_value =
            static_cast<num_t>(*opts.max_value_override);
    }
    if (opts.max_attempts_override.has_value())
    {
        params.max_attempts = *opts.max_attempts_override;
    }
    if (opts.period_override.has_value())
    {
        params.criteria.max_period = *opts.period_override;
    }
    if (opts.ratio_override.has_value())
    {
        params.criteria.max_finite_to_periodic_percent =
            *opts.ratio_override;
    }
    if (opts.no_irregular_filter)
    {
        params.require_irregular = false;
    }

    IrregularSystemWriter writer(params);
    const std::string filename =
        generate_output_filename(params.generator.seed);

    try
    {
        if (!writer.write_to_file(filename))
        {
            m_err << "Failed to write report to: " << filename << "\n";
            std::remove(filename.c_str());
            return 1;
        }
    }
    catch (const std::exception &e)
    {
        m_err << "Error while generating report: " << e.what() << "\n";
        std::remove(filename.c_str());
        return 1;
    }

    m_out << "Report written to: " << filename << "\n";
    return 0;
}