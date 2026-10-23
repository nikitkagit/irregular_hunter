# irregular_hunter

A C++17 tool that generates random systems of equations over sets of natural
numbers, solves them iteratively, and hunts for sets whose structure is
**irregular** — i.e. that do not look eventually periodic according to a
configurable heuristic.

The project is built on top of
[`conjunctive_unary`](https://github.com/nikitkagit/conjunctive_unary), which
provides the expression tree, the parser and the equation solver.

## Overview

Given a system like

```text
V1 = {1, 2, 3} | (V2 & V3)
V2 = (V1 + {1}) & {2, 3, 4}
V3 = {V1, 2} + {3} | {10}
```

`irregular_hunter`:

1. generates a random system of this shape (number of equations, depth,
   operator mix, set sizes, etc. are all configurable),
2. parses it and runs the solver for up to a configured number of iterations,
3. classifies every resulting set as **regular** (eventually periodic) or
   **irregular** according to `RegularityCriteria`,
4. retries with a new seed until a system containing at least a requested
   number of irregular variables is found (or the attempt budget is exhausted),
5. writes a plain-text report describing the system, the solution and the
   regularity verdicts.

## Features

- Random generator of equation systems with fine-grained control over
  equation count, expression depth, operator distribution, fixed-set size,
  forward references and more.
- Incremental solver with a configurable iteration cap and a "fast
  convergence" rule that marks a set regular if the system stabilises earlier
  than `min_iterations`.
- Heuristic regularity checker with several independent knobs:
  - maximum allowed ratio of finite prefix to periodic tail,
  - maximum period to try,
  - minimum periodic tail size,
  - `ignore_below` to drop noisy small values,
  - `tail_percent` to skip an unchecked tail near the maximum,
  - `min_period_additions` to require a genuine repetition of the pattern.
- Command-line interface with overrides that are **merged** into a JSON
  config file, so the config acts as persistent state.
- Automatic random seed generation when the effective seed is `0`; the
  generated seed is used for the run and appears in the output filename, but
  the `0` in the config is left untouched.

## Requirements

- CMake ≥ 3.16
- A C++17 compiler (MinGW-w64 g++, GCC, Clang or MSVC)
- [`nlohmann/json`](https://github.com/nlohmann/json) for config parsing
- [`conjunctive_unary`](https://github.com/nikitkagit/conjunctive_unary)
  for expression trees, parsing and solving

## Building

```bash
cmake -S . -B build
cmake --build build
```

Optional presets:

```bash
cmake --preset default
cmake --build --preset default
```

## Usage

```
irregular_hunter [options]
```

| Option | Argument | Default | Description |
|--------|----------|---------|-------------|
| `-s`   | `<S>`    | `0`     | RNG seed. `0` means "generate a fresh seed for this run". |
| `-n`   | `<N>`    | from config | Maximum iterations per system. |
| `-i`   | `<N>`    | from config | Minimum iterations before convergence is considered meaningful. If the system stabilises earlier, all its sets are declared regular. |
| `-e`   | `<N>`    | from config | Number of equations to generate. |
| `-d`   | `<N>`    | from config | Maximum depth of an expression tree. |
| `-m`   | `<N>`    | from config | Maximum constant value inside fixed sets. |
| `-a`   | `<N>`    | from config | Maximum number of generation attempts. |
| `-k`   | `<N>`    | from config | Required number of irregular variables per accepted system. |
| `-p`   | `<N>`    | from config | Maximum period to test for regularity. |
| `-r`   | `<P>`    | from config | Maximum finite/periodic ratio, in percent. |
| `--no-filter` | — | — | Accept the first generated system regardless of its regularity. |
| `-h`, `--help` | — | — | Print usage and exit. |

When a flag is supplied on the command line, its value is written back into
the config file. Fields that are not overridden are preserved, so the config
file can accumulate settings across runs.

## Configuration file

By default the application reads and updates `config.json` next to the
executable (or in the current working directory). All recognised keys are
optional and fall back to the built-in defaults from
`IrregularSystemWriterParameters`.

Example:

```json
{
  "generator": {
    "seed": 12345,
    "num_equations": 6,
    "max_depth": 3,
    "max_constant_value": 20,
    "min_set_size": 1,
    "max_set_size": 4,
    "internal_node_percent": 60,
    "leaf_variable_percent": 60,
    "leaf_unknown_variable_percent": 20,
    "operator_union_percent": 34,
    "operator_sum_percent": 33,
    "operator_intersection_percent": 33,
    "min_operands": 2,
    "max_operands": 3
  },
  "criteria": {
    "max_finite_to_periodic_percent": 10.0,
    "max_period": 1024,
    "min_periodic_count": 10,
    "ignore_below": 0,
    "tail_percent": 1.0,
    "min_period_additions": 10
  },
  "max_iterations": 100,
  "min_iterations": 30,
  "require_irregular": true,
  "required_irregular_count": 1,
  "max_attempts": 100
}
```

Behaviour of `seed = 0`:

- if the loaded seed is `0` — whether from the config or from an explicit
  `-s 0` — a fresh random seed is drawn for this run,
- the generated seed is used to build the report and is embedded in the
  output filename,
- the config file is **not** modified by this step; the `0` stays in the
  config so the next run will again pick a fresh seed.

Run:

```bash
irhunter
```

Console output:

```
Report written to: irregular_system_seed42_1712345678901.txt
```

The report file lists the seed used, the full system, every variable value,
and a regularity verdict per variable.

## Known limitations

- **Heuristic, not decision-theoretic.** The checker approximates "eventually
  periodic". A set can be rejected purely because of a small `min_periodic_count`,
  or accepted because a single boundary pair happens to match. Tune the
  criteria rather than assuming a mathematical property.
- **Small sets.** Sets smaller than `min_periodic_count` are rejected even
  if they are trivially periodic; the exceptions are the empty set and any
  set that becomes empty after `ignore_below`.
- **`min_iterations` and fast convergence.** If the system stabilises
  before `min_iterations`, every variable is declared regular without
  running the checker. This is deliberate and can hide genuine irregularity
  in very small systems.
- **No in-place config backup.** `ConfigUpdater` rewrites `config.json`
  in place. Keep a copy if the file matters.

## License

Released under the MIT License. See `LICENSE` for the full text.

## Acknowledgements

Built on top of [`conjunctive_unary`](https://github.com/nikitkagit/conjunctive_unary)
and [`nlohmann/json`](https://github.com/nlohmann/json). Random generation
uses the standard `<random>` facilities. The solver and expression model are
provided by the underlying library; this project only adds generation,
iteration policy, regularity analysis and reporting on top.