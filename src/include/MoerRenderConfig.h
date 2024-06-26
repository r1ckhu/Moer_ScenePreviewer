#pragma once
#include <string>
#include <unordered_map>
#include <variant>

using ConfigValueType = std::variant<std::string, int, bool>;
using ConfigMapType = std::unordered_map<std::string, ConfigValueType>;
class MoerRenderConfig {
  public:
    ConfigMapType configParameters;
};
