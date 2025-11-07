#include "core/Editor.h"
#include "core/EditCommands.h"
#include "utils/Logger.h"
#include "utils/MemoryDebugger.h"
#include <memory>

using namespace mexedit::core;

Editor::Editor(std::shared_ptr<Document> document) 
    : document_(std::move(document))
    , cursor_(std::make_shared<Cursor>())
    , commandManager_(std::make_unique<CommandManager>())
{
    TRACE_FUNC
    TRACK_MEMORY(Editor, this);
    setupCursorValidator();
}

Editor::~Editor()
{
    TRACE_FUNC
    UNTRACK_MEMORY(Editor, this);
}

void Editor::setDocument(std::shared_ptr<Document> document)
{
    TRACE_FUNC
    document_ = std::move(document);
    cursor_->setPosition({0, 0});
    commandManager_->clearHistory();
    
    if (document_)
    {
        document_->setChangeCallback([this]() { notifyDocumentChanged(); });
    }
    
    notifyDocumentChanged();
}

void Editor::moveCursor(const Cursor::Position& position)
{
    TRACE_FUNC
    cursor_->setPosition(position);
}

void Editor::moveCursorUp(size_t lines)
{
    TRACE_FUNC
    cursor_->moveUp(lines);
}

void Editor::moveCursorDown(size_t lines)
{
    TRACE_FUNC
    cursor_->moveDown(lines);
}

void Editor::moveCursorLeft(size_t columns)
{
    TRACE_FUNC
    cursor_->moveLeft(columns);
}

void Editor::moveCursorRight(size_t columns)
{
    TRACE_FUNC
    cursor_->moveRight(columns);
}

void Editor::moveCursorToLineStart()
{
    TRACE_FUNC
    cursor_->moveToLineStart();
}

void Editor::moveCursorToLineEnd()
{
    TRACE_FUNC
    cursor_->moveToLineEnd();
}

void Editor::moveCursorToDocumentStart()
{
    TRACE_FUNC
    cursor_->moveToDocumentStart();
}

void Editor::moveCursorToDocumentEnd()
{
    TRACE_FUNC
    cursor_->moveToDocumentEnd();
}

void Editor::insertText(const std::string& text)
{
    TRACE_FUNC
    if (!document_ || text.empty())
    {
        return;
    }

    auto command = std::make_unique<InsertTextCommand>(document_, cursor_, cursor_->getPosition(), text);
    commandManager_->executeCommand(std::move(command));
    
    auto pos = cursor_->getPosition();
    pos.column += text.length();
    cursor_->setPosition(pos);
}

void Editor::insertCharacter(char ch)
{
    TRACE_FUNC
    insertText(std::string(1, ch));
}

void Editor::insertNewLine()
{
    TRACE_FUNC
    if (!document_)
    {
        return;
    }

    auto pos = cursor_->getPosition();
    
    // Get current line content
    const std::string& currentLine = document_->getLine(pos.line);
    
    // Split line at cursor position
    std::string beforeCursor = currentLine.substr(0, pos.column);
    std::string afterCursor = (pos.column < currentLine.length()) ? currentLine.substr(pos.column) : "";

    auto deleteCommand = std::unique_ptr<DeleteTextCommand>(new DeleteTextCommand(document_, cursor_, {pos.line, 0}, currentLine.length()));
    commandManager_->executeCommand(std::move(deleteCommand));
    
    auto insertBeforeCommand = std::unique_ptr<InsertTextCommand>(new InsertTextCommand(document_, cursor_, {pos.line, 0}, beforeCursor));
    commandManager_->executeCommand(std::move(insertBeforeCommand));
    
    auto insertLineCommand = std::make_unique<InsertLineCommand>(document_, cursor_, pos.line + 1, afterCursor);
    commandManager_->executeCommand(std::move(insertLineCommand));
    
    // Move cursor to beginning of new line
    cursor_->setPosition({pos.line + 1, 0});
    
    // Notify that document changed
    notifyDocumentChanged();
}

void Editor::deleteCharacter()
{
    TRACE_FUNC
    if (!document_)
    {
        return;
    }

    auto pos = cursor_->getPosition();
    if (pos.line < document_->getLineCount())
    {
        const std::string& line = document_->getLine(pos.line);
        if (pos.column < line.length())
        {
            auto command = std::make_unique<DeleteTextCommand>(document_, cursor_, pos, 1);
            commandManager_->executeCommand(std::move(command));
        }
    }
}

