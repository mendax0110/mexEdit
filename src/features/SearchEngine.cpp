#include "../include/features/SearchEngine.h"
#include <algorithm>
#include <stdint.h>

using namespace mexedit::features;

SearchEngine::SearchEngine()
{

}

std::vector<SearchMatch> SearchEngine::findAll(const std::string& pattern, const std::vector<std::string>& document, const SearchOptions& options)
{
    std::vector<SearchMatch> matches;
    
    for (size_t lineNum = 0; lineNum < document.size(); ++lineNum)
    {
        auto lineMatches = searchInLine(document[lineNum], lineNum, pattern, options);
        matches.insert(matches.end(), lineMatches.begin(), lineMatches.end());
    }
    
    currentMatches_ = matches;
    currentPattern_ = pattern;
    addToHistory(pattern);
    
    return matches;
}

SearchMatch SearchEngine::findNext(const std::string& pattern, const std::vector<std::string>& document, size_t startLine, size_t startColumn, const SearchOptions& options)
{    
    for (size_t lineNum = startLine; lineNum < document.size(); ++lineNum)
    {
        size_t searchStart = (lineNum == startLine) ? startColumn : 0;
        const std::string& line = document[lineNum];
        
        if (searchStart >= line.length()) continue;
        
        std::string searchLine = line.substr(searchStart);
        auto lineMatches = searchInLine(searchLine, lineNum, pattern, options);
        
        if (!lineMatches.empty())
        {
            auto& match = lineMatches[0];
            match.startColumn += searchStart;
            match.endColumn += searchStart;
            return match;
        }
    }
    
    // Wrap around if enabled
    if (options.wrapAround && startLine > 0)
    {
        return findNext(pattern, document, 0, 0, SearchOptions{options.caseSensitive, options.useRegex, options.wholeWords, false});
    }
    
    return {};
}

SearchMatch SearchEngine::findPrevious(const std::string& pattern,const std::vector<std::string>& document,size_t startLine,size_t startColumn,const SearchOptions& options)
{
    
    for (int lineNum = static_cast<int>(std::min(startLine, document.size() - 1)); lineNum >= 0; --lineNum)
    {
        const std::string& line = document[lineNum];
        
        auto lineMatches = searchInLine(line, lineNum, pattern, options);
        if (!lineMatches.empty())
        {
            if (static_cast<size_t>(lineNum) == startLine)
            {
                for (auto it = lineMatches.rbegin(); it != lineMatches.rend(); ++it)
                {
                    if (it->startColumn < startColumn)
                    {
                        return *it;
                    }
                }
            } 
            else
            {
                return lineMatches.back();
            }
        }
    }
    
    // Wrap around if enabled
    if (options.wrapAround && startLine < document.size() - 1)
    {
        return findPrevious(pattern, document, document.size() - 1, SIZE_MAX, SearchOptions{options.caseSensitive, options.useRegex, options.wholeWords, false}); // Disable wrap for recursive call
    }
    
    return {};
}

int SearchEngine::replaceAll(const std::string& pattern, const std::string& replacement, std::vector<std::string>& document, const SearchOptions& options)
{
    int replacements = 0;
    
    for (size_t lineNum = 0; lineNum < document.size(); ++lineNum)
    {
        std::string& line = document[lineNum];
        
        try
        {
            if (options.useRegex)
            {
                std::regex regex = createRegexPattern(pattern, options);
                std::string newLine = std::regex_replace(line, regex, replacement);
                
                if (newLine != line)
                {
                    std::sregex_iterator start(line.begin(), line.end(), regex);
                    std::sregex_iterator end;
                    replacements += static_cast<int>(std::distance(start, end));
                    
                    line = newLine;
                }
            }
            else
            {
                std::string searchPattern = options.caseSensitive ? pattern : pattern;
                std::string searchLine = options.caseSensitive ? line : line;
                
                if (!options.caseSensitive)
                {
                    std::transform(searchLine.begin(), searchLine.end(), searchLine.begin(), ::tolower);
                    std::transform(searchPattern.begin(), searchPattern.end(), searchPattern.begin(), ::tolower);
                }
                
                size_t pos = 0;
                while ((pos = searchLine.find(searchPattern, pos)) != std::string::npos)
                {
                    line.replace(pos, pattern.length(), replacement);
                    searchLine.replace(pos, pattern.length(), replacement);
                    pos += replacement.length();
                    ++replacements;
                }
            }
        }
        catch (const std::regex_error&)
        {
            // Invalid regex, skip this line
        }
    }
    
    return replacements;
}

