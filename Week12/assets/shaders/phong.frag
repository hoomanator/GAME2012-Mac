#version 330 core

in vec3 position;
in vec3 normal;

out vec4 FragColor;

uniform vec3 u_lightPosition;
uniform vec3 u_lightColor;

void main()
{
    vec3 N = normalize(normal);
    vec3 L = normalize(u_lightPosition - position);
    float dotNL = max(dot(N, L), 0.0);

    vec3 diffuse = u_lightColor * dotNL;
    //if you wanto to make the color based on its normal, you can use the following line instead of the above line
    //diffuse = (N + vec3(1.0)) * 0.5;
    FragColor = vec4(diffuse, 1.0);
}
