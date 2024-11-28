#include "game.h"

// std
#include <cstdlib>

int main()
{
    cmx::Game app{};

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        spdlog::error("{0}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
