#include "../include/core/CommandManager.h"
#include "../include/utils/MemoryDebugger.h"
#include <algorithm>

using namespace mexedit::core;

CommandManager::CommandManager(size_t maxHistorySize) 
    : currentIndex_(0)
    , maxHistorySize_(maxHistorySize)
{
    TRACK_MEMORY(CommandManager, this);
}

CommandManager::~CommandManager()
{
    UNTRACK_MEMORY(CommandManager, this);
}

void CommandManager::executeCommand(CommandPtr command)
{
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
    return currentIndex_ > 0 && !history_.empty();
}

bool CommandManager::canRedo() const
{
    return currentIndex_ < history_.size();
}

void CommandManager::clearHistory()
{
    history_.clear();
    currentIndex_ = 0;
}

void CommandManager::setMaxHistorySize(size_t maxSize)
{
    maxHistorySize_ = maxSize;
    trimHistory();
}

std::string CommandManager::getUndoDescription() const
{
    if (canUndo())
    {
        return history_[currentIndex_ - 1]->getDescription();
    }
    return "";
}

std::string CommandManager::getRedoDescription() const
{
    if (canRedo())
    {
        return history_[currentIndex_]->getDescription();
    }
    return "";
}

void CommandManager::trimHistory()
{
    if (history_.size() > maxHistorySize_)
    {
        size_t excess = history_.size() - maxHistorySize_;
        history_.erase(history_.begin(), history_.begin() + excess);
        currentIndex_ = std::min(currentIndex_, history_.size());
    }
}
