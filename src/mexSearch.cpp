#include "../include/mexSearch.h"
#include <algorithm>
#include <cctype>
#include <iostream>

MexSearch::MexSearch()
    : currentMatch(0)
    , caseSensitive(false)
    , wholeWord(false)
    , regexMode(false)
{

}

void MexSearch::findMatches(const std::string &pattern, const std::vector<std::string> &document)
{
    matches.clear();
    if (pattern.empty()) return;

    std::string searchPattern = regexMode ? pattern : getRegexPattern(pattern);
    std::regex::flag_type flags = std::regex_constants::ECMAScript;
    if (!caseSensitive)
    {
        flags |= std::regex_constants::icase;
    }

    try
    {
        std::regex regexPatterns(searchPattern, flags);

        for (size_t lineNum = 0; lineNum < document.size(); ++lineNum)
        {
            const std::string& line = document[lineNum];
            std::sregex_iterator it(line.begin(), line.end(), regexPatterns);
            std::sregex_iterator end;

            for (; it != end; ++it)
            {
                matches.emplace_back(lineNum, std::make_pair(it->position(), it->position() + it->length()));
            }
        }
    }
    catch (const std::regex_error& e)
    {
        std::cerr << "Regex error: " << e.what() << std::endl;
    }
}

std::string MexSearch::getRegexPattern(const std::string &pattern) const
{
    if (wholeWord)
    {
        return "\\b" + std::regex_replace(pattern, std::regex("([.^$|()\\[\\]{}*+?\\\\])"), "\\$1") + "\\b";
    }

    return std::regex_replace(pattern, std::regex("([.^$|()\\[\\]{}*+?\\\\])"), "\\$1");
}

bool MexSearch::find(const std::string &pattern, const std::vector<std::string> &document)
{
    lastPattern = pattern;
    findMatches(pattern, document);
    currentMatch = matches.empty() ? 0 : 1;
    return !matches.empty();
}

bool MexSearch::findNext(const std::vector<std::string> &document)
{
    if (matches.empty())
    {
        if (lastPattern.empty()) return false;
        findMatches(lastPattern, document);
    }

    if (matches.empty()) return false;

    currentMatch++;
    if (currentMatch > matches.size())
    {
        currentMatch = 1;
    }

    return true;
}

bool MexSearch::findPrevious(const std::vector<std::string> &document)
{
    if (matches.empty())
    {
        if (lastPattern.empty()) return false;
        findMatches(lastPattern, document);
    }

    if (matches.empty()) return false;

    if (currentMatch <= 1)
    {
        currentMatch = matches.size();
    }
    else
    {
        currentMatch--;
    }

    return true;
}

void MexSearch::replaceCurrent(const std::string &replacement, std::vector<std::string> &document)
{
    if (matches.empty() or currentMatch == 0 or currentMatch > matches.size()) return;

    const auto& match = matches[currentMatch - 1];
    std::string& line = document[match.first];
    line.replace(match.second.first, match.second.second - match.second.first, replacement);

    size_t lengthDiff = replacement.length() - (match.second.second - match.second.first);
    for (auto& m : matches)
    {
        if (m.first == match.first and m.second.first > match.second.first)
        {
            m.second.first += lengthDiff;
            m.second.second += lengthDiff;
        }
    }
}

void MexSearch::replaceAll(const std::string &pattern, const std::string &replacement, std::vector<std::string> &document)
{
    lastPattern = pattern;
    findMatches(pattern, document);

    for (auto it = matches.rbegin(); it != matches.rend(); ++it)
    {
        std::string& line = document[it->first];
        line.replace(it->second.first, it->second.second - it->second.first, replacement);
    }
    matches.clear();
}

const std::vector<std::pair<size_t, std::pair<size_t, size_t>>>& MexSearch::getMatches() const
{
    return matches;
}

std::pair<size_t, std::pair<size_t, size_t>> MexSearch::getCurrentMatch() const
{
    if (currentMatch == 0 || currentMatch > matches.size())
    {
        return {0, {0, 0}};
    }
    return matches[currentMatch - 1];
}

void MexSearch::clearMatches()
{
    matches.clear();
    currentMatch = 0;
}

void MexSearch::setCaseSensitive(bool sensitive)
{
    caseSensitive = sensitive;
}

void MexSearch::setWholeWord(bool wholeWord)
{
    this->wholeWord = wholeWord;
}

void MexSearch::setRegexMode(bool regex)
{
    regexMode = regex;
}