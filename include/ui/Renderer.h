#ifndef MEXEDIT_RENDERER_H
#define MEXEDIT_RENDERER_H

#include "../core/Cursor.h"
#include <string>
#include <vector>
#include <termios.h>

/// @brief UI namespace for mexEdit \namespace mexedit::ui
namespace mexedit::ui
{
    /// @brief Screen size structure \struct ScreenSize
    struct ScreenSize
    {
        int width;
        int height;
    };

    /// @brief Color pair enumeration for text rendering \enum ColorPair
    enum class ColorPair
    {
        Normal = 1,
        StatusBar = 2,
        StatusBarActive = 3,
        FileExplorer = 4,
        FileExplorerSelected = 5,
        FileExplorerDirectory = 6,
        LineNumbers = 7,
        LineNumbersActive = 8,
        Syntax_Keyword = 9,
        Syntax_String = 10,
        Syntax_Comment = 11,
        Syntax_Number = 12,
        Syntax_Operator = 13,
        Syntax_Type = 14,
        Search_Highlight = 15,
        Selection = 16,
        CursorLine = 17,
        Border = 18,
        Title = 19,
        Error = 20,
        Warning = 21,
        Success = 22
    };

    /// @brief Interface for a renderer \class IRenderer
    class IRenderer
    {
    public:
        /**
         * @brief Destroy the IRenderer object
         */
        virtual ~IRenderer() = default;

        /**
         * @brief Initialize the renderer
         */
        virtual void initialize() = 0;

        /**
         * @brief Shutdown the renderer
         */
        virtual void shutdown() = 0;

        /**
         * @brief Clear the screen
         */
        virtual void clear() = 0;

        /**
         * @brief Refresh the screen
         */
        virtual void refresh() = 0;
        
        /**
         * @brief Clear a specific area of the screen
         * 
         * @param y The y-coordinate of the top-left corner
         * @param x The x-coordinate of the top-left corner
         * @param height The height of the area to clear
         * @param width The width of the area to clear
         */
        virtual void clearArea(int y, int x, int height, int width) = 0;
        
        /**
         * @brief Get the current screen size
         * 
         * @return ScreenSize 
         */
        virtual ScreenSize getScreenSize() const = 0;

        /**
         * @brief Draw text on the screen
         * 
         * @param y The y-coordinate
         * @param x The x-coordinate
         * @param text The text to draw
         */
        virtual void drawText(int y, int x, const std::string& text) = 0;
        
        /**
         * @brief Draw colored text on the screen
         * 
         * @param y The y-coordinate
         * @param x The x-coordinate
         * @param text The text to draw
         * @param color The color pair to use
         */
        virtual void drawText(int y, int x, const std::string& text, ColorPair color) = 0;
        
        /**
         * @brief Draw text with attributes on the screen
         * 
         * @param y The y-coordinate
         * @param x The x-coordinate
         * @param text The text to draw
         * @param attributes The text attributes (e.g., bold, underline)
         */
        virtual void drawTextWithAttributes(int y, int x, const std::string& text, int attributes) = 0;

        /**
         * @brief Draw a horizontal line on the screen
         * 
         * @param y The y-coordinate
         * @param startX The starting x-coordinate
         * @param endX The ending x-coordinate
         */
        virtual void drawHorizontalLine(int y, int startX, int endX) = 0;
        
        /**
         * @brief Draw a vertical line on the screen
         * 
         * @param x The x-coordinate
         * @param startY The starting y-coordinate
         * @param endY The ending y-coordinate
         */
        virtual void drawVerticalLine(int x, int startY, int endY) = 0;
        
        /**
         * @brief Draw a box on the screen
         * 
         * @param y The y-coordinate of the top-left corner
         * @param x The x-coordinate of the top-left corner
         * @param height The height of the box
         * @param width The width of the box
         */
        virtual void drawBox(int y, int x, int height, int width) = 0;

        /**
         * @brief Set the cursor position on the screen
         * 
         * @param pos The cursor position
         */
        virtual void setCursorPosition(const core::Cursor::Position& pos) = 0;
        
        /**
         * @brief Set the cursor visibility
         * 
         * @param visible true to show the cursor, false to hide it
         */
        virtual void setCursorVisible(bool visible) = 0;

        /**
         * @brief Get input from the user
         * 
         * @return int The input character or key code
         */
        virtual int getInput() = 0;

        /**
         * @brief Draw a modern-style box with rounded corners (if supported)
         * 
         * @param y The y-coordinate of the top-left corner
         * @param x The x-coordinate of the top-left corner
         * @param height The height of the box
         * @param width The width of the box
         * @param color The color pair to use for the box
         */
        virtual void drawModernBox(int y, int x, int height, int width, ColorPair color = ColorPair::Border) = 0;

        /**
         * @brief Draw a title bar with centered text
         * 
         * @param y The y-coordinate of the title bar
         * @param width The width of the title bar
         * @param title The title text
         * @param color The color pair to use
         */
        virtual void drawTitleBar(int y, int width, const std::string& title, ColorPair color = ColorPair::Title) = 0;

