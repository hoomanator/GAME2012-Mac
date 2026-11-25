#version 330 core

in vec3 position;
in vec3 normal;

out vec4 FragColor;

uniform samplerCube u_cubemap;
uniform vec3 u_cameraPosition;

void main()
{
    vec3 I = normalize(position - u_cameraPosition);
    vec3 R = reflect(I, normalize(normal));

    // Make sure our Cubemap textures are ready for both RGB and RGBA formats
    vec3 col = texture(u_cubemap, R).rgb;
    FragColor = vec4(col, 1.0);
}
