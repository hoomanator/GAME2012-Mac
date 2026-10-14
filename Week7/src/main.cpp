#include "Window.h"
#include "Shader.h"
#include "Buffer.h"

#include <imgui/imgui.h>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <ctime>

// 4 lines in 1 square -- each line has 2 vertices, therefore 8 array elements because 4 lines * 2 vertices per line = 8
// Hint: if 1 square is 8 vertices, and Assignment 2 requires you to render 8 squares, then 8 squares * 8 vertices per square = 64 vertices;
// (Consider reserving 64 vertices worth of space if you'd like to fit all your positions in a single vertex array)

static const int line_vertex_count = 8;
static const Vector2 line_vertex_positions[line_vertex_count]
{
    { -1.0f,  -1.0f },   // bottom-left
    {  1.0f,  -1.0f },   // bottom-right

    {  1.0f, -1.0f },   // bottom-right
    {  1.0f,  1.0f },   // top-right

    {   1.0f,  1.0f },   // top-right
    {  -1.0f,  1.0f },   // top-left

    { -1.0f,   1.0f },   // top-left
    { -1.0f,  -1.0f }    // bottom-left
};

static const int square_count = 8;
static const int square_vertex_count = square_count * line_vertex_count;

int main()
{
    // Square 0 is the outer square. Each following square connects the midpoints of the previous square's 4 edges.
    // Edge e of a square is stored as vertices [2e, 2e + 1], so the new square's edge e runs from the
    // midpoint of the previous edge e to the midpoint of the previous edge (e + 1) % 4.
    Vector2 square_vertex_positions[square_vertex_count];
    for (int i = 0; i < line_vertex_count; i++)
        square_vertex_positions[i] = line_vertex_positions[i];

    for (int s = 1; s < square_count; s++)
    {
        const Vector2* prev = square_vertex_positions + (s - 1) * line_vertex_count;
        Vector2* curr = square_vertex_positions + s * line_vertex_count;
        for (int e = 0; e < 4; e++)
        {
            int next = (e + 1) % 4;
            curr[e * 2 + 0] = Vector2Lerp(prev[e * 2], prev[e * 2 + 1], 0.5f);
            curr[e * 2 + 1] = Vector2Lerp(prev[next * 2], prev[next * 2 + 1], 0.5f);
        }
    }

    CreateWindow(800, 800, "Week7");
    
    GLuint a2_lines_vert = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/a2_lines.vert");
    GLuint a2_lines_frag = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/a2_lines.frag");
    GLuint a2_lines_shader = CreateProgram(a2_lines_vert, a2_lines_frag);

    GLuint vbo_line_positions = GenVertexBuffer();
    BindVertexBuffer(vbo_line_positions);
    UpdateVertexBuffer(vbo_line_positions, (void*)square_vertex_positions, sizeof(square_vertex_positions));
    UnbindVertexBuffer();

    GLuint vao_line = GenVertexArray();
    BindVertexArray(vao_line);

    EnableVertexAttribute(0);
    BindVertexBuffer(vbo_line_positions);
    SetVertexAttribute(0, 2, GL_FLOAT, sizeof(Vector2));

    UnbindVertexArray();
    UnbindVertexBuffer();

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_ESCAPE))
            SetWindowShouldClose(true);

        Matrix proj = MatrixOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 0.01f, 100.0f);
        Matrix view = MatrixLookAt({ 0.0f, 0.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, Vector3UnitY);
        Matrix world = MatrixIdentity();
        Matrix mvp = world * view * proj;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(5.0f);
        BindVertexArray(vao_line);
        BeginShader(a2_lines_shader);
        {
            SendMat4(mvp, "u_mvp");
            SendVec3(Vector3UnitX, "u_color");
            glDrawArrays(GL_LINES, 0, square_vertex_count);
        }
        EndShader();
        UnbindVertexArray();
        glLineWidth(1.0f);

        BeginGui();
        //ImGui::ShowDemoWindow(nullptr);
        EndGui();

        Loop();
    }

    glDeleteVertexArrays(1, &vao_line);
    glDeleteBuffers(1, &vbo_line_positions);
    glDeleteProgram(a2_lines_shader);
    glDeleteShader(a2_lines_frag);
    glDeleteShader(a2_lines_vert);

    DestroyWindow();
    return 0;
}
