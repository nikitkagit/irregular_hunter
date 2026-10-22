#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <cstdint>
#include <iosfwd>
#include <string>

/**
 * @brief Top-level application logic for irregular_hunter.
 *
 * Wires together CommandParametersParser, ConfigUpdater and
 * ConfigParametersLoader:
 *
 *  1. Parse argc/argv; if -h/--help, print usage and exit.
 *  2. Apply command-line overrides to the config file via ConfigUpdater,
 *     creating the file if it does not exist.
 *  3. Load IrregularSystemWriterParameters from the (now-updated) config
 *     file via ConfigParametersLoader.
 *  4. Generate an output filename from the effective seed and a
 *     millisecond timestamp.
 *  5. Run the pipeline and write the report; print the filename to the
 *     configured output stream.
 *
 * The config file path is supplied by the constructor and is the only
 * place where the file name appears; every other component receives it
 * via its own constructor.
 */
class Application
{
public:
    /**
     * @brief Constructs the application.
     * @param out Stream for normal output (e.g. std::cout).
     * @param err Stream for error messages (e.g. std::cerr).
     * @param config_path Path to the JSON config file.
     */
    Application(std::ostream &out, std::ostream &err,
                std::string config_path = "config.json");

    /**
     * @brief Runs the application.
     * @param argc Argument count from main().
     * @param argv Argument vector from main().
     * @return Process exit code: 0 on success, non-zero on failure.
     */
    int run(int argc, char **argv);

private:
    /**
     * @brief Builds an output filename from the seed and current time.
     * @param seed Generator seed (0 means "random").
     */
    std::string generate_output_filename(std::uint64_t seed) const;

    std::ostream &m_out;
    std::ostream &m_err;
    std::string m_config_path;
};

#endif // APPLICATION_HPP