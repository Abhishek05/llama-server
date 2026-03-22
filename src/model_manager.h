#pragma once

#include <string>

class ModelManager {
public:
    ModelManager(std::string storagePath);

    bool pullModel(const std::string &modelId);
    bool pullModel(const std::string &modelId, const std::string &sourceUrl); // for direct URL
    bool pullModel(const std::string &modelId, const std::string &sourceUrl, const std::string &checksum); // internal
    std::string getModelPath(const std::string &modelId) const;

private:
    std::string storagePath_;
};
