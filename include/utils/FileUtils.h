#ifndef MEXEDIT_FILE_UTILS_H
#define MEXEDIT_FILE_UTILS_H

#include <string>
#include <filesystem>
#include <vector>

/// @brief Utilities namespace for mexEdit \namespace mexedit::utils
namespace mexedit::utils
{
    /// @brief Utility class for file operations \class FileUtils
    class FileUtils
    {
    public:
        /**
         * @brief Check if a file or directory exists
         * 
         * @param path The path to check
         * @return true If the file or directory exists
         * @return false If the file or directory does not exist
         */
        static bool exists(const std::filesystem::path& path);

        /**
         * @brief Check if a file is readable
         * 
         * @param path The path to check
         * @return true If the file is readable
         * @return false If the file is not readable
         */
        static bool isReadable(const std::filesystem::path& path);
        
        /**
         * @brief Check if a file is writable
         * 
         * @param path The path to check
         * @return true If the file is writable
         * @return false If the file is not writable
         */
        static bool isWritable(const std::filesystem::path& path);
        
        /**
         * @brief Check if a path is a directory
         * 
         * @param path The path to check
         * @return true If the path is a directory
         * @return false If the path is not a directory
         */
        static bool isDirectory(const std::filesystem::path& path);

        /**
         * @brief Get the file extension
         * 
         * @param path The path to check
         * @return The file extension
         */
        static std::string getExtension(const std::filesystem::path& path);
        
        /**
         * @brief Get the basename of the file
         * 
         * @param path The path to check
         * @return The file basename
         */
        static std::string getBasename(const std::filesystem::path& path);
        
        /**
         * @brief Get the directory of the file
         * 
         * @param path The path to check
         * @return The file directory
         */
        static std::filesystem::path getDirectory(const std::filesystem::path& path);

        /**
         * @brief Read the lines of a text file
         * 
         * @param path The path to the file
         * @return A vector of strings containing the lines of the file
         */
        static std::vector<std::string> readLines(const std::filesystem::path& path);
        
        /**
         * @brief Write lines to a text file
         * 
         * @param path The path to the file
         * @param lines The lines to write
         * @return true If the write operation was successful
         * @return false If the write operation failed
         */
        static bool writeLines(const std::filesystem::path& path, const std::vector<std::string>& lines);
        
        /**
         * @brief Read the entire content of a text file
         * 
         * @param path The path to the file
         * @return The content of the file as a string
         */
        static std::string readFile(const std::filesystem::path& path);
        
        /**
         * @brief Write content to a text file
         * 
         * @param path The path to the file
         * @param content The content to write
         * @return true If the write operation was successful
         * @return false If the write operation failed
         */
        static bool writeFile(const std::filesystem::path& path, const std::string& content);

        /**
         * @brief List the entries in a directory
         * 
         * @param path The path to the directory
         * @return A vector of directory entries
         */
        static std::vector<std::filesystem::directory_entry> listDirectory(const std::filesystem::path& path);
        
        /**
         * @brief Create a directory
         * 
         * @param path The path to the directory
         * @return true If the directory was created successfully
         * @return false If the directory could not be created
         */
        static bool createDirectory(const std::filesystem::path& path);

    private:

        /**
         * @brief Private constructor to prevent instantiation
         * 
         */
        FileUtils();

        /**
         * @brief Private destructor
         */
        ~FileUtils();
    };
} // namespace mexedit::utils

#endif // MEXEDIT_FILE_UTILS_H
