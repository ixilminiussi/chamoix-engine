#include "petanque_game.h"

// std
#include <cstdlib>

// cmx
#include <cmx_editor.h>
#include <cmx_register.h>

int main()
{
    Petanque app{};

    try
    {
        app.load();
#ifndef NDEBUG
        cmx::Editor *editor = cmx::Editor::getInstance();
        editor->load(app.getWindow());
        editor->attachScene(app.getScene());
#endif
        app.run();
    }
    catch (const std::exception &e)
    {
        spdlog::critical("{0}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
