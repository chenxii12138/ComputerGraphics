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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

//��갴��ʱ����
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
//�׳�
int factorial(int n);
//�����
float C(int n, int i);

const int WIDTH = 1280;
const int HEIGHT = 720;
float vertices[100];
float q_vertices[4];
int point_num = 0;

int main()
{
	glfwInit();//��ʼ��GLFW
	//ʹ��glfwWindowHint����GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//�������汾��
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//���ôΰ汾��
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//ʹ�ú���ģʽ

	//�������ڶ���
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", NULL, NULL);//���ڿ��ߡ����ڱ���
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
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	//���ô���ά��
	glViewport(0, 0, WIDTH, HEIGHT);//ǰ�����������ƴ������½ǵ�λ�ã������ĸ�����������Ⱦ���ڵĿ�Ⱥ͸߶ȣ����أ���


	//��ʼ����������
	unsigned int VBO, VAO;


	//��Ⱦѭ��
	while (!glfwWindowShouldClose(window))//���GLFW�Ƿ�Ҫ���˳�
	{

		//����
		processInput(window);

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);//ʹ��glGenBuffers������һ������ID����һ��VBO����

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);//���´����Ļ���󶨵�GL_ARRAY_BUFFERĿ����
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//��֮ǰ����Ķ������ݸ��Ƶ�������ڴ���

																				  //������������:λ������
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);


		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(VAO);

		
		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);//��������������ɫ
		glClear(GL_COLOR_BUFFER_BIT);//�����Ļ����ɫ����

		//����
		glPointSize(5.0f);
		glDrawArrays(GL_POINTS, 0, point_num);
		//����
		glDrawArrays(GL_LINE_STRIP, 0, point_num);
		//��BezierCurve
		q_vertices[0] = vertices[0];
		q_vertices[1] = vertices[1];

		for (float t = 0.0f; t < 1.0f; t += 0.02f) {
			double p1 = pow((1 - t), point_num - 1), p2 = 1;
			for (int i = 0; i < point_num; i++) {
				q_vertices[2] += vertices[i * 2] * C(point_num - 1, i) * p1 * p2;
				q_vertices[3] += vertices[i * 2 + 1] * C(point_num - 1, i) * p1 * p2;
				p1 /= 1 - t;
				p2 *= t;
			}
			unsigned int qVAO, qVBO;
			glGenVertexArrays(1, &qVAO);
			glGenBuffers(1, &qVBO);//ʹ��glGenBuffers������һ������ID����һ��VBO����

			glBindVertexArray(qVAO);

			glBindBuffer(GL_ARRAY_BUFFER, qVBO);//���´����Ļ���󶨵�GL_ARRAY_BUFFERĿ����
			glBufferData(GL_ARRAY_BUFFER, sizeof(q_vertices), q_vertices, GL_STATIC_DRAW);//��֮ǰ����Ķ������ݸ��Ƶ�������ڴ���

																					  //������������:λ������
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);


			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(qVAO);

			if (point_num > 0) {
				glBindVertexArray(qVAO);
				glDrawArrays(GL_LINE_STRIP, 0, 2);
			}

			glDeleteVertexArrays(1, &qVAO);
			glDeleteBuffers(1, &qVBO);

			q_vertices[0] = q_vertices[2];
			q_vertices[1] = q_vertices[3];
			q_vertices[2] = 0.0f;
			q_vertices[3] = 0.0f;
		}
		//˫����
		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos); //width��height����ʵʱ���ꡣ
			vertices[point_num * 2] = (float)xpos / (float)(WIDTH) * 2.0f - 1.0f; //������
			vertices[point_num * 2 + 1] = -((float)ypos / (float)(HEIGHT) * 2.0f - 1.0f); //������
			point_num++;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (point_num > 0) {
				point_num--;
			}
			break;
		default:
			break;
		}
	}
}

int factorial(int n) {
	if (n > 1) {
		return n * factorial(n - 1);
	}
	return 1;
}

float C(int n, int i) {
	return (float)factorial(n) / (factorial(i) * factorial(n - i));
}
