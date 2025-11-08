#include "EditorApplication.h"
#include "utils/Clipboard.h"
#include "utils/MemoryDebugger.h"
#include "utils/Logger.h"
#include <ncurses.h>
#include <memory>

using namespace mexedit;

EditorApplication::EditorApplication(std::unique_ptr<ui::IRenderer> renderer)
        : renderer_(std::move(renderer))
        , running_(false)
        , showLineNumbers_(true)
        , showFileExplorer_(true)
{
    TRACE_FUNC
    TRACK_MEMORY(EditorApplication, this);

    // Initialize core components
    editor_ = std::make_unique<core::Editor>();
    syntaxHighlighter_ = std::make_unique<features::SyntaxHighlighter>();
    searchEngine_ = std::make_unique<features::SearchEngine>();
    fileExplorer_ = std::make_unique<features::FileExplorer>();

    // Setup callbacks
    editor_->setDocumentChangedCallback([this]() { onDocumentChanged(); });
    editor_->setCursorMovedCallback([this](const core::Cursor::Position& pos)
    {
        onCursorMoved(pos);
    });
}

EditorApplication::~EditorApplication()
{
    TRACE_FUNC
    UNTRACK_MEMORY(EditorApplication, this);
    if (running_)
    {
        shutdown();
    }
}

void EditorApplication::initialize()
{
    TRACE_FUNC
    renderer_->initialize();
    updateViewport();
    
    // Create new document if none loaded
    if (!editor_->hasDocument())
    {
        newFile();
    }
    
    // Mark everything for initial full redraw
    dirtyRegions_.fullRedraw = true;
}

void EditorApplication::run()
{
    TRACE_FUNC
    running_ = true;
    
    while (running_)
    {
        // Only render if there are changes to display
        if (dirtyRegions_.fullRedraw || dirtyRegions_.editorArea || 
            dirtyRegions_.fileExplorer || dirtyRegions_.statusBar || 
            dirtyRegions_.cursorOnly)
        {
            render();
        }

        int key = renderer_->getInput();
        onKeyPressed(key);
    }
}

void EditorApplication::shutdown()
{
    TRACE_FUNC
    running_ = false;
    renderer_->shutdown();
}

bool EditorApplication::openFile(const std::filesystem::path& path)
{
    TRACE_FUNC
    if (editor_->openFile(path))
    {
        syntaxHighlighter_->detectLanguage(path);
        editor_->moveCursor({0, 0});
        viewport_.scrollOffsetY = 0;
        updateViewport();
        showStatusMessage("Opened: " + path.filename().string());
        return true;
    }
    else
    {
        showStatusMessage("Failed to open: " + path.filename().string());
        return false;
    }
}

bool EditorApplication::openSearchDialog(const std::string& pattern, bool newSearch)
{
    TRACE_FUNC
    if (!editor_->hasDocument())
        return false;

    const auto& lines = editor_->getDocument()->getLines();
    features::SearchOptions options{false, false, false, true};

    size_t startLine = newSearch ? 0 : lastSearchLine;
    size_t startColumn = newSearch ? 0 : lastSearchColumn;

    auto result = searchEngine_->findNext(pattern, lines, startLine, startColumn, options);

    if (result.line < editor_->getDocument()->getLineCount())
    {
        editor_->moveCursor({result.line, result.startColumn});
        selection_.isActive = true;
        selection_.startPos = {result.line, result.startColumn};
        selection_.endPos = {result.line, result.endColumn};
        markDirty(true, false, true, false);

        searchActive_ = true;
        currentSearchPattern_ = pattern;

        lastSearchLine = result.line;
        lastSearchColumn = result.endColumn;

        showStatusMessage("Found: " + pattern + " (Press ENTER for next)");
        return true;
    }

    // No match found
    showStatusMessage("No match found for: " + pattern);
    searchActive_ = false;
    return false;
}

bool EditorApplication::saveFile(const std::filesystem::path& path)
{
    TRACE_FUNC
    if (path.empty() && editor_->getDocument()->getFilePath().empty())
    {
        showStatusMessage("Enter filename to save: ");
        auto screenSize = renderer_->getScreenSize();
        renderer_->clearArea(screenSize.height - 1, 0, 1, screenSize.width);
        renderStatusBar();
        renderer_->refresh();
        
        std::string filename;
        int ch;
        while ((ch = renderer_->getInput()) != KEY_ENTER && ch != 10 && ch != 13)
        {
            if (ch == KEY_BACKSPACE || ch == 127)
            {
                if (!filename.empty())
                {
                    filename.pop_back();
                }
            }
            else if (ch >= 32 && ch <= 126)
            {
                filename += static_cast<char>(ch);
            }
            else if (ch == 27) // ESC - cancel save
            {
                showStatusMessage("Save cancelled");
                renderer_->clearArea(screenSize.height - 1, 0, 1, screenSize.width);
                renderStatusBar();
                renderer_->refresh();
                return false;
            }
            
            // Force immediate status bar update
            showStatusMessage("Enter filename to save: " + filename);
            renderer_->clearArea(screenSize.height - 1, 0, 1, screenSize.width);
            renderStatusBar();
            renderer_->refresh();
        }
        
        if (filename.empty())
        {
            showStatusMessage("Save cancelled - no filename entered");
            return false;
        }
        
        std::filesystem::path savePath(filename);
        if (editor_->saveFile(savePath))
        {
            showStatusMessage("Saved: " + getCurrentFileName());
            return true;
        }
        else
        {
            showStatusMessage("Failed to save file: " + filename);
            return false;
        }
    }
    else
    {
        if (editor_->saveFile(path))
        {
            showStatusMessage("Saved: " + getCurrentFileName());
            return true;
        }
        else
        {
            showStatusMessage("Failed to save file");
            return false;
        }
    }
}

