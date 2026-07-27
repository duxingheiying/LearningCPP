#include "FbxOpenGLLoader.h"

#include "ufbx.h"

#include <array>
#include <memory>
#include <utility>

namespace {

std::string toString(ufbx_string value)
{
    return value.data ? std::string(value.data, value.length) : std::string{};
}

std::array<float, 16> toOpenGLMatrix(const ufbx_matrix& matrix)
{
    return { static_cast<float>(matrix.m00), static_cast<float>(matrix.m10), static_cast<float>(matrix.m20), 0.0f,
             static_cast<float>(matrix.m01), static_cast<float>(matrix.m11), static_cast<float>(matrix.m21), 0.0f,
             static_cast<float>(matrix.m02), static_cast<float>(matrix.m12), static_cast<float>(matrix.m22), 0.0f,
             static_cast<float>(matrix.m03), static_cast<float>(matrix.m13), static_cast<float>(matrix.m23), 1.0f };
}

std::string texturePath(const ufbx_material_map& map)
{
    if (!map.texture || !map.texture_enabled) return {};
    const ufbx_texture& texture = *map.texture;
    if (texture.filename.length) return toString(texture.filename);
    if (texture.relative_filename.length) return toString(texture.relative_filename);
    return toString(texture.absolute_filename);
}

OpenGLMaterial makeMaterial(const ufbx_material* source)
{
    OpenGLMaterial material;
    if (!source) return material;

    material.name = toString(source->name);
    const ufbx_material_map& baseColor = source->pbr.base_color;
    const ufbx_material_map& roughness = source->pbr.roughness;
    const ufbx_material_map& metallic = source->pbr.metalness;

    if (baseColor.has_value) {
        material.baseColor = { static_cast<float>(baseColor.value_vec4.x), static_cast<float>(baseColor.value_vec4.y),
                               static_cast<float>(baseColor.value_vec4.z), static_cast<float>(baseColor.value_vec4.w) };
    }
    if (roughness.has_value) material.roughness = static_cast<float>(roughness.value_real);
    if (metallic.has_value) material.metallic = static_cast<float>(metallic.value_real);
    material.baseColorTexturePath = texturePath(baseColor);
    material.normalTexturePath = texturePath(source->pbr.normal_map);
    if (material.normalTexturePath.empty()) material.normalTexturePath = texturePath(source->fbx.normal_map);
    return material;
}

OpenGLVertex makeVertex(const ufbx_mesh& mesh, uint32_t index)
{
    const ufbx_vec3 position = ufbx_get_vertex_vec3(&mesh.vertex_position, index);
    const ufbx_vec3 normal = ufbx_get_vertex_vec3(&mesh.vertex_normal, index);
    const ufbx_vec2 uv = mesh.vertex_uv.exists ? ufbx_get_vertex_vec2(&mesh.vertex_uv, index) : ufbx_zero_vec2;
    const ufbx_vec3 tangent = mesh.vertex_tangent.exists ? ufbx_get_vertex_vec3(&mesh.vertex_tangent, index) : ufbx_zero_vec3;
    return {
        { static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z) },
        { static_cast<float>(normal.x), static_cast<float>(normal.y), static_cast<float>(normal.z) },
        { static_cast<float>(uv.x), static_cast<float>(uv.y) },
        { static_cast<float>(tangent.x), static_cast<float>(tangent.y), static_cast<float>(tangent.z) },
    };
}

void appendMeshByMaterial(const ufbx_node& node, OpenGLSceneData& result)
{
    if (!node.mesh) return;
    const ufbx_mesh& mesh = *node.mesh;
    const size_t materialCount = node.materials.count ? node.materials.count : 1;
    std::vector<OpenGLDrawItem> items(materialCount);
    for (size_t materialIndex = 0; materialIndex < materialCount; ++materialIndex) {
        items[materialIndex].name = toString(node.name);
        if (materialCount > 1) items[materialIndex].name += "#material" + std::to_string(materialIndex);
        items[materialIndex].modelMatrix = toOpenGLMatrix(node.geometry_to_world);
        items[materialIndex].material = makeMaterial(node.materials.count ? node.materials.data[materialIndex] : nullptr);
    }

    std::vector<uint32_t> triangleIndices(mesh.max_face_triangles * 3);
    for (size_t faceIndex = 0; faceIndex < mesh.faces.count; ++faceIndex) {
        const ufbx_face face = mesh.faces.data[faceIndex];
        if (face.num_indices < 3) continue;
        const uint32_t triangleCount = ufbx_triangulate_face(triangleIndices.data(), triangleIndices.size(), &mesh, face);
        size_t materialIndex = 0;
        if (mesh.face_material.count > faceIndex && mesh.face_material.data[faceIndex] < materialCount) {
            materialIndex = mesh.face_material.data[faceIndex];
        }
        OpenGLDrawItem& item = items[materialIndex];
        for (uint32_t triangleIndex = 0; triangleIndex < triangleCount * 3; ++triangleIndex) {
            item.vertices.push_back(makeVertex(mesh, triangleIndices[triangleIndex]));
            item.indices.push_back(static_cast<uint32_t>(item.indices.size()));
        }
    }
    for (OpenGLDrawItem& item : items) {
        if (!item.indices.empty()) result.drawItems.push_back(std::move(item));
    }
}

} // namespace

bool FbxOpenGLLoader::load(const std::string& filename, OpenGLSceneData& result, std::string& errorMessage) const
{
    result.drawItems.clear();
    errorMessage.clear();

    ufbx_load_opts options {};
    options.generate_missing_normals = true;
    options.ignore_missing_external_files = true;
    ufbx_error error {};
    std::unique_ptr<ufbx_scene, decltype(&ufbx_free_scene)> scene(
        ufbx_load_file(filename.c_str(), &options, &error), ufbx_free_scene);
    if (!scene) {
        std::array<char, 4096> message {};
        ufbx_format_error(message.data(), message.size(), &error);
        errorMessage = message.data();
        return false;
    }

    for (size_t nodeIndex = 0; nodeIndex < scene->nodes.count; ++nodeIndex) {
        appendMeshByMaterial(*scene->nodes.data[nodeIndex], result);
    }
    return true;
}
