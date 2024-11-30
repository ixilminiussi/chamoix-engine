#include "demo_game.h"

// std
#include <cstdlib>

int main()
{
    Demo app{};

    try
    {
        app.load();
        app.loadEditor();
        app.run();
    }
    catch (const std::exception &e)
    {
        spdlog::error("{0}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
