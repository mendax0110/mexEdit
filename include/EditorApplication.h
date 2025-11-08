#ifndef MEXEDIT_EDITOR_APPLICATION_H
#define MEXEDIT_EDITOR_APPLICATION_H

#include "core/Editor.h"
#include "ui/Renderer.h"
#include "features/SyntaxHighlighter.h"
#include "features/SearchEngine.h"
#include "features/FileExplorer.h"
#include <memory>
#include <string>
#include <filesystem>

/// @brief Main namespace for mexEdit \namespace mexedit
namespace mexedit
{
    /// @brief Main application class for the text editor \class EditorApplication
    class EditorApplication
    {
    public:
        /**
         * @brief Construct a new Editor Application object
         * 
         * @param renderer The renderer to use for UI
         */
        explicit EditorApplication(std::unique_ptr<ui::IRenderer> renderer);
        
        /**
         * @brief Destroy the Editor Application object
         * 
         */
        ~EditorApplication();

        /**
         * @brief Initialize the Editor Application
         * 
         */
        void initialize();
        
        /**
         * @brief Run the main application loop
         * 
         */
        void run();
        
        /**
         * @brief Shutdown the Editor Application
         * 
         */
        void shutdown();

        /**
         * @brief Open a file
         * 
         * @param path The path to the file
         * @return true If the file was opened successfully
         * @return false If the file could not be opened
         */
        bool openFile(const std::filesystem::path& path);

        /**
         * @brief Open the search dialog
         *
         * @param pattern The initial search pattern
         * @param newSearch Whether this is a new search or continuing an existing one
         * @return true If the search was performed successfully
         * @return false If the search failed or was canceled
         */
        bool openSearchDialog(const std::string& pattern = "", bool newSearch = true);
        
        /**
         * @brief Save the current file
         * 
         * @param path The path to save the file to. If empty, saves to current file path.
         * @return true If the file was saved successfully
         * @return false If the file could not be saved
         */
        bool saveFile(const std::filesystem::path& path = {});
        
        /**
         * @brief Save the current file as a new file
         * 
         * @param path The path to save the file to
         * @return true If the file was saved successfully
         * @return false If the file could not be saved
         */
        bool saveFileAs(const std::filesystem::path& path);
        
        /**
         * @brief Create a new file
         * 
         */
        void newFile();

        /**
         * @brief Check if the current document has unsaved changes
         * 
         * @return true If there are unsaved changes
         * @return false If there are no unsaved changes
         */
        [[nodiscard]] bool hasUnsavedChanges() const;
        
        /**
         * @brief Get the current file name
         * 
         * @return std::string The current file name
         */
        [[nodiscard]] std::string getCurrentFileName() const;

    private:
        // Core components
        std::unique_ptr<core::Editor> editor_;
        std::unique_ptr<ui::IRenderer> renderer_;
        std::unique_ptr<features::SyntaxHighlighter> syntaxHighlighter_;
        std::unique_ptr<features::SearchEngine> searchEngine_;
        std::unique_ptr<features::FileExplorer> fileExplorer_;

        // Application state
        bool running_;
        bool showLineNumbers_;
        bool showFileExplorer_;
        std::string statusMessage_;

        bool searchActive_ = false;
        std::string currentSearchPattern_;
        size_t lastSearchLine = 0;
        size_t lastSearchColumn = 0;

        /// @brief Viewport configuration \struct ViewPort
        struct ViewPort
        {
            int fileExplorerWidth = 30;
            int editorStartX;
            int editorWidth;
            int maxVisibleLines;
            int scrollOffsetY = 0;
            int scrollOffsetX = 0;
            int fileExplorerScrollOffset = 0;
        } viewport_;
        
        /// @brief Dirty region tracking for efficient redraws \struct DirtyRegions
        struct DirtyRegions
        {
            bool fullRedraw = true;
            bool editorArea = false;
            bool fileExplorer = false;
            bool statusBar = false;
            bool cursorOnly = false;
        } dirtyRegions_;
        
