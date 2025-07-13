#include "../include/mexEdit.h"
#include <fstream>
#include <algorithm>
#include <stdexcept>

MexEdit::MexEdit()
{
    currentDirectory = fs::current_path();
    document.emplace_back();
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(1);
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    for (int i = 10; i <= 20; i++)
    {
        init_pair(i, i - 9, COLOR_BLACK);
    }


    menu = MexMenu();
    menu.addMenuItem("Help", "F1", "Show help menu", [this]() { menu.showHelp(); });
    menu.addMenuItem("Save", "F2", "Save current file", [this]() { saveFile(); });
    menu.addMenuItem("Open", "F3", "Open a file", [this]() {
        if (!fileList.empty() && selectedFileIdx < static_cast<int>(fileList.size())) {
            const auto& selected = fileList[selectedFileIdx];
            if (selected.is_directory()) {
                currentDirectory = selected.path();
                updateFileExplorer();
            } else {
                loadFile(selected.path());
            }
        }
    });
    menu.addMenuItem("Line Numbers", "F4", "Toggle line numbers", [this]() {
        showLineNumbers = !showLineNumbers;
    });
    menu.addMenuItem("New", "F5", "Create new file", [this]() {
        document.clear();
        document.emplace_back();
        currentFile.clear();
        cursorX = 0;
        cursorY = 0;
        editorScroll = 0;
    });
    menu.addMenuItem("Save As", "F6", "Save file with new name", [this]() {
        char filename[256];
        mvprintw(0, 0, "Enter filename to save as: ");
        echo();
        getnstr(filename, sizeof(filename) - 1);
        noecho();
        clear();
        if (saveFile(filename)) {
            currentFile = filename;
        }
    });
    menu.addMenuItem("Quit", "F7", "Exit the editor", [this]() {
        if (promptSaveBeforeExit()) {
            endwin();
            exit(0);
        }
    });
    menu.addMenuItem("Prev File", "F8", "Select previous file", [this]() {
        if (selectedFileIdx > 0) {
            selectedFileIdx--;
            if (selectedFileIdx < fileExplorerScroll) {
                fileExplorerScroll = selectedFileIdx;
            }
        }
    });
    menu.addMenuItem("Next File", "F9", "Select next file", [this]() {
        if (selectedFileIdx < static_cast<int>(fileList.size()) - 1) {
            selectedFileIdx++;
            if (selectedFileIdx >= fileExplorerScroll + (LINES - 1)) {
                fileExplorerScroll = selectedFileIdx - (LINES - 1) + 1;
            }
        }
    });

    updateFileExplorer();
}

MexEdit::~MexEdit()
{
    endwin();
}

void MexEdit::expandDocument(size_t newSize)
{
    if (newSize > document.size())
    {
        document.resize(newSize);
    }
}

bool MexEdit::loadFile(const fs::path& fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        return false;
    }

    document.clear();
    std::string line;
    while (std::getline(file, line))
    {
        document.push_back(line);
    }

    if (document.empty())
    {
        document.emplace_back();
    }

    currentFile = fileName;
    cursorX = 0;
    cursorY = 0;
    editorScroll = 0;
    syntaxHighlighter.detectLanguage(currentFile.string());

    return true;
}

bool MexEdit::saveFile(const fs::path& filename)
{
    fs::path savePath = filename.empty() ? currentFile : filename;
    if (savePath.empty())
    {
        return false;
    }

    std::ofstream file(savePath);
    if (!file.is_open())
    {
        return false;
    }

    for (const auto& line : document)
    {
        file << line << '\n';
    }

    if (!filename.empty())
    {
        currentFile = savePath;
        syntaxHighlighter.detectLanguage(currentFile.string());
    }

    return true;
}

void MexEdit::updateFileExplorer()
{
    fileList.clear();
    for (const auto& entry : fs::directory_iterator(currentDirectory))
    {
        fileList.push_back(entry);
    }

    std::sort(fileList.begin(), fileList.end(), [](const auto& a, const auto& b)
    {
        bool aIsDir = a.is_directory();
        bool bIsDir = b.is_directory();
        if (aIsDir not_eq bIsDir)
        {
            return aIsDir;
        }

        return a.path().filename().string() < b.path().filename().string();
    });

    selectedFileIdx = 0;
    fileExplorerScroll = 0;
}

