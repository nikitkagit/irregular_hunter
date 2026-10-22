#include "application/config_parser.hpp"

#include <fstream>
#include <stdexcept>

namespace
{
num_t get_num(const nlohmann::json &obj, const char *key, num_t def)
{
    return static_cast<num_t>(
        obj.value(key, static_cast<unsigned long long>(def)));
}
} // namespace

IrregularSystemWriterParameters ConfigParser::parse(const nlohmann::json &json)
{
    if (!json.is_object())
    {
        throw std::runtime_error(
            "ConfigParser: top-level JSON value must be an object");
    }

    IrregularSystemWriterParameters params;

    if (json.contains("generator"))
    {
        const auto &gen = json.at("generator");
        if (!gen.is_object())
        {
            throw std::runtime_error(
                "ConfigParser: 'generator' must be an object");
        }
        params.generator.seed =
            gen.value("seed", params.generator.seed);
        params.generator.num_equations =
            gen.value("num_equations", params.generator.num_equations);
        params.generator.variable_prefix =
            gen.value("variable_prefix", params.generator.variable_prefix);
        params.generator.max_constant_value =
            get_num(gen, "max_constant_value",
                    params.generator.max_constant_value);
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

    if (json.contains("criteria"))
    {
        const auto &crit = json.at("criteria");
        if (!crit.is_object())
        {
            throw std::runtime_error(
                "ConfigParser: 'criteria' must be an object");
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
        json.value("max_iterations", params.max_iterations);
    params.require_irregular =
        json.value("require_irregular", params.require_irregular);
    params.max_attempts =
        json.value("max_attempts", params.max_attempts);

    return params;
}

IrregularSystemWriterParameters ConfigParser::parse_string(
    const std::string &text)
{
    return parse(nlohmann::json::parse(text));
}

IrregularSystemWriterParameters ConfigParser::parse_file(
    const std::string &path)
{
    std::ifstream file(path);
    if (!file)
    {
        throw std::runtime_error(
            "ConfigParser: failed to open file: " + path);
    }
    return parse(nlohmann::json::parse(file));
}