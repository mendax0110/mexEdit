#include "mexSyntax.h"
#include <algorithm>
#include <iostream>

MexSyntax::MexSyntax()
{
    for (int i = 10; i <= 20; i++)
    {
        init_pair(i, i - 9, COLOR_BLACK);
    }

    initLanguages();
}

void MexSyntax::detectLanguage(const std::string& filename)
{
    if (filename.empty())
    {
        currentLanguage.clear();
        currentRules.clear();
        currentKeywords.clear();
        return;
    }

    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos)
    {
        currentLanguage.clear();
        return;
    }

    std::string ext = filename.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    auto it = fileExtensions.find(ext);
    if (it != fileExtensions.end())
    {
        currentLanguage = it->second;
        currentRules = languageRules[currentLanguage];
    }
    else
    {
        currentLanguage.clear();
        currentRules.clear();
    }
}

void MexSyntax::highlightLine(const std::string& line, int yPos, int startCol)
{
    if (currentLanguage.empty() || currentRules.empty())
    {
        mvprintw(yPos, startCol, "%s", line.c_str());
        return;
    }

    mvprintw(yPos, startCol, "%s", line.c_str());

    for (const auto& rule : currentRules)
    {
        std::sregex_iterator it(line.begin(), line.end(), rule.pattern);
        std::sregex_iterator end;

        for (; it != end; ++it)
        {
            int start = it->position() + startCol;
            int length = it->length();

            if (!rule.wholeWord ||
                (start == startCol || isWordBoundary(line[start - 1 - startCol])) &&
                (start + length == startCol + line.length() || isWordBoundary(line[start + length - startCol])))
            {
                attron(COLOR_PAIR(rule.colorPair));
                mvprintw(yPos, start, "%.*s", length, line.substr(it->position(), length).c_str());
                attroff(COLOR_PAIR(rule.colorPair));
            }
        }
    }
}

void MexSyntax::initLanguages()
{
    fileExtensions = {
            {".cpp", "C++"}, {".hpp", "C++"}, {".h", "C++"}, {".cxx", "C++"}, {".cc", "C++"},
            {".c", "C"}, {".h", "C"},
            {".py", "Python"},
            {".sh", "Shell"}, {".bash", "Shell"},
            {".md", "Markdown"}
    };

    addCPPRules();
    addCRules();
    addPythonRules();
    addShellRules();
    addMarkdownRules();
}

