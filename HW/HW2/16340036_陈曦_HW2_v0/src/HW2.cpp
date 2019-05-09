#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

using namespace std;
using namespace ImGui;

void processInput(GLFWwindow *window);

//GLSL������ɫ��
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"
	"}\0";

//Ƭ����ɫ��
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(ourColor, 1.0f);\n"
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
	GLFWwindow* window = glfwCreateWindow(400, 400, "LearnOpenGL", NULL, NULL);//���ڿ��ߡ����ڱ���
	if (window == NULL)
	{
		cout << "Failed to create GLFW window!" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);//֪ͨGLFW�����ڵ�����������Ϊ��ǰ�̵߳���������

	//��ʼ��GLAD��GLAD��������OpenGL�ĺ���ָ�룬�ڵ����κ�OpenGL�ĺ���֮ǰ��Ҫ��ʼ��GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
		return -1;
	}

	//���ô���ά��
	glViewport(0, 0, 400, 400);//ǰ�����������ƴ������½ǵ�λ�ã������ĸ�����������Ⱦ���ڵĿ�Ⱥ͸߶ȣ����أ���

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
	ImVec4 top_color = ImVec4(0.0f, 1.0f, 0.0f, 1.00f);
	ImVec4 left_color = ImVec4(1.0f, 0.0f, 0.0f, 1.00f);
	ImVec4 right_color = ImVec4(0.0f, 0.0f, 1.0f, 1.00f);
	ImVec4 bottom_color = ImVec4(0.0f, 1.0f, 1.0f, 1.00f);
	ImVec4 same_color = ImVec4(0.0f, 1.0f, 0.0f, 1.00f);
	bool the_same_color = false;
	bool draw_trangle = false;
	unsigned int VBO, VAO;
	

	


	//��Ⱦѭ��
	while (!glfwWindowShouldClose(window))//���GLFW�Ƿ�Ҫ���˳�
	{

		//����
		processInput(window);

		if (the_same_color) {
			top_color = same_color;
			left_color = same_color;
			right_color = same_color;
		}
	


		//����3D����
		float vertices[] = {
			-1.0f, -1.0f, 0.0f,right_color.x, right_color.y, right_color.z,
			1.0f, -1.0f, 0.0f, left_color.x, left_color.y, left_color.z,
			0.0f,  1.0f, 0.0f, top_color.x, top_color.y, top_color.z
		};

		
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);//ʹ��glGenBuffers������һ������ID����һ��VBO����

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);//���´����Ļ���󶨵�GL_ARRAY_BUFFERĿ����
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//��֮ǰ����Ķ������ݸ��Ƶ�������ڴ���

																				  //������������:λ������
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		//�����������ݣ���ɫ����
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		//

		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		//glBindVertexArray(0);

		glUseProgram(shaderProgram);//����������

									

		//����ImGui
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		NewFrame();
		Begin("Edit color");

		ColorEdit3("top color", (float*)&top_color);
		ColorEdit3("left color", (float*)&left_color);
		ColorEdit3("right color", (float*)&right_color);
		ColorEdit3("the same color", (float*)&same_color);

		Checkbox("the same color", &the_same_color);
		Checkbox("Draw triangle", &draw_trangle);

		
		End();

		

		//��Ⱦָ��
		Render();
		int view_width, view_height;
		glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &view_width, &view_height);
		glViewport(0, 0, view_width, view_height);
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);//��������������ɫ
		glClear(GL_COLOR_BUFFER_BIT);//�����Ļ����ɫ����
		ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());
		
		glEnableVertexAttribArray(1);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);


		
		glfwMakeContextCurrent(window);
		//˫����
		glfwSwapBuffers(window);

	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	DestroyContext();

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}