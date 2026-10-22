#include "application/command_parameters_parser.hpp"

#include <ostream>
#include <string>

namespace
{
bool parse_u64(const std::string &value, std::uint64_t &out)
{
    std::size_t consumed = 0;
    try
    {
        out = std::stoull(value, &consumed);
    }
    catch (const std::exception &)
    {
        return false;
    }
    return consumed == value.size();
}

bool parse_double(const std::string &value, double &out)
{
    std::size_t consumed = 0;
    try
    {
        out = std::stod(value, &consumed);
    }
    catch (const std::exception &)
    {
        return false;
    }
    return consumed == value.size();
}
} // namespace

bool CommandParametersParser::parse(int argc, char **argv, std::ostream &error)
{
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];

        if (arg == "-n" || arg == "--iterations")
        {
            if (i + 1 >= argc)
            {
                error << "Missing value for " << arg << "\n";
                return false;
            }
            const std::string value = argv[++i];
            std::uint64_t parsed = 0;
            if (!parse_u64(value, parsed))
            {
                error << "Invalid value for " << arg << ": " << value << "\n";
                return false;
            }
            m_options.iterations_override = parsed;
        }
        else if (arg == "-s" || arg == "--seed")
        {
            if (i + 1 >= argc)
            {
                error << "Missing value for " << arg << "\n";
                return false;
            }
            const std::string value = argv[++i];
            std::uint64_t parsed = 0;
            if (!parse_u64(value, parsed))
            {
                error << "Invalid value for " << arg << ": " << value << "\n";
                return false;
            }
            m_options.seed_override = parsed;
        }
        else if (arg == "-e" || arg == "--equations")
        {
            if (i + 1 >= argc)
            {
                error << "Missing value for " << arg << "\n";
                return false;
            }
            const std::string value = argv[++i];
            std::uint64_t parsed = 0;
            if (!parse_u64(value, parsed))
            {
                error << "Invalid value for " << arg << ": " << value << "\n";
                return false;
            }
            m_options.equations_override = static_cast<std::size_t>(parsed);
        }
        else if (arg == "-d" || arg == "--depth")
        {
            if (i + 1 >= argc)
            {
                error << "Missing value for " << arg << "\n";
                return false;
            }
            const std::string value = argv[++i];
            std::uint64_t parsed = 0;
            if (!parse_u64(value, parsed))
            {
                error << "Invalid value for " << arg << ": " << value << "\n";
                return false;
            }
            m_options.depth_override = static_cast<std::size_t>(parsed);
        }
        else if (arg == "-m" || arg == "--max-value")
        {
            if (i + 1 >= argc)
            {
                error << "Missing value for " << arg << "\n";
                return false;
            }
            const std::string value = argv[++i];
            std::uint64_t parsed = 0;
            if (!parse_u64(value, parsed))
            {
                error << "Invalid value for " << arg << ": " << value << "\n";
                return false;
            }
            m_options.max_value_override = parsed;
        }
        else if (arg == "-a" || arg == "--max-attempts")
        {
            if (i + 1 >= argc)
            {
                error << "Missing value for " << arg << "\n";
                return false;
            }
            const std::string value = argv[++i];
            std::uint64_t parsed = 0;
            if (!parse_u64(value, parsed))
            {
                error << "Invalid value for " << arg << ": " << value << "\n";
                return false;
            }
            m_options.max_attempts_override = static_cast<std::size_t>(parsed);
        }
        else if (arg == "-p" || arg == "--period")
        {
            if (i + 1 >= argc)
            {
                error << "Missing value for " << arg << "\n";
                return false;
            }
            const std::string value = argv[++i];
            std::uint64_t parsed = 0;
            if (!parse_u64(value, parsed))
            {
                error << "Invalid value for " << arg << ": " << value << "\n";
                return false;
            }
            m_options.period_override = static_cast<std::size_t>(parsed);
        }
        else if (arg == "-r" || arg == "--ratio")
        {
            if (i + 1 >= argc)
            {
                error << "Missing value for " << arg << "\n";
                return false;
            }
            const std::string value = argv[++i];
            double parsed = 0.0;
            if (!parse_double(value, parsed))
            {
                error << "Invalid value for " << arg << ": " << value << "\n";
                return false;
            }
            m_options.ratio_override = parsed;
        }
        else if (arg == "--no-filter")
        {
            m_options.no_irregular_filter = true;
        }
        else if (arg == "-h" || arg == "--help")
        {
            m_options.help_requested = true;
            return true;
        }
        else
        {
            error << "Unknown argument: " << arg << "\n";
            return false;
        }
    }
    return true;
}

const CommandParserOptions &CommandParametersParser::options() const
{
    return m_options;
}

void CommandParametersParser::print_usage(const char *program,
                                          std::ostream &out) const
{
    out << "Usage: " << program << " [options]\n"
        << "  -s, --seed <S>           Generator seed (0 = random)\n"
        << "  -n, --iterations <N>     Max iterations per system\n"
        << "  -e, --equations <N>      Number of equations\n"
        << "  -d, --depth <N>          Max expression depth\n"
        << "  -m, --max-value <N>      Max constant value in fixed sets\n"
        << "  -a, --max-attempts <N>   Max generation attempts\n"
        << "  -p, --period <N>         Max period to test for regularity\n"
        << "  -r, --ratio <P>          Max finite/periodic ratio, percent\n"
        << "      --no-filter          Accept first system, no irregular "
           "filter\n"
        << "  -h, --help               Show this help message\n";
}