bool EditorApplication::saveFileAs(const std::filesystem::path& path)
{
    TRACE_FUNC
    return saveFile(path);
}

void EditorApplication::newFile()
{
    TRACE_FUNC
    auto document = std::make_shared<core::Document>();
    editor_->setDocument(document);
    syntaxHighlighter_->setLanguage("text");
    showStatusMessage("New file created");
}

bool EditorApplication::hasUnsavedChanges() const
{
    TRACE_FUNC
    return editor_->isModified();
}

std::string EditorApplication::getCurrentFileName() const
{
    TRACE_FUNC
    if (editor_->hasDocument())
    {
        auto path = editor_->getDocument()->getFilePath();
        return path.empty() ? "[New File]" : path.filename().string();
    }
    return "[No File]";
}

void EditorApplication::onDocumentChanged()
{
    TRACE_FUNC
    markDirty(true, false, true, false);
}

void EditorApplication::onCursorMoved(const core::Cursor::Position& /* position */)
{
    int oldScrollOffsetY = viewport_.scrollOffsetY;
    int oldScrollOffsetX = viewport_.scrollOffsetX;
    scrollToEnsureCursorVisible();

    if (viewport_.scrollOffsetY != oldScrollOffsetY || viewport_.scrollOffsetX != oldScrollOffsetX)
    {
        markDirty(true, false, true, false);
    }
    else
    {
        markDirty(false, false, true, true);
    }
}

void EditorApplication::onKeyPressed(int key)
{
    TRACE_FUNC
    if ((key == 10 || key == 13) && searchActive_)
    {
        // Continue search from last match
        openSearchDialog(currentSearchPattern_, false); // false = continue search
        return;
    }

    // Handle Tab key (ASCII 9)
    if (key == 9)
    {
        // Clear selection before inserting spaces
        if (selection_.isActive)
        {
            deleteSelection();
        }
        // Insert 4 spaces for indentation
        editor_->insertText("    ");
        return;
    }

    if (key == 10 || key == 13)
    {
        // If a search is active, pressing Enter should find the next occurrence
        if (searchActive_)
        {
            openSearchDialog(currentSearchPattern_);
            return;
        }

        // File explorer check
        if (showFileExplorer_ && fileExplorer_->hasSelection())
        {
            auto entry = fileExplorer_->getSelectedEntry();
            if (entry && !entry->isDirectory)
            {
                openFile(entry->path);
                return;
            }
            else if (entry && entry->isDirectory)
            {
                fileExplorer_->navigateInto(entry->path);
                markDirty(false, true, false, false);
                return;
            }
        }

        // Otherwise handle as normal editing key (insert new line)
        handleEditingKey(key);
        return;
    }
    
    // Handle special key combinations (exclude Tab and Enter keys)
    if (key >= 1 && key <= 26 && key != 9 && key != 10 && key != 13)
    {   
        // Ctrl+A through Ctrl+Z (excluding Ctrl+I=Tab and Ctrl+J=Enter)
        handleControlKey(key);
        return;
    }
    
    // Handle function keys
    if (key >= KEY_F(1) && key <= KEY_F(12))
    {
        handleFunctionKey(key);
        return;
    }
    
    // Handle arrow keys and navigation
    if (key == KEY_UP || key == KEY_DOWN || key == KEY_LEFT || key == KEY_RIGHT ||
        key == KEY_HOME || key == KEY_END || key == KEY_PPAGE || key == KEY_NPAGE ||
        key == KEY_SLEFT || key == KEY_SRIGHT)
    {
        // Check for shift+arrow keys for selection (only SLEFT and SRIGHT are shift keys)
        bool isShiftArrow = (key == KEY_SLEFT || key == KEY_SRIGHT);
        
        if (isShiftArrow)
        {
            if (!selection_.isActive)
            {
                startSelection();
            }
            
            // Convert shift+arrow to regular arrow for movement
            switch (key)
            {
                case KEY_SLEFT: key = KEY_LEFT; break;
                case KEY_SRIGHT: key = KEY_RIGHT; break;
            }
        }
        else
        {
            // Regular arrow key - only clear selection if we're not extending it
            // Don't clear if user might be using mouse or other selection method
        }
        
        handleMovementKey(key, isShiftArrow);
        return;
    }
    
    // Handle other editing keys (Enter is handled above)
    if (key == KEY_BACKSPACE || key == 127 || key == KEY_DC)
    {
        handleEditingKey(key);
        return;
    }
    
    // Handle ESC and other special keys
    if (key == 27)
    {
        if (searchActive_)
        {
            searchActive_ = false;
            showStatusMessage("Search cancelled");
            return;
        }

        showCommandHelp();
        return;
    }
    
    // Handle printable characters
    if (key >= 32 && key <= 126)
    {
        // Clear selection before inserting character
        if (selection_.isActive)
        {
            deleteSelection();
        }
        editor_->insertCharacter(static_cast<char>(key));
    }
}

