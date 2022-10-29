#include "Config.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <QDebug>
#include <QCoreApplication>

Config::Config(std::string configFileName) : configFileName_{ configFileName }
{
    std::ifstream file{ configFileName_ };
    if (!file.is_open()) {
        throw std::exception("Config file not found");
    }
    std::stringstream stream;
    stream << file.rdbuf();
    reader_.parse(stream.str(), value_);
    for (Json::Value::const_iterator it = value_.begin(); it != value_.end(); ++it) {
        parameters_.emplace(it.key().asString(), it->asString());
    }
}

std::optional<std::string> Config::getConfigValue(std::string key)
{
    if (parameters_.find(key)==parameters_.end()) {
        return std::nullopt;
    }
    return parameters_[key];
}
