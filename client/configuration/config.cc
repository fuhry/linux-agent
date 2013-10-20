#include <iostream>
#include <fstream>
#include <exception>
#include <stdexcept>


#include "configuration/config.h"
#include "configuration/config_exception.h"

#include <boost/regex.hpp>




namespace datto_linux_client {

  using namespace std;
  using namespace boost;   //  Using boost regex library until
                           //  gcc supports c++11 regexes..  at
                           //  that point in time only the #include and
                           //  "using namespace boost" will need to change
                           

void Config::LoadDefaultConfig() {
  Config::LoadConfigFile(DEFAULT_CONFIG_PATH);
}

//
//  LoadConfigFile():  Throws an exception if key_value_map_ is ! == nullptr.  Allocates
//  the map that is stored in key_value_map_, then parses the config file into key/value
//  pairs, which are stored in the static map.  Throws exceptions under quite a few 
//  circumstances.
//

void Config::LoadConfigFile(string path) {

  if (is_loaded()) {          // Bail if we have done this before
    string err = "Error: attempt to reuse Config by loading another file";
    throw ConfigException(err);
  }

  key_value_map_ = new map<string, string>;    // Allocate a new map for the config

  ifstream configin;
  
  configin.open(path.c_str(), ios::in);

  if (!configin.is_open()) {
    string err("Error opening ");
    err += path + " for input";
    throw ConfigException(err);
  }

  regex match_notcomment("([^#]*)#*");
  regex leading_spaces("^[ \t]+");
  regex trailing_spaces("[ \t]+$");
  regex split_line( "([-A-Za-z_]+)[ \t]*=[ \t]*([^ \t].+)", regex::extended );
  
  while(configin.good()) {           // Read the file

    string linein;

    getline(configin, linein);

    if (linein.empty()) {    // ignore blank lines
      continue;
    }

    if (linein.find_last_of("\r") != string::npos) {   // No DOS line endings allowed
      string err = "error: \\r found in input";
      throw ConfigException(err);
    }

    string without_comments;
    smatch match1;

    regex_search(linein, match1, match_notcomment);

    if (match1[1].str().length() == 0) {    //  Nothing but comments; ignore
      continue;;
    } else {
      without_comments = match1[1].str();
    }


    // trim

    string trimmed = regex_replace(without_comments, leading_spaces, "", format_first_only);
    trimmed = regex_replace(trimmed, trailing_spaces, "", format_first_only);
  
    if (trimmed.size() == 0) {
      continue;
    }
    smatch match2;
    if ( ! regex_search(trimmed, match2, split_line) ) {
      string err("Illformed line in config file: ");
      err += linein;
      throw ConfigException(err);
    }

    (*key_value_map_)[ match2[1].str() ] = match2[2].str();


  }


}

void Config::DumpMap() {             // For debugging

  cout << "*** Map dump requested ***" << endl << endl;

  for (auto& elem : *key_value_map_) {
    cout << "Key: " << elem.first << endl;
    cout << "Val: " << elem.second << endl << endl;
  }

}

int32_t Config::GetInt32(string key) {

  string valstring = GetString(key);   // Fetch the string first

  size_t bad_char_pos;
  int32_t converted_value;

  try {
    converted_value = stol(valstring, &bad_char_pos, 0);
  }
  catch (invalid_argument &e) {
    string err("invalid_argument exception raised while trying to convert ");
    err += valstring + " to int32_t";
    throw ConfigException(err);
  }
  catch (out_of_range &e) {
    string err("out_of_range exception raised while trying to convert ");
    err += valstring + " to int32_t";
    throw ConfigException(err);
  }

  if (bad_char_pos != valstring.length()) {
    string err("tried to convert invalid value ");
    err += valstring + " to int32_t";
    throw ConfigException(err);
  }

  return converted_value;

}

uint32_t Config::GetUInt32(string key) {

  string valstring = GetString(key);   // Fetch the string first

  size_t bad_char_pos;
  uint32_t converted_value;

  try {
    converted_value = stoul(valstring, &bad_char_pos, 0);
  }
  catch (invalid_argument &e) {
    string err("invalid_argument exception raised while trying to convert ");
    err += valstring + " to uint32_t";
    throw ConfigException(err);
  }
  catch (out_of_range &e) {
    string err("out_of_range exception raised while trying to convert ");
    err += valstring + " to uint32_t";
    throw ConfigException(err);
  }

  if (bad_char_pos != valstring.length()) {
    string err("tried to convert invalid value ");
    err += valstring + " to uint32_t";
    throw ConfigException(err);
  }

  return converted_value;

}


int64_t Config::GetInt64(string key) {

  string valstring = GetString(key);   // Fetch the string first

  size_t bad_char_pos;
  int64_t converted_value;

  try {
    converted_value = stoll(valstring, &bad_char_pos, 0);
  }
  catch (invalid_argument &e) {
    string err("invalid_argument exception raised while trying to convert ");
    err += valstring + " to int64_t";
    throw ConfigException(err);
  }
  catch (out_of_range &e) {
    string err("out_of_range exception raised while trying to convert ");
    err += valstring + " to int64_t";
    throw ConfigException(err);
  }

  if (bad_char_pos != valstring.length()) {
    string err("tried to convert invalid value ");
    err += valstring + " to int64_t";
    throw ConfigException(err);
  }

  return converted_value;

}

uint64_t Config::GetUInt64(string key) {

  string valstring = GetString(key);   // Fetch the string first

  size_t bad_char_pos;
  uint64_t converted_value;

  try {
    converted_value = stoull(valstring, &bad_char_pos, 0);
  }
  catch (invalid_argument &e) {
    string err("invalid_argument exception raised while trying to convert ");
    err += valstring + " to uint64_t";
    throw ConfigException(err);
  }
  catch (out_of_range &e) {
    string err("out_of_range exception raised while trying to convert ");
    err += valstring + " to uint64_t";
    throw ConfigException(err);
  }

  if (bad_char_pos != valstring.length()) {
    string err("tried to convert invalid value ");
    err += valstring + " to uint64_t";
    throw ConfigException(err);
  }

  return converted_value;

}

double Config::GetDouble(string key) {


  string valstring = GetString(key);   // Fetch the string first

  size_t bad_char_pos;
  double converted_value;

  try {
    converted_value = stod(valstring, &bad_char_pos);
  }
  catch (invalid_argument &e) {
    string err("invalid_argument exception raised while trying to convert ");
    err += valstring + " to double";
    throw ConfigException(err);
  }
  catch (out_of_range &e) {
    string err("out_of_range exception raised while trying to convert ");
    err += valstring + " to double";
    throw ConfigException(err);
  }

  if (bad_char_pos != valstring.length()) {
    string err("tried to convert invalid value ");
    err += valstring + " to double";
    throw ConfigException(err);
  }

  return converted_value;

  
}

string Config::GetString(string key) {

  string valstring;

  try {
    valstring = (*key_value_map_).at(key);
  }
  catch (out_of_range &e) {
    string err("Error: attempt to retrieve value for non-existent key ");
    err += key;
    throw ConfigException(err);
  }

  return valstring;

}

//
// private methods
//


//
// statics
//

map<string, string> *Config::key_value_map_ = nullptr;


}
