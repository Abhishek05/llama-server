add_library(model_manager model_manager.cpp)
add_library(llama_wrapper llama_wrapper.cpp)
add_executable(illama_server server.cpp)

target_link_libraries(model_manager PRIVATE nlohmann_json)

target_include_directories(model_manager PRIVATE ${nlohmann_json_SOURCE_DIR}/include)

target_include_directories(model_manager PRIVATE ${CMAKE_BINARY_DIR}/_deps/nlohmann_json-src/include)

if(USE_LLAMA_CPP)
  target_compile_definitions(model_manager PRIVATE USE_LLAMA_CPP=1)
  target_compile_definitions(llama_wrapper PRIVATE USE_LLAMA_CPP=1)
  if(EXISTS "${CMAKE_SOURCE_DIR}/external/llama.cpp/CMakeLists.txt")
    add_subdirectory(${CMAKE_SOURCE_DIR}/external/llama.cpp extern_llama)
    target_include_directories(llama_wrapper PRIVATE ${CMAKE_SOURCE_DIR}/external/llama.cpp/include)
    target_include_directories(llama_wrapper PRIVATE ${CMAKE_SOURCE_DIR}/external/llama.cpp/ggml/include)
    target_link_libraries(llama_wrapper PRIVATE llama)
  else()
    message(WARNING "llama.cpp not found in external/llama.cpp; building with stub backend")
    target_compile_definitions(llama_wrapper PRIVATE LLAMA_STUB_BACKEND)
  endif()
else()
  target_compile_definitions(model_manager PRIVATE USE_LLAMA_CPP=0)
  target_compile_definitions(llama_wrapper PRIVATE LLAMA_STUB_BACKEND)
endif()

target_link_libraries(llama_wrapper PRIVATE nlohmann_json)

target_include_directories(llama_wrapper PRIVATE ${CMAKE_BINARY_DIR}/_deps/nlohmann_json-src/include)

# add_executable(illama_cli cli.cpp)

# target_link_libraries(illama_cli PRIVATE CLI11 llama_wrapper model_manager)

# add_executable(illama_tests tests.cpp)

# target_link_libraries(illama_tests PRIVATE Catch2::Catch2WithMain llama_wrapper model_manager)

target_link_libraries(illama_server PRIVATE httplib nlohmann_json llama_wrapper model_manager)

target_include_directories(illama_server PRIVATE ${httplib_SOURCE_DIR} ${nlohmann_json_SOURCE_DIR}/include)

target_include_directories(illama_server PRIVATE ${CMAKE_BINARY_DIR}/_deps/nlohmann_json-src/include)

target_include_directories(illama_server PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