void EditorApplication::render()
{
    TRACE_FUNC
    if (dirtyRegions_.fullRedraw)
    {
        renderer_->clear();
        if (showFileExplorer_)
        {
            renderFileExplorer();
        }
        renderEditor();
        renderStatusBar();
    }
    else
    {
        if (dirtyRegions_.fileExplorer && showFileExplorer_)
        {
            auto screenSize = renderer_->getScreenSize();
            int explorerStartX = screenSize.width - viewport_.fileExplorerWidth;
            renderer_->clearArea(0, explorerStartX, screenSize.height - 1, viewport_.fileExplorerWidth);
            renderFileExplorer();
        }
        
        if (dirtyRegions_.editorArea)
        {
            auto screenSize = renderer_->getScreenSize();
            int editorEndX = showFileExplorer_ ? screenSize.width - viewport_.fileExplorerWidth - 1 : screenSize.width;
            renderer_->clearArea(0, viewport_.editorStartX, viewport_.maxVisibleLines, editorEndX - viewport_.editorStartX);
            renderEditor();
        }
        
        if (dirtyRegions_.statusBar)
        {
            auto screenSize = renderer_->getScreenSize();
            renderer_->clearArea(screenSize.height - 1, 0, 1, screenSize.width);
            renderStatusBar();
        }
    }
    
    auto pos = editor_->getCursor().getPosition();
    auto screenSize = renderer_->getScreenSize();
    
    //int screenX = viewport_.editorStartX + (showLineNumbers_ ? 6 : 0) + static_cast<int>(pos.column);
    int screenX = viewport_.editorStartX + (showLineNumbers_ ? 6 : 0)+ static_cast<int>(pos.column) - viewport_.scrollOffsetX;
    int screenY = static_cast<int>(pos.line) - viewport_.scrollOffsetY;
    
    int maxX = showFileExplorer_ ? screenSize.width - viewport_.fileExplorerWidth - 1 : screenSize.width;
    
    if (screenY >= 0 && screenY < viewport_.maxVisibleLines && 
        screenX >= viewport_.editorStartX && screenX < maxX)
    {
        renderer_->setCursorPosition({static_cast<size_t>(screenY), static_cast<size_t>(screenX)});
    }
    
    renderer_->refresh();
    
    clearDirtyRegions();
}

void EditorApplication::renderFileExplorer()
{
    TRACE_FUNC
    if (!showFileExplorer_) return;
    
    auto screenSize = renderer_->getScreenSize();
    int explorerStartX = screenSize.width - viewport_.fileExplorerWidth;
    int explorerHeight = screenSize.height - 1; // Reserve space for status bar
    
    // Draw simple vertical line separator
    renderer_->drawVerticalLine(explorerStartX - 1, 0, explorerHeight - 1);
    
    // Draw simple title
    renderer_->drawText(0, explorerStartX + 1, "Files [F12:Toggle]", ui::ColorPair::FileExplorer);
    
    // Draw current directory in a highlighted style
    std::string currentDir = fileExplorer_->getCurrentDirectory().filename().string();
    if (currentDir.empty()) currentDir = "/";
    
    if (currentDir.length() > static_cast<size_t>(viewport_.fileExplorerWidth - 4))
    {
        currentDir = "..." + currentDir.substr(currentDir.length() - (viewport_.fileExplorerWidth - 7));
    }

    renderer_->drawText(1, explorerStartX + 1, currentDir, ui::ColorPair::FileExplorer);

    for (int i = 0; i < viewport_.fileExplorerWidth - 2; i++)
    {
        renderer_->drawText(2, explorerStartX + i, "-", ui::ColorPair::Border);
    }

    const auto& entries = fileExplorer_->getEntries();
    size_t selectedIndex = fileExplorer_->getSelectedIndex();
    size_t start = viewport_.fileExplorerScrollOffset;

    for (size_t i = start; i < entries.size() && static_cast<int>(i - start + 3) < explorerHeight - 1; ++i)
    {
        const auto& entry = entries[i];
        std::string displayName = entry.displayName;

        std::string prefix;
        if (entry.isDirectory)
        {
            prefix = "[#] ";
        }
        else
        {
            prefix = "  ";
        }
        displayName = prefix + displayName;

        if (!entry.isDirectory)
        {
            displayName = "|_" + displayName;
            displayName += " " + fileExplorer_->formatFileSize(entry.fileSize);
        }

        if (displayName.length() > static_cast<size_t>(viewport_.fileExplorerWidth - 2))
        {
            displayName = displayName.substr(0, viewport_.fileExplorerWidth - 5);
        }

        int y = static_cast<int>(i - start + 3);

        if (i == selectedIndex)
        {
            renderer_->drawTextWithAttributes(y, explorerStartX + 1, displayName, A_REVERSE);
        }
        else
        {
            ui::ColorPair color = entry.isDirectory ? ui::ColorPair::FileExplorerDirectory : ui::ColorPair::FileExplorer;
            renderer_->drawText(y, explorerStartX + 1, displayName, color);
        }
    }

    // Draw status info at bottom of file explorer
    int statusY = explorerHeight - 2;
    renderer_->drawText(statusY, explorerStartX + 1, "[Enter/F3:Open]", ui::ColorPair::FileExplorer);
}

