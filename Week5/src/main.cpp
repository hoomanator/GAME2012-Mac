#include "Window.h"
#include "Shader.h"
#include "raymath.h"
#include <imgui/imgui.h>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <ctime>

struct Vertex
{
    Vector2 pos;   // offset of 0
    Vector3 col;   // offset of 8 (4 bytes for pos.x + 4 bytes for pos.y = 8)
};

// Assignment 1 object 1 -- white triangle (change these vertex colours from red to white)!!!
static const Vertex vertices_white[3] =
{
    { { -0.6f, -0.4f }, { 1.0f, 1.0f, 1.0f } },
    { {  0.6f, -0.4f }, { 1.0f, 1.0f, 1.0f } },
    { {   0.f,  0.6f }, { 1.0f, 1.0f, 1.0f } }
};

// Assignment 1 object 2 -- rainbow triangle (done for you)
//static const Vertex vertices_rainbow[3] =
//{
//    { { -0.6f, -0.4f }, { 1.0f, 0.0f, 0.0f } },
//    { {  0.6f, -0.4f }, { 0.0f, 1.0f, 0.0f } },
//    { {   0.f,  0.6f }, { 0.0f, 0.0f, 1.0f } }
//};

static const Vector2 vertex_positions[3] =
{
    { -0.6f, -0.4f },
    { 0.6f, -0.4f },
    { 0.f,  0.6f }
};

static const Vector3 vertex_colors[3] =
{
    { 1.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f }
};

enum ProjectionType : int
{
    PROJ_ORTHOGRAPHIC,
    PROJ_PERSPECTIVE
};

