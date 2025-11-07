#include <utility>
#include "core/EditCommands.h"
#include "utils/MemoryDebugger.h"
#include "utils/Logger.h"

using namespace mexedit::core;

InsertTextCommand::InsertTextCommand(std::shared_ptr<Document> document, std::shared_ptr<Cursor> cursor, const Cursor::Position& position, std::string  text)
    : document_(std::move(document))
    , cursor_(std::move(cursor))
    , position_(position)
    , text_(std::move(text))
    , executed_(false)
{
    TRACE_FUNC
    TRACK_MEMORY(InsertTextCommand, this);
}

InsertTextCommand::~InsertTextCommand()
{
    TRACE_FUNC
    UNTRACK_MEMORY(InsertTextCommand, this);
}

void InsertTextCommand::execute()
{
    TRACE_FUNC
    if (document_)
    {
        document_->insertText(position_.line, position_.column, text_);
        executed_ = true;
    }
}

void InsertTextCommand::undo()
{
    TRACE_FUNC
    if (executed_ && document_)
    {
        document_->deleteText(position_.line, position_.column, text_.length());
        if (cursor_)
        {
            cursor_->setPosition(position_);
        }
    }
}

std::string InsertTextCommand::getDescription() const
{
    TRACE_FUNC
    return "Insert text: \"" + text_ + "\"";
}

DeleteTextCommand::DeleteTextCommand(std::shared_ptr<Document> document, std::shared_ptr<Cursor> cursor, const Cursor::Position& position, size_t length)
    : document_(std::move(document))
    , cursor_(std::move(cursor))
    , position_(position)
    , length_(length)
    , executed_(false)
{
    TRACE_FUNC
    TRACK_MEMORY(DeleteTextCommand, this);
}

DeleteTextCommand::~DeleteTextCommand()
{
    TRACE_FUNC
    UNTRACK_MEMORY(DeleteTextCommand, this);
}

void DeleteTextCommand::execute()
{
    TRACE_FUNC
    if (document_ && position_.line < document_->getLineCount())
    {
        const std::string& line = document_->getLine(position_.line);
        if (position_.column < line.length())
        {
            size_t actualLength = std::min(length_, line.length() - position_.column);
            deletedText_ = line.substr(position_.column, actualLength);
            document_->deleteText(position_.line, position_.column, actualLength);
            executed_ = true;
        }
    }
}

void DeleteTextCommand::undo()
{
    TRACE_FUNC
    if (executed_ && document_ && !deletedText_.empty())
    {
        document_->insertText(position_.line, position_.column, deletedText_);
        if (cursor_) {
            cursor_->setPosition({position_.line, position_.column + deletedText_.length()});
        }
    }
}

std::string DeleteTextCommand::getDescription() const
{
    TRACE_FUNC
    return "Delete text: \"" + deletedText_ + "\"";
}

InsertLineCommand::InsertLineCommand(std::shared_ptr<Document> document, std::shared_ptr<Cursor> cursor, size_t linePosition, std::string  content)
    : document_(std::move(document))
    , cursor_(std::move(cursor))
    , linePosition_(linePosition)
    , content_(std::move(content))
    , executed_(false)
{
    TRACE_FUNC
    TRACK_MEMORY(InsertLineCommand, this);
}

InsertLineCommand::~InsertLineCommand()
{
    TRACE_FUNC
    UNTRACK_MEMORY(InsertLineCommand, this);
}

void InsertLineCommand::execute()
{
    TRACE_FUNC
    if (document_)
    {
        document_->insertLine(linePosition_, content_);
        executed_ = true;
    }
}

void InsertLineCommand::undo()
{
    TRACE_FUNC
    if (executed_ && document_)
    {
        document_->deleteLine(linePosition_);
        if (cursor_ && linePosition_ > 0)
        {
            size_t prevLine = linePosition_ - 1;
            cursor_->setPosition({prevLine, document_->getLineLength(prevLine)});
        }
    }
}

std::string InsertLineCommand::getDescription() const
{
    TRACE_FUNC
    return "Insert line";
}

DeleteLineCommand::DeleteLineCommand(std::shared_ptr<Document> document, std::shared_ptr<Cursor> cursor, size_t linePosition)
    : document_(std::move(document))
    , cursor_(std::move(cursor))
    , linePosition_(linePosition)
    , executed_(false)
{
    TRACE_FUNC
    TRACK_MEMORY(DeleteLineCommand, this);
}

DeleteLineCommand::~DeleteLineCommand()
{
    TRACE_FUNC
    UNTRACK_MEMORY(DeleteLineCommand, this);
}

void DeleteLineCommand::execute()
{
    TRACE_FUNC
    if (document_ && linePosition_ < document_->getLineCount())
    {
        deletedContent_ = document_->getLine(linePosition_);
        document_->deleteLine(linePosition_);
        executed_ = true;
    }
}

void DeleteLineCommand::undo()
{
    TRACE_FUNC
    if (executed_ && document_)
    {
        document_->insertLine(linePosition_, deletedContent_);
        if (cursor_)
        {
            cursor_->setPosition({linePosition_, 0});
        }
    }
}

std::string DeleteLineCommand::getDescription() const
{
    TRACE_FUNC
    return "Delete line: \"" + deletedContent_ + "\"";
}
