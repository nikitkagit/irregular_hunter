#include "application/application.hpp"

#include "application/command_parameters_parser.hpp"
#include "application/config_parameters_loader.hpp"
#include "application/config_updater.hpp"
#include "irregular_system_writer.hpp"

#include <chrono>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

Application::Application(std::ostream &out, std::ostream &err,
                         std::string config_path)
    : m_out(out)
    , m_err(err)
    , m_config_path(std::move(config_path))
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

    ConfigUpdater updater(m_config_path);
    if (!updater.update(opts))
    {
        m_err << "Failed to update config file: " << m_config_path << "\n";
        return 1;
    }

    IrregularSystemWriterParameters params;
    try
    {
        ConfigParametersLoader loader(m_config_path);
        params = loader.load();
    }
    catch (const std::exception &e)
    {
        m_err << "Config error: " << e.what() << "\n";
        return 1;
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