#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTcoord;

uniform mat4 u_mvp;
uniform mat4 u_world[100];
uniform mat4 u_normal;

//out vec3 position;
//out vec3 normal;
out vec2 tcoord;

mat4 buildTranslation(vec3 delta) {
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0), // Column 1
        vec4(0.0, 1.0, 0.0, 0.0), // Column 2
        vec4(0.0, 0.0, 1.0, 0.0), // Column 3
        vec4(delta, 1.0)          // Column 4 (tx, ty, tz, 1.0)
    );
}

void main()
{
   int id = gl_InstanceID;
   //position = (u_world * vec4(aPosition, 1.0)).xyz;
   // Remove the translation component from the normal matrix to ensure correct normal transformation
   //normal = (u_normal * vec4(aNormal, 0.0)).xyz;
   tcoord = aTcoord;

   mat4 world = u_world[id];

   mat4 t = buildTranslation(vec3(0.1 * id, 0.0, 0.0));
   //gl_Position = u_mvp * t * vec4(aPosition, 1.0);

   gl_Position = u_mvp * world * vec4(aPosition, 1.0);
}
