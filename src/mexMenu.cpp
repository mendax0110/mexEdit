#include "../include/mexMenu.h"
#include <algorithm>
#include <utility>

MexMenu::MexMenu() = default;

void MexMenu::addMenuItem(const std::string& name, const std::string& shortcut, const std::string& description, std::function<void()> action)
{
    menuItems.push_back({name, shortcut, description, std::move(action)});
}

void MexMenu::drawCenteredBox(int height, int width)
{
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    int startY = (maxY - height) / 2;
    int startX = (maxX - width) / 2;

    attron(A_BOLD);
    for (int y = startY; y < startY + height; ++y)
    {
        mvhline(y, startX, ' ', width);

        if (y == startY || y == startY + height - 1)
        {
            mvhline(y, startX, ACS_HLINE, width);
        }
        mvvline(y, startX, ACS_VLINE, 1);
        mvvline(y, startX + width - 1, ACS_VLINE, 1);
    }

    mvaddch(startY, startX, ACS_ULCORNER);
    mvaddch(startY, startX + width - 1, ACS_URCORNER);
    mvaddch(startY + height - 1, startX, ACS_LLCORNER);
    mvaddch(startY + height - 1, startX + width - 1, ACS_LRCORNER);
    attroff(A_BOLD);
}

void MexMenu::drawMenuTitle(const std::string& title)
{
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    attron(A_BOLD | A_UNDERLINE);
    mvprintw(0, (maxX - title.length()) / 2, "%s", title.c_str());
    attroff(A_BOLD | A_UNDERLINE);
}

void MexMenu::showHelp()
{
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    const int boxHeight = std::min(50, maxY - 4);
    const int boxWidth = std::min(70, maxX - 4);

    clear();
    drawCenteredBox(boxHeight, boxWidth);
    drawMenuTitle("MexEdit Help");

    int startY = (maxY - boxHeight) / 2 + 2;
    int startX = (maxX - boxWidth) / 2 + 2;

    attron(A_BOLD);
    mvprintw(startY, startX, "%-15s %-15s %s", "Command", "Shortcut", "Description");
    attroff(A_BOLD);

    int line = 1;

    mvprintw(startY + line++, startX, "%-15s %-15s %s", "File Operations:", "", "");
    for (const auto& item : menuItems)
    {
        if (item.name == "Help") continue;
        mvprintw(startY + line++, startX, "%-15s %-15s %s",
                 item.name.c_str(), item.shortcut.c_str(), item.description.c_str());
    }

    line++;
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "Editing:", "", "");
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "Delete Line", "Ctrl+D", "Delete current line");
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "Undo", "Ctrl+Z", "Undo last action");
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "Redo", "Ctrl+Y", "Redo last action");
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "New Line", "Enter", "Insert new line");
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "Backspace", "Backspace", "Delete previous char");
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "Delete", "Delete", "Delete next char");

    line++;
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "Navigation:", "", "");
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "Move Cursor", "Arrow Keys", "Move cursor");
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "Home", "Home", "Move to line start");
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "End", "End", "Move to line end");
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "Page Up", "PgUp", "Move up one page");
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "Page Down", "PgDn", "Move down one page");

    line++;
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "Search/Command:", "", "");
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "Search", "/", "Start search mode");
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "Find Next", "Ctrl+N", "Find next match");
    mvprintw(startY + line++, startX, "%-15s %-15s %s", "Command Mode", ":", "Enter commands");

    line++;
    for (const auto& item : menuItems)
    {
        if (item.name == "Help")
        {
            mvprintw(startY + line++, startX, "%-15s %-15s %s", item.name.c_str(), item.shortcut.c_str(), item.description.c_str());
        }
    }

    attron(A_DIM);
    mvprintw(startY + boxHeight - 4, startX, "Press any key to return to editor...");
    attroff(A_DIM);

    refresh();
    getch();
}

void MexMenu::showMainMenu()
{
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    const int boxHeight = std::min(20, maxY - 4);
    const int boxWidth = std::min(50, maxX - 4);

    clear();
    drawCenteredBox(boxHeight, boxWidth);
    drawMenuTitle("MexEdit Menu");

    int startY = (maxY - boxHeight) / 2 + 3;
    int startX = (maxX - boxWidth) / 2 + 2;

    for (size_t i = 0; i < menuItems.size(); ++i)
    {
        const auto& item = menuItems[i];
        mvprintw(startY + i, startX, "%d. %-10s [%s]", i + 1, item.name.c_str(), item.shortcut.c_str());
    }

    int quickRefLine = startY + menuItems.size() + 1;
    mvprintw(quickRefLine++, startX, "Other commands:");
    mvprintw(quickRefLine++, startX, "  / - Search    : - Commands");
    mvprintw(quickRefLine++, startX, "  Ctrl+N - Find Next");
    mvprintw(quickRefLine++, startX, "  Ctrl+D/Z/Y - Delete/Undo/Redo");
    mvprintw(quickRefLine++, startX, "  Arrows - Navigation");

    attron(A_DIM);
    mvprintw(startY + boxHeight - 4, startX, "Select option (1-%zu) or press ESC to exit...", menuItems.size());
    attroff(A_DIM);

    refresh();

    int choice = getch();
    if (choice >= '1' && choice <= '0' + menuItems.size())
    {
        menuItems[choice - '1'].action();
    }
    else if (choice == '?')
    {
        showHelp();
    }
}