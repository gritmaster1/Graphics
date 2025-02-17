#include "shape/icosahedron.h"
#include <glm/glm.hpp>

#include "util/Shader.h"


icosahedron::icosahedron(
    Shader* pShader,
    const std::string& vertexFile, 
    const glm::mat4& model,
    glm::vec3 scale
)
    : Mesh(pShader, model), Scale(scale)
{
    // Initialize vertex data
    if (std::ifstream fin{ vertexFile })
    {
        glm::vec3 v1;
        glm::vec3 v2;
        glm::vec3 v3;

        while (fin >> v1.x >> v1.y >> v1.z >> v2.x >> v2.y >> v2.z >> v3.x >> v3.y >> v3.z)
        {
            glm::vec3 fn = glm::normalize(glm::cross(v2 - v1, v3 - v2));
            vertices.emplace_back(v1 * scale, fn, kColor);
            vertices.emplace_back(v2 * scale, fn, kColor);
            vertices.emplace_back(v3 * scale, fn, kColor);
        }
    }
    else
    {
        throw std::runtime_error("failed to open " + vertexFile);
    }

    ConfigurePipeline();
}


void icosahedron::subDivide()
{
    std::vector<Mesh::Vertex> NewMesh;

    for (int i = 0; i < vertices.size(); i += 3) {
        glm::vec3 v0 = vertices[i + 0].position;
        glm::vec3 v1 = vertices[i + 1].position;
        glm::vec3 v2 = vertices[i + 2].position;

        // Find midpoints of the edges of the triangle
        glm::vec3 mid01 = Scale * glm::normalize((v0 + v1) * 0.5f);
        glm::vec3 mid12 = Scale * glm::normalize((v1 + v2) * 0.5f);
        glm::vec3 mid20 = Scale * glm::normalize((v2 + v0) * 0.5f);

        // Calculate direction vectors from the origin (assumed to be the center of the sphere)
        glm::vec3 normal01 = glm::normalize(v0); // Normal pointing outwards from the origin to v0
        glm::vec3 normal12 = glm::normalize(v1); // Normal pointing outwards from the origin to v1
        glm::vec3 normal20 = glm::normalize(v2); // Normal pointing outwards from the origin to v2

        // Calculate midpoints and their normals
        glm::vec3 normalMid01 = glm::normalize( Scale * glm::normalize(normal01 + normal12) ); // Average of normals at mid01
        glm::vec3 normalMid12 = glm::normalize( Scale * glm::normalize(normal12 + normal20) ); // Average of normals at mid12
        glm::vec3 normalMid20 = glm::normalize( Scale * glm::normalize(normal20 + normal01) ); // Average of normals at mid20


        glm::vec3 color = vertices[i + 0].color; // Example color, could be dynamic based on position

        // Add the new triangles with position, normal, and color data
        NewMesh.push_back(Vertex{ v0, normal01, color });
        NewMesh.push_back(Vertex{ mid01, normalMid01, color });
        NewMesh.push_back(Vertex{ mid20, normalMid20, color });


        NewMesh.push_back(Vertex{ v1, normal12, color });
        NewMesh.push_back(Vertex{ mid01, normalMid01, color });
        NewMesh.push_back(Vertex{ mid12, normalMid12, color });
      
        NewMesh.push_back(Vertex{ v2, normal20, color });
        NewMesh.push_back(Vertex{ mid12, normalMid12, color });
        NewMesh.push_back(Vertex{ mid20, normalMid20, color });
        
        NewMesh.push_back(Vertex{ mid01, normalMid01, color });
        NewMesh.push_back(Vertex{ mid12, normalMid12, color });
        NewMesh.push_back(Vertex{ mid20, normalMid20, color });
    }

    vertices = NewMesh;
    ConfigurePipeline();
}

void icosahedron::render(float timeElapsedSinceLastFrame)
{
    pShader->use();
    pShader->setMat4("model", model);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glDrawArrays(GL_TRIANGLES,
        0,                                       // start from index 0 in current VBO
        static_cast<GLsizei>(vertices.size()));  // draw these number of elements

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void icosahedron::ConfigurePipeline()
{
    // OpenGL pipeline configuration
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER,
        static_cast<GLsizei>(this->vertices.size() * sizeof(Vertex)),
        vertices.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