bool SearchEngine::replaceNext(const std::string& pattern, const std::string& replacement, std::vector<std::string>& document, size_t& line, size_t& column, const SearchOptions& options)
{
    
    SearchMatch match = findNext(pattern, document, line, column, options);
    if (match.line < document.size())
    {
        std::string& docLine = document[match.line];
        docLine.replace(match.startColumn, match.endColumn - match.startColumn, replacement);
        
        line = match.line;
        column = match.startColumn + replacement.length();
        return true;
    }
    
    return false;
}

void SearchEngine::addToHistory(const std::string& pattern)
{
    auto it = std::find(searchHistory_.begin(), searchHistory_.end(), pattern);
    if (it != searchHistory_.end())
    {
        searchHistory_.erase(it);
    }
    
    searchHistory_.insert(searchHistory_.begin(), pattern);
    
    if (searchHistory_.size() > maxHistorySize_)
    {
        searchHistory_.resize(maxHistorySize_);
    }
}

std::vector<SearchMatch> SearchEngine::searchInLine(const std::string& line, size_t lineNumber, const std::string& pattern, const SearchOptions& options) const
{
    std::vector<SearchMatch> matches;
    
    if (pattern.empty())
    {
        return matches;
    }
    
    try
    {
        if (options.useRegex)
        {
            std::regex regex = createRegexPattern(pattern, options);
            std::sregex_iterator start(line.begin(), line.end(), regex);
            std::sregex_iterator end;
            
            for (std::sregex_iterator i = start; i != end; ++i)
            {
                const std::smatch& match = *i;
                matches.push_back({
                    lineNumber,
                    static_cast<size_t>(match.position()),
                    static_cast<size_t>(match.position() + match.length()),
                    match.str()
                });
            }
        }
        else
        {
            std::string searchLine = options.caseSensitive ? line : line;
            std::string searchPattern = options.caseSensitive ? pattern : pattern;
            
            if (!options.caseSensitive)
            {
                std::transform(searchLine.begin(), searchLine.end(), searchLine.begin(), ::tolower);
                std::transform(searchPattern.begin(), searchPattern.end(), searchPattern.begin(), ::tolower);
            }
            
            size_t pos = 0;
            while ((pos = searchLine.find(searchPattern, pos)) != std::string::npos)
            {
                matches.push_back({
                    lineNumber,
                    pos,
                    pos + pattern.length(),
                    line.substr(pos, pattern.length())
                });
                pos += pattern.length();
            }
        }
    }
    catch (const std::regex_error&)
    {
        // Invalid regex, return empty matches
    }
    
    return matches;
}

std::regex SearchEngine::createRegexPattern(const std::string& pattern, const SearchOptions& options) const
{
    std::regex_constants::syntax_option_type flags = std::regex_constants::ECMAScript;
    
    if (!options.caseSensitive)
    {
        flags |= std::regex_constants::icase;
    }
    
    std::string regexPattern = pattern;
    
    if (!options.useRegex)
    {
        regexPattern = escapeRegexSpecialChars(pattern);
    }
    
    if (options.wholeWords)
    {
        regexPattern = "\\b" + regexPattern + "\\b";
    }
    
    return std::regex(regexPattern, flags);
}

std::string SearchEngine::escapeRegexSpecialChars(const std::string& input) const
{
    std::string result;
    result.reserve(input.size() * 2); // Reserve space for potential escapes
    
    for (char c : input)
    {
        switch (c)
        {
            case '^': case '$': case '\\': case '.': case '*': case '+':
            case '?': case '(': case ')': case '[': case ']': case '{':
            case '}': case '|':
                result += '\\';
                result += c;
                break;
            default:
                result += c;
                break;
        }
    }
    
    return result;
}
