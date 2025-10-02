#version 330 core

out vec4 color;

in vec3 frag_color;
in vec2 frag_tex_coord;

// texture sampler
uniform sampler2D sampler1;

void main()
{
	color = texture(sampler1, frag_tex_coord) * vec4(frag_color, 1.0f);
}
