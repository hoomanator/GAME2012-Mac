#include "Window.h"
#include "Shader.h"
#include "raymath.h"
#include <imgui/imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
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

int main()
{
    //glfwSetErrorCallback(glfw_error_callback);
    CreateWindow(800, 800, "Graphics 1");


    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    
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

    int object_index = 0;

    GLint u_color = glGetUniformLocation(a1_tri_shader, "u_color");
    //GLint u_world = glGetUniformLocation(a1_tri_shader, "u_world");
    GLint u_mvp = glGetUniformLocation(a1_tri_shader, "u_mvp");

    // Note that we must cast to float to prevent truncation due to integer division
    float aspect = WindowWidth() / (float)WindowHeight();
    float near = 0.01f;
    float far = 100.0f;
    
    // Scale our triangle by a factor of 5, then translate it 5 units forward/"out of the screen" (OpenGL is an RHS so -z = "into the screen")
    Matrix world = MatrixScale(5.0f, 5.0f, 1.0f) * MatrixRotateZ(0.0f * DEG2RAD) *  MatrixTranslate(0.0, 0.0f, 5.0f);
    
    Vector3 camPos { 0.0f, 0.0f, 10.0f };
    Matrix view = MatrixLookAt(camPos, camPos - Vector3UnitZ, Vector3UnitY);

    // Perspective = 3D projection (closer objects = bigger, farther objects = smaller)
    //Matrix proj = MatrixPerspective(75.0f * DEG2RAD, aspect, near, far);

    // Orthographic = 2D projection (objects are the same size regardless of distance from camera)
    Matrix proj = MatrixOrtho(-10.0f, 10.0f, -10.0f, 10.0f, near, far);

    Matrix mvp = world * view * proj;

    //Whether or not to show the ImGui demo window
    bool imGUIDemo = false;

    // Generally you want to Scale * Rotate * Translate (order matters)!!!
    //world = MatrixRotateZ(30.0f * DEG2RAD) * MatrixTranslate(0.5f, 0.0f, 0.0f);

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

        /* Render here */
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (IsKeyPressed(KEY_SPACE))
        {
            ++object_index %= 6;
        }

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
            view = MatrixLookAt(camPos, camPos - Vector3UnitZ, Vector3UnitY);
            mvp = world * view * proj;
            glUniform3f(u_color, 1.0f, 0.0f, 0.0f);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp));
            glDrawArrays(GL_TRIANGLES, 0, 3);

            // Green triangle (further from the camera)
            world = MatrixTranslate(0.0f, 0.0f, 5.0f);
            view = MatrixLookAt(camPos, camPos - Vector3UnitZ, Vector3UnitY);
            mvp = world * view * proj;
            glUniform3f(u_color, 0.0f, 1.0f, 0.0f);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp));
            glDrawArrays(GL_TRIANGLES, 0, 3);
            break;

        case 1:
            glUseProgram(a1_tri_shader);
            world = MatrixTranslate(0.0f, 0.0f, 5.0f);
            view = MatrixLookAt(camPos, camPos - Vector3UnitZ, Vector3UnitY);
            mvp = world * view * proj;

            glUniform3f(u_color, 0.8, 0.8f, 0.8f);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp));
            glBindVertexArray(vertex_array_rainbow);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            break;

        case 2:
            glUseProgram(a1_tri_shader);

            world = MatrixTranslate(0.0f, 0.0f, 5.0f);
            view = MatrixLookAt(camPos, camPos - Vector3UnitZ, Vector3UnitY);
            mvp = world * view * proj;
            glUniform3f(u_color, 0.6, 0.6f, 0.6f);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp));
            glBindVertexArray(vertex_array_rainbow);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            break;

        case 3:
        {
            glUseProgram(a1_tri_shader);

            world = MatrixTranslate(0.0f, 0.0f, 5.0f);
            view = MatrixLookAt(camPos, camPos - Vector3UnitZ, Vector3UnitY);
            mvp = world * view * proj;

            Matrix s = MatrixScale(5.0f, 5.0f, 1.0f);
            Matrix r = MatrixRotateZ(tt * 100.f * DEG2RAD);
            Matrix t = MatrixTranslate(0.0f, 0.0f, 0.0f);
            Matrix mvp1 = s * r * t * view * proj;
            glUniform3f(u_color, 0.6, 0.6f, 0.6f);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp1));
            glBindVertexArray(vertex_array_rainbow);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            break;
        }
        case 4:
        {
            Matrix s4 = MatrixIdentity();
            Matrix r4 = MatrixIdentity();
            Matrix t4 = MatrixIdentity();
            Matrix view4 = MatrixLookAt(camPos, camPos - Vector3UnitZ, Vector3UnitY);
            Matrix proj4 = MatrixOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 0.01f, 100.0f);


            float time = Time();
            float t = cosf(time) * 0.5 + 0.5f;
            Vector3 A = { -1.0f, 0.0f, 0.0f };
            Vector3 B = { 1.0f, 0.0f, 0.0f };
            Vector3 C = Vector3Lerp(A, B, t);
            t4 = MatrixTranslate(C.x, C.y, C.z);

            Matrix mvp4 = s4 * r4 * t4 * view4 * proj4;

            glUseProgram(a1_tri_shader);
            glUniform3f(u_color, 0.5, 0.5f, 0.5f);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp4));
            glBindVertexArray(vertex_array_rainbow);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            break;
        }

        case 5:
        {

            Matrix view5 = MatrixLookAt(camPos, camPos - Vector3UnitZ, Vector3UnitY);
            Matrix proj5 = MatrixOrtho(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 100.0f);


            float time = Time();
            float a = cosf(time) * 0.5 + 0.5f;
            Vector3 tA = { -10.0f, 0.0f, 0.0f };
            Vector3 tB = { 10.0f, 0.0f, 0.0f };
            Vector3 tC = Vector3Lerp(tA, tB, a);

            Vector3 sA = { 1.0f, 1.0f, 1.0f };
            Vector3 sB = { 10.0f, 10.0f, 1.0f };
            Vector3 sC = Vector3Lerp(sA, sB, a);

            //Spherical Lerp because we interpolate between two quaternions (rotations) instead of two vectors (positions)
            Quaternion qA = QuaternionIdentity();
            Quaternion qB = QuaternionFromEuler(0.0f, 0.0f, 90.0f * DEG2RAD);
            Quaternion qC = QuaternionSlerp(qA, qB, a);

            Matrix s5 = MatrixScale(sC.x, sC.y, sC.z);
            Matrix r5 = QuaternionToMatrix(qC);
            Matrix t5 = MatrixTranslate(tC.x, tC.y, tC.z);

            Vector3 cA = Vector3UnitY;
            Vector3 cB = Vector3UnitZ;
            Vector3 cC = Vector3Lerp(cA, cB, a);

            Matrix mvp5 = s5 * r5 * t5  * view5 * proj5;

            glUseProgram(a1_tri_shader);
            glUniform3f(u_color, cC.x, cC.y, cC.z);
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp5));
            glBindVertexArray(vertex_array_rainbow);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            break;  
        }
        }


        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (imGUIDemo)
            ImGui::ShowDemoWindow();
        
        ImGui::SliderFloat3("Camera Position", &camPos.x, -10.f, 10.f);    
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


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


// Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    DestroyWindow();
    return 0;
}
