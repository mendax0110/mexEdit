#ifndef MEXEDIT_CURSOR_H
#define MEXEDIT_CURSOR_H

#include <functional>

/// @brief Core namespace for mexEdit \namespace mexedit::core
namespace mexedit::core
{
    /// @brief Represents the cursor position within a document \class Cursor
    class Cursor
    {
    public:
        /**
         * @brief Represents the position of the cursor within a document
         * 
         */
        struct Position
        {
            size_t line = 0;
            size_t column = 0;
            
            /**
             * @brief Inequality operator for Position
             * 
             * @param other The other Position to compare
             * @return true 
             * @return false 
             */
            bool operator==(const Position& other) const
            {
                return line == other.line && column == other.column;
            }
            
            /**
             * @brief Inequality operator for Position
             * 
             * @param other The other Position to compare
             * @return true 
             * @return false 
             */
            bool operator!=(const Position& other) const
            {
                return !(*this == other);
            }
        };

        /// @brief Type alias for a position validator function \typedef PositionValidator
        using PositionValidator = std::function<Position(const Position&)>;

        /// @brief Type alias for a move callback function \typedef MoveCallback
        using MoveCallback = std::function<void(const Position&)>;

        /**
         * @brief Construct a new Cursor object
         * 
         * @param validator Optional position validator function
         */
        explicit Cursor(PositionValidator validator = nullptr);
        
        /**
         * @brief Destroy the Cursor object
         */
        ~Cursor();

        /**
         * @brief Get the Position object
         * 
         * @return const Position& 
         */
        [[nodiscard]] const Position& getPosition() const { return position_; }
        
        /**
         * @brief Set the cursor position
         * 
         * @param pos The new position
         */
        void setPosition(const Position& pos);
        
        /**
         * @brief Set the cursor position
         * 
         * @param line The line number
         * @param column The column number
         */
        void setPosition(size_t line, size_t column) { setPosition({line, column}); }

        /**
         * @brief Move the cursor up by a certain number of lines
         * 
         * @param lines The number of lines to move up
         */
        void moveUp(size_t lines = 1);

        /**
         * @brief Move the cursor down by a certain number of lines
         * 
         * @param lines The number of lines to move down
         */
        void moveDown(size_t lines = 1);
        
        /**
         * @brief Move the cursor left by a certain number of columns
         * 
         * @param columns The number of columns to move left
         */
        void moveLeft(size_t columns = 1);

        /**
         * @brief Move the cursor right by a certain number of columns
         * 
         * @param columns The number of columns to move right
         */
        void moveRight(size_t columns = 1);

        /**
         * @brief Move the cursor to the start of the current line
         * 
         */
        void moveToLineStart();

        /**
         * @brief Move the cursor to the end of the current line
         * 
         */
        void moveToLineEnd();

        /**
         * @brief Move the cursor to the start of the document
         * 
         */
        void moveToDocumentStart();

        /**
         * @brief Move the cursor to the end of the document
         * 
         */
        void moveToDocumentEnd();

        /**
         * @brief Set the position validator function
         * 
         * @param validator The position validator
         */
        void setValidator(PositionValidator validator) { validator_ = std::move(validator); }
        
        /**
         * @brief Set the move callback function
         * 
         * @param callback The move callback
         */
        void setMoveCallback(MoveCallback callback) { moveCallback_ = std::move(callback); }

    private:
        Position position_;
        PositionValidator validator_;
        MoveCallback moveCallback_;

        /**
         * @brief Update the cursor position, applying validation and triggering callbacks
         * 
         * @param newPos The new position to set
         */
        void updatePosition(const Position& newPos);
    };
} // namespace mexedit::core

#endif // MEXEDIT_CURSOR_H
