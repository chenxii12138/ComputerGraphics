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

//鼠标按下时调用
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
//阶乘
int factorial(int n);
//组合数
float C(int n, int i);

const int WIDTH = 1280;
const int HEIGHT = 720;
float vertices[100];
float q_vertices[4];
int point_num = 0;

int main()
{
	glfwInit();//初始化GLFW
	//使用glfwWindowHint配置GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//设置主版本号
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//设置次版本号
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//使用核心模式

	//创建窗口对象
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", NULL, NULL);//窗口宽、高、窗口标题
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
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	//设置窗口维度
	glViewport(0, 0, WIDTH, HEIGHT);//前两个参数控制窗口左下角的位置，三、四个参数控制渲染窗口的宽度和高度（像素）。


	//初始化各种数据
	unsigned int VBO, VAO;


	//渲染循环
	while (!glfwWindowShouldClose(window))//检查GLFW是否被要求退出
	{

		//输入
		processInput(window);

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);//使用glGenBuffers函数和一个缓冲ID生成一个VBO对象：

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);//把新创建的缓冲绑定到GL_ARRAY_BUFFER目标上
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);//把之前定义的顶点数据复制到缓冲的内存中

																				  //解析定点数据:位置属性
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);


		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(VAO);

		
		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);//设置清屏所用颜色
		glClear(GL_COLOR_BUFFER_BIT);//清空屏幕的颜色缓冲

		//画点
		glPointSize(5.0f);
		glDrawArrays(GL_POINTS, 0, point_num);
		//画线
		glDrawArrays(GL_LINE_STRIP, 0, point_num);
		//画BezierCurve
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
			glGenBuffers(1, &qVBO);//使用glGenBuffers函数和一个缓冲ID生成一个VBO对象：

			glBindVertexArray(qVAO);

			glBindBuffer(GL_ARRAY_BUFFER, qVBO);//把新创建的缓冲绑定到GL_ARRAY_BUFFER目标上
			glBufferData(GL_ARRAY_BUFFER, sizeof(q_vertices), q_vertices, GL_STATIC_DRAW);//把之前定义的顶点数据复制到缓冲的内存中

																					  //解析定点数据:位置属性
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
		//双缓冲
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
			glfwGetCursorPos(window, &xpos, &ypos); //width和height鼠标的实时坐标。
			vertices[point_num * 2] = (float)xpos / (float)(WIDTH) * 2.0f - 1.0f; //横坐标
			vertices[point_num * 2 + 1] = -((float)ypos / (float)(HEIGHT) * 2.0f - 1.0f); //纵坐标
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
