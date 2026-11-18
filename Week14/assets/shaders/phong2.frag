#version 330 core

in vec3 position;
in vec3 normal;

out vec4 FragColor;

uniform vec3 u_lightPosition;
uniform vec3 u_lightColor;
uniform float u_lightRadius;

uniform vec3 u_cameraPosition;
uniform vec3 u_lightDirection;
uniform float u_ambientFactor;
uniform float u_diffuseFactor;
uniform float u_specularPower;

vec3 phong(vec3 position, vec3 normal, vec3 camera, vec3 light, vec3 color, float ambientFactor, float diffuseFactor, float specularPower)
{
    vec3 N = normalize(normal); 
    vec3 L = normalize(u_lightPosition - position);   
    vec3 V = normalize(camera - position);
    vec3 R = normalize(reflect(-L,N));
    float dotNL = max(dot(N, L), 0.0);
    float dotVR = max(dot(V, R), 0.0);

    vec3 lighting = vec3(0.0);
    vec3 ambient = ambientFactor * color;
    vec3 diffuse = diffuseFactor * dotNL * color;
    vec3 specular = pow(dotVR, specularPower) * color;

    lighting += ambient;
    lighting += diffuse;
    lighting += specular;

    return lighting;    
}

vec3 point_light(vec3 position, vec3 normal, vec3 camera, vec3 light, vec3 color, float ambientFactor, float diffuseFactor, float specularPower, float radius)
{
    vec3 lighting = phong(position, normal, u_cameraPosition, u_lightPosition, u_lightColor, u_ambientFactor, u_diffuseFactor, u_specularPower);
    float dist= length(light - position);
    float attenuation = clamp(radius / dist, 0.0, 1.0);

    lighting *= attenuation;
    return lighting;
}

vec3 directional_light(vec3 direction, vec3 normal, vec3 camera, vec3 color, float ambientFactor, float diffuseFactor, float specularPower)
{
    vec3 lighting = phong(vec3(0.0), normal, u_cameraPosition, -direction, u_lightColor, u_ambientFactor, u_diffuseFactor, u_specularPower);

    return lighting;
}

vec3 spot_light(vec3 position, vec3 direction, vec3 normal, vec3 camera, vec3 light, vec3 color, float ambientFactor, float diffuseFactor, float specularPower, float radius, float fov)
{
    //TODO -- figure this out for yourself 
    vec3 lighting = vec3(0.0);
    return lighting;
}


void main()
{
    vec3 lighting = directional_light(u_lightDirection, normal, u_cameraPosition, u_lightColor, u_ambientFactor, u_diffuseFactor, u_specularPower);
    // TODO -- test spot light and multiple lights
    FragColor = vec4(lighting, 1.0);
}
