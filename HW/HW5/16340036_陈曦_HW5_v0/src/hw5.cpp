#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void processInput(GLFWwindow *window);


//GLSL������ɫ��
const char *vertexShaderSource = "#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 outColor;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"   outColor = aColor;\n"
"}\0";

//Ƭ����ɫ��
const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 outColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(outColor, 1.0);\n"
"}\n\0";



int main()
{
	glfwInit();//��ʼ��GLFW
	//ʹ��glfwWindowHint����GLFW
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//�������汾��
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//���ôΰ汾��
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//ʹ�ú���ģʽ


	//�������ڶ���
	GLFWwindow* window = glfwCreateWindow(800, 800, "LearnOpenGL", NULL, NULL);//���ڿ��ߡ����ڱ���
	if (window == NULL)
	{
		cout << "Failed to create GLFW window!" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);//֪ͨGLFW�����ڵ�����������Ϊ��ǰ�̵߳���������
	glfwSwapInterval(1);

	//��ʼ��GLAD��GLAD��������OpenGL�ĺ���ָ�룬�ڵ����κ�OpenGL�ĺ���֮ǰ��Ҫ��ʼ��GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
		return -1;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//���ô���ά��
	glViewport(0, 0, 800, 800);//ǰ�����������ƴ������½ǵ�λ�ã������ĸ�����������Ⱦ���ڵĿ�Ⱥ͸߶ȣ����أ���

	glEnable(GL_DEPTH_TEST);

	//��������ImGui
	IMGUI_CHECKVERSION();
	CreateContext();
	ImGuiIO& io = GetIO(); (void)io;
	StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);//������ɫ��������ID����
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);//��ɫ��Դ�븽�ӵ���ɫ��������
	glCompileShader(vertexShader);//����


	//���Ա����Ƿ�ɹ�
	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
	}

	unsigned int framentShader;
	framentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(framentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(framentShader);

	//���Ա����Ƿ�ɹ�
	glGetShaderiv(framentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(framentShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
	}

	//������ɫ���������
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();//���ض���ID����
									  //����ɫ�����ӵ���������ϣ���glLinkProgram����
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, framentShader);
	glLinkProgram(shaderProgram);

	//���Ա����Ƿ�ɹ�
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
	}

	//ɾ����ɫ������
	glDeleteShader(vertexShader);
	glDeleteShader(framentShader);



	//��ʼ����������
	//�������������ƴ�ӳ�һ�������壨6*2��
	float vertices[] = {
	   -2.0f, -2.0f, -2.0f, 1.0f, 0.0f, 0.0f, //triangle 1 : begin
		2.0f, -2.0f, -2.0f, 1.0f, 0.0f, 0.0f,
		2.0f,  2.0f, -2.0f, 1.0f, 0.0f, 0.0f, //triangle 1 : end
		2.0f,  2.0f, -2.0f, 1.0f, 0.0f, 0.0f, // triangle 2 : begin
	   -2.0f,  2.0f, -2.0f, 1.0f, 0.0f, 0.0f,
	   -2.0f, -2.0f, -2.0f, 1.0f, 0.0f, 0.0f, // triangle 2 : end

	   -2.0f, -2.0f,  2.0f, 0.0f, 1.0f, 0.0f,
		2.0f, -2.0f,  2.0f, 0.0f, 1.0f, 0.0f,
		2.0f,  2.0f,  2.0f, 0.0f, 1.0f, 0.0f,
		2.0f,  2.0f,  2.0f, 0.0f, 1.0f, 0.0f,
	   -2.0f,  2.0f,  2.0f, 0.0f, 1.0f, 0.0f,
	   -2.0f, -2.0f,  2.0f, 0.0f, 1.0f, 0.0f,

	   -2.0f,  2.0f,  2.0f, 0.0f, 0.0f, 1.0f,
	   -2.0f,  2.0f, -2.0f, 0.0f, 0.0f, 1.0f,
	   -2.0f, -2.0f, -2.0f, 0.0f, 0.0f, 1.0f,
	   -2.0f, -2.0f, -2.0f, 0.0f, 0.0f, 1.0f,
	   -2.0f, -2.0f,  2.0f, 0.0f, 0.0f, 1.0f,
	   -2.0f,  2.0f,  2.0f, 0.0f, 0.0f, 1.0f,

		2.0f,  2.0f,  2.0f, 1.0f, 1.0f, 0.0f,
		2.0f,  2.0f, -2.0f, 1.0f, 1.0f, 0.0f,
		2.0f, -2.0f, -2.0f, 1.0f, 1.0f, 0.0f,
		2.0f, -2.0f, -2.0f, 1.0f, 1.0f, 0.0f,
		2.0f, -2.0f,  2.0f, 1.0f, 1.0f, 0.0f,
		2.0f,  2.0f,  2.0f, 1.0f, 1.0f, 0.0f,

	   -2.0f, -2.0f, -2.0f, 1.0f, 0.0f, 1.0f,
		2.0f, -2.0f, -2.0f, 1.0f, 0.0f, 1.0f,
		2.0f, -2.0f,  2.0f, 1.0f, 0.0f, 1.0f,
		2.0f, -2.0f,  2.0f, 1.0f, 0.0f, 1.0f,
	   -2.0f, -2.0f,  2.0f, 1.0f, 0.0f, 1.0f,
	   -2.0f, -2.0f, -2.0f, 1.0f, 0.0f, 1.0f,

	   -2.0f,  2.0f, -2.0f, 0.0f, 1.0f, 1.0f,
		2.0f,  2.0f, -2.0f, 0.0f, 1.0f, 1.0f,
		2.0f,  2.0f,  2.0f, 0.0f, 1.0f, 1.0f,
		2.0f,  2.0f,  2.0f, 0.0f, 1.0f, 1.0f,
	   -2.0f,  2.0f,  2.0f, 0.0f, 1.0f, 1.0f,
	   -2.0f,  2.0f, -2.0f, 0.0f, 1.0f, 1.0f
	};

	bool orthographic = true;
	bool Projection_ = true;
	bool ViewChanging_ = false;
	bool depth = false;


	float angle = 90.0f;
	float scale = 1.0f;
	float radius = 10.0f;
	float left = -10.0f;
	float right = 10.0f;
	float top = 10.0f;
	float bottom = -10.0f;
	float near_ = 0.1f;
	float far_ = 100.0f;
	float perspective = 45.0f;


	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);//ʹ��glGenBuffers������һ������ID����һ��VBO����

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);//���´����Ļ���󶨵�GL_ARRAY_BUFFERĿ����
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);//��֮ǰ����Ķ������ݸ��Ƶ�������ڴ���

																			  //������������:λ������
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	mat4 view = mat4(1.0f);
	mat4 projection = mat4(1.0f);
	view = translate(view, vec3(-1.5f, 0.5f, -20.0f));

	while (!glfwWindowShouldClose(window))
	{

		//����
		processInput(window);

		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);//��������������ɫ
		glClear(GL_COLOR_BUFFER_BIT);//�����Ļ����ɫ����		

		//����ImGui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		NewFrame();

		Begin("Edit");
		Checkbox("DepthTest", &depth);
		Checkbox("Projection_", &Projection_);
		Checkbox("ViewChanging_", &ViewChanging_);
		SliderFloat("Scale", &scale, 0.1f, 3.0f);
		SliderFloat("angle", &angle, -360.0f, 360.0f);

		if (ViewChanging_) {
			SliderFloat("Radius", &radius, 1.0f, 100.0f);
		}
		else {
			Checkbox("orthographic", &orthographic);

			if (orthographic) {

				SliderFloat("Left", &left, -400.0f, 0.0f);
				SliderFloat("Right", &right, -5.0f, 400.0f);
				SliderFloat("Top", &top, 0.0f, 400.0f);
				SliderFloat("Bottom", &bottom, -400.0f, 0.0f);
				SliderFloat("Near", &near_, -100.0f, 100.0f);
				SliderFloat("Far", &far_, -100.0f, 100.0f);
			}
			else {
				SliderFloat("Perspective", &perspective, 0.0f, 100.0f);
				SliderFloat("Near", &near_, -100.0f, 100.0f);
				SliderFloat("Far", &far_, -100.0f, 100.0f);
			}
		}

		End();

		//��Ȳ���
		if (depth) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
		}
		else {
			glClear(GL_COLOR_BUFFER_BIT);
			glDisable(GL_DEPTH_TEST);
		}

		mat4 model = mat4(1.0f);

		if (Projection_) {
			ViewChanging_ = false;

			model = translate(model, vec3(-1.5f, 0.5f, -1.5f));
			model = rotate(model, radians(angle), vec3(1.0f, 0.0f, 1.0f));
			model = glm::scale(model, glm::vec3(scale, scale, scale));
			if (orthographic) {
				projection = ortho(left, right, bottom, top, near_, far_);
			}
			else {
				projection = glm::perspective(radians(perspective), 800.0f / 800.0f, near_, far_);
			}

		}

		if (ViewChanging_) {
			Projection_ = false;

			model = translate(model, vec3(0.0f, 0.0f, 0.0f));
			model = glm::scale(model, vec3(scale, scale, scale));

			float cameraX = sin(glfwGetTime()) * radius;
			float cameraZ = cos(glfwGetTime()) * radius;
			view = lookAt(vec3(cameraX, 0.0f, cameraZ),
				vec3(0.0f, 0.0f, 0.0f),
				vec3(0.0f, 1.0f, 0.0f));
			projection = glm::perspective(radians(45.0f), 800.0f / 800.0f, 0.1f, 100.0f);

		}

		glUseProgram(shaderProgram);//����������

		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);


		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		//��Ⱦָ��
		Render();
		int view_width, view_height;
		glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &view_width, &view_height);
		glViewport(0, 0, view_width, view_height);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		//˫����
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glBindVertexArray(VAO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	DestroyContext();

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}