void EditorApplication::renderEditor()
{
    TRACE_FUNC
    if (!editor_->hasDocument()) return;
    
    auto document = editor_->getDocument();
    auto screenSize = renderer_->getScreenSize();
    
    int lineNumberWidth = showLineNumbers_ ? 6 : 0;
    int editorStartX = viewport_.editorStartX + lineNumberWidth;
    
    if (showLineNumbers_)
    {
        for (int i = 0; i < viewport_.maxVisibleLines; i++)
        {
            renderer_->drawText(i, viewport_.editorStartX + lineNumberWidth - 1, "|", ui::ColorPair::Border);
        }
    }
    
    auto currentCursor = editor_->getCursor().getPosition();
    
    for (int screenLine = 0; screenLine < viewport_.maxVisibleLines; ++screenLine)
    {
        size_t docLine = viewport_.scrollOffsetY + screenLine;
        
        if (docLine >= document->getLineCount())
        {
            if (showLineNumbers_)
            {
                std::string emptyLineNum(lineNumberWidth - 1, ' ');
                renderer_->drawText(screenLine, viewport_.editorStartX, emptyLineNum, ui::ColorPair::LineNumbers);
            }
            break;
        }
        
        if (showLineNumbers_)
        {
            std::string lineNum = std::to_string(docLine + 1);
            while (lineNum.length() < static_cast<size_t>(lineNumberWidth - 2))
            {
                lineNum = " " + lineNum;
            }
            lineNum += " ";
            
            // Highlight current line number
            ui::ColorPair lineNumColor = (docLine == currentCursor.line) ? ui::ColorPair::LineNumbersActive : ui::ColorPair::LineNumbers;
            renderer_->drawText(screenLine, viewport_.editorStartX, lineNum, lineNumColor);
        }
        
        // Draw line content with syntax highlighting
        const std::string& line = document->getLine(docLine);
        int maxWidth = screenSize.width - editorStartX;
        
        if (maxWidth > 0)
        {
            size_t startCol = static_cast<size_t>(viewport_.scrollOffsetX);
            size_t endCol = startCol + static_cast<size_t>(maxWidth);

            std::string displayLine;
            if (startCol < line.length())
            {
                displayLine = line.substr(startCol, endCol - startCol);
            }
            else
            {
                displayLine = "";
            }
            
            // Apply syntax highlighting if available
            if (syntaxHighlighter_ && syntaxHighlighter_->getCurrentLanguage() != "text")
            {
                auto tokens = syntaxHighlighter_->analyzeLine(displayLine);
                size_t lastPos = 0;
                
                for (const auto& token : tokens)
                {
                    // Draw text before token with normal color
                    if (token.start > lastPos)
                    {
                        std::string beforeToken = displayLine.substr(lastPos, token.start - lastPos);
                        renderer_->drawText(screenLine, editorStartX + static_cast<int>(lastPos), beforeToken);
                    }
                    
                    // Draw token with appropriate color and selection highlighting
                    std::string tokenText = displayLine.substr(token.start, token.length);
                    ui::ColorPair color = ui::ColorPair::Normal;
                    
                    switch (token.type)
                    {
                        case features::TokenType::Keyword:
                            color = ui::ColorPair::Syntax_Keyword;
                            break;
                        case features::TokenType::String:
                            color = ui::ColorPair::Syntax_String;
                            break;
                        case features::TokenType::Comment:
                            color = ui::ColorPair::Syntax_Comment;
                            break;
                        default:
                            color = ui::ColorPair::Normal;
                            break;
                    }
                    
                    // Check if any part of this token is selected
                    bool hasSelection = false;
                    for (size_t i = 0; i < token.length; ++i)
                    {
                        if (isPositionSelected(docLine, token.start + i))
                        {
                            hasSelection = true;
                            break;
                        }
                    }
                    
                    if (hasSelection)
                    {
                        // Draw with selection highlighting (reverse video)
                        renderer_->drawTextWithAttributes(screenLine, editorStartX + static_cast<int>(token.start), tokenText, A_REVERSE);
                    }
                    else
                    {
                        renderer_->drawText(screenLine, editorStartX + static_cast<int>(token.start), tokenText, color);
                    }
                    lastPos = token.start + token.length;
                }
                
                // Draw remaining text with selection highlighting
                if (lastPos < displayLine.length())
                {
                    std::string remaining = displayLine.substr(lastPos);
                    
                    // Check if any part of remaining text is selected
                    bool hasSelection = false;
                    for (size_t i = 0; i < remaining.length(); ++i)
                    {
                        if (isPositionSelected(docLine, lastPos + i))
                        {
                            hasSelection = true;
                            break;
                        }
                    }
                    
                    if (hasSelection)
                    {
                        renderer_->drawTextWithAttributes(screenLine, editorStartX + static_cast<int>(lastPos), remaining, A_REVERSE);
                    }
                    else
                    {
                        renderer_->drawText(screenLine, editorStartX + static_cast<int>(lastPos), remaining);
                    }
                }
            }
            else
            {
                // No syntax highlighting - check for selection highlighting
                if (selection_.isActive)
                {
                    // Draw character by character to handle selection
                    for (size_t i = 0; i < displayLine.length(); ++i)
                    {
                        char ch = displayLine[i];
                        if (isPositionSelected(docLine, i))
                        {
                            renderer_->drawTextWithAttributes(screenLine, editorStartX + static_cast<int>(i), std::string(1, ch), A_REVERSE);
                        }
                        else
                        {
                            renderer_->drawText(screenLine, editorStartX + static_cast<int>(i), std::string(1, ch));
                        }
                    }
                }
                else
                {
                    // No selection, draw normally
                    renderer_->drawText(screenLine, editorStartX, displayLine);
                }
            }
        }
    }
}

