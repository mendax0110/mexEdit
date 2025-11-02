#ifndef MEXEDIT_EDIT_COMMANDS_H
#define MEXEDIT_EDIT_COMMANDS_H

#include "../core/CommandManager.h"
#include "../core/Document.h"
#include "../core/Cursor.h"
#include <memory>

/// @brief Core namespace for mexEdit \namespace mexedit::core
namespace mexedit::core
{
    /// @brief Command for inserting text, implements ICommand \class InsertTextCommand
    class InsertTextCommand : public ICommand
    {
    public:
        /**
         * @brief Construct a new Insert Text Command object
         * 
         * @param document The document to modify
         * @param cursor The cursor to update
         * @param position The position to insert text at
         * @param text The text to insert
         */
        InsertTextCommand(std::shared_ptr<Document> document, std::shared_ptr<Cursor> cursor,
                        const Cursor::Position& position, const std::string& text);

        /**
         * @brief Execute the command
         * 
         */
        void execute() override;

        /**
         * @brief Undo the command
         * 
         */
        void undo() override;
        
        /**
         * @brief Get a description of the command
         * 
         * @return std::string 
         */
        std::string getDescription() const override;

    private:
        std::shared_ptr<Document> document_;
        std::shared_ptr<Cursor> cursor_;
        Cursor::Position position_;
        std::string text_;
        bool executed_;
    };

    /// @brief Command for deleting text, implements ICommand \class DeleteTextCommand
    class DeleteTextCommand : public ICommand
    {
    public:
        /**
         * @brief Construct a new Delete Text Command object
         * 
         * @param document The document to modify
         * @param cursor The cursor to update
         * @param position The position to delete text from
         * @param length The number of characters to delete
         */
        DeleteTextCommand(std::shared_ptr<Document> document, std::shared_ptr<Cursor> cursor,
                        const Cursor::Position& position, size_t length = 1);

        /**
         * @brief Execute the command
         * 
         */
        void execute() override;
        
        /**
         * @brief Undo the command
         * 
         */
        void undo() override;
        
        /**
         * @brief Get a description of the command
         * 
         * @return std::string 
         */
        std::string getDescription() const override;

    private:
        std::shared_ptr<Document> document_;
        std::shared_ptr<Cursor> cursor_;
        Cursor::Position position_;
        size_t length_;
        std::string deletedText_;
        bool executed_;
    };

    /// @brief Command for inserting a line, implements ICommand \class InsertLineCommand
    class InsertLineCommand : public ICommand
    {
    public:
        /**
         * @brief Construct a new Insert Line Command object
         * 
         * @param document The document to modify
         * @param cursor The cursor to update
         * @param linePosition The line position to insert the new line at
         * @param content The content of the new line
         */
        InsertLineCommand(std::shared_ptr<Document> document, std::shared_ptr<Cursor> cursor,
                        size_t linePosition, const std::string& content = "");
        
        /**
         * @brief Execute the command
         * 
         */
        void execute() override;
        
        /**
         * @brief Undo the command
         * 
         */
        void undo() override;
        
        /**
         * @brief Get a description of the command
         * 
         * @return std::string 
         */
        std::string getDescription() const override;

    private:
        std::shared_ptr<Document> document_;
        std::shared_ptr<Cursor> cursor_;
        size_t linePosition_;
        std::string content_;
        bool executed_;
    };

    /// @brief Command for deleting a line, implements ICommand \class DeleteLineCommand
    class DeleteLineCommand : public ICommand
    {
    public:
        /**
         * @brief Construct a new Delete Line Command object
         * 
         * @param document The document to modify
         * @param cursor The cursor to update
         * @param linePosition The line position to delete
         */
        DeleteLineCommand(std::shared_ptr<Document> document, std::shared_ptr<Cursor> cursor,
                        size_t linePosition);

        /**
         * @brief Execute the command
         * 
         */
        void execute() override;

        /**
         * @brief Undo the command
         * 
         */
        void undo() override;
        
        /**
         * @brief Get a description of the command
         * 
         * @return std::string 
         */
        std::string getDescription() const override;

    private:
        std::shared_ptr<Document> document_;
        std::shared_ptr<Cursor> cursor_;
        size_t linePosition_;
        std::string deletedContent_;
        bool executed_;
    };
} // namespace mexedit::core

#endif // MEXEDIT_EDIT_COMMANDS_H
