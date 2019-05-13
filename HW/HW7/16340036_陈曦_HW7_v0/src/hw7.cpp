#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include "imgui_internal.h"
#include "imconfig.h"
#include "imstb_textedit.h"
#include "imstb_rectpack.h"
#include "imstb_truetype.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace ImGui;
using namespace glm;

// Properties
const unsigned int SCR_WIDTH = 800, SCR_HEIGHT = 600;

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void RenderScene(const unsigned int shaderProgram);
void RenderCube();
unsigned int createShader(const char *vertexShader, const char *fragment);

const char *vertexShaderSource = "#version 330 core\n"
"layout(location = 0) in vec3 position;\n"
"layout(location = 1) in vec3 normal;\n"
"layout(location = 2) in vec3 aColor;\n"

"out VS_OUT{\n"
"	vec3 FragPos;\n"
"	vec3 Normal;\n"
"	vec3 AColor;\n"
"	vec4 FragPosLightSpace;\n"
"} vs_out;\n"

"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"uniform mat4 model;\n"
"uniform mat4 lightSpaceMatrix;\n"

"void main()\n"
"{\n"
"	gl_Position = projection * view * model * vec4(position, 1.0f);\n"
"	vs_out.FragPos = vec3(model * vec4(position, 1.0));\n"
"	vs_out.Normal = transpose(inverse(mat3(model))) * normal;\n"
"	vs_out.AColor = aColor;\n"
"	vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);\n"
"}\0";

const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"

"in VS_OUT{\n"
"	vec3 FragPos;\n"
"	vec3 Normal;\n"
"	vec3 AColor;\n"
"	vec4 FragPosLightSpace;\n"
"} fs_in;\n"

"uniform sampler2D diffuseTexture;\n"
"uniform sampler2D shadowMap;\n"

"uniform vec3 lightPos;\n"
"uniform vec3 viewPos;\n"

"uniform bool shadows;\n"

"float ShadowCalculation(vec4 fragPosLightSpace)\n"
"{\n"
"	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;\n"
"	projCoords = projCoords * 0.5 + 0.5;\n"
"	float closestDepth = texture(shadowMap, projCoords.xy).r;\n"
"	float currentDepth = projCoords.z;\n"
"	vec3 normal = normalize(fs_in.Normal);\n"
"	vec3 lightDir = normalize(lightPos - fs_in.FragPos);\n"
"	float bias = max(0.05 * (1.0f - dot(normal, lightDir)), 0.005f);\n"
	// PCF
"	float shadow = 0.0;\n"
"	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);\n"
"	for (int x = -1; x <= 1; ++x)\n"
"	{\n"
"		for (int y = -1; y <= 1; ++y)\n"
"		{\n"
"			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;\n"
"			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;\n"
"		}\n"
"	}\n"
"	shadow /= 9.0;\n"

"	if (projCoords.z > 1.0)\n"
"		shadow = 0.0;\n"

"	return shadow;\n"
"}\n"

"void main()\n"
"{\n"
"	vec3 color = fs_in.AColor;\n"
"	vec3 normal = normalize(fs_in.Normal);\n"
"	vec3 lightColor = vec3(0.4);\n"
"	vec3 ambient = 0.2 * color;\n"
"	vec3 lightDir = normalize(lightPos - fs_in.FragPos);\n"
"	float diff = max(dot(lightDir, normal), 0.0);\n"
"	vec3 diffuse = diff * lightColor;\n"
"	vec3 viewDir = normalize(viewPos - fs_in.FragPos);\n"
"	float spec = 0.0;\n"
"	vec3 halfwayDir = normalize(lightDir + viewDir);\n"
"	spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);\n"
"	vec3 specular = spec * lightColor;\n"
"	float shadow = shadows ? ShadowCalculation(fs_in.FragPosLightSpace) : 0.0;\n"
"	shadow = min(shadow, 0.75);\n" 
"	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;\n"

"	FragColor = vec4(lighting, 1.0f);\n"
"}\n\0";

const char *vertexSimpleDepthShaderSource = "#version 330 core\n"
"layout(location = 0) in vec3 position;\n"

"uniform mat4 lightSpaceMatrix;\n"
"uniform mat4 model;\n"

"void main()\n"
"{\n"
"	gl_Position = lightSpaceMatrix * model * vec4(position, 1.0f);\n"
"}\0";

const char *fragmentSimpleDepthShaderSource = "#version 330 core\n"

"void main()\n"
"{\n"
	// gl_FragDepth = gl_FragCoord.z;
"}\n\0";


bool shadows = true;
unsigned int planeVAO;