void EditorApplication::renderStatusBar()
{
    TRACE_FUNC
    auto screenSize = renderer_->getScreenSize();
    int statusY = screenSize.height - 1;
    
    // Simple, clean status bar
    std::string status = getCurrentFileName();
    
    if (editor_->isModified())
    {
        status += " [*]";
    }
    
    auto pos = editor_->getCursor().getPosition();
    status += " | Ln " + std::to_string(pos.line + 1) + ", Col " + std::to_string(pos.column + 1);
    
    if (!statusMessage_.empty())
    {
        status += " | " + statusMessage_;
    }
    else
    {
        status += " | F1:Help F2:Save F7:Quit";
        if (selection_.isActive)
        {
            std::string selectedText = copySelection();
            status += " | Sel: " + std::to_string(selectedText.length());
        }
    }
    
    // Truncate if too long
    if (status.length() > static_cast<size_t>(screenSize.width))
    {
        status = status.substr(0, screenSize.width - 3) + "...";
    }
    
    // Fill with spaces for clean appearance
    status += std::string(screenSize.width - status.length(), ' ');
    
    renderer_->drawText(statusY, 0, status, ui::ColorPair::StatusBar);
}

void EditorApplication::handleMovementKey(int key, bool isShiftArrow)
{
    TRACE_FUNC
    auto pos = editor_->getCursor().getPosition();
    
    switch (key)
    {
        case KEY_UP:
            if (showFileExplorer_ && fileExplorer_->hasSelection())
            {
                fileExplorer_->moveSelectionUp();
                scrollFileExplorerIntoView();
                markDirty(false, true, false, false);
            }
            else
            {
                editor_->moveCursorUp();
                if (selection_.isActive && isShiftArrow)
                {
                    endSelection();
                }
                else if (!isShiftArrow)
                {
                    clearSelection();
                }
            }
            break;
        case KEY_DOWN:
            if (showFileExplorer_ && fileExplorer_->hasSelection())
            {
                fileExplorer_->moveSelectionDown();
                scrollFileExplorerIntoView();
                markDirty(false, true, false, false);
            }
            else
            {
                editor_->moveCursorDown();
                if (selection_.isActive && isShiftArrow)
                {
                    endSelection();
                }
                else if (!isShiftArrow)
                {
                    clearSelection();
                }
            }
            break;
        case KEY_LEFT:
            editor_->moveCursorLeft();
            if (selection_.isActive && isShiftArrow)
            {
                endSelection();
            }
            else if (!isShiftArrow)
            {
                clearSelection();
            }
            break;
        case KEY_RIGHT:
            editor_->moveCursorRight();
            if (selection_.isActive && isShiftArrow)
            {
                endSelection();
            }
            else if (!isShiftArrow)
            {
                clearSelection();
            }
            break;
        case KEY_HOME:
            editor_->moveCursorToLineStart();
            if (selection_.isActive && isShiftArrow)
            {
                endSelection();
            }
            else if (!isShiftArrow)
            {
                clearSelection();
            }
            break;
        case KEY_END:
            editor_->moveCursorToLineEnd();
            if (selection_.isActive && isShiftArrow)
            {
                endSelection();
            }
            else if (!isShiftArrow)
            {
                clearSelection();
            }
            break;
        case KEY_PPAGE: // Page Up
            if (pos.line >= 10)
            {
                editor_->moveCursorUp(10);
            }
            else
            {
                editor_->moveCursorToDocumentStart();
            }
            clearSelection(); // Always clear selection for page moves
            markDirty(true, false, true, false);  // Page moves need editor area redraw
            break;
        case KEY_NPAGE: // Page Down
            editor_->moveCursorDown(10);
            clearSelection(); // Always clear selection for page moves
            markDirty(true, false, true, false);  // Page moves need editor area redraw
            break;
    }
}

void EditorApplication::handleEditingKey(int key)
{
    TRACE_FUNC
    switch (key)
    {
        case KEY_BACKSPACE:
        case 127:
            if (selection_.isActive)
            {
                deleteSelection();
            }
            else
            {
                if (editor_->getCursor().getPosition().column >=4)
                {
                    for (int i=0; i<4; ++i)
                    {
                        editor_->deleteBackward();
                    }
                }
                else
                editor_->deleteBackward();
            }
            break;
        case KEY_DC: // Delete key
            if (selection_.isActive)
            {
                deleteSelection();
            }
            else
            {
                editor_->deleteCharacter();
            }
            break;
        case 10:        // Line feed (LF)
        case 13:        // Carriage return (CR)
            if (selection_.isActive)
            {
                deleteSelection();
            }
            editor_->insertNewLine();
            break;
    }
}

void EditorApplication::handleFunctionKey(int key)
{
    TRACE_FUNC
    switch (key)
    {
        case KEY_F(1): // Help
            showStatusMessage("F1:Help F2:Save F3:Open F4:LineNumbers F7:Quit | Ctrl+A:SelectAll Ctrl+C:Copy Ctrl+V:Paste Ctrl+X:Cut");
            break;
        case KEY_F(2): // Save
            saveFile();
            break;
        case KEY_F(3): // Open
            if (fileExplorer_->hasSelection())
            {
                auto entry = fileExplorer_->getSelectedEntry();
                if (entry && !entry->isDirectory)
                {
                    openFile(entry->path);
                }
                else if (entry && entry->isDirectory)
                {
                    fileExplorer_->navigateInto(entry->path);
                }
            }
            break;
        case KEY_F(4): // Toggle line numbers
            showLineNumbers_ = !showLineNumbers_;
            updateViewport();
            markDirty(true, false, true, false);
            showStatusMessage(showLineNumbers_ ? "Line numbers ON" : "Line numbers OFF");
            break;
        case KEY_F(7): // Quit
            if (!hasUnsavedChanges() || promptSaveChanges())
            {
                shutdown();
            }
            break;
        case KEY_F(12): // Toggle file explorer
            showFileExplorer_ = !showFileExplorer_;
            updateViewport();
            markDirty(true, true, true, false);  // Full layout change
            showStatusMessage(showFileExplorer_ ? "File explorer ON" : "File explorer OFF");
            break;
    }
}

