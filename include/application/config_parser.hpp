#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include "irregular_system_writer.hpp"

#include <nlohmann/json.hpp>

#include <string>

/**
 * @brief Converts a parsed JSON document into pipeline parameters.
 *
 * Recognized top-level keys:
 * @code
 * {
 *   "generator": { ... GenerationParameters fields ... },
 *   "criteria":  { ... RegularityCriteria fields ... },
 *   "max_iterations": 1000,
 *   "require_irregular": true,
 *   "max_attempts": 100
 * }
 * @endcode
 * Missing fields keep their default values from
 * IrregularSystemWriterParameters. Unknown fields are ignored.
 *
 * @throws std::runtime_error if a recognized field has the wrong type or
 *         a container field (e.g. "generator") is not an object.
 */
class ConfigParser
{
public:
    static IrregularSystemWriterParameters parse(const nlohmann::json &json);
    static IrregularSystemWriterParameters parse_string(const std::string &text);
    static IrregularSystemWriterParameters parse_file(const std::string &path);
};

#endif // CONFIG_PARSER_HPP