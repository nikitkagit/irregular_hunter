#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "application/application.hpp"

namespace
{
const char *CONFIG_FILE = "test_application_config.json";

int run_app(const std::vector<std::string> &args,
            std::ostream &out,
            std::ostream &err)
{
    std::vector<char *> argv;
    argv.reserve(args.size());
    for (const std::string &a : args)
    {
        argv.push_back(const_cast<char *>(a.c_str()));
    }
    Application app(out, err, CONFIG_FILE);
    return app.run(static_cast<int>(argv.size()), argv.data());
}

std::string read_file(const std::string &path)
{
    std::ifstream f(path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

void write_file(const std::string &path, const std::string &content)
{
    std::ofstream f(path);
    f << content;
}

bool file_exists(const std::string &path)
{
    std::ifstream f(path);
    return f.good();
}

std::string extract_output_filename(const std::string &output)
{
    const std::string prefix = "Report written to: ";
    const auto pos = output.find(prefix);
    if (pos == std::string::npos)
    {
        return "";
    }
    const auto start = pos + prefix.size();
    const auto end = output.find('\n', start);
    return output.substr(start, end - start);
}

bool contains(const std::string &haystack, const std::string &needle)
{
    return haystack.find(needle) != std::string::npos;
}
} // namespace

int main()
{
    // Clean up leftovers from previous runs.
    std::remove(CONFIG_FILE);

    // ===== Help and argument-error paths (must not touch the config) =====

    // 1. -h prints usage to stdout, exits 0, does not create the config.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app({"app", "-h"}, out, err);
        assert(rc == 0);
        assert(contains(out.str(), "Usage:"));
        assert(err.str().empty());
        assert(!file_exists(CONFIG_FILE));
    }

    // 2. --help behaves like -h.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app({"app", "--help"}, out, err);
        assert(rc == 0);
        assert(contains(out.str(), "Usage:"));
        assert(err.str().empty());
        assert(!file_exists(CONFIG_FILE));
    }

