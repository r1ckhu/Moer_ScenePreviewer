#include "MoerRenderConfigLoader.h"

#include <iostream>

void MoerRenderConfigLoader::generate(MoerRenderConfig& config,
                                      const Json& config_json) {
   for (auto& element : config_json.items()) {
      const std::string& key = element.key();
      const Json& value = element.value();

      if (value.is_string()) {
         config.configParameters[key] = value.get<std::string>();
      } else if (value.is_number_integer()) {
         config.configParameters[key] = value.get<int>();
      } else if (value.is_boolean()) {
         config.configParameters[key] = value.get<bool>();
      } else {
         std::cerr << "Unsupported render config value type for key: " << key
                   << std::endl;
      }
   }
}
void MoerRenderConfigLoader::updateJson(const MoerRenderConfig& config,
                                        Json& config_json) {
   for (const auto& kv : config.configParameters) {
      const std::string& key = kv.first;
      const ConfigValueType& value = kv.second;

      std::visit([&config_json, &key](auto&& arg) { config_json[key] = arg; },
                 value);
   }
}
