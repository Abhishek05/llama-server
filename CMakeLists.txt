cmake_minimum_required(VERSION 3.16)
project(illama LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(USE_LLAMA_CPP "Enable llama.cpp backend integration" ON)

include(FetchContent)

# Set policy for older CMake compatibility
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.11.0
)
FetchContent_MakeAvailable(httplib)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.10.5
)
FetchContent_MakeAvailable(nlohmann_json)

add_subdirectory(src)
