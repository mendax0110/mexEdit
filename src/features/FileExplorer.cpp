#include "../include/features/FileExplorer.h"
#include "../include/utils/FileUtils.h"
#include <algorithm>
#include <utility>

using namespace mexedit::features;

FileExplorer::FileExplorer(std::filesystem::path  rootPath)
    : currentPath_(std::move(rootPath))
    , selectedIndex_(0)
    , showHiddenFiles_(false)
{
    refresh();
}

void FileExplorer::setCurrentDirectory(const std::filesystem::path& path)
{
    if (utils::FileUtils::exists(path) && utils::FileUtils::isDirectory(path))
    {
        currentPath_ = path;
        selectedIndex_ = 0;
        refresh();
    }
}

bool FileExplorer::navigateUp()
{
    auto parent = currentPath_.parent_path();
    if (parent != currentPath_)
    {
        setCurrentDirectory(parent);
        return true;
    }
    return false;
}

bool FileExplorer::navigateInto(const std::filesystem::path& subPath)
{
    std::filesystem::path fullPath;
    
    if (subPath.is_absolute())
    {
        fullPath = subPath;
    }
    else 
    {
        fullPath = currentPath_ / subPath;
    }
    
    if (utils::FileUtils::exists(fullPath) && utils::FileUtils::isDirectory(fullPath))
    {
        setCurrentDirectory(fullPath);
        return true;
    }
    return false;
}

void FileExplorer::refresh()
{
    entries_.clear();
    
    try
    {
        auto parent = currentPath_.parent_path();
        if (parent != currentPath_)
        {
            FileEntry parentEntry;
            parentEntry.path = parent;
            parentEntry.displayName = "..";
            parentEntry.isDirectory = true;
            parentEntry.fileSize = 0;
            entries_.push_back(parentEntry);
        }
        
        auto dirEntries = utils::FileUtils::listDirectory(currentPath_);
        
        for (const auto& entry : dirEntries)
        {
            std::string filename = entry.path().filename().string();
            
            if (!showHiddenFiles_ && filename.starts_with('.'))
            {
                continue;
            }
            
            FileEntry fileEntry;
            fileEntry.path = entry.path();
            fileEntry.displayName = filename;
            fileEntry.isDirectory = entry.is_directory();
            
            try
            {
                fileEntry.fileSize = fileEntry.isDirectory ? 0 : entry.file_size();
                fileEntry.lastModified = entry.last_write_time();
            }
            catch (const std::filesystem::filesystem_error&)
            {
                fileEntry.fileSize = 0;
            }
            
            entries_.push_back(fileEntry);
        }
        
        sortEntries();
        
        if (selectedIndex_ >= entries_.size())
        {
            selectedIndex_ = entries_.empty() ? 0 : entries_.size() - 1;
        }
        
    }
    catch (const std::filesystem::filesystem_error&)
    {
        // Directory is not accessible, entries_ remains empty
    }
}

void FileExplorer::setSelectedIndex(size_t index)
{
    if (index < entries_.size())
    {
        selectedIndex_ = index;
        notifySelection();
    }
}

const FileEntry* FileExplorer::getSelectedEntry() const
{
    return hasSelection() ? &entries_[selectedIndex_] : nullptr;
}

void FileExplorer::selectNext()
{
    if (!entries_.empty() && selectedIndex_ < entries_.size() - 1)
    {
        selectedIndex_++;
        notifySelection();
    }
}

void FileExplorer::selectPrevious()
{
    if (selectedIndex_ > 0)
    {
        selectedIndex_--;
        notifySelection();
    }
}

void FileExplorer::selectFirst()
{
    if (!entries_.empty())
    {
        selectedIndex_ = 0;
        notifySelection();
    }
}

void FileExplorer::selectLast()
{
    if (!entries_.empty())
    {
        selectedIndex_ = entries_.size() - 1;
        notifySelection();
    }
}

void FileExplorer::setShowHiddenFiles(bool show)
{
    if (showHiddenFiles_ != show)
    {
        showHiddenFiles_ = show;
        refresh();
    }
}

void FileExplorer::loadEntries()
{
    // This is now handled in refresh()
}

void FileExplorer::sortEntries()
{
    std::sort(entries_.begin(), entries_.end(), [](const FileEntry& a, const FileEntry& b)
    {
        if (a.displayName == "..") return true;
        if (b.displayName == "..") return false;

        if (a.isDirectory != b.isDirectory)
        {
            return a.isDirectory;
        }

        return a.displayName < b.displayName;
    });
}

void FileExplorer::notifySelection()
{
    if (selectionCallback_ && hasSelection())
    {
        selectionCallback_(entries_[selectedIndex_].path);
    }
}

std::string FileExplorer::formatFileSize(size_t size)
{
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    auto dsize = static_cast<double>(size);
    int unit = 0;
    
    while (dsize >= 1024.0 && unit < 4)
    {
        dsize /= 1024.0;
        ++unit;
    }
    
    if (unit == 0)
    {
        return std::to_string(static_cast<int>(dsize)) + " " + units[unit];
    }
    else
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.1f %s", dsize, units[unit]);
        return {buffer};
    }
}

void FileExplorer::moveSelectionUp()
{
    selectPrevious();
}

void FileExplorer::moveSelectionDown()
{
    selectNext();
}