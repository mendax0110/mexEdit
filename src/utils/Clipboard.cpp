#include "../include/utils/Clipboard.h"
#include "../include/utils/MemoryDebugger.h"
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <array>

using namespace mexedit::utils;

std::string Clipboard::clipboardContent_;

Clipboard::Clipboard()
{
    TRACK_MEMORY(Clipboard, this);
}

Clipboard::~Clipboard()
{
    UNTRACK_MEMORY(Clipboard, this);
}

// Helper function to execute a command and get output
std::string executeCommand(const std::string& cmd) 
{
    std::array<char, 128> buffer{};
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
    {
        return "";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

// Helper function to execute a command for writing
bool executeWriteCommand(const std::string& cmd, const std::string& input)
{
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "w"), pclose);
    if (!pipe)
    {
        return false;
    }
    fwrite(input.c_str(), 1, input.length(), pipe.get());
    return true;
}
    
void Clipboard::setText(const std::string& text)
{
    // Store internally as fallback
    clipboardContent_ = text;
    
    // Try to use system clipboard
    // First try xclip (most common on Linux)
    if (system("command -v xclip > /dev/null 2>&1") == 0)
    {
        executeWriteCommand("xclip -selection clipboard", text);
        return;
    }
    
    // Try xsel as alternative
    if (system("command -v xsel > /dev/null 2>&1") == 0)
    {
        executeWriteCommand("xsel --clipboard --input", text);
        return;
    }
    
    // Try wl-clipboard for Wayland
    if (system("command -v wl-copy > /dev/null 2>&1") == 0)
    {
        executeWriteCommand("wl-copy", text);
        return;
    }
    
    // If no system clipboard tool available, just use internal storage
}
    
std::string Clipboard::getText()
{
    // Try to get from system clipboard first
    std::string systemClipboard;
    
    // Try xclip first
    if (system("command -v xclip > /dev/null 2>&1") == 0)
    {
        systemClipboard = executeCommand("xclip -selection clipboard -o 2>/dev/null");
        if (!systemClipboard.empty())
        {
            // Remove trailing newline if present
            if (!systemClipboard.empty() && systemClipboard.back() == '\n')
            {
                systemClipboard.pop_back();
            }
            clipboardContent_ = systemClipboard;  // Update internal cache
            return systemClipboard;
        }
    }
    
    // Try xsel as alternative
    if (system("command -v xsel > /dev/null 2>&1") == 0)
    {
        systemClipboard = executeCommand("xsel --clipboard --output 2>/dev/null");
        if (!systemClipboard.empty()) {
            if (!systemClipboard.empty() && systemClipboard.back() == '\n')
            {
                systemClipboard.pop_back();
            }
            clipboardContent_ = systemClipboard;
            return systemClipboard;
        }
    }
    
    // Try wl-clipboard for Wayland
    if (system("command -v wl-paste > /dev/null 2>&1") == 0)
    {
        systemClipboard = executeCommand("wl-paste 2>/dev/null");
        if (!systemClipboard.empty())
        {
            if (!systemClipboard.empty() && systemClipboard.back() == '\n')
            {
                systemClipboard.pop_back();
            }
            clipboardContent_ = systemClipboard;
            return systemClipboard;
        }
    }
    
    // Fall back to internal storage
    return clipboardContent_;
}
    
bool Clipboard::hasText()
{
    return !getText().empty();
}
    
void Clipboard::clear()
{
    clipboardContent_.clear();
    
    // Try to clear system clipboard
    if (system("command -v xclip > /dev/null 2>&1") == 0)
    {
        executeWriteCommand("xclip -selection clipboard", "");
    }
    else if (system("command -v xsel > /dev/null 2>&1") == 0)
    {
        executeWriteCommand("xsel --clipboard --input", "");
    }
    else if (system("command -v wl-copy > /dev/null 2>&1") == 0)
    {
        executeWriteCommand("wl-copy", "");
    }
}

