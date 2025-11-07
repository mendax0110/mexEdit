#include "core/CommandManager.h"
#include "utils/MemoryDebugger.h"
#include "utils/Logger.h"
#include <algorithm>

using namespace mexedit::core;

CommandManager::CommandManager(size_t maxHistorySize) 
    : currentIndex_(0)
    , maxHistorySize_(maxHistorySize)
{
    TRACE_FUNC
    TRACK_MEMORY(CommandManager, this);
}

CommandManager::~CommandManager()
{
    TRACE_FUNC
    UNTRACK_MEMORY(CommandManager, this);
}

void CommandManager::executeCommand(CommandPtr command)
{
    TRACE_FUNC
    if (!command)
    {
        return;
    }

    if (currentIndex_ < history_.size())
    {
        history_.erase(history_.begin() + currentIndex_, history_.end());
    }

    command->execute();
    
    history_.push_back(std::move(command));
    currentIndex_ = history_.size();
    
    trimHistory();
}

bool CommandManager::undo()
{
    TRACE_FUNC
    if (!canUndo())
    {
        return false;
    }
    
    --currentIndex_;
    history_[currentIndex_]->undo();
    return true;
}

bool CommandManager::redo()
{
    TRACE_FUNC
    if (!canRedo())
    {
        return false;
    }
    
    history_[currentIndex_]->execute();
    ++currentIndex_;
    return true;
}

bool CommandManager::canUndo() const
{
    TRACE_FUNC
    return currentIndex_ > 0 && !history_.empty();
}

bool CommandManager::canRedo() const
{
    TRACE_FUNC
    return currentIndex_ < history_.size();
}

void CommandManager::clearHistory()
{
    TRACE_FUNC
    history_.clear();
    currentIndex_ = 0;
}

void CommandManager::setMaxHistorySize(size_t maxSize)
{
    TRACE_FUNC
    maxHistorySize_ = maxSize;
    trimHistory();
}

std::string CommandManager::getUndoDescription() const
{
    TRACE_FUNC
    if (canUndo())
    {
        return history_[currentIndex_ - 1]->getDescription();
    }
    return "";
}

std::string CommandManager::getRedoDescription() const
{
    TRACE_FUNC
    if (canRedo())
    {
        return history_[currentIndex_]->getDescription();
    }
    return "";
}

void CommandManager::trimHistory()
{
    TRACE_FUNC
    if (history_.size() > maxHistorySize_)
    {
        size_t excess = history_.size() - maxHistorySize_;
        history_.erase(history_.begin(), history_.begin() + excess);
        currentIndex_ = std::min(currentIndex_, history_.size());
    }
}