void EditorApplication::handleControlKey(int key)
{
    TRACE_FUNC
    switch (key)
    {
        case CTRL_KEY('z'):
            if (editor_->undo())
            {
                clearSelection();  // Clear selection after undo
                showStatusMessage("Undo");
            }
            else
            {
                showStatusMessage("Nothing to undo");
            }
            break;
        case CTRL_KEY('y'):
            if (editor_->redo())
            {
                clearSelection();  // Clear selection after redo
                showStatusMessage("Redo");
            }
            else
            {
                showStatusMessage("Nothing to redo");
            }
            break;
        case CTRL_KEY('s'):
            saveFile();
            break;
        case CTRL_KEY('o'):
            handleFunctionKey(KEY_F(3));
            break;
        case CTRL_KEY('n'):
            if (!hasUnsavedChanges() || promptSaveChanges())
            {
                newFile();
            }
            break;
        case CTRL_KEY('q'):
            handleFunctionKey(KEY_F(7));
            break;
        case CTRL_KEY('d'):
            editor_->deleteLine();
            clearSelection();  // Clear selection after line deletion
            showStatusMessage("Line deleted");
            break;
        case CTRL_KEY('e'):
            if (!showFileExplorer_)
            {
                showFileExplorer_ = true;
                updateViewport();
            }
            showStatusMessage("File explorer focused - Use arrows to navigate, Enter/F3 to open, ESC for commands");
            break;
        case CTRL_KEY('a'):
            // Select all text
            if (editor_->hasDocument())
            {
                selection_.isActive = true;
                selection_.startPos = {0, 0};
                auto document = editor_->getDocument();
                if (document->getLineCount() > 0)
                {
                    size_t lastLine = document->getLineCount() - 1;
                    selection_.endPos = {lastLine, document->getLineLength(lastLine)};
                }
                markDirty(true, false, true, false);
                showStatusMessage("Selected all text");
            }
            break;
        case CTRL_KEY('c'):
            // Copy selection
            if (selection_.isActive)
            {
                std::string selectedText = copySelection();
                if (!selectedText.empty())
                {
                    utils::Clipboard::setText(selectedText);
                    showStatusMessage("Copied " + std::to_string(selectedText.length()) + " characters");
                }
                else
                {
                    showStatusMessage("No text in selection");
                }
            }
            else
            {
                showStatusMessage("No text selected");
            }
            break;
        case CTRL_KEY('v'):
            // Paste from clipboard
            if (utils::Clipboard::hasText())
            {
                std::string text = utils::Clipboard::getText();
                if (selection_.isActive)
                {
                    // Replace selected text with clipboard content
                    deleteSelection();
                }
                editor_->insertText(text);
                showStatusMessage("Pasted " + std::to_string(text.length()) + " characters");
            }
            else
            {
                showStatusMessage("Clipboard is empty");
            }
            break;
        case CTRL_KEY('x'):
            // Cut selection (copy + delete)
            if (selection_.isActive)
            {
                std::string selectedText = copySelection();
                if (!selectedText.empty())
                {
                    utils::Clipboard::setText(selectedText);
                    deleteSelection();
                    showStatusMessage("Cut " + std::to_string(selectedText.length()) + " characters");
                }
            }
            else
            {
                showStatusMessage("No text selected");
            }
            break;
    }
}

void EditorApplication::updateViewport()
{
    TRACE_FUNC
    auto screenSize = renderer_->getScreenSize();
    
    // File explorer on the right side
    viewport_.editorStartX = 0;
    viewport_.editorWidth = showFileExplorer_ ? screenSize.width - viewport_.fileExplorerWidth - 1 : screenSize.width;
    viewport_.maxVisibleLines = screenSize.height - 1;
}

void EditorApplication::scrollToEnsureCursorVisible()
{
    TRACE_FUNC
    auto pos = editor_->getCursor().getPosition();
    
    if (static_cast<int>(pos.line) < viewport_.scrollOffsetY)
    {
        viewport_.scrollOffsetY = static_cast<int>(pos.line);
    }
    else if (static_cast<int>(pos.line) >= viewport_.scrollOffsetY + viewport_.maxVisibleLines)
    {
        viewport_.scrollOffsetY = static_cast<int>(pos.line) - viewport_.maxVisibleLines + 1;
    }

    int visibleCols = viewport_.editorWidth - (showLineNumbers_ ? 6 : 0);
    if (pos.column < static_cast<size_t>(viewport_.scrollOffsetX))
    {
        viewport_.scrollOffsetX = static_cast<int>(pos.column);
    }
    else if (pos.column >= static_cast<size_t>(viewport_.scrollOffsetX + visibleCols))
    {
        viewport_.scrollOffsetX = static_cast<int>(pos.column) - visibleCols + 1;
    }
}

