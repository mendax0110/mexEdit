#include "utils/FileUtils.h"
#include "utils/MemoryDebugger.h"
#include "utils/Logger.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace mexedit::utils;

FileUtils::FileUtils()
{
    TRACE_FUNC
    TRACK_MEMORY(FileUtils, this);
}

FileUtils::~FileUtils()
{
    TRACE_FUNC
    UNTRACK_MEMORY(FileUtils, this);
}

bool FileUtils::exists(const std::filesystem::path& path)
{
    TRACE_FUNC
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}

bool FileUtils::isReadable(const std::filesystem::path& path)
{
    TRACE_FUNC
    std::ifstream file(path);
    return file.good();
}

bool FileUtils::isWritable(const std::filesystem::path& path)
{
    TRACE_FUNC
    if (exists(path))
    {
        std::ofstream file(path, std::ios::app);
        return file.good();
    }
    else
    {
        // Check if parent directory is writable
        auto parent = path.parent_path();
        if (parent.empty()) parent = ".";
        return exists(parent) && isDirectory(parent);
    }
}

bool FileUtils::isDirectory(const std::filesystem::path& path)
{
    TRACE_FUNC
    std::error_code ec;
    return std::filesystem::is_directory(path, ec);
}

std::string FileUtils::getExtension(const std::filesystem::path& path)
{
    TRACE_FUNC
    return path.extension().string();
}

std::string FileUtils::getBasename(const std::filesystem::path& path)
{
    TRACE_FUNC
    return path.filename().string();
}

std::filesystem::path FileUtils::getDirectory(const std::filesystem::path& path)
{
    TRACE_FUNC
    return path.parent_path();
}

std::vector<std::string> FileUtils::readLines(const std::filesystem::path& path)
{
    TRACE_FUNC
    std::vector<std::string> lines;
    std::ifstream file(path);
    
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + path.string());
    }
    
    std::string line;
    while (std::getline(file, line))
    {
        lines.push_back(line);
    }
    
    return lines;
}

bool FileUtils::writeLines(const std::filesystem::path& path, const std::vector<std::string>& lines)
{
    TRACE_FUNC
    try
    {
        std::ofstream file(path);
        if (!file.is_open())
        {
            return false;
        }
        
        for (size_t i = 0; i < lines.size(); ++i)
        {
            file << lines[i];
            if (i < lines.size() - 1)
            {
                file << '\n';
            }
        }
        
        return file.good();
    }
    catch (const std::exception& error)
    {
        LOG_ERR("Error writing lines to file '" << path.string() << "': " << error.what());
        return false;
    }
}

std::string FileUtils::readFile(const std::filesystem::path& path)
{
    TRACE_FUNC
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + path.string());
    }
    
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

bool FileUtils::writeFile(const std::filesystem::path& path, const std::string& content)
{
    TRACE_FUNC
    try 
    {
        std::ofstream file(path);
        if (!file.is_open()) 
        {
            return false;
        }
        
        file << content;
        return file.good();
    }
    catch (const std::exception& error)
    {
        LOG_ERR("Error writing to file '" << path.string() << "': " << error.what());
        return false;
    }
}

std::vector<std::filesystem::directory_entry> FileUtils::listDirectory(const std::filesystem::path& path)
{
    TRACE_FUNC
    std::vector<std::filesystem::directory_entry> entries;

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(
                path, std::filesystem::directory_options::skip_permission_denied))
        {
            entries.push_back(entry);
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        LOG_ERR("Filesystem error listing directory '" << path.string() << "': " << e.what());
    }
    catch (const std::exception& e)
    {
        LOG_ERR("Unexpected error listing directory '" << path.string() << "': " << e.what());
    }

    return entries;
}

bool FileUtils::createDirectory(const std::filesystem::path& path)
{
    TRACE_FUNC
    std::error_code ec;
    return std::filesystem::create_directories(path, ec);
}
