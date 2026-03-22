#include "model_manager.h"

//#include <httplib.h>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static bool fileExists(const std::string &path) {
    return access(path.c_str(), F_OK) == 0;
}

static bool createDirectories(const std::string &path) {
    std::string cmd = "mkdir -p " + path;
    return system(cmd.c_str()) == 0;
}

static bool removeFile(const std::string &path) {
    return std::remove(path.c_str()) == 0;
}

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

ModelManager::ModelManager(std::string storagePath) : storagePath_(std::move(storagePath)) {
    createDirectories(storagePath_);
}

bool ModelManager::pullModel(const std::string &modelId) {
    auto targetPath = getModelPath(modelId);
    if (fileExists(targetPath)) {
        if (!isValidGGUF(targetPath)) {
            std::cerr << "[ModelManager] Found invalid GGUF file at " << targetPath << ". removing." << std::endl;
            removeFile(targetPath);
        } else {
            return true;
        }
    }

#ifdef USE_LLAMA_CPP
    std::string sourceUrl;
    if (const char* env_url = std::getenv("ILLAMA_MODEL_URL"); env_url && *env_url) {
        sourceUrl = env_url;
    } else if (modelId == "llama7b") {
        // Replace this with a valid GGUF URL you have access to.
        sourceUrl = "https://huggingface.co/<your-model>/resolve/main/llama7b.gguf";
    }

    if (sourceUrl.empty()) {
        std::cerr << "[ModelManager] No download URL configured. "
                  << "Set ILLAMA_MODEL_URL or copy a GGUF to " << targetPath << std::endl;
        return false;
    }

    return pullModel(modelId, sourceUrl);
#else
    // stub mode
    std::ofstream file(targetPath);
    file << "dummy model";
    return true;
#endif
}

bool ModelManager::pullModel(const std::string &modelId, const std::string &sourceUrl) {
    return pullModel(modelId, sourceUrl, "");
}

bool ModelManager::pullModel(const std::string &modelId, const std::string &sourceUrl, const std::string &expectedChecksum) {
    auto targetPath = getModelPath(modelId);

    if (fileExists(targetPath)) {
        // TODO: verify checksum if provided
        return true;
    }

    std::stringstream ss;
    ss << "curl -L -o " << std::quoted(targetPath) << " " << std::quoted(sourceUrl);
    std::string cmd = ss.str();
    int exitCode = std::system(cmd.c_str());
    if (exitCode != 0 || !fileExists(targetPath)) {
        return false;
    }

    if (!isValidGGUF(targetPath)) {
        std::cerr << "[ModelManager] Downloaded model file is not valid GGUF: " << targetPath << std::endl;
        removeFile(targetPath);
        return false;
    }

    // TODO: verify checksum
    return true;
}

std::string ModelManager::getModelPath(const std::string &modelId) const {
    return storagePath_ + "/" + modelId + ".gguf";
}
