#include "Window.h"
#include "Shader.h"
#include "Buffer.h"
#include "Mesh.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "raymath.h"
#include <imgui/imgui.h>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <random>

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 800;



static const int line_vertex_count = 8;
static const Vector2 line_vertex_positions[line_vertex_count]{
	{ -1.0f, -1.0f }, // bottom-left
	{ 1.0f, -1.0f },  // bottom-right

	{ 1.0f, -1.0f }, // bottom-right
	{ 1.0f, 1.0f },  // top-right

	{ 1.0f, 1.0f },  // top-right
	{ -1.0f, 1.0f }, // top-left

	{ -1.0f, 1.0f }, // top-left
	{ -1.0f, -1.0f } // bottom-left
};

struct Vertex
{
	Vector2 pos;   // offset of 0
	Vector3 col;   // offset of 8 (4 bytes for pos.x + 4 bytes for pos.y = 8)
};

// Assignment 1 object 1 -- white triangle (change these vertex colours from red to white)!!!
static const Vertex vertices_white[3] =
{
	{ { -0.6f, -0.4f },{ 1.0f, 1.0f, 1.0f } },
	{ { 0.6f, -0.4f },{ 1.0f, 1.0f, 1.0f } },
	{ { 0.f,  0.6f },{ 1.0f, 1.0f, 1.0f } }
};


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

struct Pixel
{
	stbi_uc r = 255;
	stbi_uc g = 255;
	stbi_uc b = 255;
	stbi_uc a = 255;
};

GLuint CreateSkybox(const char *skyboxFaces[6])
{

    // const char *skyboxFaces[6] = {
    //     "./assets/textures/iceriver/posx.bmp",
    //     "./assets/textures/iceriver/negx.bmp",
    //     "./assets/textures/iceriver/posy.bmp",
    //     "./assets/textures/iceriver/negy.bmp",
    //     "./assets/textures/iceriver/posz.bmp",
    //     "./assets/textures/iceriver/negz.bmp"};
    GLuint texSkybox = GL_NONE;
    glGenTextures(1, &texSkybox);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texSkybox);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_set_flip_vertically_on_load(false); // Don't flip skybox textures, as they are already oriented correctly
    for (int i = 0; i < 6; i++)
    {
        int width, height, channels;
        stbi_uc *pixels = stbi_load(skyboxFaces[i], &width, &height, &channels, 0);
        if (pixels)
        {
            GLint internalFormat = (channels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, internalFormat, GL_UNSIGNED_BYTE, pixels);
            stbi_image_free(pixels);
        }
        else
        {
            printf("Failed to load skybox texture: %s\n", skyboxFaces[i]);
            stbi_image_free(pixels);
        }
    }
    stbi_set_flip_vertically_on_load(true);

    return texSkybox;
}

