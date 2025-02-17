#ifndef icosahedron_h
#define icosahedron_h

#include <string>

#include <glm/glm.hpp>

#include "shape/Mesh.h"


class Shader;


class icosahedron : public Mesh
{
public:
    icosahedron(Shader* pShader, const std::string& vertexFile, const glm::mat4& model, glm::vec3 scale = glm::vec3(1.0f));

    ~icosahedron() noexcept override = default;

    void subDivide();

    void render(float timeElapsedSinceLastFrame) override;

    void ConfigurePipeline();

private:
    static constexpr glm::vec3 kColor{ 0.31f, 0.5f, 1.0f };
    glm::vec3 Scale;
};


#endif  // icosahedron_h
