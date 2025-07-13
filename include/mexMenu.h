#ifndef MEXEDIT_MEXMENU_H
#define MEXEDIT_MEXMENU_H

#include <vector>
#include <string>
#include <functional>
#include <ncurses.h>

/// @brief MexMenu is a class that provides a menu system for the MexEdit text editor. \class MexMenu
class MexMenu
{
public:

    /**
     * @brief Struct representing a menu item in the MexMenu. \struct MexItem
     */
    struct MexItem
    {
        std::string name;
        std::string shortcut;
        std::string description;
        std::function<void()> action;
    };

    /**
     * @brief Constructs a MexMenu object, initializing the menu system.
     */
    MexMenu();

    /**
     * @brief Shows the help menu with available commands and their descriptions.
     */
    void showHelp();

    /**
     * @brief Displays the main menu with available options.
     */
    void showMainMenu();

    /**
     * @brief Adds a new item to the commandline menu.
     * @param name The name of the menu item.
     * @param shortcut The keyboard shortcut for the menu item.
     * @param description The description of the menu item.
     * @param action The action to be executed when the menu item is selected.
     */
    void addMenuItem(const std::string& name, const std::string& shortcut,
                     const std::string& description, std::function<void()> action);

    /**
     * @brief Draws a centered box on the screen.
     * @param height The height of the box.
     * @param width The width of the box.
     */
    static void drawCenteredBox(int height, int width);

    /**
     * @brief Draws the title of the menu at the top of the screen.
     * @param title The title to be displayed.
     */
    static void drawMenuTitle(const std::string& title);

private:
    std::vector<MexItem> menuItems;
};

#endif //MEXEDIT_MEXMENU_H
