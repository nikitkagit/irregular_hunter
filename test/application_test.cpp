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
    Application app(out, err);
    return app.run(static_cast<int>(argv.size()), argv.data());
}

std::string read_file(const std::string &path)
{
    std::ifstream f(path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
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
    // 1. -h prints usage to stdout and exits 0.
    {
        std::ostringstream out, err;
        const int rc = run_app({"app", "-h"}, out, err);
        assert(rc == 0);
        assert(contains(out.str(), "Usage:"));
        assert(err.str().empty());
    }

    // 2. --help behaves like -h.
    {
        std::ostringstream out, err;
        const int rc = run_app({"app", "--help"}, out, err);
        assert(rc == 0);
        assert(contains(out.str(), "Usage:"));
        assert(err.str().empty());
    }

    // 3. Unknown argument: error + usage to stderr, non-zero exit.
    {
        std::ostringstream out, err;
        const int rc = run_app({"app", "--bogus"}, out, err);
        assert(rc != 0);
        assert(contains(err.str(), "Unknown argument"));
        assert(contains(err.str(), "Usage:"));
    }

    // 4. Missing value for -n.
    {
        std::ostringstream out, err;
        const int rc = run_app({"app", "-n"}, out, err);
        assert(rc != 0);
        assert(contains(err.str(), "Missing value for -n"));
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

    // 15. Minimal successful run produces the expected sections.
    {
        std::ostringstream out, err;
        const int rc = run_app(
            {"app", "-e", "2", "-d", "1", "-n", "2", "--no-filter"},
            out, err);
        assert(rc == 0);
        assert(err.str().empty());
        assert(contains(out.str(), "Report written to: "));

        const std::string filename = extract_output_filename(out.str());
        assert(!filename.empty());

        const std::string content = read_file(filename);
        assert(contains(content, "=== Generation ==="));
        assert(contains(content, "=== System ==="));
        assert(contains(content, "=== Solution ==="));
        assert(contains(content, "=== Summary ==="));

        std::remove(filename.c_str());
    }

    // 16. -s override is reflected in the filename.
    {
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
    }

    // 17. -s 0 produces a "random" filename and "0 (random)" in report.
    {
        std::ostringstream out, err;
        const int rc = run_app(
            {"app", "-e", "2", "-d", "1", "-n", "2",
             "-s", "0", "--no-filter"},
            out, err);
        assert(rc == 0);

        const std::string filename = extract_output_filename(out.str());
        assert(contains(filename, "random"));

        const std::string content = read_file(filename);
        assert(contains(content, "(random)"));

        std::remove(filename.c_str());
    }

    // 18. -n is reflected in the report's max_iterations.
    {
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
    }

    // 19. -e controls the number of equations and variables.
    {
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
    }

    // 20. --no-filter disables the irregular filter.
    {
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
    }

    // 21. Without --no-filter, require_irregular is enabled.
    {
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
    }

    // 22. -a bounds the number of attempts.
    {
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
    }

    // 23. -m caps the maximum constant value.
    {
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
    }

    // 24. -d 1 disables internal nodes (no operators in the system).
    {
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
    }

    // 25. -p and -r are accepted and produce a well-formed report.
    //     (The specific regularity verdict is not asserted here, because
    //     min_periodic_count is not exposed on the command line.)
    {
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
    }

    // 26. Combination of flags produces a coherent report.
    {
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
    }

    // 27. Repeated runs with the same seed produce the same report content
    //     (modulo the timestamp in the filename).
    {
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
    }

    // 28. Different seeds produce different reports.
    {
        std::ostringstream out1, err1;
        assert(run_app(
            {"app", "-e", "3", "-d", "2", "-n", "5",
             "-s", "11", "--no-filter"},
            out1, err1) == 0);
        const std::string f1 = extract_output_filename(out1.str());
        const std::string c1 = read_file(f1);
        std::remove(f1.c_str());

        std::ostringstream out2, err2;
        assert(run_app(
            {"app", "-e", "3", "-d", "2", "-n", "5",
             "-s", "22", "--no-filter"},
            out2, err2) == 0);
        const std::string f2 = extract_output_filename(out2.str());
        const std::string c2 = read_file(f2);
        std::remove(f2.c_str());

        assert(c1 != c2);
    }

    // 29. Report includes every variable name for the requested count.
    {
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
    }

    std::cout << "All Application tests passed\n";
    return 0;
}