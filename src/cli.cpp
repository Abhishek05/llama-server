#include <CLI/CLI.hpp>
#include "llama_wrapper.h"
#include "model_manager.h"
#include <iostream>

int main(int argc, char** argv) {
    CLI::App app{"Illama CLI"};

    std::string storagePath = std::string(getenv("HOME") ? getenv("HOME") : ".") + "/.illama/models";
    ModelManager manager(storagePath);
    LlamaModel model;

    auto pull_cmd = app.add_subcommand("pull", "Pull a model from registry");
    std::string model_id;
    pull_cmd->add_option("model-id", model_id, "Model ID to pull")->required();
    pull_cmd->callback([&]() {
        if (manager.pullModel(model_id)) {
            std::cout << "Model pulled successfully" << std::endl;
        } else {
            std::cout << "Failed to pull model" << std::endl;
            exit(1);
        }
    });

    auto run_cmd = app.add_subcommand("run", "Run inference with a model");
    std::string run_model_id;
    std::string prompt;
    int max_tokens = 256;
    float temperature = 0.8f;
    run_cmd->add_option("model-id", run_model_id, "Model ID to use")->required();
    run_cmd->add_option("prompt", prompt, "Prompt to generate from")->required();
    run_cmd->add_option("--max-tokens", max_tokens, "Maximum tokens to generate");
    run_cmd->add_option("--temperature", temperature, "Sampling temperature");
    run_cmd->callback([&]() {
        if (!model.loadModel(manager.getModelPath(run_model_id))) {
            std::cerr << "Cannot load model" << std::endl;
            exit(1);
        }
        std::string output = model.generateChat(prompt, max_tokens, temperature);
        std::cout << output << std::endl;
    });

    auto chat_cmd = app.add_subcommand("chat", "Interactive chat with a model");
    std::string chat_model_id;
    chat_cmd->add_option("model-id", chat_model_id, "Model ID to use")->required();
    chat_cmd->callback([&]() {
        if (!model.loadModel(manager.getModelPath(chat_model_id))) {
            std::cerr << "Cannot load model" << std::endl;
            exit(1);
        }
        std::string input;
        while (true) {
            std::cout << "You: ";
            std::getline(std::cin, input);
            if (input == "exit") break;
            std::string response = model.generateChat(input);
            std::cout << "Assistant: " << response << std::endl;
        }
    });

    CLI11_PARSE(app, argc, argv);
    return 0;
}