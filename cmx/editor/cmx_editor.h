#ifndef CMX_EDITOR
#define CMX_EDITOR

// lib
#include <glm/ext/vector_float2.hpp>
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
    void declarePlayIntent();
    void leave(float dt, int);
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
    glm::vec2 getSceneViewportSize();
    bool isViewportActorSelected();
    bool isViewportActorHovered();

    static bool isActive()
    {
        return _active;
    }

  protected:
    void play();
    bool _playIntent{false};

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