int main()
{
    CreateWindow(800, 800, "Graphics 1");
    
    GLuint a1_tri_vert = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/a1_triangle.vert");
    GLuint a1_tri_frag = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/a1_triangle.frag");
    GLuint a1_tri_shader = CreateProgram(a1_tri_vert, a1_tri_frag);

    GLuint vertex_buffer_rainbow_positions;
    GLuint vertex_buffer_rainbow_colors;
    glGenBuffers(1, &vertex_buffer_rainbow_positions);
    glGenBuffers(1, &vertex_buffer_rainbow_colors);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_rainbow_positions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_positions), vertex_positions, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_rainbow_colors);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_colors), vertex_colors, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    // Can only have 1 vertex buffer bound at a time, so must unbind in order to prevent overwriting it

    GLuint vertex_buffer_white;
    glGenBuffers(1, &vertex_buffer_white);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_white);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_white), vertices_white, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    GLuint vertex_array_rainbow;
    glGenVertexArrays(1, &vertex_array_rainbow);
    glBindVertexArray(vertex_array_rainbow);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_rainbow_positions);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2), 0);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_rainbow_colors);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), 0);

    glBindVertexArray(GL_NONE);

    GLuint vertex_array_white;
    glGenVertexArrays(1, &vertex_array_white);
    glBindVertexArray(vertex_array_white);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_white);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, col));

    glBindVertexArray(GL_NONE);

    GLint u_color = glGetUniformLocation(a1_tri_shader, "u_color");
    //GLint u_world = glGetUniformLocation(a1_tri_shader, "u_world");
    GLint u_mvp = glGetUniformLocation(a1_tri_shader, "u_mvp");

    // Note that we must cast to float to prevent truncation due to integer division
    float aspect = WindowWidth() / (float)WindowHeight();
    float fov = 75.0f * DEG2RAD;

    float left = -1.0f;
    float right = 1.0f;
    float bottom = -1.0f;
    float top = 1.0f;

    float near = 0.01f;
    float far = 100.0f;
    int proj_type = PROJ_PERSPECTIVE;

    bool obj_translate = false;
    bool obj_rotate = false;
    bool obj_scale = false;
    int object_index = 1;

    /* Loop until the user closes the window */
    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_ESCAPE))
            SetWindowShouldClose(true);

        // Colors are represented as fractions between 0.0 and 1.0, so convert using a colour-picker tool accordingly!
        float r = 239.0f / 255.0f;
        float g = 136.0f / 255.0f;
        float b = 190.0f / 255.0f;
        float a = 1.0f;

        // Time in seconds since GLFW was initialized (use this with functions like sinf and cosf for repeating animations)
        float tt = Time();

        Matrix view = MatrixLookAt({ 0.0f, 0.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, Vector3UnitY);

        Matrix world = MatrixIdentity();
        if (obj_scale)
            world = world * MatrixScale(cosf(tt) * 0.4f + 0.5f, sinf(tt) * 0.4f + 0.5f, 1.0f);
        if (obj_rotate)
            world = world * MatrixRotateZ(tt * 100.0f * DEG2RAD);
        if (obj_translate)
            world = world * MatrixTranslate(cosf(tt), 0.0f, 0.0f);
        
        Matrix proj = MatrixIdentity();
        switch (proj_type)
        {
        case PROJ_ORTHOGRAPHIC:
            proj = MatrixOrtho(left, right, bottom, top, near, far);
            break;

        case PROJ_PERSPECTIVE:
            proj = MatrixPerspective(fov, aspect, near, far);
            break;
        }

        /* Render here */
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (IsKeyPressed(KEY_SPACE))
        {
            ++object_index %= 5;
        }

        Matrix mvp = world * view * proj;
        switch (object_index)
        {
        case 0:
            // Both triangles use the same vertex data and the same shader
            glUseProgram(a1_tri_shader);
            glBindVertexArray(vertex_array_white);

            // If we disable depth-testing, whichever triangle we draw last will be visible
            // (Meaning the green triangle will render "on top of" the red triangle despite being behind the red triangle)
            //glDisable(GL_DEPTH_TEST);

            // Red triangle (closer to the camera)
            world = MatrixTranslate(0.0f, 0.0f, 9.0f);
            mvp = world * view * proj;
            glUniform3f(u_color, 1.0f, 0.0f, 0.0f);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp));
            glDrawArrays(GL_TRIANGLES, 0, 3);

            // Green triangle (further from the camera)
            world = MatrixTranslate(0.0f, 0.0f, 5.0f);
            mvp = world * view * proj;
            glUniform3f(u_color, 0.0f, 1.0f, 0.0f);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp));
            glDrawArrays(GL_TRIANGLES, 0, 3);
            break;

        case 1:
            // 1) Bind static resources such as shader (behaviour) and vertex array object (data)
            glUseProgram(a1_tri_shader);
            glBindVertexArray(vertex_array_white);

            // 2) Bind dynamic resources such as uniforms
            glUniform3f(u_color, 1.0f, 1.0f, 1.0f);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp));

            // 3) Tell GPU to render (draw call)
            glDrawArrays(GL_TRIANGLES, 0, 3);
            break;

        case 2:
            glUseProgram(a1_tri_shader);
            glBindVertexArray(vertex_array_white);

            glUniform3f(u_color, 1.0f, 1.0f, 1.0f);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp));

            glDrawArrays(GL_TRIANGLES, 0, 3);
            break;

        case 3:
            glUseProgram(a1_tri_shader);
            glBindVertexArray(vertex_array_white);

            glUniform3f(u_color, 1.0f, 1.0f, 1.0f);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp));

            glDrawArrays(GL_TRIANGLES, 0, 3);
            break;

        case 4:
            glUseProgram(a1_tri_shader);
            glBindVertexArray(vertex_array_white);

            glUniform3f(u_color, 0.5, 0.5f, 0.5f);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp));

            glDrawArrays(GL_TRIANGLES, 0, 3);
            break;
        }

        // Write our widget code within begin/end gui loop!
        BeginGui();
        ImGui::RadioButton("Orthographic", &proj_type, PROJ_ORTHOGRAPHIC); ImGui::SameLine();
        ImGui::RadioButton("Perspective", &proj_type, PROJ_PERSPECTIVE);

        ImGui::Checkbox("Translate", &obj_translate);
        ImGui::Checkbox("Rotate", &obj_rotate);
        ImGui::Checkbox("Scale", &obj_scale);

        ImGui::SliderFloat("Left", &left, -10.0f, 0.0f);
        ImGui::SliderFloat("Right", &right, 0.0f, 10.0f);
        ImGui::SliderFloat("Bottom", &bottom, -10.0f, 0.0f);
        ImGui::SliderFloat("Top", &top, 0.0f, 10.0f);

        ImGui::SliderAngle("FoV", &fov, 5.0f, 175.0f);
        ImGui::SliderFloat("Near", &near, 0.001f, 1.0f);
        ImGui::SliderFloat("Far", &far, 1.0f, 1000.0f);

        // Extra practice: add additional widgets such sliders for camera position & camera direction!
        //ImGui::ShowDemoWindow(nullptr);
        EndGui();

        // Called at end of the frame to swap buffers and update input
        Loop();
    }

    glDeleteVertexArrays(1, &vertex_array_rainbow);
    glDeleteVertexArrays(1, &vertex_array_white);
    glDeleteBuffers(1, &vertex_buffer_rainbow_positions);
    glDeleteBuffers(1, &vertex_buffer_rainbow_colors);
    glDeleteBuffers(1, &vertex_buffer_white);
    glDeleteProgram(a1_tri_shader);
    glDeleteShader(a1_tri_frag);
    glDeleteShader(a1_tri_vert);

    DestroyWindow();
    return 0;
}
