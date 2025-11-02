#include "../include/ui/Renderer.h"
#include <ncurses.h>
#include <stdexcept>
#include <signal.h>
#include <csignal>
#include <termios.h>
#include <unistd.h>
#include <algorithm>

using namespace mexedit::ui;

NCursesRenderer::NCursesRenderer() : initialized_(false)
{
}

NCursesRenderer::~NCursesRenderer()
{
    if (initialized_)
    {
        shutdown();
    }
}

void NCursesRenderer::initialize()
{
    if (initialized_)
    {
        return;
    }

    // Initialize ncurses
    initscr();
    if (stdscr == nullptr)
    {
        throw std::runtime_error("Failed to initialize ncurses");
    }

    // Configure terminal attributes first
    configureTerminal();
    
    // Configure ncurses for optimal performance
    raw();                   // Enable raw input mode to catch control characters (Ctrl+C, Ctrl+Z, etc.)
    keypad(stdscr, TRUE);    // Enable function keys
    noecho();                // Don't echo keys to screen
    curs_set(1);            // Show cursor
    
    // Critical anti-flicker optimizations
    nodelay(stdscr, FALSE);  // Blocking input (prevents busy waiting)
    scrollok(stdscr, FALSE); // Disable automatic scrolling
    leaveok(stdscr, FALSE);  // Update cursor position after refresh
    immedok(stdscr, FALSE);  // Don't refresh immediately on changes
    
    // Setup colors if available
    if (has_colors())
    {
        start_color();
        setupColors();
    }

    initialized_ = true;
}

void NCursesRenderer::shutdown()
{
    if (initialized_)
    {
        restoreTerminal();
        endwin();
        initialized_ = false;
    }
}

void NCursesRenderer::clear()
{
    ::clear();
}

void NCursesRenderer::refresh()
{
    // Use double-buffering approach for smoother updates
    wnoutrefresh(stdscr);
    doupdate();
}

void NCursesRenderer::clearArea(int y, int x, int height, int width)
{
    // Use more efficient area clearing
    for (int row = y; row < y + height; ++row)
    {
        mvhline(row, x, ' ', width);
    }
}

void NCursesRenderer::refreshArea(int y, int x, int height, int width)
{
    // optimized refresh for specific areas
    touchline(stdscr, y, height);
    wnoutrefresh(stdscr);
    doupdate();
}

ScreenSize NCursesRenderer::getScreenSize() const
{
    int height, width;
    getmaxyx(stdscr, height, width);
    return {width, height};
}

void NCursesRenderer::drawText(int y, int x, const std::string& text)
{
    mvprintw(y, x, "%s", text.c_str());
}

void NCursesRenderer::drawText(int y, int x, const std::string& text, ColorPair color)
{
    attron(COLOR_PAIR(static_cast<int>(color)));
    mvprintw(y, x, "%s", text.c_str());
    attroff(COLOR_PAIR(static_cast<int>(color)));
}

void NCursesRenderer::drawTextWithAttributes(int y, int x, const std::string& text, int attributes)
{
    attron(attributes);
    mvprintw(y, x, "%s", text.c_str());
    attroff(attributes);
}

void NCursesRenderer::drawHorizontalLine(int y, int startX, int endX)
{
    mvhline(y, startX, ACS_HLINE, endX - startX + 1);
}

void NCursesRenderer::drawVerticalLine(int x, int startY, int endY)
{
    mvvline(startY, x, ACS_VLINE, endY - startY + 1);
}

void NCursesRenderer::drawBox(int y, int x, int height, int width)
{
    // Draw corners
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + width - 1, ACS_URCORNER);
    mvaddch(y + height - 1, x, ACS_LLCORNER);
    mvaddch(y + height - 1, x + width - 1, ACS_LRCORNER);
    
    // Draw horizontal lines
    mvhline(y, x + 1, ACS_HLINE, width - 2);
    mvhline(y + height - 1, x + 1, ACS_HLINE, width - 2);
    
    // Draw vertical lines
    mvvline(y + 1, x, ACS_VLINE, height - 2);
    mvvline(y + 1, x + width - 1, ACS_VLINE, height - 2);
}

void NCursesRenderer::setCursorPosition(const core::Cursor::Position& pos)
{
    move(pos.line, pos.column);
}

void NCursesRenderer::setCursorVisible(bool visible)
{
    curs_set(visible ? 1 : 0);
}

int NCursesRenderer::getInput()
{
    return getch();
}