void Editor::deleteBackward()
{
    TRACE_FUNC
    if (!document_)
    {
        return;
    }

    auto pos = cursor_->getPosition();
    
    if (pos.column > 0)
    {
        pos.column--;
        cursor_->setPosition(pos);
        
        auto command = std::make_unique<DeleteTextCommand>(document_, cursor_, pos, 1);
        commandManager_->executeCommand(std::move(command));
    }
    else if (pos.line > 0)
    {
        size_t prevLineLength = document_->getLineLength(pos.line - 1);
        std::string currentLineContent = document_->getLine(pos.line);
        
        auto deleteLineCommand = std::make_unique<DeleteLineCommand>(document_, cursor_, pos.line);
        commandManager_->executeCommand(std::move(deleteLineCommand));
        
        if (!currentLineContent.empty())
        {
            auto insertCommand = std::unique_ptr<InsertTextCommand>(new InsertTextCommand(document_, cursor_, {pos.line - 1, prevLineLength}, currentLineContent));
            commandManager_->executeCommand(std::move(insertCommand));
        }
        
        cursor_->setPosition({pos.line - 1, prevLineLength});
    }
}

void Editor::deleteLine()
{
    TRACE_FUNC
    if (!document_)
    {
        return;
    }

    auto pos = cursor_->getPosition();
    if (pos.line < document_->getLineCount())
    {
        auto command = std::make_unique<DeleteLineCommand>(document_, cursor_, pos.line);
        commandManager_->executeCommand(std::move(command));
        
        if (pos.line >= document_->getLineCount() && document_->getLineCount() > 0)
        {
            cursor_->setPosition({document_->getLineCount() - 1, 0});
        }
        else
        {
            cursor_->setPosition({pos.line, 0});
        }
    }
}

bool Editor::openFile(const std::filesystem::path& path)
{
    TRACE_FUNC
    auto newDocument = std::make_shared<Document>();
    if (newDocument->load(path))
    {
        setDocument(newDocument);
        return true;
    }
    return false;
}

bool Editor::saveFile(const std::filesystem::path& path)
{
    TRACE_FUNC
    return document_ && document_->save(path);
}

bool Editor::isModified() const
{
    TRACE_FUNC
    return document_ && document_->isModified();
}

bool Editor::undo()
{
    TRACE_FUNC
    return commandManager_->undo();
}

bool Editor::redo()
{
    TRACE_FUNC
    return commandManager_->redo();
}

bool Editor::canUndo() const
{
    TRACE_FUNC
    return commandManager_->canUndo();
}

bool Editor::canRedo() const
{
    TRACE_FUNC
    return commandManager_->canRedo();
}

void Editor::setDocumentChangedCallback(DocumentChangedCallback callback)
{
    TRACE_FUNC
    documentChangedCallback_ = std::move(callback);
}

void Editor::setCursorMovedCallback(CursorMovedCallback callback)
{
    TRACE_FUNC
    cursorMovedCallback_ = std::move(callback);
    cursor_->setMoveCallback([this](const Cursor::Position& pos)
    {
        (void)pos;
        notifyCursorMoved();
    });
}

void Editor::setupCursorValidator()
{
    TRACE_FUNC
    cursor_->setValidator([this](const Cursor::Position& pos) -> Cursor::Position
    {
        return validateCursorPosition(pos);
    });
}

void Editor::notifyDocumentChanged()
{
    TRACE_FUNC
    if (documentChangedCallback_)
    {
        documentChangedCallback_();
    }
}

void Editor::notifyCursorMoved()
{
    TRACE_FUNC
    if (cursorMovedCallback_)
    {
        cursorMovedCallback_(cursor_->getPosition());
    }
}

Cursor::Position Editor::validateCursorPosition(const Cursor::Position& position) const
{
    TRACE_FUNC
    if (!document_ || document_->isEmpty())
    {
        return {0, 0};
    }

    Cursor::Position validated = position;
    
    validated.line = std::min(validated.line, document_->getLineCount() - 1);

    size_t lineLength = document_->getLineLength(validated.line);
    validated.column = std::min(validated.column, lineLength);
    
    return validated;
}
