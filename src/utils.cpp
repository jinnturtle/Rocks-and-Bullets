#include "utils.hpp"

#include <GL/glew.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "logs.hpp"

// TODO - this is quick'n'dirty, rewrite properly
GLuint load_shaders(
    const char* vertex_file_path,
    const char* fragment_file_path)
{
    GLuint vertex_shader_ID {glCreateShader(GL_VERTEX_SHADER)};
    GLuint fragment_shader_ID {glCreateShader(GL_FRAGMENT_SHADER)};

    // read the Vertex Shader code from the file
    std::string vertex_shader_code;
    std::ifstream vertex_shader_stream(vertex_file_path, std::ios::in);
    if(vertex_shader_stream.is_open()){
        std::stringstream sstr;
        sstr << vertex_shader_stream.rdbuf();
        vertex_shader_code = sstr.str();
        vertex_shader_stream.close();
    } else {
        logs::err("can not open ", vertex_file_path);
        return 0;
    }

    // read the Fragment Shader code from the file
    std::string fragment_shader_code;
    std::ifstream fragment_shader_stream(fragment_file_path, std::ios::in);
    if(fragment_shader_stream.is_open()){
        std::stringstream sstr;
        sstr << fragment_shader_stream.rdbuf();
        fragment_shader_code = sstr.str();
        fragment_shader_stream.close();
    } else {
        logs::err("can not open ", fragment_file_path);
        return 0;
    }

    GLint result {GL_FALSE};
    int info_log_length;

    // compile Vertex Shader
    logs::info("compiling vertex shader: ", vertex_file_path);
    char const* vertex_shader_code_p {vertex_shader_code.c_str()};
    glShaderSource(vertex_shader_ID, 1, &vertex_shader_code_p, NULL);
    glCompileShader(vertex_shader_ID);

    // check Vertex Shader
    glGetShaderiv(vertex_shader_ID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertex_shader_ID, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        std::vector<char> err_msg(info_log_length+1);
        glGetShaderInfoLog(
            vertex_shader_ID,
            info_log_length,
            nullptr,
            &err_msg[0]);
        logs::err("could not compile vertex shader:\n", &err_msg[0]);
    }

    // compile Fragment Shader
    logs::info("compiling fragment shader: ", fragment_file_path);
    char const* fragment_shader_code_p {fragment_shader_code.c_str()};
    glShaderSource(fragment_shader_ID, 1, &fragment_shader_code_p , nullptr);
    glCompileShader(fragment_shader_ID);

    // check Fragment Shader
    glGetShaderiv(fragment_shader_ID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragment_shader_ID, GL_INFO_LOG_LENGTH, &info_log_length);
    if ( info_log_length > 0 ){
        std::vector<char> err_msg(info_log_length+1);
        glGetShaderInfoLog(
            fragment_shader_ID,
            info_log_length,
            nullptr,
            &err_msg[0]);
        logs::err("could not compile fragment shader:\n", &err_msg[0]);
    }

    // link the program
    DBG(1, "linking shader program");
    GLuint program_ID {glCreateProgram()};
    if (program_ID == 0) {
        logs::err("could not create shader program");
        return 0;
    }
    glAttachShader(program_ID, vertex_shader_ID);
    glAttachShader(program_ID, fragment_shader_ID);
    glLinkProgram(program_ID);

    // check the program
    glGetProgramiv(program_ID, GL_LINK_STATUS, &result);
    glGetProgramiv(program_ID, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0){
        std::vector<char> err_msg(info_log_length+1);
        glGetProgramInfoLog(
            program_ID,
            info_log_length,
            nullptr,
            &err_msg[0]);
        logs::err("could not link shader program:\n", &err_msg[0]);
    }

    glDetachShader(program_ID, vertex_shader_ID);
    glDetachShader(program_ID, fragment_shader_ID);

    glDeleteShader(vertex_shader_ID);
    glDeleteShader(fragment_shader_ID);

    return program_ID;
}
