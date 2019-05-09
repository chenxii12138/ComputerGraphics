#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>

using namespace std;
using namespace ImGui;

void processInput(GLFWwindow *window);

vector<int> Bresenham(int x0, int y0, int x1, int y1);
vector<int> DrawTriangleBresenham(int x1, int y1, int x2, int y2, int x3, int y3);

void getAllPoints(int x0, int y0, int x, int y, vector<int> &points);
vector<int> DrawCircleBresenham(int x0, int y0, int r);


float *creatVertices(vector<int> v);

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
	GLFWwindow* window = glfwCreateWindow(1280, 720, "LearnOpenGL", NULL, NULL);//���ڿ��ߡ����ڱ���
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

	//���ô���ά��
	glViewport(0, 0, 1280, 720);//ǰ�����������ƴ������½ǵ�λ�ã������ĸ�����������Ⱦ���ڵĿ�Ⱥ͸߶ȣ����أ���

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
	bool draw_cricle = false;
	unsigned int VBO, VAO;

	// get the vector for triangle
	//vector<int> triangle_lines = DrawTriangleBresenham(0, 200, -100, 0, 100, 0);



	//��Ⱦѭ��
	while (!glfwWindowShouldClose(window))//���GLFW�Ƿ�Ҫ���˳�
	{

		//����
		processInput(window);

		//����ImGui
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		NewFrame();
		Begin("Edit");
		Checkbox("Draw circle", &draw_cricle);

		End();

		static float f = 0.0f;

		//�ı�Բ�뾶����
		if (draw_cricle) {
			Begin("Change R", &draw_cricle);
			Text("Change the R of the circle(0~100)");
			SliderFloat("R", &f, 0.0f, 100.0f);
			if (Button("Close")) {
				draw_cricle = false;
			}
			End();
		}

		vector<int> test;

		if (draw_cricle) {
			test = DrawCircleBresenham(0, 0, (int)f);
		}
		else
		{
			test = DrawTriangleBresenham(0, 200, -100, 0, 100, 0);
		}

		
		int point_num = test.size() / 2;
		int total = point_num * 3;
		float *vertices = creatVertices(test);

		

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);//ʹ��glGenBuffers������һ������ID����һ��VBO����

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);//���´����Ļ���󶨵�GL_ARRAY_BUFFERĿ����
		glBufferData(GL_ARRAY_BUFFER, total*sizeof(float), vertices, GL_STREAM_DRAW);//��֮ǰ����Ķ������ݸ��Ƶ�������ڴ���

																				  //������������:λ������
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);


		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(VAO);


		glUseProgram(shaderProgram);//����������

		delete vertices;


		//��Ⱦָ��
		Render();
		int view_width, view_height;
		glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &view_width, &view_height);
		glViewport(0, 0, view_width, view_height);
		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);//��������������ɫ
		glClear(GL_COLOR_BUFFER_BIT);//�����Ļ����ɫ����
		ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());

		glDrawArrays(GL_POINTS, 0, point_num);


		glfwMakeContextCurrent(window);
		//˫����
		glfwSwapBuffers(window);

	}

	glBindVertexArray(0);
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

vector<int> Bresenham(int x0, int y0, int x1, int y1) {
	vector<int> points;
	points.push_back(x0);
	points.push_back(y0);
	int dx = x1 - x0;
	int dy = y1 - y0;
	int direct_x = dx > 0 ? 1 : -1;
	int direct_y = dy > 0 ? 1 : -1;
	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;
	if (abs(dx) > abs(dy)) {
		int p = 2 * dy - dx;
		int x = x0;
		int y = y0;
		int dy2 = 2 * dy;
		int dy2_sub_dx2 = 2 * dy - 2 * dx;

		while (x != x1) {
			points.push_back(x);
			points.push_back(y);
			if (p > 0) {
				y += direct_y;
				p += dy2_sub_dx2;
			}
			else
			{
				p += dy2;
			}
			x += direct_x;
		}
	}
	else {
		int p = 2 * dx - dy;
		int x = x0;
		int y = y0;
		int dx2 = 2 * dx;
		int dx2_sub_dy2 = 2 * dx - 2 * dy;
		while (y != y1) {
			points.push_back(x);
			points.push_back(y);
			if (p > 0) {
				x += direct_x;
				p += dx2_sub_dy2;
			}
			else
			{
				p += dx2;
			}
			y += direct_y;
		}
	}
	points.push_back(x1);
	points.push_back(y1);
	return points;
}

vector<int> DrawTriangleBresenham(int x1, int y1, int x2, int y2, int x3, int y3) {
	vector<int> points;
	vector< vector<int> > edges;
	edges.push_back(Bresenham(x1, y1, x2, y2));
	edges.push_back(Bresenham(x1, y1, x3, y3));
	edges.push_back(Bresenham(x3, y3, x2, y2));
	for (int i = 0; i < edges.size(); i++) {
		for (int j = 0; j < edges[i].size(); j++) {
			points.push_back(edges[i][j]);
		}
	}
	return points;
}

void getAllPoints(int x0, int y0, int x, int y, vector<int> &points) {
	points.push_back(x0 + x); points.push_back(y0 + y);
	points.push_back(x0 + x); points.push_back(y0 - y);
	points.push_back(x0 - x); points.push_back(y0 + y);
	points.push_back(x0 - x); points.push_back(y0 - y);
	points.push_back(x0 + y); points.push_back(y0 + x);
	points.push_back(x0 + y); points.push_back(y0 - x);
	points.push_back(x0 - y); points.push_back(y0 + x);
	points.push_back(x0 - y); points.push_back(y0 - x);
}

vector<int> DrawCircleBresenham(int x0, int y0, int r) {
	vector<int> points;
	int x, y, d;
	y = r;
	x = 0;
	d = 3 - 2 * r;
	getAllPoints(x0, y0, x, y, points);
	while (x < y) {
		if (d < 0) {
			d += 4 * x + 6;
		}
		else
		{
			d += 4 * (x - y) + 10;
			y--;
		}
		x++;
		getAllPoints(x0, y0, x, y, points);
	}
	return points;
}

float *creatVertices(vector<int> v) {
	int point_num = v.size() / 2;
	int total = point_num * 3;
	float *vertices = new float[total];
	for (int i = 0; i < point_num; i++) {
		vertices[i * 3 + 0] = (float)v[i * 2 + 0] / (float)640;
		vertices[i * 3 + 1] = (float)v[i * 2 + 1] / (float)360;
		vertices[i * 3 + 2] = 0.0f;
	}
	return vertices;
}

