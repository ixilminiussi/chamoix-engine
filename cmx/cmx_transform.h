#ifndef CMX_TRANSFORM
#define CMX_TRANSFORM

// lib
#include "tinyxml2.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>
//
#include ".external/imguizmo/ImGuizmo.h"

namespace cmx
{

struct Transform
{
    glm::vec3 position{};
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::quat rotation{glm::vec3{0.f}};

    glm::mat4 mat4() const;
    void fromMat4(const glm::mat4 &);
    glm::mat4 mat4_noScale() const;
    glm::mat3 normalMatrix() const;
    glm::vec3 forward() const;
    glm::vec3 right() const;
    glm::vec3 up() const;

    void editor();
    tinyxml2::XMLElement &save(tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *parentElement) const;
    void load(tinyxml2::XMLElement *);

    static Transform ONE;

    friend Transform operator+(const Transform &a, const Transform &b);

  private:
    glm::vec3 _euler{glm::vec3{0.f}};
    bool _additive = false;
    bool _wasActive = false;
    bool _isActive = false;
};

class Transformable
{
  public:
    Transformable() : _transform{Transform::ONE} {};
    Transformable(const Transform &transform) : _transform{transform} {};
    ~Transformable() = default;

    virtual const Transform &getLocalSpaceTransform() const = 0;
    virtual Transform getWorldSpaceTransform() const = 0;

    void editor(class Camera *camera);

    void setPosition(const glm::vec3 &position);
    void setRotation(const glm::quat &rotation);
    void setRotation(const glm::vec3 &euler);
    void setScale(const glm::vec3 &scale);

    glm::vec3 getWorldSpaceForward()
    {
        return getWorldSpaceTransform().forward();
    }
    glm::vec3 getWorldSpaceRight()
    {
        return getWorldSpaceTransform().right();
    }
    glm::vec3 getWorldSpaceUp()
    {
        return getWorldSpaceTransform().up();
    }

    static ImGuizmo::OPERATION currentGuizmoOperation;
    static bool guizmoSnap;
    static float guizmoSnapTo;

  protected:
    Transform _transform;
};

Transform operator+(const Transform &a, const Transform &b);

} // namespace cmx

#endif
