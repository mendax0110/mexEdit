#include "../include/mexEdit.h"
#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        MexEdit editor;

        if (argc > 1)
        {
            editor.loadFile(argv[1]);
        }

        editor.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}