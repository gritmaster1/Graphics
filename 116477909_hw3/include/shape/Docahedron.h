#ifndef DOCA_H
#define DOCA_H

#include <string>

#include <glm/glm.hpp>

#include "shape/Mesh.h"


class Shader;


class docadehedron : public Mesh
{
public:
    docadehedron(Shader* pShader, const std::string& vertexFile, const glm::mat4& model, int shapetype);

    ~docadehedron() noexcept override = default;

    void subDivide();

    void render(float timeElapsedSinceLastFrame) override;

    void ConfigurePipeline();

private:
    static constexpr glm::vec3 kColor{ 0.31f, 0.5f, 1.0f };
    int shapetypr;
};


#endif  // DOCA_H
