#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_tex_coord;

out vec3 frag_color;
out vec2 frag_tex_coord;

uniform mat4 projection; // projection matrix
uniform mat4 view;       // view matrix
uniform mat4 transform;  // transform matrix
uniform vec3 color;

void main()
{
    gl_Position =
        projection * view * transform * vec4(in_pos, 1.0f);

    frag_color = color;
    frag_tex_coord = in_tex_coord;
}
