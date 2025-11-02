#ifndef MEXEDIT_COMMAND_MANAGER_H
#define MEXEDIT_COMMAND_MANAGER_H

#include <vector>
#include <memory>
#include <string>

/// @brief Core namespace for mexEdit \namespace mexedit::core
namespace mexedit::core
{
    /// @brief Abstract command interface for the Command pattern \class ICommand
    class ICommand
    {
    public:
        /**
         * @brief Destroy the ICommand object
         */
        virtual ~ICommand() = default;

        /**
         * @brief Execute the command
         */
        virtual void execute() = 0;

        /**
         * @brief Undo the command
         */
        virtual void undo() = 0;

        /**
         * @brief Get a description of the command
         */
        virtual std::string getDescription() const = 0;
    };

    /// @brief Type alias for a unique pointer to an ICommand \typedef CommandPtr
    using CommandPtr = std::unique_ptr<ICommand>;

    /// @brief Manages command execution and undo/redo history \class CommandManager
    class CommandManager
    {
    public:
        /**
         * @brief Construct a new Command Manager object
         * 
         * @param maxHistorySize Maximum number of commands to keep in history
         */
        explicit CommandManager(size_t maxHistorySize = 100);
        
        /**
         * @brief Destroy the Command Manager object
         */
        ~CommandManager() = default;

        /**
         * @brief Execute a command and add it to the history
         * 
         * @param command The command to execute
         */
        void executeCommand(CommandPtr command);

        /**
         * @brief Undo the last executed command
         * 
         * @return true if a command was undone, false if no command to undo
         */
        bool undo();
        
        /**
         * @brief Redo the last undone command
         * 
         * @return true if a command was redone, false if no command to redo
         */
        bool redo();

        /**
         * @brief Check if there is a command to undo
         * 
         * @return true if there is a command to undo, false otherwise
         */
        bool canUndo() const;

        /**
         * @brief Check if there is a command to redo
         * 
         * @return true if there is a command to redo, false otherwise
         */
        bool canRedo() const;

        /**
         * @brief Clear the command history
         */
        void clearHistory();
        
        /**
         * @brief Get the current size of the command history
         * 
         * @return size_t Number of commands in history
         */
        size_t getHistorySize() const { return history_.size(); }
        
        /**
         * @brief Set the maximum size of the command history
         * 
         * @param maxSize Maximum number of commands to keep in history
         */
        void setMaxHistorySize(size_t maxSize);

        /**
         * @brief Get description of the next command to undo
         * 
         * @return std::string Description of the command
         */
        std::string getUndoDescription() const;
        
        /**
         * @brief Get description of the next command to redo
         * 
         * @return std::string Description of the command
         */
        std::string getRedoDescription() const;

    private:
        std::vector<CommandPtr> history_;
        size_t currentIndex_;
        size_t maxHistorySize_;

        /**
         * @brief Trim the history to the maximum size
         */
        void trimHistory();
    };
} // namespace mexedit::core

#endif // MEXEDIT_COMMAND_MANAGER_H
