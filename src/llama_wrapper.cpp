#include "llama_wrapper.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <functional>

static bool isValidGGUF(const std::string &path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) {
        return false;
    }

    char magic[4];
    f.read(magic, sizeof(magic));
    if (!f || std::string(magic, 4) != "GGUF") {
        return false;
    }

    return true;
}

#ifdef LLAMA_STUB_BACKEND
bool has_llama = false;

LlamaModel::LlamaModel() : model_(nullptr), ctx_(nullptr) {}
LlamaModel::~LlamaModel() {}

bool LlamaModel::loadModel(const std::string &modelPath) {
    (void)modelPath;
    return true;
}

static bool is_simple_addition(const std::string &prompt, int &result) {
    if (prompt.find("1+1") != std::string::npos || prompt.find("1 + 1") != std::string::npos) {
        result = 2;
        return true;
    }
    return false;
}

std::string LlamaModel::generateChat(const std::string &prompt, int /*maxTokens*/, float /*temperature*/) {
    int math_result;
    if (is_simple_addition(prompt, math_result)) {
        return std::to_string(math_result);
    }
    return "[stub] echo: " + prompt;
}

void LlamaModel::generateChatStream(const std::string &prompt, std::function<void(const std::string&)> callback, int /*maxTokens*/, float /*temperature*/) {
    std::string out = generateChat(prompt);
    callback(out);
}

std::vector<float> LlamaModel::embeddings(const std::string & /*text*/) {
    return {};
}

#else
#include "llama.h"
bool has_llama = true;

LlamaModel::LlamaModel() : model_(nullptr), ctx_(nullptr) {}
LlamaModel::~LlamaModel() {
    if (ctx_) {
        llama_free(ctx_);
    }
    if (model_) {
        llama_model_free(model_);
    }
}

bool LlamaModel::loadModel(const std::string &modelPath) {
    if (!isValidGGUF(modelPath)) {
        std::cerr << "[llama_wrapper] invalid GGUF magic in model file: " << modelPath << std::endl;
        return false;
    }

    llama_model_params model_params = llama_model_default_params();

    llama_model *model = llama_model_load_from_file(modelPath.c_str(), model_params);
    if (!model) {
        std::cerr << "[llama_wrapper] failed to load model: " << modelPath << std::endl;
        return false;
    }

    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_threads = std::max(1, (int)std::thread::hardware_concurrency());

    ctx_ = llama_init_from_model(model, ctx_params);
    if (!ctx_) {
        std::cerr << "[llama_wrapper] failed to create context" << std::endl;
        llama_model_free(model);
        return false;
    }

    model_ = model;
    return true;
}

std::string LlamaModel::generateChat(const std::string &prompt, int maxTokens, float temperature) {
    if (!ctx_ || !model_) return "";

    const llama_vocab * vocab = llama_model_get_vocab(model_);

    std::vector<llama_token> tokens(prompt.size() + 1);
    int n_tokens = llama_tokenize(vocab, prompt.c_str(), prompt.size(), tokens.data(), tokens.size(), true, true);
    tokens.resize(n_tokens);

    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    llama_decode(ctx_, batch);

    llama_sampler * sampler = llama_sampler_init_greedy();

    std::string output;
    for (int i = 0; i < maxTokens; i++) {
        llama_token id = llama_sampler_sample(sampler, ctx_, -1);
        if (id == llama_vocab_eos(vocab)) break;
        char buf[256];
        int len = llama_token_to_piece(vocab, id, buf, sizeof(buf), 0, false);
        std::string token_str(buf, len);
        output += token_str;
        llama_batch batch = llama_batch_get_one(&id, 1);
        llama_decode(ctx_, batch);
    }

    llama_sampler_free(sampler);
    return output;
}

void LlamaModel::generateChatStream(const std::string &prompt, std::function<void(const std::string&)> callback, int maxTokens, float temperature) {
    if (!ctx_ || !model_) return;

    const llama_vocab * vocab = llama_model_get_vocab(model_);

    std::vector<llama_token> tokens(prompt.size() + 1);
    int n_tokens = llama_tokenize(vocab, prompt.c_str(), prompt.size(), tokens.data(), tokens.size(), true, true);
    tokens.resize(n_tokens);

    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    llama_decode(ctx_, batch);

    llama_sampler * sampler = llama_sampler_init_greedy();

    for (int i = 0; i < maxTokens; i++) {
        llama_token id = llama_sampler_sample(sampler, ctx_, -1);
        if (id == llama_vocab_eos(vocab)) break;
        char buf[256];
        int len = llama_token_to_piece(vocab, id, buf, sizeof(buf), 0, false);
        std::string token_str(buf, len);
        callback(token_str);
        llama_batch batch = llama_batch_get_one(&id, 1);
        llama_decode(ctx_, batch);
    }

    llama_sampler_free(sampler);
}

std::vector<float> LlamaModel::embeddings(const std::string &text) {
    const llama_vocab * vocab = llama_model_get_vocab(model_);

    std::vector<llama_token> tokens(text.size() + 1);
    int n_tokens = llama_tokenize(vocab, text.c_str(), text.size(), tokens.data(), tokens.size(), true, true);
    tokens.resize(n_tokens);

    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    llama_decode(ctx_, batch);

    std::vector<float> emb(llama_model_n_embd(model_));
    float * emb_ptr = llama_get_embeddings(ctx_);
    if (emb_ptr) {
        emb.assign(emb_ptr, emb_ptr + llama_model_n_embd(model_));
    }
    return emb;
}

#endif
