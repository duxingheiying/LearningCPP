#include "FbxSceneReader.h"

#include "ufbx.h"

#include <array>
#include <iomanip>
#include <memory>
#include <string>

namespace {

std::string toString(ufbx_string value)
{
    return value.data ? std::string(value.data, value.length) : std::string{};
}

void printIndent(std::ostream& output, uint32_t depth)
{
    for (uint32_t index = 0; index < depth; ++index) {
        output << "  ";
    }
}

void printNode(const ufbx_node* node, std::ostream& output)
{
    if (!node || node->is_root) return;

    printIndent(output, node->node_depth - 1);
    output << "- " << toString(node->name);
    if (node->mesh) {
        const ufbx_mesh& mesh = *node->mesh;
        output << " [mesh: vertices=" << mesh.num_vertices << ", faces=" << mesh.num_faces
               << ", triangles=" << mesh.num_triangles
               << ", normals=" << (mesh.vertex_normal.exists ? "yes" : "no")
               << ", uvSets=" << mesh.uv_sets.count << "]";
    } else if (node->camera) {
        output << " [camera]";
    } else if (node->light) {
        output << " [light]";
    } else if (node->bone) {
        output << " [bone]";
    }

    const ufbx_vec3 position = node->local_transform.translation;
    output << " localPosition=(" << position.x << ", " << position.y << ", " << position.z << ")\n";
    for (size_t index = 0; index < node->children.count; ++index) {
        printNode(node->children.data[index], output);
    }
}

} // namespace

bool FbxSceneReader::printSummary(const std::string& filename, std::ostream& output, std::ostream& errors) const
{
    ufbx_load_opts options{};
    options.generate_missing_normals = true;
    options.ignore_missing_external_files = true;

    ufbx_error error{};
    std::unique_ptr<ufbx_scene, decltype(&ufbx_free_scene)> scene(
        ufbx_load_file(filename.c_str(), &options, &error), ufbx_free_scene);
    if (!scene) {
        std::array<char, 4096> message{};
        ufbx_format_error(message.data(), message.size(), &error);
        errors << "Failed to parse FBX file '" << filename << "': " << message.data() << '\n';
        return false;
    }

    output << std::fixed << std::setprecision(3);
    output << "FBX: " << filename << '\n'
           << "Units: " << scene->settings.unit_meters << " m\n"
           << "Nodes: " << scene->nodes.count << ", Meshes: " << scene->meshes.count
           << ", Materials: " << scene->materials.count << ", Textures: " << scene->textures.count
           << ", Animation stacks: " << scene->anim_stacks.count << "\n\n";
    output << "Materials:\n";
    if (scene->materials.count == 0) output << "  (none)\n";
    for (size_t index = 0; index < scene->materials.count; ++index) {
        const ufbx_material& material = *scene->materials.data[index];
        output << "  - " << toString(material.name) << " (shader="
               << toString(material.shading_model_name) << ", textures="
               << material.textures.count << ")\n";
    }
    output << "\nNode hierarchy:\n";
    for (size_t index = 0; index < scene->root_node->children.count; ++index) {
        printNode(scene->root_node->children.data[index], output);
    }
    return true;
}
