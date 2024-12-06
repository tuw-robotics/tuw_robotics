/**
 * @author Markus Bader <markus.bader@tuwien.ac.at>
 **/
#ifndef TUW__STRING_HPP
#define TUW__STRING_HPP

#include <string>

namespace tuw
{
/**
   * compares two string case sensitive or  case insensitive
   * @param str1
   * @param str2
   * @param caseinsensitive on true it compares case insensitive
   * @return true if the strings are equal
   */
bool equal(const std::string & str1, const std::string & str2, bool caseinsensitive = true);

}  // namespace tuw
#endif  // TUW__STRING_HPP
