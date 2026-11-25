#version 330 core

in vec2 tcoord;

uniform sampler2D u_tex1;
uniform sampler2D u_tex2;

out vec4 FragColor;

void main()
{
    //FragColor = vec4(tcoord, 0.0, 1.0);
    FragColor = texture(u_tex1, tcoord) * texture(u_tex2, tcoord);
}