void NCursesRenderer::setupColors()
{
    // Enable extended colors if available
    use_default_colors();
    
    // Professional Dark Theme - Ensure black background throughout
    // Basic UI colors - Clean and professional
    init_pair(static_cast<int>(ColorPair::Normal), COLOR_WHITE, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::StatusBar), COLOR_WHITE, COLOR_BLUE);
    init_pair(static_cast<int>(ColorPair::StatusBarActive), COLOR_YELLOW, COLOR_BLUE);
    
    // File Explorer colors - Modern sidebar look
    init_pair(static_cast<int>(ColorPair::FileExplorer), COLOR_WHITE, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::FileExplorerSelected), COLOR_BLACK, COLOR_WHITE);
    init_pair(static_cast<int>(ColorPair::FileExplorerDirectory), COLOR_CYAN, COLOR_BLACK);
    
    // Line numbers - Subtle but visible
    init_pair(static_cast<int>(ColorPair::LineNumbers), COLOR_BLUE, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::LineNumbersActive), COLOR_YELLOW, COLOR_BLACK);
    
    // Enhanced syntax highlighting - Professional colors
    init_pair(static_cast<int>(ColorPair::Syntax_Keyword), COLOR_MAGENTA, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::Syntax_String), COLOR_GREEN, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::Syntax_Comment), COLOR_CYAN, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::Syntax_Number), COLOR_RED, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::Syntax_Operator), COLOR_YELLOW, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::Syntax_Type), COLOR_BLUE, COLOR_BLACK);
    
    // Selection and search - High contrast
    init_pair(static_cast<int>(ColorPair::Search_Highlight), COLOR_BLACK, COLOR_YELLOW);
    init_pair(static_cast<int>(ColorPair::Selection), COLOR_BLACK, COLOR_WHITE);
    init_pair(static_cast<int>(ColorPair::CursorLine), COLOR_WHITE, COLOR_BLACK);
    
    // UI Elements - Clean borders and titles
    init_pair(static_cast<int>(ColorPair::Border), COLOR_WHITE, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::Title), COLOR_BLACK, COLOR_WHITE);
    
    // Status colors - Clear indication
    init_pair(static_cast<int>(ColorPair::Error), COLOR_RED, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::Warning), COLOR_YELLOW, COLOR_BLACK);
    init_pair(static_cast<int>(ColorPair::Success), COLOR_GREEN, COLOR_BLACK);
}

void NCursesRenderer::configureTerminal()
{
    // https://stackoverflow.com/questions/76128214/ncursesw-application-wont-catch-ctrl-c-s-z
    tcgetattr(STDIN_FILENO, &originalTermios_);
    struct termios newTermios = originalTermios_;
    
    // Configure terminal for proper control character handling:
    newTermios.c_lflag &= ~(ICANON | ECHO | ISIG);
    
    // Disable input processing that might interfere with control keys:
    newTermios.c_iflag &= ~(IXON | IXOFF | ICRNL | INLCR);
    
    // Disable special control characters to prevent signal generation
    newTermios.c_cc[VINTR] = _POSIX_VDISABLE;
    newTermios.c_cc[VSUSP] = _POSIX_VDISABLE;
    newTermios.c_cc[VSTOP] = _POSIX_VDISABLE;
    newTermios.c_cc[VSTART] = _POSIX_VDISABLE;

    // Set minimum characters and timeout for read
    newTermios.c_cc[VMIN] = 1;
    newTermios.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);
}

void NCursesRenderer::restoreTerminal()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios_);
}

void NCursesRenderer::drawModernBox(int y, int x, int height, int width, ColorPair color)
{
    attron(COLOR_PAIR(static_cast<int>(color)));
    
    // Use ncurses ACS (Alternate Character Set) for consistent box drawing
    // These work across different terminals and provide clean lines
    
    // Draw corners
    mvaddch(y, x, ACS_ULCORNER);                          // ┌
    mvaddch(y, x + width - 1, ACS_URCORNER);             // ┐
    mvaddch(y + height - 1, x, ACS_LLCORNER);            // └
    mvaddch(y + height - 1, x + width - 1, ACS_LRCORNER); // ┘
    
    // Draw horizontal lines (top and bottom)
    for (int i = x + 1; i < x + width - 1; i++) {
        mvaddch(y, i, ACS_HLINE);                         // ─
        mvaddch(y + height - 1, i, ACS_HLINE);           // ─
    }
    
    // Draw vertical lines (left and right)
    for (int i = y + 1; i < y + height - 1; i++) {
        mvaddch(i, x, ACS_VLINE);                         // │
        mvaddch(i, x + width - 1, ACS_VLINE);            // │
    }
    
    attroff(COLOR_PAIR(static_cast<int>(color)));
}

void NCursesRenderer::drawTitleBar(int y, int width, const std::string& title, ColorPair color)
{
    attron(COLOR_PAIR(static_cast<int>(color)));
    
    // Create a centered title with padding
    std::string titleBar(width, ' ');
    if (static_cast<int>(title.length()) < width - 4)
    {
        int startPos = (width - static_cast<int>(title.length())) / 2;
        for (size_t i = 0; i < title.length() && startPos + static_cast<int>(i) < width; i++)
        {
            titleBar[startPos + i] = title[i];
        }
    }
    else
    {
        // Truncate if too long
        std::string truncated = title.substr(0, width - 6) + "...";
        int startPos = (width - static_cast<int>(truncated.length())) / 2;
        for (size_t i = 0; i < truncated.length() && startPos + static_cast<int>(i) < width; i++)
        {
            titleBar[startPos + i] = truncated[i];
        }
    }
    
    mvprintw(y, 0, "%s", titleBar.c_str());
    
    attroff(COLOR_PAIR(static_cast<int>(color)));
}

void NCursesRenderer::drawProgressBar(int y, int x, int width, float progress, ColorPair color)
{
    attron(COLOR_PAIR(static_cast<int>(color)));
    
    // Clamp progress to 0.0-1.0
    progress = std::max(0.0f, std::min(1.0f, progress));
    
    int filledWidth = static_cast<int>(width * progress);
    
    // Draw filled portion using standard characters
    for (int i = 0; i < filledWidth; i++)
    {
        mvaddch(y, x + i, '#'); // Use # for filled part
    }
    
    // Draw empty portion
    attron(COLOR_PAIR(static_cast<int>(ColorPair::Normal)));
    for (int i = filledWidth; i < width; i++)
    {
        mvaddch(y, x + i, '-'); // Use - for empty part
    }
    attroff(COLOR_PAIR(static_cast<int>(ColorPair::Normal)));
    
    attroff(COLOR_PAIR(static_cast<int>(color)));
}

