#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"

#include <string>
#include <vector>
using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Vertex
{
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    // bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    // weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct Material
{
    // Ambient
    glm::vec4 Ka;
    // Diffuse
    glm::vec4 Kd;
    // Specular
    glm::vec4 Ks;
    // Shininess
    float shininess;
    float transparency;
    bool hasTexture;
};

struct Texture
{
    unsigned int id;
    string type;
    string path;
};

class Mesh
{
public:
    // mesh Data
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    Material mat;
    bool isBulb;
    bool isGlass;
    bool isWater;
    aiString name;
    unsigned int VAO;

    // constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures, Material mat, aiString name)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        this->mat = mat;
        this->name = name;
        bool condition1 = strcmp(name.C_Str(),"Lightbulb")==0;
        bool condition2 = strcmp(name.C_Str(),"spotlight")==0;
        bool condition3 = strcmp(name.C_Str(),"lampLight")==0;
        bool condition4 = strcmp(name.C_Str(),"wallLight")==0;
        bool condition5 = strcmp(name.C_Str(),"floorLight")==0;
        if( strcmp(this->name.C_Str(),"light")==0 || condition1 || condition2 || condition3 || condition4 || condition5 )
        {
            // static int index = 1;
            this->isBulb = true;
            // std::cerr << index << std::endl; 
            // ++index;
        }
        else this->isBulb = false;

        if( strcmp(this->name.C_Str(),"glass")==0  ) this->isGlass = true;
        else this->isGlass = false;

        if( strcmp(this->name.C_Str(),"water")==0 ) this->isWater = true;
        else this->isWater = false;

        // std::cerr << textures.size() << std::endl;


        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    // render the mesh
    void Draw(Shader &shader, bool isLighting, GLuint cubetex)
    {
        //enable gl blend
        if( isLighting && this->isGlass ) glEnable(GL_BLEND);

        //set the lighting uniforms
        if( isLighting )
        {
            shader.setVec4("material.ambient", mat.Ka);
            shader.setVec4("material.diffuse", mat.Kd);
            shader.setVec4("material.specular",mat.Ks);
            shader.setFloat("material.shininess",mat.shininess);

            shader.setBool("material.hasTexture",mat.hasTexture);
        }

        // bind appropriate textures
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        // auto index = 0;
        for (unsigned int i = 1; i <= (textures.size()); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i ); // active proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            string number;
            string name = textures[i-1].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++); // transfer unsigned int to string
            else if (name == "texture_normal")
                number = std::to_string(normalNr++); // transfer unsigned int to string
            else if (name == "texture_height")
                number = std::to_string(heightNr++); // transfer unsigned int to string

            // now set the sampler to the correct texture unit
            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);

            glActiveTexture(GL_TEXTURE0 + i);
            // ++index;
            // and finally bind the texture
            glBindTexture(GL_TEXTURE_2D, textures[i-1].id);
        }
        // if( isLighting && this->isGlass )
        // {
        //     glActiveTexture(GL_TEXTURE0+index);
        //     glBindTexture(GL_TEXTURE_CUBE_MAP, cubetex); 
        // }

        if( isLighting )
        {
            shader.setBool("isBulb", isBulb);
            shader.setBool("isGlass", isGlass);
            shader.setBool("isWater", isWater);
        }
        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);

        //disable gl_blend
        if( isLighting && this->isGlass ) glDisable(GL_BLEND);
    }

private:
    // render data
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));
        // ids
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void *)offsetof(Vertex, m_BoneIDs));

        // weights
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, m_Weights));
        glBindVertexArray(0);
    }
};
#endif
