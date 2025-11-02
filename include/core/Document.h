#ifndef MEXEDIT_DOCUMENT_H
#define MEXEDIT_DOCUMENT_H

#include <vector>
#include <string>
#include <filesystem>
#include <functional>

/// @brief Core namespace for mexEdit \namespace mexedit::core
namespace mexedit::core
{
    /// @brief Represents a text document \class Document
    class Document
    {
    public:
        /// @brief Type alias for a change callback function \typedef ChangeCallback
        using ChangeCallback = std::function<void()>;
        
        /**
         * @brief Construct a new Document object
         * 
         */
        Document();
        
        /**
         * @brief Construct a new Document object and load from file
         * 
         * @param path The file path to load the document from
         */
        explicit Document(const std::filesystem::path& path);
        
        /**
         * @brief Destroy the Document object
         * 
         */
        ~Document() = default;

        /**
         * @brief Load the document from a file
         * 
         * @param path The file path to load the document from
         * @return true if successful, false otherwise
         */
        bool load(const std::filesystem::path& path);
        
        /**
         * @brief Save the document to a file
         * 
         * @param path The file path to save the document to. If empty, saves to current file path.
         * @return true if successful, false otherwise
         */
        bool save(const std::filesystem::path& path = {});
        
        /**
         * @brief Check if the document has unsaved changes
         * 
         * @return true if the document is modified, false otherwise
         */
        bool isModified() const { return isModified_; }
        
        /**
         * @brief Get the current file path of the document
         * 
         * @return const std::filesystem::path& The file path
         */
        const std::filesystem::path& getFilePath() const { return filePath_; }

        /**
         * @brief Get the Line object
         * 
         * @param line The line number
         * @return const std::string& 
         */
        const std::string& getLine(size_t line) const;
        
        /**
         * @brief Get the entire text of the document
         * 
         * @return std::string 
         */
        std::string getText() const;
        
        /**
         * @brief Get the number of lines in the document
         * 
         * @return size_t 
         */
        size_t getLineCount() const { return lines_.size(); }
        
        /**
         * @brief Get the length of a specific line
         * 
         * @param line The line number
         * @return size_t 
         */
        size_t getLineLength(size_t line) const;
        
        /**
         * @brief Check if the document is empty
         * 
         * @return true 
         * @return false 
         */
        bool isEmpty() const { return lines_.empty() || (lines_.size() == 1 && lines_[0].empty()); }

        /**
         * @brief Insert text at a specific position
         * 
         * @param line The line number
         * @param column The column number
         * @param text The text to insert
         */
        void insertText(size_t line, size_t column, const std::string& text);
        
        /**
         * @brief Delete text from a specific position
         * 
         * @param line The line number
         * @param column The column number
         * @param length The number of characters to delete
         */
        void deleteText(size_t line, size_t column, size_t length = 1);
        
        /**
         * @brief Insert a new line at a specific position
         * 
         * @param line The line number
         * @param content The content of the new line
         */
        void insertLine(size_t line, const std::string& content = "");
        
        /**
         * @brief Delete a line at a specific position
         * 
         * @param line The line number
         */
        void deleteLine(size_t line);
        
        /**
         * @brief Clear the document content
         * 
         */
        void clear();

        /**
         * @brief Set the callback function to be called on document changes
         * 
         * @param callback The callback function
         */
        void setChangeCallback(ChangeCallback callback) { changeCallback_ = std::move(callback); }

    private:
        std::vector<std::string> lines_;
        std::filesystem::path filePath_;
        bool isModified_;
        ChangeCallback changeCallback_;

        /**
         * @brief Mark the document as modified and trigger change callback
         * 
         */
        void markModified();
        
        /**
         * @brief Ensure the specified line exists in the document
         * 
         * @param line The line number to ensure
         */
        void ensureLineExists(size_t line);
    };
} // namespace mexedit::core

#endif // MEXEDIT_DOCUMENT_H
