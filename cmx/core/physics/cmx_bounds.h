#ifndef CMX_BOUNDS
#define CMX_BOUNDS

// lib
#include <array>
#include <glm/ext/vector_float3.hpp>
#include <vector>

namespace cmx
{

class Bounds
{
  public:
    Bounds();
    Bounds(const glm::vec3 startPoint);

    [[nodiscard]] std::array<glm::vec3, 8> getAllPoints() const;
    void expand(const glm::vec3 &);
    void expand(const std::vector<glm::vec3> &);

    float getLengthX();
    float getLengthY();
    float getLengthZ();

    const glm::vec3 &getMin();
    const glm::vec3 &getMax();

    [[nodiscard]] bool contains(const glm::vec3 &) const;

  protected:
    glm::vec3 _min, _max;
};

} // namespace cmx

#endif
