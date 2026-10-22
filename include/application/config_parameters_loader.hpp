#ifndef CONFIG_PARAMETERS_LOADER_HPP
#define CONFIG_PARAMETERS_LOADER_HPP

#include "irregular_system_writer_parameters.hpp"

#include <string>

/**
 * @brief Builds IrregularSystemWriterParameters from a JSON config file.
 *
 * If the file does not exist, or if a given field is absent, the
 * corresponding default from IrregularSystemWriterParameters is used.
 * Malformed JSON, or a recognized field with the wrong type, causes
 * load() to throw std::runtime_error.
 */
class ConfigParametersLoader
{
public:
    /**
     * @brief Constructs the loader for the given config file.
     * @param config_path Path to the JSON config file.
     */
    explicit ConfigParametersLoader(std::string config_path);

    /**
     * @brief Loads the parameters from the config file.
     * @return Parameters with defaults for missing keys.
     * @throws std::runtime_error on malformed JSON or wrong field types.
     */
    IrregularSystemWriterParameters load() const;

private:
    std::string m_config_path;
};

#endif // CONFIG_PARAMETERS_LOADER_HPP