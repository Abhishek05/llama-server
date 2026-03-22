#pragma once

// Forward declarations to avoid requiring llama.h in headers.
struct llama_model;
struct llama_context;

#include <string>
#include <vector>
#include <functional>

class LlamaModel {
public:
    LlamaModel();
    ~LlamaModel();

    bool loadModel(const std::string &modelPath);
    std::string generateChat(const std::string &prompt, int maxTokens = 256, float temperature = 0.8f);
    void generateChatStream(const std::string &prompt, std::function<void(const std::string&)> callback, int maxTokens = 256, float temperature = 0.8f);
    std::vector<float> embeddings(const std::string &text);

private:
    llama_model *model_;
    llama_context *ctx_;
};
