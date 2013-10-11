#include "configuration/config.h"
#include "configuration/config_exception.h"

namespace datto_linux_client {

void Config::LoadDefaultConfig() {
  Config::LoadConfigFile(DEFAULT_CONFIG_PATH);
}

void Config::LoadConfigFile(std::string path) {
  throw ConfigException("Unimplemented");
}

std::string Config::GetString(std::string key) {
  throw ConfigException("Unimplemented");
}

}
