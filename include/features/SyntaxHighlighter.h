#ifndef MEXEDIT_SYNTAX_HIGHLIGHTER_H
#define MEXEDIT_SYNTAX_HIGHLIGHTER_H

#include <string>
#include <vector>
#include <unordered_set>
#include <regex>
#include <filesystem>

/// @brief Features namespace for mexEdit \namespace mexedit::features
namespace mexedit::features
{
    /// @brief Types of syntax tokens \enum TokenType
    enum class TokenType
    {
        Normal,
        Keyword,
        String,
        Comment,
        Number,
        Operator,
        Preprocessor
    };

    /// @brief Syntax token with position and type \struct SyntaxToken
    struct SyntaxToken
    {
        size_t start;
        size_t length;
        TokenType type;
    };

    /// @brief Language definition for syntax highlighting \struct LanguageDefinition
    struct LanguageDefinition
    {
        std::string name;
        std::unordered_set<std::string> keywords;
        std::string lineCommentStart;
        std::string blockCommentStart;
        std::string blockCommentEnd;
        std::string stringDelimiters;
        std::regex numberPattern;
    };

    /// @brief Provides syntax highlighting for various programming languages
    class SyntaxHighlighter
    {
    public:
        /**
         * @brief Construct a new Syntax Highlighter object
         * 
         */
        SyntaxHighlighter();
        
        /**
         * @brief Destroy the Syntax Highlighter object
         * 
         */
        ~SyntaxHighlighter() = default;

        /**
         * @brief Detect the programming language of a file
         * 
         * @param filePath The path to the file
         */
        void detectLanguage(const std::filesystem::path& filePath);
        
        /**
         * @brief Set the current programming language for syntax highlighting
         * 
         * @param languageName The name of the programming language
         */
        void setLanguage(const std::string& languageName);
        
        /**
         * @brief Get the current programming language
         * 
         * @return std::string 
         */
        std::string getCurrentLanguage() const { return currentLanguage_; }

        /**
         * @brief Analyze a line of text and return syntax tokens
         * 
         * @param line The line of text to analyze
         * @return std::vector<SyntaxToken> 
         */
        std::vector<SyntaxToken> analyzeLine(const std::string& line) const;
        
        /**
         * @brief Check if a word is a keyword in the current language
         * 
         * @param word The word to check
         * @return true 
         * @return false 
         */
        bool isKeyword(const std::string& word) const;

        /**
         * @brief Get the list of supported programming languages
         * 
         * @return std::vector<std::string> 
         */
        static std::vector<std::string> getSupportedLanguages();

    private:
        std::string currentLanguage_;
        LanguageDefinition currentDefinition_;

        /**
         * @brief Load language definitions from files
         * 
         */
        void loadLanguageDefinitions();
        
        /**
         * @brief Get the language definition by name
         * 
         * @param languageName The name of the programming language
         * @return LanguageDefinition 
         */
        static LanguageDefinition getLanguageDefinition(const std::string& languageName) ;

        /**
         * @brief Create the language definition for C++
         * 
         * @return LanguageDefinition 
         */
        static LanguageDefinition createCppDefinition();
        
        /**
         * @brief Create the language definition for Python
         * 
         * @return LanguageDefinition 
         */
        static LanguageDefinition createPythonDefinition();
        
        /**
         * @brief Create the language definition for JavaScript
         * 
         * @return LanguageDefinition 
         */
        static LanguageDefinition createShellDefinition();
        
        /**
         * @brief Create the language definition for Plain Text
         * 
         * @return LanguageDefinition 
         */
        static LanguageDefinition createPlainTextDefinition();
    };
} // namespace mexedit::features

#endif // MEXEDIT_SYNTAX_HIGHLIGHTER_H
