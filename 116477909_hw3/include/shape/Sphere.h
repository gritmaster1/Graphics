#ifndef SPHERE_H
#define SPHERE_H

#include <glm/glm.hpp>

#include "shape/GLShape.h"


class Shader;


class Sphere : public Renderable, public GLShape
{
public:
    Sphere(
        Shader* pShader,
        const glm::vec3& center,
        float radius,
        const glm::vec3& color,
        const glm::mat4& model,
        const int shapetype = 0,
        float minorradius = 0.5f
    );

    ~Sphere() noexcept override = default;

    void render(float timeElapsedSinceLastFrame) override;

private:
    static constexpr float kNull {0.0f};

private:
    glm::vec3 center {0.0f, 0.0f, 0.0f};
    float radius {1.0f};
    float minorradius{0.50f};
    glm::vec3 color {1.0f, 0.5f, 0.31f};
    int shapetype;//use sphere class for rendering Other Parametric shapes in Tesselationshaders
};


#endif  // SPHERE_H
