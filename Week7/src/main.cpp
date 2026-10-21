#include "Window.h"
#include "Shader.h"
#include "Buffer.h"

#include <imgui/imgui.h>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <random>

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 800;

// 4 lines in 1 square -- each line has 2 vertices, therefore 8 array elements because 4 lines * 2 vertices per line = 8
// Hint: if 1 square is 8 vertices, and Assignment 2 requires you to render 8 squares, then 8 squares * 8 vertices per square = 64 vertices;
// (Consider reserving 64 vertices worth of space if you'd like to fit all your positions in a single vertex array)

static const int line_vertex_count = 8;
static const Vector2 line_vertex_positions[line_vertex_count]{
    {-1.0f, -1.0f}, // bottom-left
    {1.0f, -1.0f},  // bottom-right

    {1.0f, -1.0f}, // bottom-right
    {1.0f, 1.0f},  // top-right

    {1.0f, 1.0f},  // top-right
    {-1.0f, 1.0f}, // top-left

    {-1.0f, 1.0f}, // top-left
    {-1.0f, -1.0f} // bottom-left
};

struct Vertex
{
    Vector2 pos; // offset of 0
    Vector3 col; // offset of 8 (4 bytes for pos.x + 4 bytes for pos.y = 8)
};

static const Vertex vertices_white[3] =
    {
        {{-0.6f, -0.4f}, {1.0f, 1.0f, 1.0f}},
        {{0.6f, -0.4f}, {1.0f, 1.0f, 1.0f}},
        {{0.f, 0.6f}, {1.0f, 1.0f, 1.0f}}};

static const Vector2 vertex_positions[3] =
    {
        {-0.6f, -0.4f},
        {0.6f, -0.4f},
        {0.f, 0.6f}};

static const Vector3 vertex_colors[3] =
    {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}};

static const int square_count = 8;
static const int square_vertex_count = square_count * line_vertex_count;

enum ProjectionType : int
{
    PERSPECTIVE = 0,
    ORTHOGRAPHIC = 1
};

