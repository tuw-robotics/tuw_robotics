/**
 * @author Markus Bader <markus.bader@tuwien.ac.at>
 **/

#include <tuw/string.hpp>

bool tuw::equal(const std::string & str1, const std::string & str2, bool caseinsensitive)
{
  if (str1.size() != str2.size()) {
    return false;
  }
  if (caseinsensitive) {
    return std::equal(
      str1.begin(), str1.end(), str2.begin(), [](unsigned char a, unsigned char b) {
        auto A = std::tolower(a);
        auto B = std::tolower(b);
        return A == B;
      });
  } else {
    return std::equal(str1.begin(), str1.end(), str2.begin());
  }
}
