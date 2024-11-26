#include "game.h"

// std
#include <cstdlib>
#include <iostream>

int main()
{
    cmx::Game app{};

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