        /**
         * @brief Draw a progress bar
         * 
         * @param y The y-coordinate
         * @param x The x-coordinate
         * @param width The width of the progress bar
         * @param progress The progress value (0.0 to 1.0)
         * @param color The color pair to use
         */
        virtual void drawProgressBar(int y, int x, int width, float progress, ColorPair color = ColorPair::StatusBarActive) = 0;
    };

    /// @brief NCurses-based renderer implementation \class NCursesRenderer
    class NCursesRenderer : public IRenderer
    {
    public:
        /**
         * @brief Construct a new NCurses Renderer object
         * 
         */
        NCursesRenderer();
        
        /**
         * @brief Destroy the NCurses Renderer object
         * 
         */
        ~NCursesRenderer() override;

        /**
         * @brief Initialize the NCurses Renderer
         * 
         */
        void initialize() override;
        
        /**
         * @brief Shutdown the NCurses Renderer
         * 
         */
        void shutdown() override;
        
        /**
         * @brief Clear the screen
         * 
         */
        void clear() override;
        
        /**
         * @brief Refresh the screen
         * 
         */
        void refresh() override;
        
        /**
         * @brief Clear a specific area of the screen
         * 
         * @param y The y-coordinate of the top-left corner
         * @param x The x-coordinate of the top-left corner
         * @param height The height of the area to clear
         * @param width The width of the area to clear
         */
        void clearArea(int y, int x, int height, int width) override;
        
        /**
         * @brief Refresh only a specific area of the screen
         * 
         * @param y The y-coordinate of the top-left corner
         * @param x The x-coordinate of the top-left corner
         * @param height The height of the area to refresh
         * @param width The width of the area to refresh
         */
        void refreshArea(int y, int x, int height, int width);
        
        /**
         * @brief Get the current screen size
         * 
         * @return ScreenSize 
         */
        ScreenSize getScreenSize() const override;

        /**
         * @brief Draw text on the screen
         * 
         * @param y The y-coordinate
         * @param x The x-coordinate
         * @param text The text to draw
         */
        void drawText(int y, int x, const std::string& text) override;

        /**
         * @brief Draw text on the screen with a color
         * 
         * @param y The y-coordinate
         * @param x The x-coordinate
         * @param text The text to draw
         * @param color The color pair to use
         */
        void drawText(int y, int x, const std::string& text, ColorPair color) override;
        
        /**
         * @brief Draw text with attributes on the screen
         * 
         * @param y The y-coordinate
         * @param x The x-coordinate
         * @param text The text to draw
         * @param attributes The text attributes (e.g., bold, underline)
         */
        void drawTextWithAttributes(int y, int x, const std::string& text, int attributes) override;

        /**
         * @brief Draw a horizontal line on the screen
         * 
         * @param y The y-coordinate
         * @param startX The starting x-coordinate
         * @param endX The ending x-coordinate
         */
        void drawHorizontalLine(int y, int startX, int endX) override;
        
        /**
         * @brief Draw a vertical line on the screen
         * 
         * @param x The x-coordinate
         * @param startY The starting y-coordinate
         * @param endY The ending y-coordinate
         */
        void drawVerticalLine(int x, int startY, int endY) override;
        
        /**
         * @brief Draw a box on the screen
         * 
         * @param y The y-coordinate of the top-left corner
         * @param x The x-coordinate of the top-left corner
         * @param height The height of the box
         * @param width The width of the box
         */
        void drawBox(int y, int x, int height, int width) override;

        /**
         * @brief Set the cursor position on the screen
         * 
         * @param pos The cursor position
         */
        void setCursorPosition(const core::Cursor::Position& pos) override;
        
        /**
         * @brief Set the cursor visibility
         * 
         * @param visible true to show the cursor, false to hide it
         */
        void setCursorVisible(bool visible) override;

        /**
         * @brief Get input from the user
         * 
         * @return int The input character or key code
         */
        int getInput() override;

        /**
         * @brief Draw a modern-style box with rounded corners (if supported)
         * 
         * @param y The y-coordinate of the top-left corner
         * @param x The x-coordinate of the top-left corner
         * @param height The height of the box
         * @param width The width of the box
         * @param color The color pair to use for the box
         */
        void drawModernBox(int y, int x, int height, int width, ColorPair color = ColorPair::Border) override;

        /**
         * @brief Draw a title bar with centered text
         * 
         * @param y The y-coordinate of the title bar
         * @param width The width of the title bar
         * @param title The title text
         * @param color The color pair to use
         */
        void drawTitleBar(int y, int width, const std::string& title, ColorPair color = ColorPair::Title) override;

        /**
         * @brief Draw a progress bar
         * 
         * @param y The y-coordinate
         * @param x The x-coordinate
         * @param width The width of the progress bar
         * @param progress The progress value (0.0 to 1.0)
         * @param color The color pair to use
         */
        void drawProgressBar(int y, int x, int width, float progress, ColorPair color = ColorPair::StatusBarActive) override;

    private:
        bool initialized_;
        struct termios originalTermios_;
        
        /**
         * @brief Setup color pairs for rendering
         * 
         */
        void setupColors();
        
        /**
         * @brief Configure terminal attributes for proper key handling
         * 
         */
        void configureTerminal();
        
        /**
         * @brief Restore original terminal attributes
         * 
         */
        void restoreTerminal();
    };
} // namespace mexedit::ui

#endif // MEXEDIT_RENDERER_H