        /// @brief Text selection state \struct SelectionState
        struct SelectionState
        {
            bool isActive = false;
            core::Cursor::Position startPos = {0, 0};
            core::Cursor::Position endPos = {0, 0};
        } selection_;

        /**
         * @brief Handle document changes
         * 
         */
        void onDocumentChanged();

        /**
         * @brief Handle cursor movement
         * 
         * @param position The new cursor position
         */
        void onCursorMoved(const core::Cursor::Position& position);
        
        /**
         * @brief Handle key press events
         * 
         * @param key The key code of the pressed key
         */
        void onKeyPressed(int key);

        /**
         * @brief Render the editor UI
         * 
         */
        void render();
        
        /**
         * @brief Render the file explorer panel
         * 
         */
        void renderFileExplorer();
        
        /**
         * @brief Render the status bar
         * 
         */
        void renderEditor();

        /**
         * @brief Render the status bar
         * 
         */
        void renderStatusBar();

        /**
         * @brief Handle movement key events
         * 
         * @param key The key code of the pressed key
         * @param isShiftArrow Whether this is a shift+arrow key for selection
         */
        void handleMovementKey(int key, bool isShiftArrow = false);
        
        /**
         * @brief Handle editing key events
         * 
         * @param key The key code of the pressed key
         */
        void handleEditingKey(int key);
        
        /**
         * @brief Handle function key events
         * 
         * @param key The key code of the pressed key
         */
        void handleFunctionKey(int key);
        
        /**
         * @brief Handle control key events
         * 
         * @param key The key code of the pressed key
         */
        void handleControlKey(int key);

        /**
         * @brief Update the viewport dimensions
         * 
         */
        void updateViewport();
        
        /**
         * @brief Scroll the viewport to ensure the cursor is visible
         * 
         */
        void scrollToEnsureCursorVisible();

        void scrollFileExplorerIntoView();
        
        /**
         * @brief Prompt the user to save changes if there are unsaved changes
         * 
         * @return true If the user chose to save or discard changes
         * @return false If the user canceled the operation
         */
        bool promptSaveChanges();
        
        /**
         * @brief Show a status message in the status bar
         * 
         * @param message The message to display
         */
        void showStatusMessage(const std::string& message);

        /**
         * @brief Show command help dialog
         * 
         */
        void showCommandHelp();

        /**
         * @brief Handle command input
         * 
         * @param command The command to execute
         */
        void handleCommand(const std::string& command);

        /**
         * @brief Mark specific regions as dirty for selective redrawing
         * 
         * @param editor Mark editor area as dirty
         * @param explorer Mark file explorer as dirty
         * @param status Mark status bar as dirty
         * @param cursor Mark only cursor position as dirty
         */
        void markDirty(bool editor = false, bool explorer = false, bool status = false, bool cursor = false);
        
        /**
         * @brief Clear all dirty region flags
         * 
         */
        void clearDirtyRegions();

        /**
         * @brief Perform rendering updates based on dirty regions
         */
        void doRenderUpdateBar();
        
        /**
         * @brief Start text selection at current cursor position
         * 
         */
        void startSelection();
        
        /**
         * @brief End text selection
         * 
         */
        void endSelection();
        
        /**
         * @brief Clear current selection
         * 
         */
        void clearSelection();
        
        /**
         * @brief Check if a position is within the current selection
         * 
         * @param line The line number
         * @param column The column number
         * @return true if position is selected
         */
        bool isPositionSelected(size_t line, size_t column) const;
        
        /**
         * @brief Copy selected text to clipboard (if selection exists)
         * 
         * @return std::string The selected text
         */
        [[nodiscard]] std::string copySelection() const;
        
        /**
         * @brief Delete selected text
         * 
         */
        void deleteSelection();
        
        /**
         * @brief Get the current file name
         * 
         * @param c The character to convert
         * @return constexpr int 
         */
        static constexpr int CTRL_KEY(char c) { return c & 0x1f; }
    };
} // namespace mexedit

#endif // MEXEDIT_EDITOR_APPLICATION_H