void EditorApplication::scrollFileExplorerIntoView()
{
    TRACE_FUNC
    if (!showFileExplorer_) return;

    int explorerHeight = renderer_->getScreenSize().height - 1;
    size_t selectedIndex = fileExplorer_->getSelectedIndex();

    if (selectedIndex < static_cast<size_t>(viewport_.fileExplorerScrollOffset))
    {
        viewport_.fileExplorerScrollOffset = static_cast<size_t>(selectedIndex);
    }
    else if (selectedIndex >= static_cast<size_t>(viewport_.fileExplorerScrollOffset + explorerHeight -3))
    {
        viewport_.fileExplorerScrollOffset = static_cast<int>(selectedIndex) - (explorerHeight - 4);
    }
}

bool EditorApplication::promptSaveChanges()
{
    TRACE_FUNC
    showStatusMessage("Save changes? (y/n)");
    auto screenSize = renderer_->getScreenSize();
    renderer_->clearArea(screenSize.height - 1, 0, 1, screenSize.width);
    renderStatusBar();
    renderer_->refresh();
    
    int key = renderer_->getInput();
    statusMessage_.clear();
    
    return (key == 'y' || key == 'Y') ? saveFile() : (key == 'n' || key == 'N');
}

void EditorApplication::showStatusMessage(const std::string& message)
{
    TRACE_FUNC
    statusMessage_ = message;
}

void EditorApplication::doRenderUpdateBar()
{
    TRACE_FUNC
    auto screenSize = renderer_->getScreenSize();
    renderer_->clearArea(screenSize.height - 1, 0, 1, screenSize.width);
    renderStatusBar();
    renderer_->refresh();
}

void EditorApplication::showCommandHelp()
{
    TRACE_FUNC
    showStatusMessage("Commands: :help :open :save :new :quit :toggle-explorer :toggle-numbers :search | ESC:Cancel");
    doRenderUpdateBar();
    
    std::string command;
    int key;
    
    while ((key = renderer_->getInput()) != 27) // ESC to cancel
    {
        if (key == 10 || key == 13) // Enter to execute
        {
            if (!command.empty())
            {
                handleCommand(command);
            }
            return;
        }
        else if (key == KEY_BACKSPACE || key == 127)
        {
            if (!command.empty())
            {
                command.pop_back();
            }
        }
        else if (key >= 32 && key <= 126)
        {
            command += static_cast<char>(key);
        }
        
        // Force immediate status bar update without going through dirty region system
        showStatusMessage("Command: " + command + " | ESC:Cancel ENTER:Execute");
        doRenderUpdateBar();
    }
    
    showStatusMessage("Command cancelled");
    doRenderUpdateBar();
}

void EditorApplication::markDirty(bool editor, bool explorer, bool status, bool cursor)
{
    TRACE_FUNC
    if (editor) dirtyRegions_.editorArea = true;
    if (explorer) dirtyRegions_.fileExplorer = true;
    if (status) dirtyRegions_.statusBar = true;
    if (cursor) dirtyRegions_.cursorOnly = true;
    
    // If we're marking editor/explorer/status as dirty, that takes priority over cursor-only moves
    if (editor || explorer || status)
    {
        dirtyRegions_.fullRedraw = false;
        dirtyRegions_.cursorOnly = false;  // Clear cursor-only flag when other areas need updating
    }
    //it's just a cursor move and nothing else is already dirty
    else if (cursor && !dirtyRegions_.editorArea && !dirtyRegions_.fileExplorer)
    {
        dirtyRegions_.fullRedraw = false;
    }
}

void EditorApplication::clearDirtyRegions()
{
    TRACE_FUNC
    dirtyRegions_.fullRedraw = false;
    dirtyRegions_.editorArea = false;
    dirtyRegions_.fileExplorer = false;
    dirtyRegions_.statusBar = false;
    dirtyRegions_.cursorOnly = false;
}

void EditorApplication::startSelection()
{
    TRACE_FUNC
    selection_.isActive = true;
    selection_.startPos = editor_->getCursor().getPosition();
    selection_.endPos = selection_.startPos;
    markDirty(true, false, true, false);  // Redraw editor and status bar
}

void EditorApplication::endSelection()
{
    TRACE_FUNC
    if (selection_.isActive)
    {
        selection_.endPos = editor_->getCursor().getPosition();
        // Keep selection active so it remains visible
        markDirty(true, false, false, false);  // Redraw editor area
    }
}

void EditorApplication::clearSelection()
{
    TRACE_FUNC
    if (selection_.isActive)
    {
        selection_.isActive = false;
        markDirty(true, false, true, false);  // Redraw editor and status bar
    }
}

bool EditorApplication::isPositionSelected(size_t line, size_t column) const
{
    TRACE_FUNC
    if (!selection_.isActive)
        return false;
    
    // Normalize selection (ensure start <= end)
    auto start = selection_.startPos;
    auto end = selection_.endPos;
    
    if (start.line > end.line || (start.line == end.line && start.column > end.column))
    {
        std::swap(start, end);
    }
    
    // Check if position is within selection
    if (line < start.line || line > end.line)
        return false;
    
    if (line == start.line && line == end.line)
    {
        return column >= start.column && column < end.column;
    }
    else if (line == start.line)
    {
        return column >= start.column;
    }
    else if (line == end.line)
    {
        return column < end.column;
    }
    else
    {
        return true;  // Line is between start and end
    }
}

