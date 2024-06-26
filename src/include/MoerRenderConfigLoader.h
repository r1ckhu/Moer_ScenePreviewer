#pragma once
#include <nlohmann/json.hpp>

#include "MoerRenderConfig.h"

using Json = nlohmann::json;
class MoerRenderConfigLoader {
  public:
   static void generate(MoerRenderConfig& config, const Json& config_json);
   static void updateJson(const MoerRenderConfig& config, Json& config_json);
};