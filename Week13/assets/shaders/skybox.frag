#version 330 core

in vec3 position;
out vec4 FragColor;
uniform samplerCube u_cubemap;

void main()
{
    FragColor = texture(u_cubemap, position);
}
