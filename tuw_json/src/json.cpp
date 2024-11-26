#include "tuw_json/json.hpp"

#include <json/json.h>

#include <fstream>
#include <iostream>

void tuw_json::write(const std::string & filename, const std::string & key, const Json::Value & src)
{
  Json::Value json_data;
  json_data[key] = src;
  Json::StreamWriterBuilder writerBuilder;
  writerBuilder.settings_["indentation"] = " ";  // Disable indentation
  writerBuilder.settings_["sortKeys"] = false;

  std::string json_str = Json::writeString(writerBuilder, json_data);
  std::ofstream json_file(filename);
  if (json_file.is_open()) {
    json_file << json_str;
    json_file.close();
  } else {
    throw std::runtime_error("Failed to write json file " + filename);
  }
}

Json::Value tuw_json::read(const std::string & filename, const std::string & key)
{
  Json::Value des;
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open json file " + filename);
  }
  std::string jsonString((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  file.close();

  // Parse the JSON string
  Json::Value root;
  Json::Reader reader;

  if (reader.parse(jsonString, root)) {
    if (root.isMember(key)) {
      des = root[key];
    } else {
      des = root;
    }
  } else {
    throw std::runtime_error("Failed to parse json file " + filename);
  }
  return des;
}
