#ifndef MEXEDIT_MEXEDIT_H
#define MEXEDIT_MEXEDIT_H

#include <vector>
#include <string>
#include <string_view>
#include <filesystem>
#include <memory>
#include <ncurses.h>
#include "mexMenu.h"
#include "mexSyntax.h"
#include "mexSearch.h"

namespace fs = std::filesystem;

/// @brief MexEdit is a simple text editor with file management and basic editing features. \class MexEdit
class MexEdit
{
public:

    /**
     * @brief Constructs a MexEdit object, initializing the editor and menu.
     */
    MexEdit();

    /**
     * @brief Destructor for MexEdit, cleans up resources.
     */
    ~MexEdit();

    /**
     * @brief Loads a file into the editor.
     * @param fileName The path to the file to load.
     * @return A boolean indicating whether the file was successfully loaded.
     */
    bool loadFile(const fs::path& fileName);

    /**
     * @brief Saves the current document to a file.
     * @param filename The path to the file to save. If empty, saves to the current file.
     * @return A boolean indicating whether the file was successfully saved.
     */
    bool saveFile(const fs::path& filename = {});

    /**
     * @brief Runs the text editor application.
     */
    void run();

private:
    std::vector<std::string> document;
    std::vector<std::string> buffer;
    fs::path currentFile;
    bool showLineNumbers = true;

    std::vector<fs::directory_entry> fileList;
    fs::path currentDirectory;
    int fileExplorerWidth = 30;
    int selectedFileIdx = 0;
    int fileExplorerScroll = 0;

    int cursorX = 0;
    int cursorY = 0;
    int editorScroll = 0;

    std::vector<std::vector<std::string>> documentHistory;
    size_t historyIndex = 0;

    /**
     * @brief Converts a key code to a control character.
     * @param k The key code to convert.
     * @return A control character corresponding to the key code.
     */
    static constexpr int CTRL(int k) { return (k) & 0x1f; }

    /**
     * @brief Expands the document to a new size.
     * @param newSize The new size to expand the document to.
     */
    void expandDocument(size_t newSize);

    /**
     * @brief Updates the file explorer with the current directory contents.
     */
    void updateFileExplorer();

    /**
     * @brief Draws the user interface of the editor.
     */
    void drawInterface();

    /**
     * @brief Handles user input and updates the editor state accordingly.
     * @param ch The character input from the user.
     */
    void handleInput(int ch);

    /**
     * @brief Moves the cursor to a specific position in the document.
     * @param dx The x-coordinate (column) to move the cursor to.
     * @param dy The y-coordinate (line) to move the cursor to.
     */
    void moveCursor(int dx, int dy);

    /**
     * @brief Inserts a character at the current cursor position.
     * @param ch The character to insert.
     */
    void insertChar(char ch);

    /**
     * @brief Deletes a character at the current cursor position.
     */
    void deleteChar();

    /**
     * @brief Inserts a new line at the current cursor position.
     */
    void insertLine();

    /**
     * @brief Deletes the current line.
     */
    void deleteLine();

    /**
     * @brief Saves the current state of the document to the history for undo/redo functionality.
     */
    void saveState();

    /**
     * @brief Undoes the last action in the document.
     */
    void undo();

    /**
     * @brief Redoes the last undone action in the document.
     */
    void redo();

    /**
     * @brief Prompts the user to save changes before exiting the editor.
     * @return A boolean indicating whether the user chose to save changes.
     */
    bool promptSaveBeforeExit();

    /**
     * @brief Diplays the current search status in the editor.
     * @param message The message to display in the search status bar.
     */
    static void showSearchStatus(const std::string& message);

    /**
     * @brief Performs a replace operation in the document.
     * @param replacement The string to replace the found matches with.
     * @param all The flag indicating whether to replace all occurrences or just the current match.
     */
    void performReplace(const std::string& replacement, bool all);

    /**
     * @brief Starts the command mode for executing commands.
     */
    void startCommandMode();

    MexMenu menu;
    MexSyntax syntaxHighlighter;

    MexSearch searchEngine;
    bool searchMode = false;
    std::string searchString;
};

#endif //MEXEDIT_MEXEDIT_H