int main() 
{
	glfwInit();//初始化GLFW
	//使用glfwWindowHint配置GLFW
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//设置主版本号
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//设置次版本号
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//使用核心模式


	//创建窗口对象
	GLFWwindow* window = glfwCreateWindow(800, 800, "LearnOpenGL", NULL, NULL);//窗口宽、高、窗口标题
	if (window == NULL)
	{
		cout << "Failed to create GLFW window!" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);//通知GLFW将窗口的上下文设置为当前线程的主上下文
	glfwSwapInterval(1);

	//初始化GLAD，GLAD用来管理OpenGL的函数指针，在调用任何OpenGL的函数之前需要初始化GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
		return -1;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//设置窗口维度
	glViewport(0, 0, 800, 800);//前两个参数控制窗口左下角的位置，三、四个参数控制渲染窗口的宽度和高度（像素）。

	glEnable(GL_DEPTH_TEST);

	//创建并绑定ImGui
	IMGUI_CHECKVERSION();
	CreateContext();
	ImGuiIO& io = GetIO(); (void)io;
	StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	unsigned int shaderProgram, simpleDepthShader;
	shaderProgram = createShader(vertexShaderSource, fragmentShaderSource);
	simpleDepthShader = createShader(vertexSimpleDepthShaderSource, fragmentSimpleDepthShaderSource);

	// Set texture samples
	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shaderProgram, "shadowMap"), 1);

	float planeVertices[] = {
		// Positions          // Normals         // Color
		25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 0.8f, 0.8f, 0.8f,
	   -25.0f, -0.5f,-25.0f, 0.0f, 1.0f, 0.0f, 0.8f, 0.8f, 0.8f,
	   -25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 0.8f, 0.8f, 0.8f,

		25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 0.8f, 0.8f, 0.8f,
		25.0f, -0.5f,-25.0f, 0.0f, 1.0f, 0.0f, 0.8f, 0.8f, 0.8f,
	   -25.0f, -0.5f,-25.0f, 0.0f, 1.0f, 0.0f, 0.8f, 0.8f, 0.8f,
	};
	// Setup plane VAO
	unsigned int planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	// Light source
	vec3 lightPos(-2.0f, 4.0f, -1.0f);

	// Configure depth map FBO
	const unsigned  int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned  int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// - Create depth texture
	unsigned  int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	bool orth = true;

	while (!glfwWindowShouldClose(window)) 
	{

		//输入
		processInput(window);

		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);//设置清屏所用颜色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//清空屏幕的颜色缓冲
		glEnable(GL_DEPTH_TEST);


		//创建ImGui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		NewFrame();

		Begin("Edit");
		Checkbox("orth", &orth);
		End();

		// 1. Render depth of scene to texture (from light's perspective)
		// - Get light projection/view matrix.
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		float near_plane = 1.0f, far_plane = 7.5f;
		if (orth) {
			lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		}
		else {
			lightProjection = glm::perspective(45.0f, (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
		}
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		lightSpaceMatrix = lightProjection * lightView;
		// - now render scene from light's point of view
		glUseProgram(simpleDepthShader);
		glUniformMatrix4fv(glGetUniformLocation(simpleDepthShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		RenderScene(simpleDepthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2. Render scene as normal 
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgram);
		glm::mat4 projection = glm::perspective(radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		mat4 view = lookAt(vec3(0.0f, 0.0f, 3.0f),
			vec3(0.0f, 0.0f, 2.0f),
			vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, &lightPos[0]);
		glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), 0.0f, 0.0f, 3.0f);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		glUniform1i(glGetUniformLocation(shaderProgram, "shadows"), shadows);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		RenderScene(shaderProgram);


		//渲染指令
		Render();
		int view_width, view_height;
		glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &view_width, &view_height);
		glViewport(0, 0, view_width, view_height);
		ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());

		//双缓冲
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	DestroyContext();

	glfwTerminate();
	return 0;
}


void RenderScene(const unsigned int shaderProgram)
{
	// Floor
	glm::mat4 model = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	// Cubes
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	RenderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(1.0f, 0.0f, -0.5f));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	RenderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -0.5f));
	model = glm::rotate(model, 60.0f, glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	RenderCube();
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;

// RenderCube() Renders a 1x1 3D cube in NDC.
void RenderCube()
{
	// Initialize (if necessary)
	if (cubeVAO == 0) {
		float vertices[] = {
			// Back face
		   -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, // Bottom-left
			0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, // top-right
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, // bottom-right         
			0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, // top-right
		   -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, // bottom-left
		   -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, // top-left
		    // Front face
		   -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // top-right
			0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // top-right
		   -0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // top-left
		   -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			// Left face
		   -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top-right
		   -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top-left
		   -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
		   -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
		   -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
		   -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top-right
			// Right face
			0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // top-left
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
			0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // top-right         
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
			0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // top-left
			0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-left     
			// Bottom face
		   -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-right
			0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
			0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, // bottom-left
			0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, // bottom-left
		   -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, // bottom-right
		   -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-right
			// Top face
		   -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
			0.5f, 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // bottom-right
			0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right     
			0.5f, 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // bottom-right
		   -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
		   -0.5f, 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// Fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// Link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// Render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

unsigned int createShader(const char *vertexSource, const char *fragmentSource) {
	//compile vertex & fragment shader
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	int  success;
	char log[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, log);
		cout << "顶点着色器编译失败！\n" << log << endl;
	}

	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, log);
		cout << "片段着色器编译失败！\n" << log << endl;
	}

	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return shaderProgram;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}


