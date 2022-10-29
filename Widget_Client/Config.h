#pragma once
#include <string>
#include "../Json/json/json.h"
#include <optional>

class Config {
    std::string configFileName_;
    Json::Value value_;
    Json::Reader reader_;
    std::map<std::string, std::string> parameters_;
public:
    Config(std::string configFileName);
    std::optional<std::string> getConfigValue(std::string key);
};
