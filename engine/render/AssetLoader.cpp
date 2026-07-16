#include "AssetLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

namespace mmo::render {

    bool AssetLoader::LoadModel(const std::string& filepath, Mesh& outMesh) {
        Assimp::Importer importer;
        
        const aiScene* scene = importer.ReadFile(filepath, 
            aiProcess_Triangulate | 
            aiProcess_GenSmoothNormals | 
            aiProcess_FlipUVs | 
            aiProcess_JoinIdenticalVertices);
            
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Assimp Error loading " << filepath << ": " << importer.GetErrorString() << "\n";
            return false;
        }

        // Just take the first mesh for now
        if (scene->mNumMeshes == 0) {
            std::cerr << "No meshes found in " << filepath << "\n";
            return false;
        }

        aiMesh* mesh = scene->mMeshes[0];

        // Reserve space
        outMesh.vertices.reserve(mesh->mNumVertices);
        outMesh.indices.reserve(mesh->mNumFaces * 3);

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex{};
            vertex.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            
            if (mesh->HasNormals()) {
                vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            } else {
                vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            // For now, give a default color (white). 
            // In a real material system we'd use textures or vertex colors
            vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);

            outMesh.vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                outMesh.indices.push_back(face.mIndices[j]);
            }
        }

        std::cout << "Successfully loaded " << filepath << " with " << outMesh.vertices.size() << " vertices and " << outMesh.indices.size() << " indices.\n";
        return true;
    }

}
