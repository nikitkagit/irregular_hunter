cmake_minimum_required(VERSION 3.16)

project(conjuctive_unary LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3)
FetchContent_MakeAvailable(nlohmann_json)

find_package(conjunctive_unary CONFIG REQUIRED)

add_library(irregular_hunter
    src/regularity_checker.cpp
    src/random_system_generator.cpp
    src/irregular_system_writer.cpp
    src/application/application.cpp
    src/application/command_parameters_parser.cpp
)

target_link_libraries(irregular_hunter PRIVATE nlohmann_json::nlohmann_json)

target_include_directories(irregular_hunter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(irregular_hunter PUBLIC
    conjunctive_unary::conjunctive_unary_library
)

add_executable(irhunt
    src/main.cpp
)
target_link_libraries(irhunt PRIVATE irregular_hunter)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(irregular_hunter PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(irhunt PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(irregular_hunter PRIVATE /W4)
    target_compile_options(irhunt PRIVATE /W4)
endif()

enable_testing()

# helper: creates executable test file from one .cpp,
# links with library and register in CTest.
function(add_project_test source)
    get_filename_component(name "${source}" NAME_WE)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE irregular_hunter)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_project_test(test/regularity_checker_test.cpp)
add_project_test(test/random_system_generator_test.cpp)
add_project_test(test/irregular_system_writer_test.cpp)
add_project_test(test/application_test.cpp)