#ifndef MEXEDIT_FILE_EXPLORER_H
#define MEXEDIT_FILE_EXPLORER_H

#include <filesystem>
#include <vector>
#include <functional>

/// @brief Features namespace for mexEdit \namespace mexedit::features
namespace mexedit::features
{
    /// @brief Represents a file or directory entry in the file explorer \struct FileEntry
    struct FileEntry
    {
        std::filesystem::path path;
        std::string displayName;
        bool isDirectory;
        size_t fileSize;
        std::filesystem::file_time_type lastModified;
    };

/// @brief File explorer feature for browsing the filesystem \class FileExplorer
class FileExplorer
{
public:
    /// @brief Type alias for a file selection callback \typedef SelectionCallback
    using SelectionCallback = std::function<void(const std::filesystem::path&)>;

    /**
     * @brief Construct a new File Explorer object
     * 
     * @param rootPath The initial root path to explore
     */
    explicit FileExplorer(const std::filesystem::path& rootPath = std::filesystem::current_path());
    
    /**
     * @brief Destroy the File Explorer object
     * 
     */
    ~FileExplorer() = default;

    /**
     * @brief Set the Current Directory object
     * 
     * @param path The new current directory path
     */
    void setCurrentDirectory(const std::filesystem::path& path);

    /**
     * @brief Get the Current Directory object
     * 
     * @return const std::filesystem::path& 
     */
    const std::filesystem::path& getCurrentDirectory() const { return currentPath_; }
    
    /**
     * @brief Navigate up to the parent directory
     * 
     * @return true if the navigation was successful
     * @return false if the navigation failed
     */
    bool navigateUp();

    /**
     * @brief Navigate into a subdirectory
     * 
     * @param subPath The subdirectory path to navigate into
     * @return true if the navigation was successful
     * @return false if the navigation failed
     */
    bool navigateInto(const std::filesystem::path& subPath);
    
    /**
     * @brief Refresh the file explorer entries
     * 
     */
    void refresh();

    /**
     * @brief Get the file entries
     * 
     * @return const std::vector<FileEntry>& 
     */
    const std::vector<FileEntry>& getEntries() const { return entries_; }
    
    /**
     * @brief Get the number of entries
     * 
     * @return size_t 
     */
    size_t getEntryCount() const { return entries_.size(); }

    /**
     * @brief Set the selected index
     * 
     * @param index The index to select
     */
    void setSelectedIndex(size_t index);
    
    /**
     * @brief Get the selected index
     * 
     * @return size_t 
     */
    size_t getSelectedIndex() const { return selectedIndex_; }
    
    /**
     * @brief Get the selected entry
     * 
     * @return const FileEntry* 
     */
    const FileEntry* getSelectedEntry() const;
    
    /**
     * @brief Check if there is a selection
     * 
     * @return true 
     * @return false 
     */
    bool hasSelection() const { return selectedIndex_ < entries_.size(); }

    /**
     * @brief Select the next entry
     * 
     */
    void selectNext();
    
    /**
     * @brief Select the previous entry
     * 
     */
    void selectPrevious();
    
    /**
     * @brief Select the first entry
     * 
     */
    void selectFirst();
    
    /**
     * @brief Select the last entry
     * 
     */
    void selectLast();

    /**
     * @brief Set the selection callback
     * 
     * @param callback The callback function
     */
    void setSelectionCallback(SelectionCallback callback) { selectionCallback_ = std::move(callback); }

    /**
     * @brief Set whether to show hidden files
     * 
     * @param show true to show hidden files, false to hide
     */
    void setShowHiddenFiles(bool show);
    
    /**
     * @brief Get whether hidden files are shown
     * 
     * @return true if hidden files are shown
     * @return false if hidden files are hidden
     */
    bool getShowHiddenFiles() const { return showHiddenFiles_; }

    /**
     * @brief Move the selection up by one entry
     * 
     */
    void moveSelectionUp();

    /**
     * @brief Move the selection down by one entry
     * 
     */
    void moveSelectionDown();

private:
    std::filesystem::path currentPath_;
    std::vector<FileEntry> entries_;
    size_t selectedIndex_;
    bool showHiddenFiles_;
    SelectionCallback selectionCallback_;

    /** 
     * @brief Load the file and directory entries from the current path
     * 
     */
    void loadEntries();
    
    /**
     * @brief Sort the file and directory entries
     * 
     */
    void sortEntries();
    
    /**
     * @brief Notify selection change via callback
     * 
     */
    void notifySelection();
    
    /**
     * @brief Format file size as a human-readable string
     * 
     * @param size The file size in bytes
     * @return std::string The formatted file size
     */
    std::string formatFileSize(size_t size) const;
};

} // namespace mexedit::features

#endif // MEXEDIT_FILE_EXPLORER_H
