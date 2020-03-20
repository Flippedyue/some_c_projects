#ifndef shader_h
#define shader_h

#include <GL/glew.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"

class Shader{
public:
    GLuint ID;
    Shader(const GLchar* vertexPath,const GLchar* fragmentPath,const GLchar*geometryPath=nullptr);
    void use();
    void close();
    void setBool(const std::string &name,bool value)const;
    void setInt(const std::string &name,int value)const;
    void setFloat(const std::string &name,float value)const;
    void setVec3(const std::string &name,float x,float y,float z)const;
    void setVec3(const std::string &name,glm::vec3 value)const;
    void setMat4(const std::string &name,const glm::mat4& matrix)const;
    void deleteShader();
};

GLuint generalTex(const std::string&);

#endif /* shader_h */
