#ifndef MEXEDIT_CLIPBOARD_H
#define MEXEDIT_CLIPBOARD_H

#include <string>

/// @brief Utilities namespace for mexEdit \namespace mexedit::utils
namespace mexedit::utils
{
    /// @brief Simple clipboard functionality \class Clipboard
    class Clipboard
    {
    public:
        /**
         * @brief Set the Text object
         * 
         * @param text The text to set in the clipboard
         */
        static void setText(const std::string& text);

        /**
         * @brief Get the Text object
         * 
         * @return std::string The text from the clipboard
         */
        static std::string getText();

        /**
         * @brief Check if clipboard has content
         * 
         * @return true if clipboard has content, false otherwise
         */
        static bool hasText();
        
        /**
         * @brief Clear clipboard
         * 
         */
        static void clear();
        
    private:
        static std::string clipboardContent_;

        /**
         * @brief Private constructor to prevent instantiation
         */
        Clipboard();

        /**
         * @brief Private destructor
         */
        ~Clipboard();
    };
}

#endif // MEXEDIT_CLIPBOARD_H
