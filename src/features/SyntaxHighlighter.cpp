#include "../include/features/SyntaxHighlighter.h"
#include "../include/utils/FileUtils.h"
#include <unordered_map>

using namespace mexedit::features;

SyntaxHighlighter::SyntaxHighlighter() : currentLanguage_("text")
{
    loadLanguageDefinitions();
}

void SyntaxHighlighter::detectLanguage(const std::filesystem::path& filePath)
{
    std::string extension = utils::FileUtils::getExtension(filePath);
    
    if (extension == ".cpp" || extension == ".cxx" || extension == ".cc" || 
        extension == ".c" || extension == ".h" || extension == ".hpp")
    {
        setLanguage("cpp");
    }
    else if (extension == ".py")
    {
        setLanguage("python");
    }
    else if (extension == ".sh" || extension == ".bash")
    {
        setLanguage("shell");
    }
    else
    {
        setLanguage("text");
    }
}

void SyntaxHighlighter::setLanguage(const std::string& languageName)
{
    currentLanguage_ = languageName;
    currentDefinition_ = getLanguageDefinition(languageName);
}

std::vector<SyntaxToken> SyntaxHighlighter::analyzeLine(const std::string& line) const
{
    std::vector<SyntaxToken> tokens;
    
    size_t pos = 0;
    while (pos < line.length())
    {
        // Skip whitespace
        while (pos < line.length() && std::isspace(line[pos]))
        {
            ++pos;
        }
        
        if (pos >= line.length()) break;
        
        // Find word boundary
        size_t start = pos;
        while (pos < line.length() && (std::isalnum(line[pos]) || line[pos] == '_'))
        {
            ++pos;
        }
        
        if (pos > start)
        {
            std::string word = line.substr(start, pos - start);
            TokenType type = isKeyword(word) ? TokenType::Keyword : TokenType::Normal;
            tokens.push_back({start, pos - start, type});
        }
        else
        {
            ++pos; // Skip non-alphanumeric character
        }
    }
    
    return tokens;
}

bool SyntaxHighlighter::isKeyword(const std::string& word) const
{
    return currentDefinition_.keywords.count(word) > 0;
}

std::vector<std::string> SyntaxHighlighter::getSupportedLanguages()
{
    return {"cpp", "python", "shell", "text"};
}

void SyntaxHighlighter::loadLanguageDefinitions()
{
    // Languages are loaded on demand
}

LanguageDefinition SyntaxHighlighter::getLanguageDefinition(const std::string& languageName)
{
    if (languageName == "cpp")
    {
        return createCppDefinition();
    }
    else if (languageName == "python")
    {
        return createPythonDefinition();
    }
    else if (languageName == "shell")
    {
        return createShellDefinition();
    }
    else
    {
        return createPlainTextDefinition();
    }
}

LanguageDefinition SyntaxHighlighter::createCppDefinition()
{
    LanguageDefinition def;
    def.name = "C++";
    def.keywords = {
        "auto", "break", "case", "char", "class", "const", "continue", "default",
        "do", "double", "else", "enum", "extern", "float", "for", "goto",
        "if", "int", "long", "namespace", "register", "return", "short", "signed",
        "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned",
        "void", "volatile", "while", "bool", "true", "false", "new", "delete",
        "public", "private", "protected", "virtual", "template", "typename",
        "using", "std", "include"
    };
    def.lineCommentStart = "//";
    def.blockCommentStart = "/*";
    def.blockCommentEnd = "*/";
    def.stringDelimiters = "\"'";
    return def;
}

LanguageDefinition SyntaxHighlighter::createPythonDefinition()
{
    LanguageDefinition def;
    def.name = "Python";
    def.keywords = {
        "and", "as", "assert", "break", "class", "continue", "def", "del",
        "elif", "else", "except", "exec", "finally", "for", "from", "global",
        "if", "import", "in", "is", "lambda", "not", "or", "pass", "print",
        "raise", "return", "try", "while", "with", "yield", "True", "False", "None"
    };
    def.lineCommentStart = "#";
    def.stringDelimiters = "\"'";
    return def;
}

LanguageDefinition SyntaxHighlighter::createShellDefinition()
{
    LanguageDefinition def;
    def.name = "Shell";
    def.keywords = {
        "if", "then", "else", "elif", "fi", "case", "esac", "for", "select",
        "while", "until", "do", "done", "function", "time", "coproc", "echo",
        "cd", "pwd", "ls", "cat", "grep", "awk", "sed", "sort", "uniq"
    };
    def.lineCommentStart = "#";
    def.stringDelimiters = "\"'";
    return def;
}

LanguageDefinition SyntaxHighlighter::createPlainTextDefinition()
{
    LanguageDefinition def;
    def.name = "Plain Text";
    return def;
}