void MexEdit::drawInterface()
{
    erase();
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    if (searchMode)
    {
        mvprintw(maxY -1, 0, "/%s", searchString.c_str());
    }

    attron(COLOR_PAIR(1));
    int explorerEnd = fileExplorerWidth;
    for (int i = 0; i < maxY; ++i)
    {
        mvhline(i, explorerEnd, ACS_VLINE, 1);
    }

    int fileToShow = std::min(maxY - 1, static_cast<int>(fileList.size()) - fileExplorerScroll);
    for (int i = 0; i < fileToShow; ++i)
    {
        const auto& entry = fileList[i + fileExplorerScroll];
        bool isSelected = (i + fileExplorerScroll == selectedFileIdx);

        if (isSelected)
        {
            attron(A_REVERSE);
        }

        std::string displayName = entry.path().filename().string();
        if (entry.is_directory())
        {
            displayName.append("/");
        }

        if (displayName.length() > static_cast<size_t>(fileExplorerWidth - 2))
        {
            displayName = displayName.substr(0, fileExplorerWidth - 5) + "...";
        }

        mvprintw(i, 1, "%s", displayName.c_str());

        if (isSelected)
        {
            attroff(A_REVERSE);
        }
    }

    attroff(COLOR_PAIR(1));

    int editorStart = fileExplorerWidth + 1;
    int editorWidth = maxX - editorStart;
    int linesToShow = std::min(maxY - 2, static_cast<int>(document.size()) - editorScroll);


    for (int i = 0; i < linesToShow; ++i)
    {
        int lineNum = i + editorScroll;
        const auto& line = document[lineNum];
        int lineStart = showLineNumbers ? editorStart + 5 : editorStart;
        int lineLength = std::min(static_cast<int>(line.size()), editorWidth - (showLineNumbers ? 5 : 0));

        if (showLineNumbers)
        {
            attron(COLOR_PAIR(2));
            mvprintw(i, editorStart, "%4d ", lineNum + 1);
            attroff(COLOR_PAIR(2));
        }

        if (lineLength > 0)
        {
            syntaxHighlighter.highlightLine(line, i, lineStart);
        }

        for (const auto& match : searchEngine.getMatches())
        {
            if (match.first == static_cast<size_t>(lineNum))
            {
                int start = match.second.first;
                int end = match.second.second;

                if (start < lineLength and end <= lineLength)
                {
                    attron(A_REVERSE);
                    mvprintw(i, lineStart + start, "%.*s", end - start, line.substr(start, end - start).c_str());
                    attroff(A_REVERSE);
                }
            }
        }
    }

    attron(COLOR_PAIR(3) | A_REVERSE);
    std::string status = currentFile.empty() ? "[No File]" : currentFile.filename().string();
    status += " - " + std::to_string(cursorY + 1) + "," + std::to_string(cursorX + 1);
    status += " | F1:Help ESC:Menu";
    mvprintw(maxY - 1, 0, "%-*s", maxX, status.c_str());
    attroff(COLOR_PAIR(3) | A_REVERSE);

    if (cursorY >= editorScroll and cursorY < editorScroll + linesToShow)
    {
        int lineIndex = cursorY - editorScroll;
        const auto& line = document[cursorY];
        int lineStart = showLineNumbers ? editorStart + 5 : editorStart;
        int cursorPos = std::min(cursorX, static_cast<int>(line.size()));
        move(lineIndex, lineStart + cursorPos);
    }

    refresh();
}

void MexEdit::showSearchStatus(const std::string &message)
{
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);
    attron(COLOR_PAIR(3) | A_REVERSE);
    mvprintw(maxY - 1, 0, "%-*s", maxX, message.c_str());
    attroff(COLOR_PAIR(3) | A_REVERSE);
    refresh();
    napms(1000);
}

void MexEdit::performReplace(const std::string &replacement, bool all)
{
    if (all)
    {
        searchEngine.replaceAll(searchEngine.getLastPattern(), replacement, document);
        showSearchStatus("Replaced all occurrences. + " + std::to_string(searchEngine.getMatches().size()) + " matches found.");
    }
    else
    {
        searchEngine.replaceCurrent(replacement, document);
        if (searchEngine.findNext(document))
        {
            const auto& matches = searchEngine.getMatches();
            cursorY = searchEngine.getCurrentMatch().first;
            cursorX = searchEngine.getCurrentMatch().second.first;
            editorScroll = std::max(0, static_cast<int>(cursorY) - LINES / 2);
        }
    }
}

void MexEdit::moveCursor(int dx, int dy)
{
    int newX = cursorX + dx;
    int newY = cursorY + dy;

    if (newY >= 0 and newY < static_cast<int>(document.size()))
    {
        cursorY = newY;
        cursorX = std::min(newX, static_cast<int>(document[cursorY].size()));
    }

    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    if (cursorY < editorScroll)
    {
        editorScroll = cursorY;
    }
    else if (cursorY >= editorScroll + maxY - 2)
    {
        editorScroll = cursorY - (maxY - 2) + 1;
    }
}

