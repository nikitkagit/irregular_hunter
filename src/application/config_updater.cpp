#include "application/config_updater.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <istream>
#include <string>
#include <utility>

ConfigUpdater::ConfigUpdater(std::string config_path)
    : m_config_path(std::move(config_path))
{
}

bool ConfigUpdater::update(const CommandParserOptions &options)
{
    nlohmann::json config;

    std::ifstream in(m_config_path);
    if (in && in.peek() != std::ifstream::traits_type::eof())
    {
        try
        {
            in >> config;
        }
        catch (const std::exception &)
        {
            return false;
        }
    }
    if (!config.is_object())
    {
        config = nlohmann::json::object();
    }

    if (options.seed_override.has_value())
    {
        config["generator"]["seed"] = *options.seed_override;
    }
    if (options.equations_override.has_value())
    {
        config["generator"]["num_equations"] = *options.equations_override;
    }
    if (options.depth_override.has_value())
    {
        config["generator"]["max_depth"] = *options.depth_override;
    }
    if (options.max_value_override.has_value())
    {
        config["generator"]["max_constant_value"] =
            *options.max_value_override;
    }
    if (options.iterations_override.has_value())
    {
        config["max_iterations"] = *options.iterations_override;
    }
    if (options.max_attempts_override.has_value())
    {
        config["max_attempts"] = *options.max_attempts_override;
    }
    if (options.period_override.has_value())
    {
        config["criteria"]["max_period"] = *options.period_override;
    }
    if (options.ratio_override.has_value())
    {
        config["criteria"]["max_finite_to_periodic_percent"] =
            *options.ratio_override;
    }
    if (options.no_irregular_filter)
    {
        config["require_irregular"] = false;
    }

    std::ofstream out(m_config_path);
    if (!out)
    {
        return false;
    }
    out << config.dump(4);
    return static_cast<bool>(out);
}