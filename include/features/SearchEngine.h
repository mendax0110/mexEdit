#ifndef MEXEDIT_SEARCH_ENGINE_H
#define MEXEDIT_SEARCH_ENGINE_H

#include <string>
#include <vector>
#include <regex>
#include <functional>

/// @brief Features namespace for mexEdit \namespace mexedit::features
namespace mexedit::features
{
    /// @brief Represents a search match within a document \struct SearchMatch
    struct SearchMatch
    {
        size_t line;
        size_t startColumn;
        size_t endColumn;
        std::string matchedText;
    };

    /// @brief Options for search operations \struct SearchOptions
    struct SearchOptions
    {
        bool caseSensitive = false;
        bool useRegex = false;
        bool wholeWords = false;
        bool wrapAround = true;
    };

    /// @brief Provides search and replace functionality within text documents \class SearchEngine
    class SearchEngine
    {
    public:
        /// @brief Type alias for a document provider function \typedef DocumentProvider
        using DocumentProvider = std::function<std::vector<std::string>()>;

        /// @brief Type alias for a document updater function \typedef DocumentUpdater
        using DocumentUpdater = std::function<void(const std::vector<std::string>&)>;

        /**
         * @brief Construct a new Search Engine object
         * 
         */
        SearchEngine();
        
        /**
         * @brief Destroy the Search Engine object
         * 
         */
        ~SearchEngine() = default;

        /**
         * @brief Find all occurrences of a pattern in a document
         * 
         * @param pattern The search pattern
         * @param document The document to search in
         * @param options The search options
         * @return std::vector<SearchMatch> 
         */
        std::vector<SearchMatch> findAll(const std::string& pattern, const std::vector<std::string>& document, const SearchOptions& options = {});

        /**
         * @brief Find the next occurrence of a pattern in a document
         * 
         * @param pattern The search pattern
         * @param document The document to search in
         * @param startLine The line to start searching from
         * @param startColumn The column to start searching from
         * @param options The search options
         * @return SearchMatch 
         */
        SearchMatch findNext(const std::string& pattern, const std::vector<std::string>& document, size_t startLine = 0, size_t startColumn = 0, const SearchOptions& options = {});

        /**
         * @brief Find the previous occurrence of a pattern in a document
         * 
         * @param pattern The search pattern
         * @param document The document to search in
         * @param startLine The line to start searching from
         * @param startColumn The column to start searching from
         * @param options The search options
         * @return SearchMatch 
         */
        SearchMatch findPrevious(const std::string& pattern, const std::vector<std::string>& document, size_t startLine = 0, size_t startColumn = 0, const SearchOptions& options = {});

        /**
         * @brief Replace all occurrences of a pattern with a replacement string in a document
         * 
         * @param pattern The search pattern
         * @param replacement The replacement string
         * @param document The document to perform replacements in
         * @param options The search options
         * @return int The number of replacements made
         */
        int replaceAll(const std::string& pattern, const std::string& replacement, std::vector<std::string>& document, const SearchOptions& options = {});

        /**
         * @brief Replace the next occurrence of a pattern with a replacement string in a document
         * 
         * @param pattern The search pattern
         * @param replacement The replacement string
         * @param document The document to perform replacements in
         * @param line The line to start searching from
         * @param column The column to start searching from
         * @param options The search options
         * @return true if a replacement was made, false otherwise
         */
        bool replaceNext(const std::string& pattern, const std::string& replacement, std::vector<std::string>& document, size_t& line, size_t& column, const SearchOptions& options = {});

        /**
         * @brief Add a search pattern to the history
         * 
         * @param pattern The search pattern
         */
        void addToHistory(const std::string& pattern);
        
        /**
         * @brief Get the search history
         * 
         * @return std::vector<std::string> 
         */
        std::vector<std::string> getSearchHistory() const { return searchHistory_; }

        /**
         * @brief Clear the search history
         * 
         */
        void clearHistory() { searchHistory_.clear(); }

        /**
         * @brief Get the current search matches
         * 
         * @return const std::vector<SearchMatch>& 
         */
        const std::vector<SearchMatch>& getCurrentMatches() const { return currentMatches_; }
        
        /**
         * @brief Clear the current search matches
         * 
         */
        void clearCurrentSearch() { currentMatches_.clear(); currentPattern_.clear(); }

    private:
        std::vector<SearchMatch> currentMatches_;
        std::string currentPattern_;
        std::vector<std::string> searchHistory_;
        static constexpr size_t maxHistorySize_ = 50;

        /**
         * @brief Search for all occurrences of a pattern in a single line
         * 
         * @param line The line to search in
         * @param lineNumber The line number in the document
         * @param pattern The search pattern
         * @param options The search options
         * @return std::vector<SearchMatch> 
         */
        std::vector<SearchMatch> searchInLine(const std::string& line, size_t lineNumber, const std::string& pattern,  const SearchOptions& options) const;

        /**
         * @brief Create a regex pattern based on the search options
         * 
         * @param pattern The search pattern
         * @param options The search options
         * @return std::regex 
         */
        std::regex createRegexPattern(const std::string& pattern, const SearchOptions& options) const;
        
        /**
         * @brief Escape special regex characters in a string
         * 
         * @param input The input string
         * @return std::string The escaped string
         */
        static std::string escapeRegexSpecialChars(const std::string& input) ;
    };
} // namespace mexedit::features

#endif // MEXEDIT_SEARCH_ENGINE_H