void MexEdit::insertChar(char ch)
{
    saveState();
    if (cursorY >= static_cast<int>(document.size()))
    {
        expandDocument(cursorY + 1);
    }

    document[cursorY].insert(cursorX, 1, ch);
    cursorX++;
}

void MexEdit::deleteChar()
{
    saveState();
    if (cursorX > 0)
    {
        document[cursorY].erase(cursorX - 1, 1);
        cursorX--;
    }
    else if (cursorY > 0)
    {
        cursorX = document[cursorY - 1].size();
        document[cursorY - 1] += document[cursorY];
        document.erase(document.begin() + cursorY);
        cursorY--;
    }
}

void MexEdit::insertLine()
{
    saveState();
    std::string remainder = document[cursorY].substr(cursorX);
    document[cursorY].resize(cursorX);
    document.insert(document.begin() + cursorY + 1, remainder);
    cursorY++;
    cursorX = 0;
}

void MexEdit::deleteLine()
{
    saveState();
    if (document.size() > 1)
    {
        document.erase(document.begin() + cursorY);
        if (cursorY >= static_cast<int>(document.size()))
        {
            cursorY = document.size() - 1;
        }
        cursorX = std::min(cursorX, static_cast<int>(document[cursorY].size()));
    }
}

void MexEdit::saveState()
{
    if (historyIndex < documentHistory.size())
    {
        documentHistory.erase(documentHistory.begin() + historyIndex, documentHistory.end());
    }

    documentHistory.push_back(document);
    historyIndex++;

    if (documentHistory.size() > 100)
    {
        documentHistory.erase(documentHistory.begin());
        historyIndex--;
    }
}

void MexEdit::undo()
{
    if (historyIndex > 1)
    {
        historyIndex--;
        document = documentHistory[historyIndex - 1];
    }
}

void MexEdit::redo()
{
    if (historyIndex < documentHistory.size())
    {
        historyIndex++;
        document = documentHistory[historyIndex - 1];
    }
}

bool MexEdit::promptSaveBeforeExit()
{
    if (currentFile.empty() and (document.size() == 1 and document[0].empty()))
    {
        return true;
    }

    mvprintw(0, 0, "Unsaved changes. Save before exiting? (y/n): ");
    echo();
    int answer = getch();
    noecho();
    clear();

    if (answer == 'y' or answer == 'Y')
    {
        if (currentFile.empty())
        {
            char fileName[256];
            mvprintw(0, 0, "Enter filename to save as: ");
            echo();
            getnstr(fileName, sizeof(fileName) - 1);
            noecho();
            clear();

            if (!saveFile(fileName))
            {
                mvprintw(1, 0, "Failed to save file: %s Press any key to continue...", fileName);
                getch();
                return false;
            }
            currentFile = fileName;
        }
        else
        {
            if (!saveFile())
            {
                mvprintw(1, 0, "Failed to save file: %s Press any key to continue...", currentFile.string().c_str());
                getch();
                return false;
            }
        }
    }

    return true;
}

void MexEdit::startCommandMode()
{
    char command[256];
    echo();
    mvprintw(LINES-1, 0, "Command: ");
    getnstr(command, sizeof(command) - 1);
    noecho();

    if (strncmp(command, "s/", 2) == 0)
    {
        char* pattern = command + 2;
        char* replacement = strchr(pattern, '/');
        if (replacement)
        {
            *replacement++ = '\0';
            char* flags = strchr(replacement, '/');
            if (flags) *flags++ = '\0';

            bool all = flags && strchr(flags, 'g');
            searchEngine.find(pattern, document);
            performReplace(replacement, all);
        }
    }
}

