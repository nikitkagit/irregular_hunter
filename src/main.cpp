#include <iostream>

#include "application/application.hpp"

/**
 * @brief Entry point for the irregular_hunter application.
 *
 * Constructs an Application bound to the standard streams and delegates
 * all argument parsing and execution to it.
 */
int main(int argc, char **argv)
{
    Application app(std::cout, std::cerr);
    return app.run(argc, argv);
}