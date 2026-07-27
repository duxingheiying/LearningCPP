#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

// CPU-side layout ready for glVertexAttribPointer(): locations 0, 1, 2, 3.
// The loader expands FBX's independently-indexed attributes into one vertex
// per triangle corner, making the index buffer safe for OpenGL drawing.
struct OpenGLVertex {
    float position[3];
    float normal[3];
    float texCoord[2];
    float tangent[3];
};

struct OpenGLMaterial {
    std::string name;
    std::array<float, 4> baseColor { 1.0f, 1.0f, 1.0f, 1.0f };
    float metallic = 0.0f;
    float roughness = 1.0f;
    std::string baseColorTexturePath;
    std::string normalTexturePath;
};

struct OpenGLDrawItem {
    std::string name;
    // Column-major 4x4 matrix for glUniformMatrix4fv(..., GL_FALSE, modelMatrix.data()).
    std::array<float, 16> modelMatrix {};
    std::vector<OpenGLVertex> vertices;
    std::vector<std::uint32_t> indices;
    OpenGLMaterial material;
};

struct OpenGLSceneData {
    std::vector<OpenGLDrawItem> drawItems;
};

class FbxOpenGLLoader {
public:
    // Reads a binary or ASCII FBX. Geometry stays in model space; apply each
    // draw item's modelMatrix in the vertex shader.
    bool load(const std::string& filename, OpenGLSceneData& result, std::string& errorMessage) const;
};
