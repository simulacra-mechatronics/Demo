#ifndef SHADER_H
#define SHADER_H

#include <windows.h>

#include <fstream>
using std::ifstream;

#include <iostream>

#include <vector>
using std::vector;

#include <string>
using std::string;

#include <GL/glew.h>
#include <GL/glext.h>


class Shader
{
    public:
        Shader();
        Shader(const std::string& vsFileName, const std::string& fsFileName);
        ~Shader();

        void init(const std::string& vsFileName, const std::string& fsFileName);
        void Bind();
        void unbind();

        unsigned int id();
        bool inited;                    // Whether or not we have initialized the shader

    private:
        struct hShader
        {
            unsigned int id;
            string source;
        };

        hShader hVertexShader;          // A vertex shader manipulates vertices and allows access to vertex normals, lighting, object materials, etc.
        hShader hFragmentShader;        // A fragment shader manipulates pixels and create effects such as bloom, lighting, texturing, fog, etc.
        GLuint hShaderProgram;

        string readFile(const string& filename);
        bool compileShader(const Shader::hShader& shader);
        void echoShaderLog(unsigned int shaderID);
};

#endif // SHADER_H
