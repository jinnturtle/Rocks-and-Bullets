#version 330 core

layout(location = 0) in vec3 model_verts;

out vec3 fragment_color;

uniform mat4 projection; // projection matrix
uniform mat4 view;       // view matrix
uniform mat4 transform;  // transform matrix
uniform vec3 color;

void main() {
	gl_Position =
        projection * view * transform * vec4(model_verts, 1.0f);

    fragment_color = color;
}
