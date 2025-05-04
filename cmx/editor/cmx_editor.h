#ifndef CMX_EDITOR
#define CMX_EDITOR

// lib
#include <vulkan/vulkan_core.h>

// std
#include <memory>
#include <string>

namespace cmx
{

class Editor
{
  public:
    static Editor *getInstance();

    void load(class Window &);
    void attachScene(class Scene *);
    void toggle(float dt, int);
    void update(float dt);
    void render(const struct FrameInfo &);

    void initInputManager(class Window &,
                          const std::string &shortcutsPath = std::string(EDITOR_FILES) + std::string("shortcuts.xml"));

    class InputManager *getInputManager()
    {
        return _inputManager.get();
    }
    class ViewportActor *getViewportActor()
    {
        return _viewportActor.get();
    }

    static bool isActive()
    {
        return _active;
    }

  protected:
    Editor();

    static Editor *_instance;

    std::unique_ptr<class ViewportActor> _viewportActor;
    std::unique_ptr<class ViewportUI> _viewportUI;
    std::unique_ptr<class InputManager> _inputManager;

    class Scene *_scene;

    static bool _active;
};

} // namespace cmx

#endif
