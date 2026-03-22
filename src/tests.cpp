#include <catch2/catch_test_macros.hpp>
#include "model_manager.h"
#include "llama_wrapper.h"

TEST_CASE("ModelManager can get model path", "[model_manager]") {
    ModelManager manager("/tmp/test_models");
    REQUIRE(manager.getModelPath("test-model") == "/tmp/test_models/test-model.gguf");
}

TEST_CASE("LlamaModel stub works", "[llama_wrapper]") {
    LlamaModel model;
    // Assuming stub backend
    REQUIRE(model.loadModel("dummy"));
    REQUIRE(model.generateChat("hello") == "[stub] echo: hello");
}