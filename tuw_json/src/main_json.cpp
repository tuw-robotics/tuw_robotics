#include <json/json.h>

#include <iostream>

int main()
{
  // Creating a JSON object
  Json::Value jsonObject;

  // Adding key-value pairs to the JSON object
  jsonObject["name"] = "John Doe";
  jsonObject["age"] = 25;
  jsonObject["city"] = "New York";

  // Converting the JSON object to a string
  std::string jsonString = jsonObject.toStyledString();

  // Printing the JSON string
  std::cout << "JSON String:\n" << jsonString << "\n\n";

  // Parsing a JSON string
  Json::Value parsedObject;
  Json::CharReaderBuilder reader;
  std::istringstream jsonStream(jsonString);
  Json::parseFromStream(reader, jsonStream, &parsedObject, nullptr);

  // Accessing values from the parsed JSON object
  std::string name = parsedObject["name"].asString();
  int age = parsedObject["age"].asInt();
  std::string city = parsedObject["city"].asString();

  // Displaying the parsed values
  std::cout << "Parsed Values:\n";
  std::cout << "Name: " << name << "\n";
  std::cout << "Age: " << age << "\n";
  std::cout << "City: " << city << "\n";

  return 0;
}
