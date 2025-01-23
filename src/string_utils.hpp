#pragma once

#include <string>
#include <vector>

namespace manelemax::stringutils
{

std::string  remove_ro_diacritics(const std::wstring& wstr);
std::string& to_lower(std::string& str);
std::string& keep_alpha_and_spaces(std::string& str);

std::vector<std::string> split(const std::string& str);
std::vector<std::string>
all_word_aligned_substrings(const std::string& str, std::size_t max_words = 0);

}  // namespace manelemax::stringutils
