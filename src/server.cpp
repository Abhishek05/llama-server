#include "llama_wrapper.h"
#include "model_manager.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

int main(int argc, char** argv) {
    std::string storagePath = std::string(getenv("HOME") ? getenv("HOME") : ".") + "/.illama/models";

    ModelManager manager(storagePath);
    LlamaModel model;

    if (argc >= 2) {
        std::string cmd = argv[1];
        if (cmd == "pull") {
            if (argc != 3) {
                std::cerr << "usage: illama pull <model-id>" << std::endl;
                return 1;
            }
            bool ok = manager.pullModel(argv[2]);
            std::cout << (ok ? "model pulled" : "model pull failed") << std::endl;
            return ok ? 0 : 2;
        } else if (cmd == "run") {
            if (argc < 4) {
                std::cerr << "usage: illama run <model-id> <prompt>" << std::endl;
                return 1;
            }
            if (!model.loadModel(manager.getModelPath(argv[2]))) {
                std::cerr << "cannot load model" << std::endl;
                return 3;
            }
            std::string prompt = argv[3];
            std::string out = model.generateChat(prompt);
            std::cout << out << std::endl;
            return 0;
        } else if (cmd == "serve") {
            if (argc >= 3) {
                std::string modelPathOrId = argv[2];
                std::string modelPath;
                if (modelPathOrId.size() > 5 && modelPathOrId.substr(modelPathOrId.size() - 5) == ".gguf") {
                    modelPath = modelPathOrId;
                } else {
                    if (!manager.pullModel(modelPathOrId)) {
                        std::cerr << "cannot pull model " << modelPathOrId << ", please ensure .gguf exists or set ILLAMA_MODEL_URL" << std::endl;
                        return 5;
                    }
                    modelPath = manager.getModelPath(modelPathOrId);
                }

                if (!model.loadModel(modelPath)) {
                    std::cerr << "cannot load model" << std::endl;
                    return 4;
                }
            }

            httplib::Server svr;
            svr.Post("/v1/chat/completions", [&](const httplib::Request& req, httplib::Response& res){
                try {
                    auto payload = json::parse(req.body);
                    std::string model_name = payload.value("model", "default");
                    bool stream = payload.value("stream", false);
                    auto messages = payload["messages"];
                    if (messages.empty()) {
                        throw std::runtime_error("messages array is required");
                    }
                    std::string prompt;
                    for (auto& msg : messages) {
                        std::string role = msg["role"];
                        std::string content = msg["content"];
                        if (role == "user") {
                            prompt += content + "\n";
                        } else if (role == "assistant") {
                            prompt += content + "\n";
                        }
                        // ignore system for now
                    }
                    prompt = prompt.substr(0, prompt.size() - 1); // remove last \n

                    if (stream) {
                        res.set_header("Content-Type", "text/plain");
                        res.set_chunked_content_provider("text/plain", [&](size_t offset, httplib::DataSink& sink) {
                            model.generateChatStream(prompt, [&](const std::string& token) {
                                sink.write(token.c_str(), token.size());
                            });
                            sink.done();
                            return true;
                        });
                    } else {
                        std::string reply = model.generateChat(prompt);

                        json out = {
                            {"id", "illama-" + std::to_string(time(nullptr))},
                            {"object", "chat.completion"},
                            {"created", time(nullptr)},
                            {"model", model_name},
                            {"choices", json::array({
                                {
                                    {"index", 0},
                                    {"message", {
                                        {"role", "assistant"},
                                        {"content", reply}
                                    }},
                                    {"finish_reason", "stop"}
                                }
                            })},
                            {"usage", {
                                {"prompt_tokens", 0}, // TODO: count tokens
                                {"completion_tokens", 0},
                                {"total_tokens", 0}
                            }}
                        };
                        res.set_content(out.dump(), "application/json");
                    }
                } catch (std::exception &e) {
                    res.status = 400;
                    res.set_content(json({{"error", {{"message", e.what()}}}}).dump(), "application/json");
                }
            });

            std::cout << "Starting server at 0.0.0.0:11434 (or ctrl-C to stop)" << std::endl;
            svr.listen("0.0.0.0", 11434);
            return 0;
        }
    }

    std::cout << "Usage:\n"
              << "  illama pull <model-id>\n"
              << "  illama run <model-id> <prompt>\n"
              << "  illama serve <model-id>\n";

    return 0;
}
