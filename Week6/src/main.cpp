#include "Window.h"
#include "Shader.h"
#include "raymath.h"
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

int main()
{
    // How to form the vertices for the 2nd square:
    Vector2 line_vertex_positions2[8];

    line_vertex_positions2[0] = Vector2Lerp(line_vertex_positions[0], line_vertex_positions[1], 0.5f);
    line_vertex_positions2[1] = Vector2Lerp(line_vertex_positions[2], line_vertex_positions[3], 0.5f);

    line_vertex_positions2[2] = Vector2Lerp(line_vertex_positions[2], line_vertex_positions[3], 0.5f);
    line_vertex_positions2[3] = Vector2Lerp(line_vertex_positions[4], line_vertex_positions[5], 0.5f);

    line_vertex_positions2[4] = Vector2Lerp(line_vertex_positions[4], line_vertex_positions[5], 0.5f);
    line_vertex_positions2[5] = Vector2Lerp(line_vertex_positions[6], line_vertex_positions[7], 0.5f);
    
    line_vertex_positions2[6] = Vector2Lerp(line_vertex_positions[6], line_vertex_positions[7], 0.5f);
    line_vertex_positions2[7] = Vector2Lerp(line_vertex_positions[0], line_vertex_positions[1], 0.5f);
    // (For full marks, you need to automate this with loops or recursion for 8 iterations [meaning 8 squares])

    CreateWindow(800, 800, "Graphics 1");
    
    // Hint: The a1_triangle shaders handle vertex position AND vertex colour.
    // Vertex colour is needed in order to receive full marks on this assignment!
    GLuint a2_lines_vert = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/a2_lines.vert");
    GLuint a2_lines_frag = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/a2_lines.frag");
    GLuint a2_lines_shader = CreateProgram(a2_lines_vert, a2_lines_frag);

    GLuint vbo_line_positions;
    glGenBuffers(1, &vbo_line_positions);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_line_positions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertex_positions), line_vertex_positions, GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertex_positions2), line_vertex_positions2, GL_STATIC_DRAW);
    // Comment/uncomment to see 1st square vs 2nd square.
    // Your job is to automate the generation of squares so all 8 squares are rendered at once!

    GLuint vao_line;
    glGenVertexArrays(1, &vao_line);
    glBindVertexArray(vao_line);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_line_positions);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2), 0);

    glBindVertexArray(GL_NONE);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_ESCAPE))
            SetWindowShouldClose(true);

        // Note: change MatrixOrtho's left/right/bottom/top values to modify the extents of your screen!
        Matrix proj = MatrixOrtho(-1.1f, 1.1f, -1.1f, 1.1f, 0.01f, 100.0f);
        Matrix view = MatrixLookAt({ 0.0f, 0.0f, 10.0f }, { 0.0f, 0.0f, 0.0f }, Vector3UnitY);
        Matrix world = MatrixIdentity();
        Matrix mvp = world * view * proj;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(5.0f);
        glBindVertexArray(vao_line);

        BeginShader(a2_lines_shader);
        {
            SendMat4(mvp, "u_mvp");
            SendVec3(Vector3UnitX, "u_color");
            glDrawArrays(GL_LINES, 0, line_vertex_count);
        }
        EndShader();

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
