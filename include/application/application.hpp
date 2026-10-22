#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <cstdint>
#include <iosfwd>
#include <string>

/**
 * @brief Top-level application logic for irregular_hunter.
 *
 * Wires together CommandParametersParser and IrregularSystemWriter:
 *
 *  1. Parse argc/argv; if -h/--help, print usage and exit.
 *  2. Build IrregularSystemWriterParameters from built-in defaults,
 *     applying command-line overrides.
 *  3. Generate an output filename from the effective seed and a
 *     millisecond timestamp.
 *  4. Run the pipeline and write the report; print the filename to the
 *     configured output stream.
 */
class Application
{
public:
    /**
     * @brief Constructs the application.
     * @param out Stream for normal output (e.g. std::cout).
     * @param err Stream for error messages (e.g. std::cerr).
     */
    Application(std::ostream &out, std::ostream &err);

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
};

#endif // APPLICATION_HPP