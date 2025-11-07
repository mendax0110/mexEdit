#include "core/Document.h"
#include "utils/FileUtils.h"
#include "utils/MemoryDebugger.h"
#include "utils/Logger.h"
#include <sstream>
#include <stdexcept>

using namespace mexedit::core;

Document::Document() : isModified_(false)
{
    TRACE_FUNC
    TRACK_MEMORY(Document, this);
    lines_.emplace_back();
}

Document::Document(const std::filesystem::path& path)
    : filePath_(path)
    , isModified_(false)
{
    TRACE_FUNC
    TRACK_MEMORY(Document, this);
    if (!load(path))
    {
        lines_.emplace_back();
    }
}

Document::~Document()
{
    TRACE_FUNC
    UNTRACK_MEMORY(Document, this);
}

bool Document::load(const std::filesystem::path& path)
{
    TRACE_FUNC
    if (!utils::FileUtils::exists(path) || !utils::FileUtils::isReadable(path))
    {
        return false;
    }

    try
    {
        auto loadedLines = utils::FileUtils::readLines(path);
        if (loadedLines.empty())
        {
            loadedLines.emplace_back();
        }
        
        lines_ = std::move(loadedLines);
        filePath_ = path;
        isModified_ = false;
        
        if (changeCallback_)
        {
            changeCallback_();
        }
        
        return true;
    }
    catch (const std::exception& error)
    {
        LOG_ERR("Failed to load file: " << error.what());
        return false;
    }
}

bool Document::save(const std::filesystem::path& path)
{
    TRACE_FUNC
    std::filesystem::path savePath = path.empty() ? filePath_ : path;
    
    if (savePath.empty())
    {
        return false;
    }

    try
    {
        if (utils::FileUtils::writeLines(savePath, lines_))
        {
            filePath_ = savePath;
            isModified_ = false;
            
            if (changeCallback_)
            {
                changeCallback_();
            }
            
            return true;
        }
    }
    catch (const std::exception& error)
    {
        LOG_ERR("Failed to save file: " << error.what());
    }
    
    return false;
}

const std::string& Document::getLine(size_t line) const
{
    TRACE_FUNC
    static const std::string emptyLine;
    return (line < lines_.size()) ? lines_[line] : emptyLine;
}

std::string Document::getText() const
{
    TRACE_FUNC
    std::ostringstream oss;
    for (size_t i = 0; i < lines_.size(); ++i)
    {
        if (i > 0) oss << '\n';
        oss << lines_[i];
    }
    return oss.str();
}

size_t Document::getLineLength(size_t line) const
{
    TRACE_FUNC
    return (line < lines_.size()) ? lines_[line].length() : 0;
}

void Document::insertText(size_t line, size_t column, const std::string& text)
{
    TRACE_FUNC
    ensureLineExists(line);

    if (column > lines_[line].length())
    {
        column = lines_[line].length();
    }

    if (text.find('\n') != std::string::npos)
    {
        std::istringstream iss(text);
        std::string firstPart;
        std::getline(iss, firstPart);
        
        lines_[line].insert(column, firstPart);
        
        std::string remainderOfCurrentLine = lines_[line].substr(column + firstPart.length());
        lines_[line].erase(column + firstPart.length());
        
        std::string nextLine;
        while (std::getline(iss, nextLine))
        {
            lines_.insert(lines_.begin() + line + 1, nextLine);
            ++line;
        }
        
        if (!remainderOfCurrentLine.empty())
        {
            lines_[line] += remainderOfCurrentLine;
        }
    }
    else
    {
        lines_[line].insert(column, text);
    }
    
    markModified();
}

void Document::deleteText(size_t line, size_t column, size_t length)
{
    TRACE_FUNC
    if (line >= lines_.size())
    {
        return;
    }

    if (column >= lines_[line].length())
    {
        return;
    }
    
    size_t availableLength = lines_[line].length() - column;
    size_t deleteLength = std::min(length, availableLength);
    
    lines_[line].erase(column, deleteLength);
    markModified();
}

void Document::insertLine(size_t line, const std::string& content)
{
    TRACE_FUNC
    if (line > lines_.size())
    {
        line = lines_.size();
    }
    
    lines_.insert(lines_.begin() + line, content);
    markModified();
}

void Document::deleteLine(size_t line)
{
    TRACE_FUNC
    if (line < lines_.size() && lines_.size() > 1)
    {
        lines_.erase(lines_.begin() + line);
        markModified();
    }
}

void Document::clear()
{
    TRACE_FUNC
    lines_.clear();
    lines_.emplace_back();
    filePath_.clear();
    markModified();
}

void Document::markModified()
{
    TRACE_FUNC
    isModified_ = true;
    if (changeCallback_)
    {
        changeCallback_();
    }
}

void Document::ensureLineExists(size_t line)
{
    TRACE_FUNC
    while (lines_.size() <= line)
    {
        lines_.emplace_back();
    }
}
