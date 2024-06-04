#include "shader_class.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    //get source code from file
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    //exception stuff
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        //open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        //read files
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        //close files
        vShaderFile.close();
        fShaderFile.close();
        //convert stream to string
        vertexCode   = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    //compile shaders
    unsigned int vertex, fragment;

    //vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    //print errors
    error_log(&vertex, GL_COMPILE_STATUS);

    //fragment shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    //print errors
    error_log(&fragment, GL_COMPILE_STATUS);

    //shader program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    //print errors
    error_log(&ID, GL_LINK_STATUS);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use()
{
    glUseProgram(ID);
}

void Shader::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void error_log(unsigned int* shader_var, int status_type)
{
    int success;
    if (status_type == GL_COMPILE_STATUS)
    {
        glGetShaderiv(*shader_var, status_type, &success);
    }
    else if (status_type == GL_LINK_STATUS)
    {
        glGetProgramiv(*shader_var, status_type, &success);
    }

    char infoLog[512];
    if (!success) {
        const char* fail_type = NULL;
        if (status_type == GL_COMPILE_STATUS)
        {
            fail_type = "COMPILE";
            glGetShaderInfoLog(*shader_var, 512, NULL, infoLog);
        }
        else if (status_type == GL_LINK_STATUS)
        {
            fail_type = "PROGRAM";
            glGetProgramInfoLog(*shader_var, 512, NULL, infoLog);
        }
        std::cout << "ERROR::SHADER::" << fail_type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}