void MexSyntax::addCPPRules()
{
    std::string lang = "C++";

    addKeyword(lang, "int", 10);
    addKeyword(lang, "float", 10);
    addKeyword(lang, "double", 10);
    addKeyword(lang, "char", 10);
    addKeyword(lang, "void", 10);
    addKeyword(lang, "bool", 10);
    addKeyword(lang, "auto", 10);
    addKeyword(lang, "const", 10);

    addKeyword(lang, "if", 11);
    addKeyword(lang, "else", 11);
    addKeyword(lang, "for", 11);
    addKeyword(lang, "while", 11);
    addKeyword(lang, "do", 11);
    addKeyword(lang, "switch", 11);
    addKeyword(lang, "case", 11);
    addKeyword(lang, "default", 11);
    addKeyword(lang, "break", 11);
    addKeyword(lang, "continue", 11);
    addKeyword(lang, "return", 11);
    addKeyword(lang, "goto", 11);

    addKeyword(lang, "class", 12);
    addKeyword(lang, "struct", 12);
    addKeyword(lang, "namespace", 12);
    addKeyword(lang, "template", 12);
    addKeyword(lang, "typename", 12);
    addKeyword(lang, "public", 13);
    addKeyword(lang, "private", 13);
    addKeyword(lang, "protected", 13);
    addKeyword(lang, "virtual", 13);
    addKeyword(lang, "override", 13);
    addKeyword(lang, "final", 13);

    addRule(lang, R"("(\\.|[^"\\])*")", 14);
    addRule(lang, R"('(\\.|[^'\\])')", 14);

    addRule(lang, R"(//.*$)", 15);
    addRule(lang, R"(/\*.*?\*/)", 15);

    addRule(lang, R"(\b[0-9]+\b)", 16);
    addRule(lang, R"(\b0x[0-9a-fA-F]+\b)", 16);
    addRule(lang, R"(\b[0-9]+\.[0-9]+([eE][+-]?[0-9]+)?\b)", 16);
}

void MexSyntax::addCRules()
{
    std::string lang = "C";

    addKeyword(lang, "int", 10);
    addKeyword(lang, "float", 10);
    addKeyword(lang, "double", 10);
    addKeyword(lang, "char", 10);
    addKeyword(lang, "void", 10);
    addKeyword(lang, "short", 10);
    addKeyword(lang, "long", 10);
    addKeyword(lang, "signed", 10);
    addKeyword(lang, "unsigned", 10);
    addKeyword(lang, "const", 10);
    addKeyword(lang, "volatile", 10);

    addKeyword(lang, "if", 11);
    addKeyword(lang, "else", 11);
    addKeyword(lang, "for", 11);
    addKeyword(lang, "while", 11);
    addKeyword(lang, "do", 11);
    addKeyword(lang, "switch", 11);
    addKeyword(lang, "case", 11);
    addKeyword(lang, "default", 11);
    addKeyword(lang, "break", 11);
    addKeyword(lang, "continue", 11);
    addKeyword(lang, "return", 11);
    addKeyword(lang, "goto", 11);

    addKeyword(lang, "struct", 12);
    addKeyword(lang, "union", 12);
    addKeyword(lang, "enum", 12);
    addKeyword(lang, "typedef", 12);

    addRule(lang, R"(^#\s*[a-zA-Z]+\b)", 13);

    addRule(lang, R"("(\\.|[^"\\])*")", 14);
    addRule(lang, R"('(\\.|[^'\\])')", 14);
    addRule(lang, R"(//.*$)", 15);
    addRule(lang, R"(/\*.*?\*/)", 15);
    addRule(lang, R"(\b[0-9]+\b)", 16);
}

void MexSyntax::addPythonRules()
{
    std::string lang = "Python";

    addKeyword(lang, "def", 10);
    addKeyword(lang, "class", 10);
    addKeyword(lang, "lambda", 10);
    addKeyword(lang, "if", 11);
    addKeyword(lang, "elif", 11);
    addKeyword(lang, "else", 11);
    addKeyword(lang, "for", 11);
    addKeyword(lang, "while", 11);
    addKeyword(lang, "try", 11);
    addKeyword(lang, "except", 11);
    addKeyword(lang, "finally", 11);
    addKeyword(lang, "with", 11);
    addKeyword(lang, "return", 11);
    addKeyword(lang, "yield", 11);
    addKeyword(lang, "import", 11);
    addKeyword(lang, "from", 11);
    addKeyword(lang, "as", 11);
    addKeyword(lang, "pass", 11);
    addKeyword(lang, "break", 11);
    addKeyword(lang, "continue", 11);
    addKeyword(lang, "raise", 11);
    addKeyword(lang, "and", 11);
    addKeyword(lang, "or", 11);
    addKeyword(lang, "not", 11);
    addKeyword(lang, "is", 11);
    addKeyword(lang, "in", 11);
    addKeyword(lang, "None", 12);
    addKeyword(lang, "True", 12);
    addKeyword(lang, "False", 12);

    addRule(lang, R"("(\\"|.)*?")", 14);
    addRule(lang, R"('(\\'|.)*?')", 14);
    addRule(lang, R"(""".*?""")", 14);
    addRule(lang, R"('''.*?''')", 14);

    addRule(lang, R"(#.*$)", 15);

    addRule(lang, R"(\b[0-9]+\b)", 16);
    addRule(lang, R"(\b0x[0-9a-fA-F]+\b)", 16);
    addRule(lang, R"(\b[0-9]+\.[0-9]+([eE][+-]?[0-9]+)?\b)", 16);

    addRule(lang, R"(@[a-zA-Z_][a-zA-Z0-9_]*)", 13);
}

void MexSyntax::addShellRules()
{
    std::string lang = "Shell";

    addKeyword(lang, "if", 10);
    addKeyword(lang, "then", 10);
    addKeyword(lang, "else", 10);
    addKeyword(lang, "elif", 10);
    addKeyword(lang, "fi", 10);
    addKeyword(lang, "for", 11);
    addKeyword(lang, "while", 11);
    addKeyword(lang, "do", 11);
    addKeyword(lang, "done", 11);
    addKeyword(lang, "case", 11);
    addKeyword(lang, "esac", 11);
    addKeyword(lang, "function", 12);
    addKeyword(lang, "return", 12);

    addRule(lang, R"("(\\"|.)*?")", 14);
    addRule(lang, R"('(\\'|.)*?')", 14);

    addRule(lang, R"(#.*$)", 15);

    addRule(lang, R"(\$\w+)", 16);

    addRule(lang, R"(\b[a-zA-Z_][a-zA-Z0-9_]*\b)", 17);
}

void MexSyntax::addMarkdownRules()
{
    std::string lang = "Markdown";

    addRule(lang, R"(^#{1,6}\s.*$)", 10);

    addRule(lang, R"(\*\*.*?\*\*)", 11);
    addRule(lang, R"(\*.*?\*)", 12);

    addRule(lang, R"(\[.*?\]\(.*?\))", 13);

    addRule(lang, R"(```.*?```)", 14);
    addRule(lang, R"(`[^`]+`)", 14);

    addRule(lang, R"(^[-*+] .*$)", 15);
    addRule(lang, R"(^\d+\. .*$)", 15);

    addRule(lang, R"(^> .*$)", 16);

    addRule(lang, R"(---|___|\*\*\*)", 17);
}

void MexSyntax::addRule(const std::string& lang, const std::string& pattern, int colorPair, bool wholeWord)
{
    try
    {
        HighlightRule rule;
        rule.pattern = std::regex(pattern);
        rule.colorPair = colorPair;
        rule.wholeWord = wholeWord;
        languageRules[lang].push_back(rule);
    }
    catch (const std::regex_error& e)
    {
        std::cerr << "Regex error in language '" << lang << "': " << e.what() << std::endl;
    }
}

void MexSyntax::addKeyword(const std::string& lang, const std::string& keyword, int colorPair)
{
    std::string pattern = R"(\b)" + keyword + R"(\b)";
    addRule(lang, pattern, colorPair, true);
}

bool MexSyntax::isWordBoundary(char c)
{
    return !(isalnum(c) || c == '_');
}

void MexSyntax::applyHighlight(int y, int x, int length, int colorPair)
{
    attron(COLOR_PAIR(colorPair));
    mvprintw(y, x, "%.*s", length, std::string(length, ' ').c_str());
    attroff(COLOR_PAIR(colorPair));
}

void MexSyntax::clearCache()
{
    currentRules.clear();
    currentKeywords.clear();
}