std::string EditorApplication::copySelection() const
{
    TRACE_FUNC
    if (!selection_.isActive || !editor_->hasDocument())
        return "";
    
    auto document = editor_->getDocument();
    std::string result;
    
    // Normalize selection
    auto start = selection_.startPos;
    auto end = selection_.endPos;
    
    if (start.line > end.line || (start.line == end.line && start.column > end.column))
    {
        std::swap(start, end);
    }
    
    // Extract selected text
    for (size_t line = start.line; line <= end.line && line < document->getLineCount(); ++line)
    {
        const std::string& lineContent = document->getLine(line);
        
        if (start.line == end.line)
        {
            // Single line selection
            size_t startCol = std::min(start.column, lineContent.length());
            size_t endCol = std::min(end.column, lineContent.length());
            result += lineContent.substr(startCol, endCol - startCol);
        }
        else if (line == start.line)
        {
            // First line of multi-line selection
            size_t startCol = std::min(start.column, lineContent.length());
            result += lineContent.substr(startCol) + "\n";
        }
        else if (line == end.line)
        {
            // Last line of multi-line selection
            size_t endCol = std::min(end.column, lineContent.length());
            result += lineContent.substr(0, endCol);
        }
        else
        {
            // Middle lines of multi-line selection
            result += lineContent + "\n";
        }
    }
    
    return result;
}

void EditorApplication::deleteSelection()
{
    TRACE_FUNC
    if (!selection_.isActive || !editor_->hasDocument())
        return;
    
    auto document = editor_->getDocument();
    
    // Normalize selection
    auto start = selection_.startPos;
    auto end = selection_.endPos;
    
    if (start.line > end.line || (start.line == end.line && start.column > end.column))
    {
        std::swap(start, end);
    }
    
    // Move cursor to start of selection
    editor_->moveCursor(start);
    
    // Delete text from end to start to avoid position shifting issues
    if (start.line == end.line)
    {
        // Single line deletion
        size_t length = end.column - start.column;
        for (size_t i = 0; i < length; ++i)
        {
            editor_->deleteCharacter();
        }
    }
    else
    {
        // Multi-line deletion - more complex, delete line by line from end to start
        for (size_t line = end.line; line > start.line; --line)
        {
            if (line == end.line)
            {
                // Delete from beginning of end line to end column
                editor_->moveCursor({line, 0});
                for (size_t col = 0; col < end.column; ++col)
                {
                    editor_->deleteCharacter();
                }
                // Delete the line itself if it became empty or merge with previous
                editor_->moveCursor({line - 1, document->getLineLength(line - 1)});
                editor_->deleteCharacter(); // Delete newline to merge lines
            }
            else
            {
                // Delete entire middle lines
                editor_->moveCursor({line, 0});
                editor_->deleteLine();
            }
        }
        
        // Delete from start column to end of start line
        editor_->moveCursor(start);
        const std::string& startLine = document->getLine(start.line);
        for (size_t col = start.column; col < startLine.length(); ++col)
        {
            editor_->deleteCharacter();
        }
    }
    
    clearSelection();
}

void EditorApplication::handleCommand(const std::string& command)
{
    TRACE_FUNC
    if (command == ":help" || command == "help")
    {
        showStatusMessage("Commands: :open <file> :save [file] :new :quit :toggle-explorer :toggle-numbers");
    }
    else if (command == ":quit" || command == "quit" || command == "q")
    {
        handleFunctionKey(KEY_F(7)); // Use existing quit logic
    }
    else if (command == ":new" || command == "new")
    {
        if (!hasUnsavedChanges() || promptSaveChanges())
        {
            newFile();
        }
    }
    else if (command == ":save" || command == "save")
    {
        saveFile();
    }
    else if (command.substr(0, 6) == ":save " || command.substr(0, 5) == "save ")
    {
        std::string filename = command.substr(command.find(' ') + 1);
        saveFile(filename);
    }
    else if (command.substr(0, 6) == ":open " || command.substr(0, 5) == "open ")
    {
        std::string filename = command.substr(command.find(' ') + 1);
        openFile(filename);
    }
    else if (command == ":toggle-explorer" || command == "toggle-explorer")
    {
        showFileExplorer_ = !showFileExplorer_;
        updateViewport();
        markDirty(true, true, true, false);  // Mark everything as dirty for layout changes
        showStatusMessage(showFileExplorer_ ? "File explorer ON" : "File explorer OFF");
    }
    else if (command == ":toggle-numbers" || command == "toggle-numbers")
    {
        showLineNumbers_ = !showLineNumbers_;
        updateViewport();
        markDirty(true, false, true, false);  // Mark editor and status as dirty
        showStatusMessage(showLineNumbers_ ? "Line numbers ON" : "Line numbers OFF");
    }
    else if (command == ":search" || command == "search")
    {
        auto inputSearchTerm = [this]() -> std::string
        {
            std::string searchTerm;
            showStatusMessage("Search: ");
            doRenderUpdateBar();

            int key;
            while ((key = renderer_->getInput()) != 10 && key != 13)
            {
                if (key == KEY_BACKSPACE || key == 127)
                {
                    if (!searchTerm.empty())
                        searchTerm.pop_back();
                }
                else if (key >= 32 && key <= 126)
                {
                    searchTerm += static_cast<char>(key);
                }

                showStatusMessage("Search: " + searchTerm);
                doRenderUpdateBar();
            }
            return searchTerm;
        };

        std::string term = inputSearchTerm();

        lastSearchLine = 0;
        lastSearchColumn = 0;

        openSearchDialog(term, true);
    }

    else
    {
        showStatusMessage("Unknown command: " + command + " | Type :help for available commands");
    }
}