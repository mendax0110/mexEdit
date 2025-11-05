#include "../include/EditorApplication.h"
#include "../include/ui/Renderer.h"
#include "../include/utils/MemoryDebugger.h"
#include <iostream>
#include <memory>
#include <filesystem>
#include <cstdlib>

int main(int argc, char* argv[])
{
    try
    {
        std::cout << "mexEdit starting with memory debugging enabled..." << std::endl;
        {
            auto renderer = std::make_unique<mexedit::ui::NCursesRenderer>();
            mexedit::EditorApplication app(std::move(renderer));
            app.initialize();

            if (argc > 1)
            {
                std::filesystem::path filePath(argv[1]);
                if (!app.openFile(filePath))
                {
                    std::cerr << "Warning: Could not open file: " << argv[1] << std::endl;
                }
            }

            app.run();
        } // Ensure all objects are destroyed before memory check

        auto& memDebugger = mexedit::utils::MemoryDebugger::getInstance();
        memDebugger.describeLeakAndMemoryMap();
        memDebugger.printStats();

        if (memDebugger.hasLeaks())
        {
            std::cerr << "WARNING: Potential memory leaks detected!" << std::endl;
            return EXIT_FAILURE;
        }
        else
        {
            std::cout << "No memory leaks detected. Application exited cleanly." << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        mexedit::utils::MemoryDebugger::getInstance().printStats();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}