    // 3. Unknown argument: error + usage, no config file written.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app({"app", "--bogus"}, out, err);
        assert(rc != 0);
        assert(contains(err.str(), "Unknown argument"));
        assert(contains(err.str(), "Usage:"));
        assert(!file_exists(CONFIG_FILE));
    }

    // 4. Missing value for -n.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app({"app", "-n"}, out, err);
        assert(rc != 0);
        assert(contains(err.str(), "Missing value for -n"));
        assert(!file_exists(CONFIG_FILE));
    }

    // 5. Missing value for -s.
    {
        std::ostringstream out, err;
        const int rc = run_app({"app", "-s"}, out, err);
        assert(rc != 0);
        assert(contains(err.str(), "Missing value for -s"));
    }

    // 6. Missing value for -e.
    {
        std::ostringstream out, err;
        const int rc = run_app({"app", "-e"}, out, err);
        assert(rc != 0);
        assert(contains(err.str(), "Missing value for -e"));
    }

    // 7. Missing value for -d.
    {
        std::ostringstream out, err;
        const int rc = run_app({"app", "-d"}, out, err);
        assert(rc != 0);
        assert(contains(err.str(), "Missing value for -d"));
    }

    // 8. Missing value for -m.
    {
        std::ostringstream out, err;
        const int rc = run_app({"app", "-m"}, out, err);
        assert(rc != 0);
        assert(contains(err.str(), "Missing value for -m"));
    }

    // 9. Missing value for -a.
    {
        std::ostringstream out, err;
        const int rc = run_app({"app", "-a"}, out, err);
        assert(rc != 0);
        assert(contains(err.str(), "Missing value for -a"));
    }

    // 10. Missing value for -p.
    {
        std::ostringstream out, err;
        const int rc = run_app({"app", "-p"}, out, err);
        assert(rc != 0);
        assert(contains(err.str(), "Missing value for -p"));
    }

    // 11. Missing value for -r.
    {
        std::ostringstream out, err;
        const int rc = run_app({"app", "-r"}, out, err);
        assert(rc != 0);
        assert(contains(err.str(), "Missing value for -r"));
    }

    // 12. Non-numeric -n.
    {
        std::ostringstream out, err;
        const int rc = run_app({"app", "-n", "abc"}, out, err);
        assert(rc != 0);
        assert(contains(err.str(), "Invalid value for -n"));
    }

    // 13. Non-numeric -s.
    {
        std::ostringstream out, err;
        const int rc = run_app({"app", "-s", "abc"}, out, err);
        assert(rc != 0);
        assert(contains(err.str(), "Invalid value for -s"));
    }

    // 14. Non-numeric -r.
    {
        std::ostringstream out, err;
        const int rc = run_app({"app", "-r", "xyz"}, out, err);
        assert(rc != 0);
        assert(contains(err.str(), "Invalid value for -r"));
    }

    // ===== Successful runs and config integration =====

    // 15. Minimal run creates the config and produces all sections.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app(
            {"app", "-e", "2", "-d", "1", "-n", "2", "--no-filter"},
            out, err);
        assert(rc == 0);
        assert(err.str().empty());
        assert(contains(out.str(), "Report written to: "));
        assert(file_exists(CONFIG_FILE));

        const std::string filename = extract_output_filename(out.str());
        assert(!filename.empty());

        const std::string content = read_file(filename);
        assert(contains(content, "=== Generation ==="));
        assert(contains(content, "=== System ==="));
        assert(contains(content, "=== Solution ==="));
        assert(contains(content, "=== Summary ==="));

        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    // 16. CLI overrides end up in the config file.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app(
            {"app", "-s", "42", "-e", "3", "-d", "2",
             "-n", "5", "-a", "7", "-m", "9",
             "-p", "64", "-r", "50.5", "--no-filter"},
            out, err);
        assert(rc == 0);

        const std::string config = read_file(CONFIG_FILE);
        assert(contains(config, "\"seed\": 42"));
        assert(contains(config, "\"num_equations\": 3"));
        assert(contains(config, "\"max_depth\": 2"));
        assert(contains(config, "\"max_iterations\": 5"));
        assert(contains(config, "\"max_attempts\": 7"));
        assert(contains(config, "\"max_constant_value\": 9"));
        assert(contains(config, "\"max_period\": 64"));
        assert(contains(config, "\"require_irregular\": false"));

        const std::string filename = extract_output_filename(out.str());
        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    // 17. Pre-existing config fields that are not overridden survive the
    //     run.
    {
        write_file(CONFIG_FILE, R"({
            "custom_field": "hello",
            "max_iterations": 99,
            "generator": { "seed": 111, "max_depth": 7 }
        })");
        std::ostringstream out, err;
        const int rc = run_app(
            {"app", "-n", "5", "-e", "1", "-d", "1", "--no-filter"},
            out, err);
        assert(rc == 0);

        const std::string config = read_file(CONFIG_FILE);
        assert(contains(config, "\"custom_field\": \"hello\""));
        assert(contains(config, "\"max_iterations\": 5"));  // overridden
        assert(contains(config, "\"max_depth\": 1"));       // overridden
        assert(contains(config, "\"seed\": 111"));          // preserved

        const std::string filename = extract_output_filename(out.str());
        assert(contains(filename, "seed111"));
        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    // 18. Config values are used when the CLI does not override them.
    {
        write_file(CONFIG_FILE, R"({
            "generator": {
                "seed": 42,
                "num_equations": 2,
                "max_depth": 1,
                "leaf_variable_percent": 0
            },
            "require_irregular": false,
            "max_iterations": 3
        })");
        std::ostringstream out, err;
        const int rc = run_app({"app"}, out, err);
        assert(rc == 0);
        assert(err.str().empty());

        const std::string filename = extract_output_filename(out.str());
        assert(contains(filename, "seed42"));

        const std::string content = read_file(filename);
        assert(contains(content, "seed: 42"));
        assert(contains(content, "max_iterations: 3"));
        assert(contains(content, "num_equations: 2"));

        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    // 19. Malformed config: application fails before running the pipeline.
    {
        write_file(CONFIG_FILE, "{ not valid json ");
        std::ostringstream out, err;
        const int rc = run_app({"app", "-n", "1"}, out, err);
        assert(rc != 0);
        assert(contains(err.str(), "config") || contains(err.str(), "Config"));
        std::remove(CONFIG_FILE);
    }

    // 20. Config with wrong-typed field: application fails cleanly.
    {
        write_file(CONFIG_FILE, R"({ "generator": "not an object" })");
        std::ostringstream out, err;
        const int rc = run_app({"app", "-n", "1"}, out, err);
        assert(rc != 0);
        assert(contains(err.str(), "generator") ||
               contains(err.str(), "Config"));
        std::remove(CONFIG_FILE);
    }

    // 21. -s override is reflected in the filename.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app(
            {"app", "-e", "2", "-d", "1", "-n", "2",
             "-s", "777", "--no-filter"},
            out, err);
        assert(rc == 0);

        const std::string filename = extract_output_filename(out.str());
        assert(contains(filename, "seed777"));

        const std::string content = read_file(filename);
        assert(contains(content, "seed: 777"));

        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    // 22. -s 0 triggers automatic seed generation for this run: the
    //     filename reflects the generated seed, the report uses it, and
    //     the config file is left untouched (a 0 in the config stays 0).
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app(
            {"app", "-e", "2", "-d", "1", "-n", "2",
             "-s", "0", "--no-filter"},
            out, err);
        assert(rc == 0);

        const std::string filename = extract_output_filename(out.str());
        assert(!filename.empty());
        assert(contains(filename, "seed"));
        assert(!contains(filename, "random"));

        const std::string content = read_file(filename);
        assert(!contains(content, "(random)"));
        assert(contains(content, "seed: "));

        // The config was updated by ConfigUpdater to record the CLI
        // override (seed 0), and must still contain 0 — the generated
        // seed is not persisted.
        const std::string config = read_file(CONFIG_FILE);
        assert(contains(config, "\"seed\": 0"));

        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    // 23. -n is reflected in the report's max_iterations.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app(
            {"app", "-e", "2", "-d", "1", "-n", "7",
             "-s", "42", "--no-filter"},
            out, err);
        assert(rc == 0);

        const std::string filename = extract_output_filename(out.str());
        const std::string content = read_file(filename);
        assert(contains(content, "max_iterations: 7"));

        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    // 24. -e controls the number of equations and variables.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app(
            {"app", "-e", "4", "-d", "1", "-n", "2",
             "-s", "42", "--no-filter"},
            out, err);
        assert(rc == 0);

        const std::string filename = extract_output_filename(out.str());
        const std::string content = read_file(filename);
        assert(contains(content, "num_equations: 4"));
        assert(contains(content, "V1"));
        assert(contains(content, "V4"));
        assert(!contains(content, "V5"));

        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    // 25. --no-filter disables the irregular filter.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app(
            {"app", "-e", "2", "-d", "1", "-n", "2",
             "-s", "42", "--no-filter"},
            out, err);
        assert(rc == 0);

        const std::string filename = extract_output_filename(out.str());
        const std::string content = read_file(filename);
        assert(contains(content, "require_irregular: no"));

        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    // 26. Without --no-filter, require_irregular is enabled.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app(
            {"app", "-e", "2", "-d", "1", "-n", "2",
             "-s", "42", "-a", "1"},
            out, err);
        assert(rc == 0);

        const std::string filename = extract_output_filename(out.str());
        const std::string content = read_file(filename);
        assert(contains(content, "require_irregular: yes"));

        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    // 27. -a bounds the number of attempts.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app(
            {"app", "-e", "2", "-d", "1", "-n", "2",
             "-s", "1", "-a", "1"},
            out, err);
        assert(rc == 0);

        const std::string filename = extract_output_filename(out.str());
        const std::string content = read_file(filename);
        assert(contains(content, "attempts_used: 1"));

        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    // 28. -m caps the maximum constant value.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app(
            {"app", "-e", "3", "-d", "1", "-n", "2",
             "-s", "42", "-m", "3", "--no-filter"},
            out, err);
        assert(rc == 0);

        const std::string filename = extract_output_filename(out.str());
        const std::string content = read_file(filename);
        for (int v = 4; v <= 9; ++v)
        {
            assert(!contains(content, "{ " + std::to_string(v) + " }"));
            assert(!contains(content, ", " + std::to_string(v) + " }"));
            assert(!contains(content, ", " + std::to_string(v) + ","));
        }

        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    // 29. -d 1 disables internal nodes (no operators in the system).
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app(
            {"app", "-e", "5", "-d", "1", "-n", "2",
             "-s", "42", "--no-filter"},
            out, err);
        assert(rc == 0);

        const std::string filename = extract_output_filename(out.str());
        const std::string content = read_file(filename);

        const auto b = content.find("=== System ===");
        const auto e = content.find("=== Solution ===", b);
        assert(b != std::string::npos);
        assert(e != std::string::npos);
        const std::string system = content.substr(b, e - b);
        assert(!contains(system, " | "));
        assert(!contains(system, " & "));
        assert(!contains(system, " + "));

        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    // 30. -p and -r are accepted and produce a well-formed report.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app(
            {"app", "-e", "3", "-d", "1", "-n", "2",
             "-s", "42", "-p", "1", "-r", "10000",
             "--no-filter"},
            out, err);
        assert(rc == 0);
        assert(err.str().empty());

        const std::string filename = extract_output_filename(out.str());
        assert(!filename.empty());

        const std::string content = read_file(filename);
        assert(contains(content, "=== Solution ==="));
        assert(contains(content, "=== Summary ==="));
        assert(contains(content, "irregular variables:"));

        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    // 31. Combination of flags produces a coherent report.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        const int rc = run_app(
            {"app", "-e", "4", "-d", "2", "-n", "5",
             "-s", "123", "-a", "3", "-m", "10",
             "-p", "64", "-r", "50", "--no-filter"},
            out, err);
        assert(rc == 0);

        const std::string filename = extract_output_filename(out.str());
        assert(contains(filename, "seed123"));

        const std::string content = read_file(filename);
        assert(contains(content, "seed: 123"));
        assert(contains(content, "num_equations: 4"));
        assert(contains(content, "max_iterations: 5"));
        assert(contains(content, "require_irregular: no"));
        assert(contains(content, "irregular variables:"));

        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    // 32. Repeated runs with the same overrides produce the same report
    //     content (modulo the timestamp in the filename).
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out1, err1;
        assert(run_app(
            {"app", "-e", "3", "-d", "2", "-n", "5",
             "-s", "999", "--no-filter"},
            out1, err1) == 0);
        const std::string f1 = extract_output_filename(out1.str());
        const std::string c1 = read_file(f1);
        std::remove(f1.c_str());

        std::ostringstream out2, err2;
        assert(run_app(
            {"app", "-e", "3", "-d", "2", "-n", "5",
             "-s", "999", "--no-filter"},
            out2, err2) == 0);
        const std::string f2 = extract_output_filename(out2.str());
        const std::string c2 = read_file(f2);
        std::remove(f2.c_str());

        assert(c1 == c2);
        std::remove(CONFIG_FILE);
    }

    // 33. Different seeds produce different reports.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out1, err1;
        assert(run_app(
            {"app", "-e", "3", "-d", "2", "-n", "5",
             "-s", "11", "--no-filter"},
            out1, err1) == 0);
        const std::string f1 = extract_output_filename(out1.str());
        const std::string c1 = read_file(f1);
        std::remove(f1.c_str());
        std::remove(CONFIG_FILE);

        std::ostringstream out2, err2;
        assert(run_app(
            {"app", "-e", "3", "-d", "2", "-n", "5",
             "-s", "22", "--no-filter"},
            out2, err2) == 0);
        const std::string f2 = extract_output_filename(out2.str());
        const std::string c2 = read_file(f2);
        std::remove(f2.c_str());
        std::remove(CONFIG_FILE);

        assert(c1 != c2);
    }

    // 34. Report includes every variable name for the requested count.
    {
        std::remove(CONFIG_FILE);
        std::ostringstream out, err;
        assert(run_app(
            {"app", "-e", "5", "-d", "1", "-n", "2",
             "-s", "5", "--no-filter"},
            out, err) == 0);

        const std::string filename = extract_output_filename(out.str());
        const std::string content = read_file(filename);
        for (int i = 1; i <= 5; ++i)
        {
            assert(contains(content, "V" + std::to_string(i)));
        }

        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    // 35. Consecutive runs accumulate overrides in the config file.
    {
        std::remove(CONFIG_FILE);
        {
            std::ostringstream out, err;
            assert(run_app(
                {"app", "-e", "2", "-d", "1", "-n", "2",
                 "-s", "1", "--no-filter"},
                out, err) == 0);
            const std::string filename =
                extract_output_filename(out.str());
            std::remove(filename.c_str());
        }
        {
            // Second run only overrides -n; other fields from the first
            // run persist in the config file.
            std::ostringstream out, err;
            assert(run_app({"app", "-n", "9"}, out, err) == 0);
            const std::string config = read_file(CONFIG_FILE);
            assert(contains(config, "\"num_equations\": 2"));
            assert(contains(config, "\"max_depth\": 1"));
            assert(contains(config, "\"seed\": 1"));
            assert(contains(config, "\"max_iterations\": 9"));

            const std::string filename =
                extract_output_filename(out.str());
            assert(contains(filename, "seed1"));
            std::remove(filename.c_str());
        }
        std::remove(CONFIG_FILE);
    }

    // 36. ignore_below from config filters small sets to empty, so they
    //     are reported as regular even though they would be rejected as
    //     too small by min_periodic_count without the filter.
    {
        write_file(CONFIG_FILE, R"({
            "generator": {
                "seed": 7,
                "num_equations": 2,
                "max_depth": 1,
                "leaf_variable_percent": 0,
                "min_set_size": 1,
                "max_set_size": 1,
                "max_constant_value": 5
            },
            "criteria": { "ignore_below": 20 },
            "require_irregular": false,
            "max_iterations": 2
        })");
        std::ostringstream out, err;
        const int rc = run_app({"app"}, out, err);
        assert(rc == 0);

        const std::string filename = extract_output_filename(out.str());
        const std::string content = read_file(filename);
        assert(!contains(content, "IRREGULAR"));

        // The config must still record ignore_below from the file.
        const std::string config = read_file(CONFIG_FILE);
        assert(contains(config, "\"ignore_below\": 20"));

        std::remove(filename.c_str());
        std::remove(CONFIG_FILE);
    }

    std::cout << "All Application tests passed\n";
    return 0;
}