int main()
{

	// Calculates new vertices of nested squares based on the midpoints
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


	CreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Week13 - Environment Mapping");


	//shaders

	GLuint a1_tri_vert = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/a1_triangle.vert");
	GLuint a1_tri_frag = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/a1_triangle.frag");
	GLuint a1_tri_shader = CreateProgram(a1_tri_vert, a1_tri_frag);

	///////////////////////////////LINE SHADER //////////////////////////////

	GLuint a2_lines_vert = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/a2_lines.vert");
	GLuint a2_lines_frag = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/a2_lines.frag");
	GLuint a2_lines_shader = CreateProgram(a2_lines_vert, a2_lines_frag);

	// Vertex shaders:
	GLuint vs = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/default.vert");
	GLuint vsLines = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/lines.vert");
	GLuint vsVertexPositionColor = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/vertex_color.vert");
	GLuint vsColorBufferColor = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/buffer_color.vert");
	GLuint vsSkybox = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/skybox.vert");

	// Fragment shaders:
	GLuint fsLines = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/lines.frag");
	GLuint fsUniformColor = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/uniform_color.frag");
	GLuint fsVertexColor = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/vertex_color.frag");
	GLuint fsTcoords = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/tcoord_color.frag");
	GLuint fsNormals = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/normal_color.frag");
	GLuint fsTexture = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/texture_color.frag");
	GLuint fsTextureMix = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/texturemix_color.frag");
	GLuint fsSkybox = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/skybox.frag");
	GLuint fsPhong = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/phong.frag");
    GLuint fsPhong2 = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/phong2.frag");
	GLuint fsReflect = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/reflect.frag");
    GLuint fsRefract = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/refract.frag");


	// Shader programs:
	GLuint shaderUniformColor = CreateProgram(vs, fsUniformColor);
	GLuint shaderVertexPositionColor = CreateProgram(vsVertexPositionColor, fsVertexColor);
	GLuint shaderVertexBufferColor = CreateProgram(vsColorBufferColor, fsVertexColor);
	GLuint shaderLines = CreateProgram(vsLines, fsLines);
	GLuint shaderTcoords = CreateProgram(vs, fsTcoords);
	GLuint shaderNormals = CreateProgram(vs, fsNormals);
	GLuint shaderTexture = CreateProgram(vs, fsTexture);
	GLuint shaderTextureMix = CreateProgram(vs, fsTextureMix);
	GLuint shaderSkybox = CreateProgram(vsSkybox, fsSkybox);
	GLuint shaderPhong = CreateProgram(vs, fsPhong);
    GLuint shaderPhong2 = CreateProgram(vs, fsPhong2);
	GLuint shaderReflect = CreateProgram(vs, fsReflect);
    GLuint shaderRefract = CreateProgram(vs, fsRefract);

	// Add the triangle shaders
	GLuint triangleVs = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/vertex_color.vert");
	GLuint triangleFs = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/vertex_color.frag");
	GLuint shaderTriangle = CreateProgram(triangleVs, triangleFs);

	// Add point shader
	GLuint vsPoints = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/points.vert");
	GLuint shaderPoints = CreateProgram(vsPoints, fsVertexColor);




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


	//////////////////////// Render 1 line for every horizontal pixel ///////////////

	std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> start_y(-1.0f, 1.0f);
	std::uniform_real_distribution<float> end_y(-1.0f, 1.0f);
	std::uniform_real_distribution<float> col_r(0.0f, 1.0f);

	std::vector<Line> lines(SCREEN_WIDTH);
	for (int i = 0; i < SCREEN_WIDTH; i++)
	{
		Line& line = lines[i];
		float x = Remap(i, 0, SCREEN_WIDTH, -1.0f, 1.0f);
		line.start.x = lines[i].end.x = x;

		lines[i].start.y = start_y(rng);
		lines[i].end.y = end_y(rng);
		//printf("Line %d: start=(%f, %f), end=(%f, %f)\n", i, lines[i].start.x, lines[i].start.y, lines[i].end.x, lines[i].end.y);
	}

	std::vector<Vector3> linesColor(SCREEN_WIDTH * 2);
	for (int i = 0; i < SCREEN_WIDTH * 2; i++)
	{
		linesColor[i] = Vector3{ col_r(rng), col_r(rng), col_r(rng) };
	}


	//////////////////////////// Create Textures & Load texture to GPU ///////

	stbi_set_flip_vertically_on_load(true); // Flip the image vertically when loading, because OpenGL expects the 0.0 coordinate on the y-axis to be at the bottom of the image)

	int carWidth, carHeight, carChannels = 0;
	stbi_uc* pixelsCar = stbi_load("assets/textures/ct4_grey.png", &carWidth, &carHeight, &carChannels, 0);

	GLuint carTexture = GL_NONE;
	glGenTextures(1, &carTexture);
	glBindTexture(GL_TEXTURE_2D, carTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, carWidth, carHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsCar);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
	free(pixelsCar);
	pixelsCar = nullptr;

	/////////////////////////////////// Head Texture //////////////////////////////////////////

	int headWidth, headHeight, headChannels = 0;
	stbi_uc* pixelsHead = stbi_load("assets/textures/head.png", &headWidth, &headHeight, &headChannels, 0);

	GLuint headTexture = GL_NONE;
	glGenTextures(1, &headTexture);
	glBindTexture(GL_TEXTURE_2D, headTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, headWidth, headHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsHead);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
	free(pixelsHead);
	pixelsHead = nullptr;

	///////////////////////////////// Gradient Texture ////////////////////////////

	int  texGradientWidth = 256;
	int  texGradientHeight = 256; 
	Pixel* pixelsGradient = (Pixel*)malloc(texGradientWidth * texGradientHeight * sizeof(Pixel));
	for (int y = 0; y < texGradientHeight; y++)
	{
		for (int x = 0; x < texGradientWidth; x++)
		{

			float u = x / (float)(texGradientWidth);
			float v = y / (float)(texGradientHeight);

			Pixel pixel;

			pixel.r = u * 255.0f;
			pixel.g = v * 255.0f;
			pixel.b = 255;
			pixel.a = 255;

			pixelsGradient[y * texGradientWidth + x] = pixel; 
		}
	}

	GLuint texGradient = GL_NONE;
	glGenTextures(1, &texGradient);
	glBindTexture(GL_TEXTURE_2D, texGradient);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texGradientWidth, texGradientHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsGradient);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, GL_NONE);
	free(pixelsGradient);
	pixelsGradient = nullptr;


	///// skybox texture
	/*const char* skyboxFaces[6] =
	{
		"./assets/textures/skybox/sky_x+.png",
		"./assets/textures/skybox/sky_x-.png",
		"./assets/textures/skybox/sky_y+.png",
		"./assets/textures/skybox/sky_y-.png",
		"./assets/textures/skybox/sky_z+.png",
		"./assets/textures/skybox/sky_z-.png"
	};*/

	const char* skyboxFaces[6] =
	{
		"./assets/textures/iceriver/posx.bmp",
		"./assets/textures/iceriver/negx.bmp",
		"./assets/textures/iceriver/posy.bmp",
		"./assets/textures/iceriver/negy.bmp",
		"./assets/textures/iceriver/posz.bmp",
		"./assets/textures/iceriver/negz.bmp"
	};

	GLuint skyboxTexture = GL_NONE;
	glGenTextures(1, &skyboxTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(false);
	for (int i = 0; i < 6; i++)
	{
		int width, height, channels;
		stbi_uc* pixels = stbi_load(skyboxFaces[i], &width, &height, &channels, 0);
		if (pixels)
		{
			int format = (channels == 3) ? GL_RGB : GL_RGBA;
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
			stbi_image_free(pixels);
		}
		else
		{
			printf("Failed to load skybox texture: %s\n", skyboxFaces[i]);
		}
	}
	// Restore flipping only AFTER all 6 cubemap faces are loaded
	stbi_set_flip_vertically_on_load(true);


	////////////////////////////////////////////////////////////////////////////

	// Create Mesh
	//An OBJ file stores the physical 3D geometry of a model (such as vertices, faces, normals, and UV coordinates), while an MTL (Material Template Library) file stores the appearance and surface properties (such as colors, shininess, transparency, and links to texture images)
	//git add -f *.obj  (your gitignore file may be ignoring .obj files, so you may need to force add them to your repo)
	Mesh shapeMesh, cubeMesh, sphereMesh, knotMesh, planeMesh, headMesh, rockMesh, torusMesh, ct4Mesh, dynoMesh;
	CreateMesh(&shapeMesh, PLANE);
	CreateMesh(&cubeMesh, CUBE);
	CreateMesh(&sphereMesh, SPHERE);
	CreateMesh(&knotMesh, KNOT);
	CreateMesh(&rockMesh, ROCK);
	CreateMesh(&planeMesh, "assets/meshes/plane.obj");
	CreateMesh(&headMesh, "assets/meshes/head.obj");
	CreateMesh(&torusMesh, TORUS);
	CreateMesh(&ct4Mesh, "assets/meshes/ct4.obj");
	CreateMesh(&dynoMesh, "assets/meshes/dynotex.obj");


	//////////////////Lighting///////////////////////////
	Vector3 lightPositionDir = {10.0f, 10.0f, 10.0f};
    Vector3 lightColor = {1.0f, 0.6471f, 0.0f};
    float lightRadiusDir = 1.0f;

    Vector3 lightPosition = {2.0f, 0.0f, 2.0f};
    float lightRadius = 1.0f;
    float ambientFactor = 0.1f;
    float diffuseFactor = 0.8f;
    float specularPower = 32.0f;
    float refractionIndex = 1.52f; // Glass

    // Case 4: positions of the reflective/refractive objects and which one the keyboard controls
    Vector3 reflectPosition = { -2.0f, 0.0f, 0.0f };
    Vector3 refractPosition = {  2.0f, 0.0f, 0.0f };
    int controlledObject = 0; // 0 = reflective, 1 = refractive (Tab toggles)
    float objectMoveSpeed = 3.0f;

	/////////////////////////////////////////////

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_LINE_SMOOTH);


	//////////////////////////////////////////////////////////////////////


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

	Vector3 cameraPosition = { 0.f, 0.f, 3.f };

	// Mouse-look camera (hold RIGHT mouse button and drag to look around)
	float cameraYaw = -90.0f * DEG2RAD;	// -90 degrees = looking down -Z
	float cameraPitch = 0.0f;
	float mouseSensitivity = 0.003f;	// radians per pixel
	float cameraMoveSpeed = 3.0f;		// units per second

	/* Loop until the user closes the window */
	while (!WindowShouldClose())
	{
		if (IsKeyPressed(KEY_ESCAPE))
			SetWindowShouldClose(true);

		if (IsKeyPressed(KEY_SPACE))
		{
			++object_index %= 10;
		}


		// Colors are represented as fractions between 0.0 and 1.0, so convert using a colour-picker tool accordingly!
		float r = 239.0f / 255.0f;
		float g = 136.0f / 255.0f;
		float b = 190.0f / 255.0f;
		float a = 1.0f;

		// Time in seconds since GLFW was initialized (use this with functions like sinf and cosf for repeating animations)

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

		// Mouse look: use ImGui's mouse state (ignored while the mouse is over an ImGui window)
		ImGuiIO& io = ImGui::GetIO();
		if (!io.WantCaptureMouse && ImGui::IsMouseDown(ImGuiMouseButton_Right))
		{
			cameraYaw += io.MouseDelta.x * mouseSensitivity;
			cameraPitch -= io.MouseDelta.y * mouseSensitivity;

			// Clamp pitch so the camera can't flip over the top/bottom
			const float maxPitch = 89.0f * DEG2RAD;
			if (cameraPitch > maxPitch) cameraPitch = maxPitch;
			if (cameraPitch < -maxPitch) cameraPitch = -maxPitch;
		}

		Vector3 cameraForward =
		{
			cosf(cameraPitch) * cosf(cameraYaw),
			sinf(cameraPitch),
			cosf(cameraPitch) * sinf(cameraYaw)
		};

		// WASD movement relative to where the camera is looking (Q/E = down/up, hold Shift = faster)
		if (!io.WantCaptureKeyboard)
		{
			Vector3 cameraRight = Vector3Normalize(Vector3CrossProduct(cameraForward, Vector3UnitY));
			float speed = cameraMoveSpeed * io.DeltaTime;
			if (IsKeyDown(KEY_LEFT_SHIFT)) speed *= 3.0f;

			if (IsKeyDown(KEY_W)) cameraPosition += cameraForward * speed;
			if (IsKeyDown(KEY_S)) cameraPosition -= cameraForward * speed;
			if (IsKeyDown(KEY_D)) cameraPosition += cameraRight * speed;
			if (IsKeyDown(KEY_A)) cameraPosition -= cameraRight * speed;
			if (IsKeyDown(KEY_E)) cameraPosition += Vector3UnitY * speed;
			if (IsKeyDown(KEY_Q)) cameraPosition -= Vector3UnitY * speed;

			// Case 4 object controls: Tab = switch object, I/K = forward/back, J/L = left/right, U/O = down/up
			// Movement is relative to the camera's facing direction (flattened onto the ground plane)
			if (object_index == 4)
			{
				if (IsKeyPressed(KEY_TAB)) controlledObject = 1 - controlledObject;

				Vector3& objPos = (controlledObject == 0) ? reflectPosition : refractPosition;
				Vector3 flatForward = Vector3Normalize({ cameraForward.x, 0.0f, cameraForward.z });
				float objSpeed = objectMoveSpeed * io.DeltaTime;
				if (IsKeyDown(KEY_LEFT_SHIFT)) objSpeed *= 3.0f;

				if (IsKeyDown(KEY_I)) objPos += flatForward * objSpeed;
				if (IsKeyDown(KEY_K)) objPos -= flatForward * objSpeed;
				if (IsKeyDown(KEY_L)) objPos += cameraRight * objSpeed;
				if (IsKeyDown(KEY_J)) objPos -= cameraRight * objSpeed;
				if (IsKeyDown(KEY_O)) objPos += Vector3UnitY * objSpeed;
				if (IsKeyDown(KEY_U)) objPos -= Vector3UnitY * objSpeed;
			}
		}

		Matrix view = MatrixLookAt(cameraPosition, cameraPosition + cameraForward, Vector3UnitY);

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

		/* Render here */
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		switch (object_index)
		{
		case 0:
		{
			glUseProgram(shaderSkybox);
			GLint u_mvp = glGetUniformLocation(shaderSkybox, "u_mvp");
			Matrix world2 = MatrixIdentity(); // static skybox (no spinning)

			Matrix viewSky = view;
			viewSky.m12 = viewSky.m13 = viewSky.m14 = 0.0f;

			// Skybox needs a perspective projection (orthographic flattens it into a single face)
			Matrix projSky = MatrixPerspective(fov, aspect, near, far);

			// Use the translation-free view so the skybox stays centered on the camera
			Matrix mvp2 = world2 * viewSky * projSky;
			glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp2));

			GLint u_cubemap = glGetUniformLocation(shaderSkybox, "u_cubemap");
			glUniform1i(u_cubemap, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
			glDepthMask(GL_FALSE); // Disable depth writing for skybox
			DrawMesh(cubeMesh);
			glDepthMask(GL_TRUE);

			break;
		}
		case 1:
		{
			glUseProgram(shaderTexture);

			GLint u_mvp = glGetUniformLocation(shaderTexture, "u_mvp");
			Matrix world2 = MatrixRotateY(100.0f * tt * DEG2RAD);
			Matrix mvp2 = world2 * view * proj;
			glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp2));

			GLint u_texture = glGetUniformLocation(shaderTexture, "u_tex");
			glUniform1i(u_texture, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, headTexture);
			DrawMesh(headMesh);
			break;		
		}
		case 2:
		{
			glUseProgram(shaderTexture);

			GLint u_mvp = glGetUniformLocation(shaderTexture, "u_mvp");
			Matrix world2 = MatrixRotateY(100.0f * tt * DEG2RAD);
			Matrix mvp2 = world2 * view * proj;
			glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp2));

			GLint u_texture = glGetUniformLocation(shaderTexture, "u_tex");
			glUniform1i(u_texture, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texGradient);
			DrawMesh(headMesh);
			break;
		}
		case 3:
		{
			glUseProgram(shaderTextureMix);

			GLint u_mvp = glGetUniformLocation(shaderTextureMix, "u_mvp");
			Matrix world2 = MatrixRotateY(100.0f * tt * DEG2RAD);
			Matrix mvp2 = world2 * view * proj;
			glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp2));

			GLint u_texture1 = glGetUniformLocation(shaderTextureMix, "u_tex1");
			glUniform1i(u_texture1, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texGradient);

			GLint u_texture2 = glGetUniformLocation(shaderTextureMix, "u_tex2");
			glUniform1i(u_texture2, 1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, headTexture);

			DrawMesh(headMesh);
			break;
		}

		case 4:
		{
			//skybox + environment mapping
            //Add an imgui slider float from 1.0 to 3.0 that changes the reflective index (u_ration)
            // Add FPS camera to see all 6 faces of the skybox
            // Add FPS controls to objects & make a key to cycle between controlling reflected and refracted object
            			glUseProgram(shaderSkybox);
			GLint u_mvp = glGetUniformLocation(shaderSkybox, "u_mvp");
			Matrix world2 = MatrixIdentity(); // static skybox (no spinning)

			Matrix viewSky = view;
			viewSky.m12 = viewSky.m13 = viewSky.m14 = 0.0f;

			// Skybox needs a perspective projection (orthographic flattens it into a single face)
			Matrix projSky = MatrixPerspective(fov, aspect, near, far);

			// Use the translation-free view so the skybox stays centered on the camera
			Matrix mvp2 = world2 * viewSky * projSky;
			glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp2));

			GLint u_cubemap = glGetUniformLocation(shaderSkybox, "u_cubemap");
			glUniform1i(u_cubemap, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
			glDepthMask(GL_FALSE); // Disable depth writing for skybox
			DrawMesh(cubeMesh);
			glDepthMask(GL_TRUE);

            // The skybox cube map is still bound to texture unit 0, so both shaders sample it from there.

            // Reflective cube (left)
            glUseProgram(shaderReflect);
            world = MatrixTranslate(reflectPosition.x, reflectPosition.y, reflectPosition.z);
            mvp = world * view * proj;
            glUniformMatrix4fv(glGetUniformLocation(shaderReflect, "u_world"), 1, GL_FALSE, MatrixToFloat(world));
            glUniformMatrix4fv(glGetUniformLocation(shaderReflect, "u_mvp"), 1, GL_FALSE, MatrixToFloat(mvp));
            glUniform3f(glGetUniformLocation(shaderReflect, "u_cameraPosition"), cameraPosition.x, cameraPosition.y, cameraPosition.z);
            glUniform1i(glGetUniformLocation(shaderReflect, "u_cubemap"), 0);
            DrawMesh(cubeMesh);

            // Refractive cube (right)
            glUseProgram(shaderRefract);
            world = MatrixTranslate(refractPosition.x, refractPosition.y, refractPosition.z);
            mvp = world * view * proj;
            glUniformMatrix4fv(glGetUniformLocation(shaderRefract, "u_world"), 1, GL_FALSE, MatrixToFloat(world));
            glUniformMatrix4fv(glGetUniformLocation(shaderRefract, "u_mvp"), 1, GL_FALSE, MatrixToFloat(mvp));
            glUniform3f(glGetUniformLocation(shaderRefract, "u_cameraPosition"), cameraPosition.x, cameraPosition.y, cameraPosition.z);
            glUniform1i(glGetUniformLocation(shaderRefract, "u_cubemap"), 0);
            glUniform1f(glGetUniformLocation(shaderRefract, "u_refractiveRatio"), 1.0f / refractionIndex); // air (1.0) -> material
            DrawMesh(cubeMesh);
            break;
		}
		
		case 5:
		{
			glUseProgram(shaderTcoords);

			GLint u_mvp = glGetUniformLocation(shaderTcoords, "u_mvp");
			// Torus is ~1.1 units in radius, so shrink it to fit and tilt/spin it to show its shape
			Matrix world2 = MatrixScale(0.8f, 0.8f, 0.8f) * MatrixRotateX(60.0f * DEG2RAD) * MatrixRotateY(100.0f * tt * DEG2RAD);
			Matrix mvp2 = world2 * view * proj;
			glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp2));

			DrawMesh(torusMesh);
			break;

		}

		case 6:
		{
			glUseProgram(shaderNormals);

			GLint u_mvp = glGetUniformLocation(shaderNormals, "u_mvp");
			Matrix world2 = MatrixRotateY(100.0f * tt * DEG2RAD) * MatrixRotateX(100.0f * tt * DEG2RAD);
			Matrix mvp2 = world2 * view * proj;
			glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp2));

			DrawMesh(cubeMesh);
			break;
		}

		case 7:
		{
			glUseProgram(shaderTexture);

			GLint u_mvp = glGetUniformLocation(shaderTexture, "u_mvp");
			// Car is 2.5 units long and sits on y = 0..0.75, so center it and scale it to fit the view
			Matrix world2 = MatrixTranslate(0.0f, -0.375f, 0.0f) * MatrixScale(0.8f, 0.8f, 0.8f) * MatrixRotateY(100.0f * tt * DEG2RAD);
			Matrix mvp2 = world2 * view * proj;
			glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp2));

			GLint u_texture = glGetUniformLocation(shaderTexture, "u_tex");
			glUniform1i(u_texture, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, carTexture);
			DrawMesh(ct4Mesh);
			break;
		}

		case 8:
		{
			glUseProgram(shaderPhong2);

            mvp = world * view * proj;

            GLint u_world = glGetUniformLocation(shaderPhong2, "u_world");
            glUniformMatrix4fv(u_world, 1, GL_FALSE, MatrixToFloat(world));

            // Uniform scale preserves the direction of the right, up, and forward vectors, so we can safely use the normal matrix without worrying about scaling issues.
            // Non-uniform scale can distort the normals, so we need to ensure that the normal matrix is correctly calculated to account for any scaling transformations applied to the model.

            Matrix normal = MatrixInvert(MatrixTranspose(world));
            GLint u_normal = glGetUniformLocation(shaderPhong2, "u_normal");
            glUniformMatrix4fv(u_normal, 1, GL_FALSE, MatrixToFloat((normal)));

            GLint u_mvp = glGetUniformLocation(shaderPhong2, "u_mvp");
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp));

            GLint u_lightPosition = glGetUniformLocation(shaderPhong2, "u_lightPosition");
            glUniform3fv(u_lightPosition, 1, (float *)&lightPosition);

            GLint u_lightColor = glGetUniformLocation(shaderPhong2, "u_lightColor");
            glUniform3fv(u_lightColor, 1, (float *)&lightColor);

            GLint u_lightRadius = glGetUniformLocation(shaderPhong2, "u_lightRadius");
            glUniform1f(u_lightRadius, lightRadius);

            GLint u_cameraPosition = glGetUniformLocation(shaderPhong2, "u_cameraPosition");
            glUniform3fv(u_cameraPosition, 1, (float *)&cameraPosition);

            GLint u_ambientFactor = glGetUniformLocation(shaderPhong2, "u_ambientFactor");
            glUniform1f(u_ambientFactor, ambientFactor);

            GLint u_diffuseFactor = glGetUniformLocation(shaderPhong2, "u_diffuseFactor");
            glUniform1f(u_diffuseFactor, diffuseFactor);
            
            GLint u_specularPower = glGetUniformLocation(shaderPhong2, "u_specularPower");
            glUniform1f(u_specularPower, specularPower);

            DrawMesh(sphereMesh);
            break;
		}

		case 9:
		{
			glUseProgram(shaderPhong2);

            mvp = world * view * proj;

            GLint u_world = glGetUniformLocation(shaderPhong2, "u_world");
            glUniformMatrix4fv(u_world, 1, GL_FALSE, MatrixToFloat(world));

            // Uniform scale preserves the direction of the right, up, and forward vectors, so we can safely use the normal matrix without worrying about scaling issues.
            // Non-uniform scale can distort the normals, so we need to ensure that the normal matrix is correctly calculated to account for any scaling transformations applied to the model.

            Matrix normal = MatrixInvert(MatrixTranspose(world));
            GLint u_normal = glGetUniformLocation(shaderPhong2, "u_normal");
            glUniformMatrix4fv(u_normal, 1, GL_FALSE, MatrixToFloat((normal)));

            GLint u_mvp = glGetUniformLocation(shaderPhong2, "u_mvp");
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp));

            GLint u_lightPosition = glGetUniformLocation(shaderPhong2, "u_lightPosition");
            glUniform3fv(u_lightPosition, 1, (float *)&lightPosition);

            GLint u_lightColor = glGetUniformLocation(shaderPhong2, "u_lightColor");
            glUniform3fv(u_lightColor, 1, (float *)&lightColor);

            GLint u_lightRadius = glGetUniformLocation(shaderPhong2, "u_lightRadius");
            glUniform1f(u_lightRadius, lightRadius);

            GLint u_cameraPosition = glGetUniformLocation(shaderPhong2, "u_cameraPosition");
            glUniform3fv(u_cameraPosition, 1, (float *)&cameraPosition);

            GLint u_ambientFactor = glGetUniformLocation(shaderPhong2, "u_ambientFactor");
            glUniform1f(u_ambientFactor, ambientFactor);

            GLint u_diffuseFactor = glGetUniformLocation(shaderPhong2, "u_diffuseFactor");
            glUniform1f(u_diffuseFactor, diffuseFactor);
            
            GLint u_specularPower = glGetUniformLocation(shaderPhong2, "u_specularPower");
            glUniform1f(u_specularPower, specularPower);

            DrawMesh(sphereMesh);

			//To render our light as a color sphere
            glUseProgram(shaderUniformColor);
            Matrix world2 = MatrixScale(lightRadius, lightRadius, lightRadius) * MatrixTranslate(lightPosition.x, lightPosition.y, lightPosition.z);;
            Matrix mvp2 = world2 * view * proj;
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, MatrixToFloat(mvp2));
            normal = MatrixIdentity();
            GLint u_color = glGetUniformLocation(shaderUniformColor, "u_color");
            glUniform3f(u_color, 1.0f, 1.0f, 1.0f);
            GLint u_intensity = glGetUniformLocation(shaderUniformColor, "u_intensity");
            glUniform1f(u_intensity, 1.0f);
            //Draw our light as a sphere
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            DrawMesh(sphereMesh);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
		}

		}

		// Write our widget code within begin/end gui loop!
		BeginGui();
		ImGui::RadioButton("Orthographic", &projection_type, ORTHOGRAPHIC); ImGui::SameLine();
		ImGui::RadioButton("Perspective", &projection_type, PERSPECTIVE);

		ImGui::SliderFloat3("Camera Position", &cameraPosition.x, -10.0f, 10.0f);
		ImGui::Text("Hold right mouse button + drag to look around");
		ImGui::Text("WASD = move, Q/E = down/up, Shift = faster");
		ImGui::SliderFloat("Move Speed", &cameraMoveSpeed, 0.5f, 20.0f);
		if (ImGui::Button("Reset Camera"))
		{
			cameraPosition = { 0.f, 0.f, 3.f };
			cameraYaw = -90.0f * DEG2RAD;
			cameraPitch = 0.0f;
		}

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

		ImGui::SliderFloat3("Light Color", &lightColor.x, 0.0f, 1.0f);
        ImGui::SliderFloat3("Orbit Light Position", &lightPosition.x, -10.0f, 10.0f);
        ImGui::SliderFloat("Orbit Light Radius", &lightRadius, 0.25f, 10.0f);
        ImGui::SliderFloat("Refractive Index", &refractionIndex, 1.0f, 3.0f);

        ImGui::Separator();
        ImGui::Text("Case 4 objects: Tab = switch, I/K/J/L = move, U/O = down/up");
        ImGui::RadioButton("Control Reflective", &controlledObject, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Control Refractive", &controlledObject, 1);
        ImGui::SliderFloat3("Reflective Position", &reflectPosition.x, -10.0f, 10.0f);
        ImGui::SliderFloat3("Refractive Position", &refractPosition.x, -10.0f, 10.0f);
        ImGui::SliderFloat("Object Move Speed", &objectMoveSpeed, 0.5f, 20.0f);
        if (ImGui::Button("Reset Objects"))
        {
            reflectPosition = { -2.0f, 0.0f, 0.0f };
            refractPosition = {  2.0f, 0.0f, 0.0f };
        }


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