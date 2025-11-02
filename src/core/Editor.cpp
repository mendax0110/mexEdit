#include "../include/core/Editor.h"
#include "../include/core/EditCommands.h"
#include <algorithm>

using namespace mexedit::core;

Editor::Editor(std::shared_ptr<Document> document) 
    : document_(std::move(document))
    , cursor_(std::make_shared<Cursor>())
    , commandManager_(std::make_unique<CommandManager>())
{
    setupCursorValidator();
}

void Editor::setDocument(std::shared_ptr<Document> document)
{
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
    cursor_->setPosition(position);
}

void Editor::moveCursorUp(size_t lines)
{
    cursor_->moveUp(lines);
}

void Editor::moveCursorDown(size_t lines)
{
    cursor_->moveDown(lines);
}

void Editor::moveCursorLeft(size_t columns)
{
    cursor_->moveLeft(columns);
}

void Editor::moveCursorRight(size_t columns)
{
    cursor_->moveRight(columns);
}

void Editor::moveCursorToLineStart()
{
    cursor_->moveToLineStart();
}

void Editor::moveCursorToLineEnd()
{
    cursor_->moveToLineEnd();
}

void Editor::moveCursorToDocumentStart()
{
    cursor_->moveToDocumentStart();
}

void Editor::moveCursorToDocumentEnd()
{
    cursor_->moveToDocumentEnd();
}

void Editor::insertText(const std::string& text)
{
    if (!document_ || text.empty())
    {
        return;
    }

    auto command = std::unique_ptr<InsertTextCommand>(new InsertTextCommand(document_, cursor_, cursor_->getPosition(), text));
    commandManager_->executeCommand(std::move(command));
    
    auto pos = cursor_->getPosition();
    pos.column += text.length();
    cursor_->setPosition(pos);
}

void Editor::insertCharacter(char ch)
{
    insertText(std::string(1, ch));
}

void Editor::insertNewLine()
{
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
    
    auto insertLineCommand = std::unique_ptr<InsertLineCommand>(new InsertLineCommand(document_, cursor_, pos.line + 1, afterCursor));
    commandManager_->executeCommand(std::move(insertLineCommand));
    
    // Move cursor to beginning of new line
    cursor_->setPosition({pos.line + 1, 0});
    
    // Notify that document changed
    notifyDocumentChanged();
}

void Editor::deleteCharacter()
{
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
            auto command = std::unique_ptr<DeleteTextCommand>(new DeleteTextCommand(document_, cursor_, pos, 1));
            commandManager_->executeCommand(std::move(command));
        }
    }
}

void Editor::deleteBackward()
{
    if (!document_)
    {
        return;
    }

    auto pos = cursor_->getPosition();
    
    if (pos.column > 0)
    {
        pos.column--;
        cursor_->setPosition(pos);
        
        auto command = std::unique_ptr<DeleteTextCommand>(new DeleteTextCommand(document_, cursor_, pos, 1));
        commandManager_->executeCommand(std::move(command));
    }
    else if (pos.line > 0)
    {
        size_t prevLineLength = document_->getLineLength(pos.line - 1);
        std::string currentLineContent = document_->getLine(pos.line);
        
        auto deleteLineCommand = std::unique_ptr<DeleteLineCommand>(new DeleteLineCommand(document_, cursor_, pos.line));
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
    if (!document_)
    {
        return;
    }

    auto pos = cursor_->getPosition();
    if (pos.line < document_->getLineCount())
    {
        auto command = std::unique_ptr<DeleteLineCommand>(new DeleteLineCommand(document_, cursor_, pos.line));
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
    return document_ ? document_->save(path) : false;
}

bool Editor::isModified() const
{
    return document_ ? document_->isModified() : false;
}

bool Editor::undo()
{
    return commandManager_->undo();
}

bool Editor::redo()
{
    return commandManager_->redo();
}

bool Editor::canUndo() const
{
    return commandManager_->canUndo();
}

bool Editor::canRedo() const
{
    return commandManager_->canRedo();
}

void Editor::setDocumentChangedCallback(DocumentChangedCallback callback)
{
    documentChangedCallback_ = std::move(callback);
}

void Editor::setCursorMovedCallback(CursorMovedCallback callback)
{
    cursorMovedCallback_ = std::move(callback);
    cursor_->setMoveCallback([this](const Cursor::Position& pos)
    {
        notifyCursorMoved();
    });
}

void Editor::setupCursorValidator()
{
    cursor_->setValidator([this](const Cursor::Position& pos) -> Cursor::Position
    {
        return validateCursorPosition(pos);
    });
}

void Editor::notifyDocumentChanged()
{
    if (documentChangedCallback_)
    {
        documentChangedCallback_();
    }
}

void Editor::notifyCursorMoved()
{
    if (cursorMovedCallback_)
    {
        cursorMovedCallback_(cursor_->getPosition());
    }
}

Cursor::Position Editor::validateCursorPosition(const Cursor::Position& position) const
{
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
