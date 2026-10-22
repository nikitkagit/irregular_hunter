#include "application/config_parameters_loader.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

ConfigParametersLoader::ConfigParametersLoader(std::string config_path)
    : m_config_path(std::move(config_path))
{
}

IrregularSystemWriterParameters ConfigParametersLoader::load() const
{
    IrregularSystemWriterParameters params;

    std::ifstream in(m_config_path);
    if (!in)
    {
        return params;
    }

    nlohmann::json config;
    try
    {
        in >> config;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(
            "ConfigParametersLoader: malformed JSON: "
            + std::string(e.what()));
    }
    if (!config.is_object())
    {
        throw std::runtime_error(
            "ConfigParametersLoader: top-level JSON must be an object");
    }

    if (config.contains("generator"))
    {
        const auto &gen = config.at("generator");
        if (!gen.is_object())
        {
            throw std::runtime_error(
                "ConfigParametersLoader: 'generator' must be an object");
        }
        params.generator.seed =
            gen.value("seed", params.generator.seed);
        params.generator.num_equations =
            gen.value("num_equations", params.generator.num_equations);
        params.generator.variable_prefix =
            gen.value("variable_prefix", params.generator.variable_prefix);
        params.generator.max_constant_value =
            static_cast<num_t>(gen.value(
                "max_constant_value",
                static_cast<unsigned long long>(
                    params.generator.max_constant_value)));
        params.generator.min_set_size =
            gen.value("min_set_size", params.generator.min_set_size);
        params.generator.max_set_size =
            gen.value("max_set_size", params.generator.max_set_size);
        params.generator.max_depth =
            gen.value("max_depth", params.generator.max_depth);
        params.generator.internal_node_percent =
            gen.value("internal_node_percent",
                      params.generator.internal_node_percent);
        params.generator.leaf_variable_percent =
            gen.value("leaf_variable_percent",
                      params.generator.leaf_variable_percent);
        params.generator.leaf_unknown_variable_percent =
            gen.value("leaf_unknown_variable_percent",
                      params.generator.leaf_unknown_variable_percent);
        params.generator.operator_union_percent =
            gen.value("operator_union_percent",
                      params.generator.operator_union_percent);
        params.generator.operator_sum_percent =
            gen.value("operator_sum_percent",
                      params.generator.operator_sum_percent);
        params.generator.operator_intersection_percent =
            gen.value("operator_intersection_percent",
                      params.generator.operator_intersection_percent);
        params.generator.min_operands =
            gen.value("min_operands", params.generator.min_operands);
        params.generator.max_operands =
            gen.value("max_operands", params.generator.max_operands);
    }

    if (config.contains("criteria"))
    {
        const auto &crit = config.at("criteria");
        if (!crit.is_object())
        {
            throw std::runtime_error(
                "ConfigParametersLoader: 'criteria' must be an object");
        }
        params.criteria.max_finite_to_periodic_percent =
            crit.value("max_finite_to_periodic_percent",
                       params.criteria.max_finite_to_periodic_percent);
        params.criteria.max_period =
            crit.value("max_period", params.criteria.max_period);
        params.criteria.min_periodic_count =
            crit.value("min_periodic_count",
                       params.criteria.min_periodic_count);
    }

    params.max_iterations =
        config.value("max_iterations", params.max_iterations);
    params.require_irregular =
        config.value("require_irregular", params.require_irregular);
    params.max_attempts =
        config.value("max_attempts", params.max_attempts);

    return params;
}