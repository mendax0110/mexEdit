#ifndef MEXEDIT_EDITOR_H
#define MEXEDIT_EDITOR_H

#include "../core/Document.h"
#include "../core/Cursor.h"
#include "../core/CommandManager.h"
#include <memory>
#include <functional>

/// @brief Core namespace for mexEdit \namespace mexedit::core
namespace mexedit::core
{
    /// @brief Main editor class managing document, cursor, and commands \class Editor
    class Editor
    {
    public:
        /// @brief Type alias for document changed callback \typedef DocumentChangedCallback
        using DocumentChangedCallback = std::function<void()>;

        /// @brief Type alias for cursor moved callback \typedef CursorMovedCallback
        using CursorMovedCallback = std::function<void(const Cursor::Position&)>;

        /**
         * @brief Construct a new Editor object
         * 
         * @param document Optional initial document
         */
        explicit Editor(std::shared_ptr<Document> document = nullptr);

        /**
         * @brief Destroy the Editor object
         */
        ~Editor();

        /**
         * @brief Set the document
         * 
         * @param document The document to set
         */
        void setDocument(std::shared_ptr<Document> document);
        
        /**
         * @brief Get the current document
         * 
         * @return std::shared_ptr<Document> 
         */
        [[nodiscard]] std::shared_ptr<Document> getDocument() const { return document_; }
        
        /**
         * @brief Check if a document is loaded
         * 
         * @return true 
         * @return false 
         */
        [[nodiscard]] bool hasDocument() const { return document_ != nullptr; }

        /**
         * @brief Get the cursor
         * 
         * @return const Cursor& 
         */
        [[nodiscard]] const Cursor& getCursor() const { return *cursor_; }
        
        /**
         * @brief Move the cursor to a specific position
         * 
         * @param position The position to move the cursor to
         */
        void moveCursor(const Cursor::Position& position);
        
        /**
         * @brief Move the cursor up by a certain number of lines
         * 
         * @param lines The number of lines to move up 
         */
        void moveCursorUp(size_t lines = 1);

        /**
         * @brief Move the cursor down by a certain number of lines
         * 
         * @param lines The number of lines to move down 
         */
        void moveCursorDown(size_t lines = 1);
        
        /**
         * @brief Move the cursor left by a certain number of columns
         * 
         * @param columns The number of columns to move left 
         */
        void moveCursorLeft(size_t columns = 1);

        /**
         * @brief Move the cursor right by a certain number of columns
         * 
         * @param columns The number of columns to move right 
         */
        void moveCursorRight(size_t columns = 1);

        /**
         * @brief Move the cursor to the start of the current line
         * 
         */
        void moveCursorToLineStart();

        /**
         * @brief Move the cursor to the end of the current line
         * 
         */
        void moveCursorToLineEnd();

        /**
         * @brief Move the cursor to the start of the document
         * 
         */
        void moveCursorToDocumentStart();
        
        /**
         * @brief Move the cursor to the end of the document
         * 
         */
        void moveCursorToDocumentEnd();

        /**
         * @brief Insert text at the current cursor position
         * 
         * @param text The text to insert
         */
        void insertText(const std::string& text);

        /**
         * @brief Insert a single character at the current cursor position
         * 
         * @param ch The character to insert
         */
        void insertCharacter(char ch);
        
        /**
         * @brief Insert a new line at the current cursor position
         * 
         */
        void insertNewLine();
        
        /**
         * @brief Delete a character at the current cursor position
         * 
         */
        void deleteCharacter();
        
        /**
         * @brief Delete a character before the current cursor position
         * 
         */
        void deleteBackward();
        
        /**
         * @brief Delete the current line
         * 
         */
        void deleteLine();

        /**
         * @brief Open a file
         * 
         * @param path The path to the file
         * @return true if the file was opened successfully
         * @return false if the file could not be opened
         */
        bool openFile(const std::filesystem::path& path);
        
        /**
         * @brief Save the current document to a file
         * 
         * @param path The path to save the file to. If empty, saves to current document path.
         * @return true if the file was saved successfully
         * @return false if the file could not be saved
         */
        bool saveFile(const std::filesystem::path& path = {});
        
        /**
         * @brief Check if the current document has unsaved changes
         * 
         * @return true if the document has unsaved changes
         * @return false if the document is saved
         */
        [[nodiscard]] bool isModified() const;

        /**
         * @brief Undo the last action
         * 
         * @return true if the action was undone successfully
         * @return false if there are no actions to undo
         */
        bool undo();
        
        /**
         * @brief Redo the last undone action
         * 
         * @return true if the action was redone successfully
         * @return false if there are no actions to redo
         */
        bool redo();
        
        /**
         * @brief Check if there are actions to undo
         * 
         * @return true 
         * @return false 
         */
        [[nodiscard]] bool canUndo() const;
        
        /**
         * @brief Check if there are actions to redo
         * 
         * @return true 
         * @return false 
         */
        [[nodiscard]] bool canRedo() const;

        /**
         * @brief Set the document changed callback
         * 
         * @param callback The callback function
         */
        void setDocumentChangedCallback(DocumentChangedCallback callback);
        
        /**
         * @brief Set the cursor moved callback
         * 
         * @param callback The callback function
         */
        void setCursorMovedCallback(CursorMovedCallback callback);

    private:
        std::shared_ptr<Document> document_;
        std::shared_ptr<Cursor> cursor_;
        std::unique_ptr<CommandManager> commandManager_;

        DocumentChangedCallback documentChangedCallback_;
        CursorMovedCallback cursorMovedCallback_;

        /**
         * @brief Setup the cursor validator
         * 
         */
        void setupCursorValidator();

        /**
         * @brief Notify that the document has changed
         * 
         */
        void notifyDocumentChanged();

        /**
         * @brief Notify that the cursor has moved
         * 
         */
        void notifyCursorMoved();

        /**
         * @brief Validate the cursor position within the document bounds
         * 
         * @param position The position to validate
         * @return Cursor::Position The validated position
         */
        [[nodiscard]] Cursor::Position validateCursorPosition(const Cursor::Position& position) const;
    };
} // namespace mexedit::core

#endif // MEXEDIT_EDITOR_H
