#ifndef MEXEDIT_MEXSYNTAX_H
#define MEXEDIT_MEXSYNTAX_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <ncurses.h>
#include <regex>
#include <memory>

/// MexSyntax is a class that provides syntax highlighting for various programming languages in the MexEdit text editor. \class MexSyntax
class MexSyntax
{
public:

    /**
     * @brief Constructs a MexSyntax object, initializing syntax highlighting rules for various languages.
     */
    MexSyntax();

    /**
     * @brief Detects the language of the file based on its extension and applies the corresponding syntax highlighting rules.
     * @param filename The name of the file to detect the language for.
     */
    void detectLanguage(const std::string& filename);

    /**
     * @brief Highlights a line of code based on the current language's syntax rules.
     * @param line The line of code to highlight.
     * @param yPos The vertical position (line number) in the editor where the line is displayed.
     * @param startCol The starting column position in the editor where the line begins.
     */
    void highlightLine(const std::string& line, int yPos, int startCol);

    /**
     * @brief Clears the cached syntax highlighting rules and keywords for the current language.
     */
    void clearCache();

    /**
     * @brief Struct representing a syntax highlighting rule. \struct HighlightRule
     */
    struct HighlightRule
    {
        std::regex pattern;
        int colorPair;
        bool wholeWord;
    };

private:
    std::string currentLanguage;
    std::unordered_map<std::string, std::vector<HighlightRule>> languageRules;
    std::unordered_map<std::string, std::string> fileExtensions;

    std::vector<HighlightRule> currentRules;
    std::unordered_set<std::string> currentKeywords;

    /**
     * @brief Initializes the syntax highlighting rules for various programming languages.
     */
    void initLanguages();

    /**
     * @brief Adds syntax highlighting rules for C++ language.
     */
    void addCPPRules();

    /**
     * @brief Adds syntax highlighting rules for C language.
     */
    void addCRules();

    /**
     * @brief Adds syntax highlighting rules for Shell scripts.
     */
    void addShellRules();

    /**
     * @brief Adds syntax highlighting rules for Python language.
     */
    void addPythonRules();

    /**
     * @brief Adds syntax highlighting rules for Markdown language.
     */
    void addMarkdownRules();

    /**
     * @brief Adds a syntax highlighting rule for a specific language.
     * @param lang The language to which the rule applies.
     * @param pattern The regex pattern for the rule.
     * @param colorPair The color pair to use for highlighting.
     * @param wholeWord Whether the rule should match whole words only.
     */
    void addRule(const std::string& lang, const std::string& pattern,
                 int colorPair, bool wholeWord = false);

    /**
     * @brief Adds a keyword for syntax highlighting in a specific language.
     * @param lang The language to which the keyword applies.
     * @param keyword The keyword to add.
     * @param colorPair The color pair to use for highlighting the keyword.
     */
    void addKeyword(const std::string& lang, const std::string& keyword,
                    int colorPair);

    /**
     * @brief Checks if a character is a word boundary.
     * @param c The character to check.
     * @return A boolean indicating whether the character is a word boundary.
     */
    static bool isWordBoundary(char c);

    /**
     * @brief Applies syntax highlighting to a specific position in the editor.
     * @param y The vertical position (line number) in the editor.
     * @param x The horizontal position (column) in the editor.
     * @param length The length of the text to highlight.
     * @param colorPair The color pair to use for highlighting.
     */
    static void applyHighlight(int y, int x, int length, int colorPair);
};

#endif // MEXEDIT_MEXSYNTAX_H