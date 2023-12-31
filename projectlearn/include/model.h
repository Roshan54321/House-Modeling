#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "assimp_glm_helpers.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "mesh.h"
#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <math.h>
#include <vector>
#include <algorithm>
#include "animdata.h"


using namespace std;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

struct Bulbs {
    glm::vec3 Color;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular; 
    glm::vec3 position;
    glm::vec3 normal;
    float angle; //in degrees
    float constant;
    float linear;
    float exp; 
};

class Model 
{
public:
    // model data 
    vector<Texture>textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh>meshes;
    // int MAX_BULBS = 5;
    vector<Bulbs>bulbs;
    vector<Bulbs>pointBulbs;
    string directory;
    bool gammaCorrection;

    // constructor, expects a filepath to a 3D model.
    Model(string const &path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw(Shader &shader, bool isLighting, GLuint cubetex)
    {
        if( bulbs.size()>0 )
        {
            shader.setInt("numBulbs",(int)bulbs.size());
            // std::cerr << bulbs.size() << std::endl;
            for( auto i=0; i<bulbs.size(); ++i )
            {
                shader.setVec3((string("bulbs[")+to_string(i)+string("].base.position")).c_str(), bulbs[i].position);
                // shader.setVec3((string("bulbs[")+to_string(i)+string("].base.Color")).c_str(), bulbs[i].Color);
                shader.setVec3((string("bulbs[")+to_string(i)+string("].base.base.ambient")).c_str(), bulbs[i].ambient);
                shader.setVec3((string("bulbs[")+to_string(i)+string("].base.base.diffuse")).c_str(), bulbs[i].diffuse);
                shader.setVec3((string("bulbs[")+to_string(i)+string("].base.base.specular")).c_str(), bulbs[i].specular);
                shader.setFloat((string("bulbs[")+to_string(i)+string("].base.atten.constant")).c_str(), bulbs[i].constant);
                shader.setFloat((string("bulbs[")+to_string(i)+string("].base.atten.linear")).c_str(), bulbs[i].linear);
                shader.setFloat((string("bulbs[")+to_string(i)+string("].base.atten.exp")).c_str(), bulbs[i].exp);
                shader.setFloat((string("bulbs[")+to_string(i)+string("].cutoff")).c_str(), cos(glm::radians(bulbs[i].angle)));
                shader.setVec3((string("bulbs[")+to_string(i)+string("].direction")).c_str(), bulbs[i].normal);
            }
        }
        else 
        {
            shader.setInt("numBulbs",0);
        }
        if( pointBulbs.size()>0 )
        {
             shader.setInt("numpBulbs",(int)pointBulbs.size());
            // std::cerr << bulbs.size() << std::endl;
            for( auto i=0; i<pointBulbs.size(); ++i )
            {
                shader.setVec3((string("pointBulbs[")+to_string(i)+string("].position")).c_str(), pointBulbs[i].position);
                // shader.setVec3((string("pointBulbs[")+to_string(i)+string("].base.Color")).c_str(), pointBulbs[i].Color);
                shader.setVec3((string("pointBulbs[")+to_string(i)+string("].base.ambient")).c_str(), pointBulbs[i].ambient);
                shader.setVec3((string("pointBulbs[")+to_string(i)+string("].base.diffuse")).c_str(), pointBulbs[i].diffuse);
                shader.setVec3((string("pointBulbs[")+to_string(i)+string("].base.specular")).c_str(), pointBulbs[i].specular);
                shader.setFloat((string("pointBulbs[")+to_string(i)+string("].atten.constant")).c_str(), pointBulbs[i].constant);
                shader.setFloat((string("pointBulbs[")+to_string(i)+string("].atten.linear")).c_str(), pointBulbs[i].linear);
                shader.setFloat((string("pointBulbs[")+to_string(i)+string("].atten.exp")).c_str(), pointBulbs[i].exp);
                // shader.setFloat((string("pointBulbs[")+to_string(i)+string("].cutoff")).c_str(), cos(pointBulbs[i].angle*3.1415/180));
                // shader.setVec3((string("pointBulbs[")+to_string(i)+string("].direction")).c_str(), pointBulbs[i].normal);
            }           
        }
        else
        {
            shader.setInt("numpBulbs",0);
        }
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader, isLighting, cubetex);
    }
    auto& GetBoneInfoMap() { return m_BoneInfoMap; }
	int& GetBoneCount() { return m_BoneCounter; }
    
private:
    std::map<string, BoneInfo> m_BoneInfoMap;
	int m_BoneCounter = 0;
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const &path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
       // const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices  |aiProcess_SortByPType | aiProcess_FlipUVs);
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate|aiProcess_CalcTangentSpace);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene)
    {
        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }
    void SetVertexBoneDataToDefault(Vertex& vertex)
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
		{
			vertex.m_BoneIDs[i] = -1;
			vertex.m_Weights[i] = 0.0f;
		}
	}

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;
        glm::vec3 position; //for light bulbs
        glm::vec3 normal;

        // walk through each of the mesh's vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            SetVertexBoneDataToDefault(vertex);
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            position = vector;
            vertex.Position = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                normal = vector;
                vertex.Normal = vector;
            }
            // texture coordinates
            if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);        
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiString meshName = material->GetName();
        bool condition1 = strcmp(meshName.C_Str(),"Lightbulb")==0;
        bool condition2 = strcmp(meshName.C_Str(),"spotlight")==0;
        bool condition3 = strcmp(meshName.C_Str(),"lampLight")==0;
        // bool condition4 = strcmp(meshName.C_Str(),"wallLight")==0;
        bool condition5 = strcmp(meshName.C_Str(),"floorLight")==0;
        if( condition1 || condition2 || condition3 || condition5 )
        {
            Bulbs bulb;
            bulb.position = position;
            bulb.ambient = glm::vec3(0.24725,0.1995,0.0745);
            bulb.diffuse = glm::vec3(0.75164,0.60648,0.22648);
            bulb.specular = glm::vec3(0.628281,0.555802,0.366065);
                // bulb.ambient = glm::vec3(0.19125,0.0735,0.0225);
                // bulb.diffuse = glm::vec3(0.7038,0.27048,0.0828);
                // bulb.specular = glm::vec3(0.256777,0.137622,0.086014);
            // if(mesh->HasNormals())bulb.normal = normal;
            if(condition2)
            {
                // bulb.ambient = glm::vec3(0.19125,0.0735,0.0225);
                // bulb.diffuse = glm::vec3(0.7038,0.27048,0.0828);
                // bulb.specular = glm::vec3(0.256777,0.137622,0.086014);
                bulb.normal = glm::vec3(10.0,-1.0, -1.0);
                bulb.angle = 10;
                bulb.constant = 1.f;
                bulb.linear = 0.5f;
                bulb.exp = 0.8f;
            }
            else if(condition1)
            {
                // bulb.ambient = glm::vec3(0.05,0.00,0.0);
                // bulb.diffuse = glm::vec3(0.5,0.0,0.4);
                // bulb.specular = glm::vec3(0.7,0.7,0.04);  
                bulb.normal = glm::vec3(0.0,-7.0,0.0);
                bulb.angle = 4;
                bulb.constant = 1.f;
                bulb.linear = 0.0f;
                bulb.exp = 0.25f;
            }
            else if(condition3)
            {
                // bulb.ambient = glm::vec3(0.05,0.00,0.0);
                // bulb.diffuse = glm::vec3(0.5,0.0,0.4);
                // bulb.specular = glm::vec3(0.7,0.7,0.04);  
                bulb.normal = glm::vec3(0.0,-7.0,0.0);
                bulb.angle = 4;
                bulb.constant = 1.f;
                bulb.linear = 0.0f;
                bulb.exp = 0.08f;
            }
            // else if(condition4)
            // {
            //     bulb.normal = glm::vec3(0.0,1.0,0.0);
            //     bulb.angle = 10;
            //     bulb.constant = 1.f;
            //     bulb.linear = 0.4f;
            //     bulb.exp = 0.8f;
            // } 
            else if(condition5)
            {
                // bulb.ambient = glm::vec3(0.05,0.00,0.0);
                // bulb.diffuse = glm::vec3(0.5,0.0,0.4);
                // bulb.specular = glm::vec3(0.7,0.7,0.04);  
                // bulb.ambient = glm::vec3(0.0,0.1,0.06);
                // bulb.diffuse = glm::vec3(0.0,0.50980392,0.50980392);
                // bulb.specular = glm::vec3(0.50196078,0.50196078,0.50196078);
                // bulb.normal = glm::vec3(-1.0,0.0,0.0);
                // bulb.angle = 10;
                // bulb.constant = 1.f;
                // bulb.linear = 0.04f;
                // bulb.exp = 0.01f;
            }
            // bulb.ambientIntensity = 0.2f;
            // bulb.diffuseIntensity = 0.3f;

            bulbs.push_back(bulb);
        }

        bool conditionl = strcmp(meshName.C_Str(), "light")==0;
        if( conditionl )
        {
            Bulbs bulb;
            bulb.position = position;
            bulb.ambient = glm::vec3(0.24725,0.1995,0.0745);
            bulb.diffuse = glm::vec3(0.75164,0.60648,0.22648);
            bulb.specular = glm::vec3(0.628281,0.555802,0.366065);
                // bulb.ambient = glm::vec3(0.19125,0.0735,0.0225);
                // bulb.diffuse = glm::vec3(0.7038,0.27048,0.0828);
                // bulb.specular = glm::vec3(0.256777,0.137622,0.086014);
            {
                // bulb.ambient = glm::vec3(0.05,0.00,0.0);
                // bulb.diffuse = glm::vec3(0.5,0.0,0.4);
                // bulb.specular = glm::vec3(0.7,0.7,0.04);  
                // bulb.normal = glm::vec3(0.0,-7.0,0.0);
                // bulb.angle = 0;
                bulb.constant = 1.f;
                bulb.linear = 0.01f;
                bulb.exp = 0.08f;
            }
            // bulb.ambientIntensity = 0.2f;
            // bulb.diffuseIntensity = 0.3f;

            pointBulbs.push_back(bulb);
        }

        Material mat;
        aiColor3D color;
        
        //Read the vertex data of the mtl file
        float shininess, transparency;
        material->Get(AI_MATKEY_SHININESS, shininess);
        mat.shininess = shininess;
        material->Get(AI_MATKEY_COLOR_TRANSPARENT, transparency);
        if(strcmp(meshName.C_Str(),"glass")==0) transparency = 0.5; 
        material->Get(AI_MATKEY_COLOR_AMBIENT, color);
        mat.Ka = glm::vec4(color.r,color.g,color.b,transparency);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        mat.Kd = glm::vec4(color.r,color.g,color.b,transparency);       
        material->Get(AI_MATKEY_COLOR_SPECULAR, color);
        mat.Ks = glm::vec4(color.r,color.g,color.b,transparency);

        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        if( material->GetTextureCount(aiTextureType_DIFFUSE)==0 )mat.hasTexture = false;
        else mat.hasTexture = true;

        // 1. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        ExtractBoneWeightForVertices(vertices,mesh,scene);
        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures, mat, meshName);
    }
    void SetVertexBoneData(Vertex& vertex, int boneID, float weight)
	{
		for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
		{
			if (vertex.m_BoneIDs[i] < 0)
			{
				vertex.m_Weights[i] = weight;
				vertex.m_BoneIDs[i] = boneID;
				break;
			}
		}
	}

    void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
	{
		auto& boneInfoMap = m_BoneInfoMap;
		int& boneCount = m_BoneCounter;

		for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			int boneID = -1;
			std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				BoneInfo newBoneInfo;
				newBoneInfo.id = boneCount;
				newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
				boneInfoMap[boneName] = newBoneInfo;
				boneID = boneCount;
				boneCount++;
			}
			else
			{
				boneID = boneInfoMap[boneName].id;
			}
			assert(boneID != -1);
			auto weights = mesh->mBones[boneIndex]->mWeights;
			int numWeights = mesh->mBones[boneIndex]->mNumWeights;

			for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
			{
				int vertexId = weights[weightIndex].mVertexId;
				float weight = weights[weightIndex].mWeight;
				assert(vertexId <= vertices.size());
				SetVertexBoneData(vertices[vertexId], boneID, weight);
			}
		}
	}


    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for(unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if(!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }
};


inline unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
#endif