struct Line
{
    Vector2 start;
    Vector2 end;
};

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
        const Vector2 *prev = square_vertex_positions + (s - 1) * line_vertex_count;
        Vector2 *curr = square_vertex_positions + s * line_vertex_count;
        for (int e = 0; e < 4; e++)
        {
            int next = (e + 1) % 4;
            curr[e * 2 + 0] = Vector2Lerp(prev[e * 2], prev[e * 2 + 1], 0.5f);
            curr[e * 2 + 1] = Vector2Lerp(prev[next * 2], prev[next * 2 + 1], 0.5f);
        }
    }

    CreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Week7");


    /////////////////////////// Triangle Shader //////////////////////////////////

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

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, col));

    glBindVertexArray(GL_NONE);



    ////////////////////////////////// Vetex with interleaved  attributes/////////////////////////////////

    GLuint vertex_buffer_white;
    glGenBuffers(1, &vertex_buffer_white);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_white);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_white), vertices_white, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    
    GLuint vertex_array_white;
    glGenVertexArrays(1, &vertex_array_white);
    glBindVertexArray(vertex_array_white);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_white);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, col));

    glBindVertexArray(GL_NONE);


    ///////////////////////////////LINE SHADER //////////////////////////////

    GLuint a2_lines_vert = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/a2_lines.vert");
    GLuint a2_lines_frag = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/a2_lines.frag");
    GLuint a2_lines_shader = CreateProgram(a2_lines_vert, a2_lines_frag);

    GLuint vbo_line_positions = GenVertexBuffer();
    BindVertexBuffer(vbo_line_positions);
    UpdateVertexBuffer(vbo_line_positions, (void *)square_vertex_positions, sizeof(square_vertex_positions));
    UnbindVertexBuffer();

    GLuint vao_line = GenVertexArray();
    BindVertexArray(vao_line);

    EnableVertexAttribute(0);
    BindVertexBuffer(vbo_line_positions);
    SetVertexAttribute(0, 2, GL_FLOAT, sizeof(Vector2));

    UnbindVertexArray();
    UnbindVertexBuffer();

    //////////////////////// Render 1 line for every horizontal pixel ///////////////

    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> start_y(-1.0f, 1.0f);
    std::uniform_real_distribution<float> end_y(-1.0f, 1.0f);

    std::vector<Line> lines(SCREEN_WIDTH);
    for (int i = 0; i < SCREEN_WIDTH; i++)
    {
        Line &line = lines[i];
        float x = Remap(i, 0, SCREEN_WIDTH, -1.0f, 1.0f);
        line.start.x = lines[i].end.x = x;

        lines[i].start.y = start_y(rng);
        lines[i].end.y = end_y(rng);
        printf("Line %d: start=(%f, %f), end=(%f, %f)\n", i, lines[i].start.x, lines[i].start.y, lines[i].end.x, lines[i].end.y);
    }

    GLuint vbo_lines;
    glGenBuffers(1, &vbo_lines);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_lines);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lines) * sizeof(Line) * SCREEN_WIDTH, lines.data(), GL_STATIC_DRAW);

    GLuint vao_lines;
    glGenVertexArrays(1, &vao_lines);
    glBindVertexArray(vao_lines);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_lines);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2), 0);

    glBindVertexArray(GL_NONE);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    ///////////////////////////////////////

    int object_index = 0;
    int projection_type = ORTHOGRAPHIC;

    float aspect = (float)WindowWidth() / (float)WindowHeight();
    float near = 0.1f;
    float far = 100.0f;
    float fov = 75.0f * DEG2RAD;

    float left = -1.1;
    float right = 1.1;
    float bottom = -1.1;
    float top = 1.1;

    bool obj_translate = false;
    bool obj_rotate = false;
    bool obj_scale = false;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_ESCAPE))
            SetWindowShouldClose(true);

        if (IsKeyPressed(KEY_SPACE))
        {
            ++object_index %= 5;
        }

        float tt = Time();

        Matrix world = MatrixIdentity();

        if (obj_translate)
        {
            world = MatrixTranslate(cosf(tt) * 0.4f + 0.5f, sin(tt) * 0.4f + 0.5f, 1.f);
        }

        if (obj_rotate)
        {
            world = MatrixRotateZ(tt * 100 * DEG2RAD);
        }

        if (obj_scale)
        {
            world = MatrixScale(cosf(tt) * 0.4f + 0.5f, cosf(tt) * 0.4f + 0.5f, 1.f);
        }

        Matrix view = MatrixLookAt({0.f, 0.f, 10.f}, {0.f, 0.f, 0.f}, Vector3UnitY);

        Matrix proj = MatrixIdentity();

        switch (projection_type)
        {
        case PERSPECTIVE:
            proj = MatrixPerspective(fov, aspect, near, far);
            break;
        case ORTHOGRAPHIC:
            proj = MatrixOrtho(left, right, bottom, top, near, far);
            break;
        }

        Matrix mvp = world * view * proj;

        // Colors are represented as fractions between 0.0 and 1.0, so convert using a colour-picker tool accordingly!
        float r = 239.0f / 255.0f;
        float g = 136.0f / 255.0f;
        float b = 90.0f / 255.0f;
        float a = 1.0f;

        /* Render here */
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        switch (object_index)
        {
        case 0:
        {
            glLineWidth(55.0f);
            BindVertexArray(vao_line);
            BeginShader(a2_lines_shader);
            {
                SendMat4(mvp, "u_mvp");
                SendVec3(Vector3UnitX, "u_color");
                glDrawArrays(GL_LINES, 0, square_vertex_count);
            }
            EndShader();
            UnbindVertexArray();
            //glLineWidth(1.0f);
            break;
        }

        case 1:
        {

            glLineWidth(15.0f);
            glBindVertexArray(vao_lines);

            BeginShader(a2_lines_shader);
            {
                SendMat4(mvp, "u_mvp");
                SendVec3(Vector3UnitX, "u_color");
                glDrawArrays(GL_LINES, 0, SCREEN_WIDTH * 2);
            }
            EndShader();
            break;
        }

        case 2:
        {

            glUseProgram(a1_tri_shader);
            GLint u_color = glGetUniformLocation(a1_tri_shader, "u_color");
            GLint u_mvp = glGetUniformLocation(a1_tri_shader, "u_mvp");
            // world = MatrixScale(2.f, 2.f, 2.f) * MatrixTranslate(5.f, 5.f, 0.f);
            mvp = world * view * proj;
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp));
            glUniform3f(u_color, 0.0f, 1.0f, 0.0f);
            glBindVertexArray(vertex_array_white);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            break;
        }

        case 3:
        {
            glUseProgram(a1_tri_shader);
            GLint u_color = glGetUniformLocation(a1_tri_shader, "u_color");
            GLint u_mvp = glGetUniformLocation(a1_tri_shader, "u_mvp");
            world = MatrixScale(2.f, 2.f, 2.f);
            mvp = world * view * proj;
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp));
            glUniform3f(u_color, 1.0f, 1.0f, 1.0f);
            glBindVertexArray(vertex_array_rainbow);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            break;
        }

        case 4:
        {

            glUseProgram(a1_tri_shader);
            GLint u_color = glGetUniformLocation(a1_tri_shader, "u_color");
            GLint u_mvp = glGetUniformLocation(a1_tri_shader, "u_mvp");

            Matrix view = MatrixLookAt({0.f, 0.f, 10.f}, {0.f, 0.f, 0.f}, {0.f, 1.f, 0.f});
            Matrix proj = MatrixOrtho(-10.f, 10.f, -10.f, 10.f, 0.1f, 100.f);

            float time = Time();
            float a = cosf(time) * 0.5f + 0.5f;

            // Translation Interpolation
            Vector3 tA = {-10.0f, 10.0f, 0.0f};
            Vector3 tB = {10.0f, -10.0f, 0.0f};
            Vector3 tC = Vector3Lerp(tA, tB, a);

            Vector3 sA = {1.0f, 1.0f, 1.0f};
            Vector3 sB = {5.0f, 5.0f, 1.0f};
            Vector3 sC = Vector3Lerp(sA, sB, a);

            Quaternion qA = QuaternionIdentity();
            Quaternion qB = QuaternionFromEuler(0.0f, 0.0f, 180.0f * DEG2RAD);
            Quaternion qC = QuaternionSlerp(qA, qB, a);

            Matrix s = MatrixScale(sC.x, sC.y, sC.z);
            Matrix r = QuaternionToMatrix(qC);
            Matrix t = MatrixTranslate(tC.x, tC.y, tC.z);

            Matrix world = s * r * t;
            Matrix mvp = world * view * proj;

            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp));

            glUniform3f(u_color, 1.0f, 1.0f, 1.0f);
            glBindVertexArray(vertex_array_rainbow);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            break;
        }

        default:
            break;
        }

        BeginGui();
        // ImGui::ShowDemoWindow(nullptr);
        ImGui::Text("This is some useful text.");
        ImGui::RadioButton("Perspective", &projection_type, PERSPECTIVE);
        ImGui::SameLine();
        ImGui::RadioButton("Orthographic", &projection_type, ORTHOGRAPHIC);

        ImGui::Checkbox("Translate", &obj_translate);
        ImGui::Checkbox("Rotate", &obj_rotate);
        ImGui::Checkbox("Scale", &obj_scale);

        ImGui::SliderFloat("Left", &left, -10.0f, 10.0f);
        ImGui::SliderFloat("Right", &right, -10.0f, 10.0f);
        ImGui::SliderFloat("Bottom", &bottom, -10.0f, 10.0f);
        ImGui::SliderFloat("Top", &top, -10.0f, 10.0f);

        ImGui::SliderFloat("Near", &near, 0.1f, 10.0f);
        ImGui::SliderFloat("Far", &far, 10.0f, 100.0f);
        ImGui::SliderAngle("FOV", &fov, 1.0f, 179.0f);

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
