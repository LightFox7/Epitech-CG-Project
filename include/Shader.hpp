#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <GL/glew.h>

class Shader
{
public:
    GLuint program;
    std::vector<GLuint> shaders;

    // Contructor, creates program
    Shader() { program = 0; }
    // Destructor, deletes program, detaches any remaining shad
    ~Shader() {
        DestroyShaders();
        glDeleteProgram(program);
    }
    // Detaches and deletes shaders, removes them from the shader list
    void DestroyShaders() {
        for (GLuint it : shaders) {
            glDetachShader(program, it);
            glDeleteShader(it);
        }
        shaders.clear();
    }
    // Loads shader into program
    bool LoadShader(std::string shaderPath, GLenum shaderType) {
        std::string shaderCode;
        std::ifstream shaderFile;
        shaderFile.exceptions(std::ifstream::badbit);
        try {
            // Open files
            shaderFile.open(shaderPath);
            std::stringstream shaderStream;
            // Read file's buffer contents into streams
            shaderStream << shaderFile.rdbuf();
            // close file handler
            shaderFile.close();
            // Convert stream into string
            shaderCode = shaderStream.str();
        } catch (std::ifstream::failure e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << shaderPath << std::endl;
            return false;
        }
        const GLchar* code = shaderCode.c_str();
        return CompileShader(code, shaderType);
    }
    // Compiles and attaches given shader
    bool CompileShader(const GLchar *shaderCode, GLenum shaderType) {
        GLint success;
        GLchar infoLog[512];
        GLuint shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &shaderCode, nullptr);
        glCompileShader(shader);
        // Print compile errors if any
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
            return false;
        }
        return AttachShader(shader);
    }
    bool AttachShader(GLuint shader) {
        shaders.push_back(shader);
        return true;
    }
    bool LinkProgram() {
        GLint success;
        GLchar infoLog[512];
        // Link program
        this->program = glCreateProgram();
        std::cout << "shader count: " << shaders.size() << std::endl;
        for (auto it: shaders)
            glAttachShader(program, it);
        glLinkProgram(program);
        DestroyShaders();
        // Print linking errors if any
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            return false;
        }
        // Delete the shaders as they're linked into our program now and no longer necessary
        return true;
    }
    // Uses the current shader
    void Use()
    {
        glUseProgram(program);
    }
};