#ifndef CONFIG_UPDATER_HPP
#define CONFIG_UPDATER_HPP

#include "application/command_parameters_parser.hpp"

#include <string>

/**
 * @brief Applies command-line overrides to a JSON config file.
 *
 * Loads the existing config (if present), merges every supplied override
 * from CommandParserOptions into it, and writes the file back. Fields that
 * are not present in CommandParserOptions are left untouched, so unrelated
 * config keys survive across runs.
 *
 * An empty file is treated as an empty object. Malformed JSON causes
 * update() to return false without touching the file, so a broken config
 * is never silently overwritten.
 */
class ConfigUpdater
{
public:
    /**
     * @brief Constructs the updater for the given config file.
     * @param config_path Path to the JSON config file.
     */
    explicit ConfigUpdater(std::string config_path);

    /**
     * @brief Merges the supplied overrides and rewrites the config file.
     * @param options Command-line options to merge into the config.
     * @return True on success, false if the file could not be read,
     *         parsed, or written.
     */
    bool update(const CommandParserOptions &options);

private:
    std::string m_config_path;
};

#endif // CONFIG_UPDATER_HPP