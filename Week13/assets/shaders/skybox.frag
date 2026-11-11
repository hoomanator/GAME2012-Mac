#version 330 core

in vec3 position;
out vec4 FragColor;
uniform samplerCube u_cubemap;

void main()
{
    // Make sure our Cubemap textures are ready for both RGB and RGBA formats
    vec3 col = texture(u_cubemap, position).rgb;
    FragColor = vec4(col, 1.0);
}
