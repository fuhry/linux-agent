#ifndef DATTO_CLIENT_CONFIGURATION_CONFIG_H_
#define DATTO_CLIENT_CONFIGURATION_CONFIG_H_

#include <string>
#include <map>

#include <stdint.h>

#include "configuration/config_exception.h"

namespace datto_linux_client {

static const char DEFAULT_CONFIG_DIR[] = "/etc/datto/";
static const char DEFAULT_CONFIG_PATH[] = "/etc/datto/dattod.conf";

// A configuration file should be of the following format:
//
// # Comment lines start with a #
//
// # Above line is blank, and skipped
// # Keys can be [-A-Za-z_]
// string_key=string_value_no_quotes
// double_key=1234.984873234 # comment can go here too
// int_key=1234 # newlines are \n only, throw ConfigException on \r\n
//  key_with_leading_space = isokay # as are spaces before and after the =
//
//
// Config is never initialized, it is only static methods. As we can't
// initalize it, don't worry about destructors / non-copying
class Config {
 public:
  // If either of these two methods are called more than once, throw
  // an exception on the second call
  // 
  // All values should be loaded as strings and parsed to the correct type
  // on the corresponding Get*(std::string key) call. The caller will be
  // responsible for saving the parsed value - don't worry about caching
  // in this class.
  //
  // ConfigException if an unparsable line is encounted
  // ConfigException if a key isn't defined
  static void LoadDefaultConfig();
  static void LoadConfigFile(std::string path);

  // ConfigException if a value isn't completely convertable to the
  //                 corresponding type (12ab is not convertable to 12)
  // ConfigException if the file wasn't loaded yet
  // ConfigException if the value doesn't exist
  static int32_t GetInt32(std::string key);
  static uint32_t GetUInt32(std::string key);

  static int64_t GetInt64(std::string key);
  static uint64_t GetUInt64(std::string key);

  static double GetDouble(std::string key);

  static std::string GetString(std::string key);

  // Don't worry about GetArray or other data types until we need them

 private:
  Config() { };
  static void ParseConfigFile();

  // This pointer will never be freed and should be initialized in either
  // LoadDefaultConfig or LoadConfigFile. See http://goo.gl/uQG7FR for details
  static std::map<std::string, std::string> *key_value_map_;
};
}

#endif //  DATTO_CLIENT_CONFIGURATION_CONFIG_H_

