cmake_minimum_required(VERSION 3.13)

project(codecrafters-interpreter)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

add_executable(interpreter ${SOURCE_FILES})

# Local test suites. tests/run.sh is the real entrypoint and does the build
# itself; these targets exist so `ctest` works for anyone who expects it to.
# See tests/README.md for what the suites do and do not cover.
enable_testing()
find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_test(NAME smoke
         COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/smoke.py
                 $<TARGET_FILE:interpreter>)

add_test(NAME corpus
         COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/corpus/run_corpus.py
                 ${CMAKE_CURRENT_SOURCE_DIR}/tests/corpus/corpus.json
                 $<TARGET_FILE:interpreter>)

# Guard against a suite file quietly growing past the ~500-line budget.
add_test(NAME suite_size
         COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_size.py
                 ${CMAKE_CURRENT_SOURCE_DIR}/tests)
