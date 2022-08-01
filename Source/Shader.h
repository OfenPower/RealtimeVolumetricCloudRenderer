#pragma once

#include "glm/glm.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

struct Shader
{
    Shader();
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);

    // activate the shader
    void use();

    // utility uniform functions
    //

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec2(const std::string& name, float x, float y) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setVec4(const std::string& name, float x, float y, float z, float w);
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;    
    void setMat4(const std::string& name, const glm::mat4& mat) const;

    // utility function for checking shader compilation/linking errors.
    void checkCompileErrors(unsigned int shader, std::string type);

    unsigned int ID;
};

