#ifndef TUW_JSON__JSON_HPP_
#define TUW_JSON__JSON_HPP_

#include <string>

namespace Json
{
class Value;
}

namespace tuw_json
{
void write(const std::string & filename, const std::string & key, const Json::Value & src);
Json::Value read(const std::string & filename, const std::string & key);

}  // namespace tuw_json
#endif  // TUW_JSON__JSON_HPP_
