#ifndef COMMAND_PARAMETERS_PARSER_HPP
#define COMMAND_PARAMETERS_PARSER_HPP

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>

/**
 * @brief Parsed command-line options.
 *
 * Every optional field overrides the corresponding default in
 * IrregularSystemWriterParameters. Options that are not supplied keep the
 * built-in default value.
 *
 * The output filename is not exposed on the command line: it is generated
 * by the application from the effective seed and the current timestamp.
 */
struct CommandParserOptions
{
    /// Override for IrregularSystemWriterParameters::max_iterations.
    std::optional<unsigned long long> iterations_override;

    /// Override for IrregularSystemWriterParameters::generator.seed.
    std::optional<std::uint64_t> seed_override;

    /// Override for GenerationParameters::num_equations.
    std::optional<std::size_t> equations_override;

    /// Override for GenerationParameters::max_depth.
    std::optional<std::size_t> depth_override;

    /// Override for GenerationParameters::max_constant_value.
    std::optional<unsigned long long> max_value_override;

    /// Override for IrregularSystemWriterParameters::max_attempts.
    std::optional<std::size_t> max_attempts_override;

    /// Override for RegularityCriteria::max_period.
    std::optional<std::size_t> period_override;

    /// Override for RegularityCriteria::max_finite_to_periodic_percent.
    std::optional<double> ratio_override;

    /// When true, forces IrregularSystemWriterParameters::require_irregular
    /// to false, so the very first generated system is reported regardless
    /// of its regularity.
    bool no_irregular_filter = false;

    /// True if @c -h/--help was given.
    bool help_requested = false;
};

/**
 * @brief Parses argc/argv into CommandParserOptions.
 *
 * On error, writes a descriptive message to the provided error stream and
 * returns false. Unknown flags and missing values are rejected.
 */
class CommandParametersParser
{
public:
    /**
     * @brief Parses the given argument list.
     * @param argc Argument count from main().
     * @param argv Argument vector from main().
     * @param error Stream for error messages.
     * @return True on success or when help was requested; false on error.
     */
    bool parse(int argc, char **argv, std::ostream &error);

    /// @brief Returns the parsed options.
    const CommandParserOptions &options() const;

    /// @brief Prints the usage message.
    void print_usage(const char *program, std::ostream &out) const;

private:
    CommandParserOptions m_options;
};

#endif // COMMAND_PARAMETERS_PARSER_HPP