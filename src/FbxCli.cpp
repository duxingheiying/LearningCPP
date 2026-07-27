#include "FbxOpenGLLoader.h"

#include <iostream>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: LearningCPP <model.fbx>\n";
        return 1;
    }
    OpenGLSceneData scene;
    std::string error;
    FbxOpenGLLoader loader;
    if (!loader.load(argv[1], scene, error)) {
        std::cerr << "Failed to parse FBX file: " << error << '\n';
        return 2;
    }

    std::cout << "OpenGL draw items: " << scene.drawItems.size() << '\n';
    for (const OpenGLDrawItem& item : scene.drawItems) {
        std::cout << "- " << item.name << ": " << item.vertices.size()
                  << " vertices, " << item.indices.size() << " indices\n";
    }
    return 0;
}
