#ifndef MEXEDIT_MEXSEARCH_H
#define MEXEDIT_MEXSEARCH_H

#include <string>
#include <vector>
#include <utility>
#include <regex>

/// @brief MexSearch is a class that provides search and replace functionality in the MexEdit text editor. \class MexSearch
class MexSearch
{
public:

    /**
     * @brief Constructs a MexSearch object, initializing search settings and state.
     */
    MexSearch();

    /**
     * @brief Finds all occurrences of a pattern in the document.
     * @param pattern The search pattern to find. Can be a simple string or a regex pattern.
     * @param document The document to search in, represented as a vector of strings (lines).
     * @return A boolean indicating whether any matches were found.
     */
    bool find(const std::string& pattern, const std::vector<std::string>& document);

    /**
     * @brief Finds the next occurrence of the last searched pattern in the document.
     * @param document The document to search in, represented as a vector of strings (lines).
     * @return A boolean indicating whether a next match was found.
     */
    bool findNext(const std::vector<std::string>& document);

    /**
     * @brief Finds the previous occurrence of the last searched pattern in the document.
     * @param document The document to search in, represented as a vector of strings (lines).
     * @return A boolean indicating whether a previous match was found.
     */
    bool findPrevious(const std::vector<std::string>& document);

    /**
     * @brief Replaces the current match with a replacement string.
     * @param replacement The string to replace the current match with.
     * @param document The document to modify, represented as a vector of strings (lines).
     */
    void replaceCurrent(const std::string& replacement, std::vector<std::string>& document);

    /**
     * @brief Replaces all occurrences of the last searched pattern with a replacement string.
     * @param pattern The search pattern to replace.
     * @param replacement The string to replace the pattern with.
     * @param document The document to modify, represented as a vector of strings (lines).
     */
    void replaceAll(const std::string& pattern, const std::string& replacement,
                    std::vector<std::string>& document);

    /**
     * @brief Gets the matches found by the last search operation.
     */
    const std::vector<std::pair<size_t, std::pair<size_t, size_t>>>& getMatches() const;

    /**
     * @brief Clears the current matches and resets the search state.
     */
    void clearMatches();

    /**
     * @brief Gets the current match, if any.
     * @return A pair containing the line number and a pair of start and end indices of the match.
     */
    std::pair<size_t, std::pair<size_t, size_t>> getCurrentMatch() const;

    /**
     * @brief Gets the last search pattern used.
     * @return The last search pattern as a string.
     */
    const std::string& getLastPattern() const { return lastPattern; }

    /**
     * @brief Sets whether the search should be case sensitive.
     * @param sensitive A boolean indicating whether the search should be case sensitive.
     */
    void setCaseSensitive(bool sensitive);

    /**
     * @brief Sets whether the search should match whole words only.
     * @param wholeWord A boolean indicating whether to match whole words only.
     */
    void setWholeWord(bool wholeWord);

    /**
     * @brief Sets whether the search should use regex patterns.
     * @param regex A boolean indicating whether to use regex for searching.
     */
    void setRegexMode(bool regex);

private:
    std::string lastPattern;
    std::vector<std::pair<size_t, std::pair<size_t, size_t>>> matches; // line, (start, end)
    size_t currentMatch;
    bool caseSensitive;
    bool wholeWord;
    bool regexMode;

    /**
     * @brief Finds matches in the document based on the current search settings.
     * @param pattern The search pattern to find.
     * @param document The document to search in, represented as a vector of strings (lines).
     */
    void findMatches(const std::string& pattern, const std::vector<std::string>& document);

    /**
     * @brief Converts a search pattern to a regex pattern if regex mode is enabled.
     * @param pattern The search pattern to convert.
     * @return A string containing the regex pattern.
     */
    std::string getRegexPattern(const std::string& pattern) const;
};
#endif // MEXEDIT_MEXSEARCH_H