void MexEdit::handleInput(int ch)
{
    static bool escapePressed = false;

    if (searchMode)
    {
        if (ch == 27)
        {
            searchMode = false;
            searchString.clear();
            noecho();
            return;
        }
        else if (ch == '\n')
        {
            searchEngine.find(searchString, document);
            searchMode = false;
            if (!searchEngine.getMatches().empty())
            {
                const auto& matches = searchEngine.getMatches();
                cursorY = matches[0].first;
                cursorX = matches[0].second.first;
                editorScroll = std::max(0, static_cast<int>(cursorY) - LINES / 2);
            }
            searchString.clear();
            return;
        }
        else if (ch == KEY_BACKSPACE || ch == 127)
        {
            if (!searchString.empty())
            {
                searchString.pop_back();
            }
        }
        else if (isprint(ch))
        {
            searchString += ch;
        }
        return;
    }

    if (escapePressed)
    {
        if (ch == ':')
        {
            startCommandMode();
        }
        escapePressed = false;
        return;
    }

    switch (ch)
    {
        case KEY_UP:
            moveCursor(0, -1);
            break;
        case KEY_DOWN:
            moveCursor(0, 1);
            break;
        case KEY_LEFT:
            moveCursor(-1, 0);
            break;
        case KEY_RIGHT:
            moveCursor(1, 0);
            break;
        case KEY_BACKSPACE:
        case 127:
            deleteChar();
            break;
        case KEY_ENTER:
        case '\n':
            insertLine();
            break;
        case KEY_DC:
            if (cursorX < static_cast<int>(document[cursorY].size()))
            {
                document[cursorY].erase(cursorX, 1);
            }
            else if (cursorY < static_cast<int>(document.size()) - 1)
            {
                document[cursorY] += document[cursorY + 1];
                document.erase(document.begin() + cursorY + 1);
            }
            break;
        case KEY_HOME:
            cursorX = 0;
            break;
        case KEY_END:
            cursorX = document[cursorY].size();
            break;
        case KEY_PPAGE:
            moveCursor(0, -10);
            break;
        case KEY_NPAGE:
            moveCursor(0, 10);
            break;
        case KEY_F(1):
            menu.showHelp();
            break;
        case KEY_F(2):
            saveFile();
            break;
        case KEY_F(3):
            if (!fileList.empty() and selectedFileIdx < static_cast<int>(fileList.size()))
            {
                const auto& selected = fileList[selectedFileIdx];
                if (selected.is_directory())
                {
                    currentDirectory = selected.path();
                    updateFileExplorer();
                }
                else
                {
                    loadFile(selected.path());
                }
            }
            break;
        case KEY_F(4):
            showLineNumbers = !showLineNumbers;
            break;
        case KEY_F(5):
            document.clear();
            document.emplace_back();
            currentFile.clear();
            cursorX = 0;
            cursorY = 0;
            editorScroll = 0;
            break;
        case KEY_F(6): // save as
        {
            char filename[256];
            mvprintw(0, 0, "Enter filename to save as: ");
            echo();
            getnstr(filename, sizeof(filename) - 1);
            noecho();
            clear();

            if (!saveFile(filename))
            {
                mvprintw(0, 0, "Failed to save file: %s", filename);
            }
            else
            {
                currentFile = filename;
                syntaxHighlighter.detectLanguage(currentFile.string());
                mvprintw(0, 0, "File saved as: %s", filename);
            }
            getch();
            clear();
            break;
        }
        case KEY_F(7): // quit
            if (promptSaveBeforeExit())
            {
                endwin();
                exit(0);
            }
            break;
        case KEY_F(8):
            if (selectedFileIdx > 0)
            {
                selectedFileIdx--;
                if (selectedFileIdx < fileExplorerScroll)
                {
                    fileExplorerScroll = selectedFileIdx;
                }
            }
            break;
        case KEY_F(9):
            if (selectedFileIdx < static_cast<int>(fileList.size()) - 1)
            {
                selectedFileIdx++;
                if (selectedFileIdx >= fileExplorerScroll + (LINES - 1))
                {
                    fileExplorerScroll = selectedFileIdx - (LINES - 1) + 1;
                }
            }
            break;
        case CTRL('d'):
            deleteLine();
            break;
        case CTRL('z'):
            undo();
            break;
        case CTRL('y'):
            redo();
            break;
        case 27:
            menu.showMainMenu();
            drawInterface();
            escapePressed = true;
            break;
        case '/':
            searchMode = true;
            searchString.clear();
            echo();
            break;
        case CTRL('n'):
            if (searchEngine.findNext(document))
            {
                const auto& matches = searchEngine.getMatches();
                cursorY = searchEngine.getCurrentMatch().first;
                cursorX = searchEngine.getCurrentMatch().second.first;
                editorScroll = std::max(0, static_cast<int>(cursorY) - LINES / 2);
            }
            break;
        case ':':
            insertChar(':');
            break;
        case '\t':
            for (int i = 0; i < 4; ++i)
            {
                insertChar(' ');
            }
            break;
        default:
            if (isprint(ch))
            {
                insertChar(ch);
            }
            break;
    }
}

void MexEdit::run()
{
    while (true)
    {
        drawInterface();
        int ch = getch();
        handleInput(ch);
    }
}