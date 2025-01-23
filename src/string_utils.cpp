#include "string_utils.hpp"

#include <algorithm>
#include <functional>
#include <cwchar>
#include <cctype>
#include <sstream>
#include <memory>

namespace manelemax::stringutils
{

std::string remove_ro_diacritics(const std::wstring& wstr)
{
    if (wstr.empty())
    {
        return "";
    }

    std::wstring wstr_new {wstr};

    constexpr auto convert = [](wchar_t c) {
        switch (c)
        {
            case L'ă': return L'a';
            case L'â': return L'a';
            case L'Ă': return L'A';
            case L'Â': return L'A';
            case L'î': return L'i';
            case L'Î': return L'I';
            case L'ș': return L's';
            case L'Ș': return L'S';
            case L'ț': return L't';
            case L'Ț': return L'T';
        }
        return c;
    };

    constexpr auto cannot_narrow = [](wchar_t c) { return std::wctob(c) == EOF; };

    for (wchar_t& c : wstr_new)
    {
        c = convert(c);
    }

    wstr_new.erase(std::remove_if(wstr_new.begin(), wstr_new.end(), cannot_narrow));

    std::string str;
    str.reserve(wstr.size());
    for (const wchar_t& c : wstr_new)
    {
        str.push_back(char(std::wctob(c)));
    }

    return str;
}

std::string& to_lower(std::string& str)
{
    for (char& c : str)
    {
        c = char(std::tolower(c));
    }
    return str;
}

std::string& keep_alpha_and_spaces(std::string& str)
{
    str.erase(std::remove_if(str.begin(), str.end(), [](char c) {
        return !std::isalpha(c) && !std::isspace(c);
    }));
    return str;
}

std::vector<std::string> split(const std::string& str)
{
    if (str.empty())
    {
        return {};
    }

    std::istringstream       ss {str};
    std::vector<std::string> words;
    for (std::string word; ss >> word;)
    {
        words.push_back(std::move(word));
    }
    return words;
}

std::vector<std::string> all_word_aligned_substrings(const std::string& str, std::size_t max_words)
{
    if (str.empty())
    {
        return {};
    }

    std::vector<std::string>       result;
    const std::vector<std::string> words = split(str);

    if (max_words == 0 || max_words > words.size())
    {
        max_words = words.size();
    }

    for (size_t word_count = 1; word_count <= max_words; ++word_count)
    {
        for (size_t start_word = 0; start_word < words.size() - word_count + 1; ++start_word)
        {
            std::ostringstream ss;
            for (size_t crt_word = start_word; crt_word != start_word + word_count; ++crt_word)
            {
                ss << words[crt_word];
                if (crt_word != start_word + word_count - 1)
                {
                    ss << ' ';
                }
            }
            result.push_back(std::move(ss).str());
        }
    }

    return result;
}

}  // namespace manelemax::stringutils
