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

//GLSL������ɫ��
const char *lightVertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"

"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"

"void main()\n"
"{\n"
"   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"}\0";

//Ƭ����ɫ��
const char *lightFragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"   FragColor = vec4(1.0);\n"
"}\n\0";

const char *PhongVertexShaderSource = "#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aNormal;\n"

"out vec3 FragPos;\n" //Ƭ��λ�ã�����ռ��еĶ���λ��
"out vec3 Normal;\n" //������

"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"

"void main()\n"
"{\n"
"	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"   FragPos = vec3(model * vec4(aPos, 1.0));\n" //ͨ���Ѷ���λ�����Գ���ģ�;��󣨲��ǹ۲��ͶӰ�����������任������ռ�����
"   Normal = mat3(transpose(inverse(model))) * aNormal;\n" //���ɷ��߾��󣬰ѱ�������ľ���ǿ��ת��Ϊ3��3����
"}\0";

const char *PhongFragmentShaderSource = "#version 330 core\n"
"in vec3 FragPos;\n"
"in vec3 Normal;\n"
"out vec4 FragColor;\n"

"uniform vec3 lightPos;\n" //��Դλ��
"uniform vec3 viewPos;\n"  //�����λ������
"uniform vec3 lightColor;\n"
"uniform vec3 objectColor;\n"

"uniform float ambientStrength;\n" //ambient����
"uniform float diffuseStrength;\n" //diffuse����
"uniform float specularStrength;\n" //specular����
"uniform int reflectance;\n" //�����

"void main()\n"
"{\n"
"   vec3 ambient = ambientStrength * lightColor;\n" //�����ɫ����һ����С�ĳ�����������
"   vec3 norm = normalize(Normal);\n" //��׼��
"   vec3 lightDir = normalize(lightPos - FragPos);\n" //��Դ��Ƭ��λ��֮��ķ�������
"   float diff = max(dot(norm, lightDir), 0.0);\n" //��norm��lightDir�������е�ˣ������Դ�Ե�ǰƬ��ʵ�ʵ�������Ӱ��
"   vec3 diffuse = diffuseStrength * diff * lightColor;\n" //�õ����������
"   vec3 viewDir = normalize(viewPos - FragPos);\n" //���߷�������
"   vec3 reflectDir = reflect(-lightDir, norm);\n"  //��Ӧ�����ŷ�����ķ�������
"   float spec = pow(max(dot(viewDir, reflectDir), 0.0), reflectance);\n" //���㾵�����
"   vec3 specular = specularStrength * spec * lightColor;\n" //���㾵�����
"   vec3 result = (ambient + diffuse + specular) * objectColor;\n"
"   FragColor = vec4(result, 1.0);\n"
"}\n\0";


const char *GouraudVertexShaderSource = "#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aNormal;\n"

"out vec3 LightingColor;\n"

"uniform vec3 lightPos;\n"
"uniform vec3 viewPos;"
"uniform vec3 lightColor;"

"uniform float ambientStrength;\n"
"uniform float diffuseStrength;\n"
"uniform float specularStrength;\n"
"uniform int reflectance;\n"

"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"

"   vec3 Position = vec3(model * vec4(aPos, 1.0));\n"
"   vec3 Normal = mat3(transpose(inverse(model))) * aNormal;\n"

"   vec3 ambient = ambientStrength * lightColor;\n"

"   vec3 norm = normalize(Normal);\n"
"   vec3 lightDir = normalize(lightPos - Position);\n"
"   float diff = max(dot(norm, lightDir), 0.0);\n"
"   vec3 diffuse = diffuseStrength * diff * lightColor;\n"

"   vec3 viewDir = normalize(viewPos - Position);\n"
"   vec3 reflectDir = reflect(-lightDir, norm);\n"
"   float spec = pow(max(dot(viewDir, reflectDir), 0.0), reflectance);\n"
"   vec3 specular = specularStrength * spec * lightColor;\n"

"   LightingColor = ambient + diffuse + specular;\n"
"}\0";

const char *GouraudFragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"

"in vec3 LightingColor;\n"

"uniform vec3 objectColor;\n"

"void main()\n"
"{\n"
"   FragColor = vec4(LightingColor * objectColor, 1.0);\n"
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

	//===============================================================
	//lightShader
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);//������ɫ��������ID����
	glShaderSource(vertexShader, 1, &lightVertexShaderSource, NULL);//��ɫ��Դ�븽�ӵ���ɫ��������
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
	glShaderSource(framentShader, 1, &lightFragmentShaderSource, NULL);
	glCompileShader(framentShader);

	//���Ա����Ƿ�ɹ�
	glGetShaderiv(framentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(framentShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
	}

	//������ɫ���������
	unsigned int lightShader;
	lightShader = glCreateProgram();//���ض���ID����
									  //����ɫ�����ӵ���������ϣ���glLinkProgram����
	glAttachShader(lightShader, vertexShader);
	glAttachShader(lightShader, framentShader);
	glLinkProgram(lightShader);

	//���Ա����Ƿ�ɹ�
	glGetProgramiv(lightShader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(lightShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
	}

	//ɾ����ɫ������
	glDeleteShader(vertexShader);
	glDeleteShader(framentShader);


    //=============================================================
	//GouraudShader
	//unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);//������ɫ��������ID����
	glShaderSource(vertexShader, 1, &GouraudVertexShaderSource, NULL);//��ɫ��Դ�븽�ӵ���ɫ��������
	glCompileShader(vertexShader);//����


	//���Ա����Ƿ�ɹ�
	//int  success;
	//char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
	}

	//unsigned int framentShader;
	framentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(framentShader, 1, &GouraudFragmentShaderSource, NULL);
	glCompileShader(framentShader);

	//���Ա����Ƿ�ɹ�
	glGetShaderiv(framentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(framentShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
	}

	//������ɫ���������
	unsigned int GouraudShader;
	GouraudShader = glCreateProgram();//���ض���ID����
									  //����ɫ�����ӵ���������ϣ���glLinkProgram����
	glAttachShader(GouraudShader, vertexShader);
	glAttachShader(GouraudShader, framentShader);
	glLinkProgram(GouraudShader);

	//���Ա����Ƿ�ɹ�
	glGetProgramiv(GouraudShader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(GouraudShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
	}

	//ɾ����ɫ������
	glDeleteShader(vertexShader);
	glDeleteShader(framentShader);

	//==========================================================
	//PhongShader
	//unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);//������ɫ��������ID����
	glShaderSource(vertexShader, 1, &PhongVertexShaderSource, NULL);//��ɫ��Դ�븽�ӵ���ɫ��������
	glCompileShader(vertexShader);//����


	//���Ա����Ƿ�ɹ�
	//int  success;
	//char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
	}

	//unsigned int framentShader;
	framentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(framentShader, 1, &PhongFragmentShaderSource, NULL);
	glCompileShader(framentShader);

	//���Ա����Ƿ�ɹ�
	glGetShaderiv(framentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(framentShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
	}

	//������ɫ���������
	unsigned int PhongShader;
	PhongShader = glCreateProgram();//���ض���ID����
									  //����ɫ�����ӵ���������ϣ���glLinkProgram����
	glAttachShader(PhongShader, vertexShader);
	glAttachShader(PhongShader, framentShader);
	glLinkProgram(PhongShader);

	//���Ա����Ƿ�ɹ�
	glGetProgramiv(PhongShader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(PhongShader, 512, NULL, infoLog);
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

	bool Phong = true;
	bool depth = true;
	bool bonus = false;
	float ambientStrength = 0.1;
	float diffuseStrength = 1.0;
	float specularStrength = 0.5;
	int reflectance = 32;
	float radius = 4.0f;

	unsigned int VAO, VBO;
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(VAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO); 
	glBindVertexArray(lightVAO);
	// ֻ��Ҫ��VBO�����ٴ�����VBO�����ݣ���Ϊ���ӵ�VBO�������Ѿ���������ȷ�������嶥������
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// ���õ�������Ķ������ԣ������ǵĵ���˵����ֻ��λ�����ݣ�
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

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
		Checkbox("DepthTesting", &depth);
		Checkbox("Bonus", &bonus);
		Checkbox("PhongShading ", &Phong);
		SliderFloat("Ambient", &ambientStrength, 0.0f, 1.0f);
		SliderFloat("Diffuse", &diffuseStrength, 1.0f, 5.0f);
		SliderFloat("Specular", &specularStrength, 0.0f, 1.0f);
		SliderInt("Reflectance", &reflectance, 2, 256);
		SliderFloat("Radius", &radius, 3.0f, 10.0f);
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
		model = translate(model, vec3(0.0f, 0.0f, 0.0f));
		mat4 projection = glm::perspective(radians(45.0f), 800.0f / 800.0f, 0.1f, 100.0f);
		mat4 view = lookAt(vec3(4.0f, 4.0f, 15.0f),
			vec3(0.0f, 1.0f, 0.0f),
			vec3(0.0f, 0.0f, -1.0f));
		
		
		vec3 lightPos(radius, radius, 0.0f);
		mat4 lightModel = model;

		if (bonus) {
			// change the light's position values over time (can be done anywhere in the render loop actually, but try to do it at least before using the light source positions)
			lightPos.x = cos((float)glfwGetTime()) * radius;
			lightPos.y = sin((float)glfwGetTime()) * radius;
		}
		
		lightModel = glm::translate(lightModel, lightPos);
		lightModel = glm::scale(lightModel, vec3(0.1f));

		if (Phong) {
			glUseProgram(PhongShader);

			glUniform3f(glGetUniformLocation(PhongShader, "objectColor"), 1.0f, 0.5f, 0.31f);
			glUniform3f(glGetUniformLocation(PhongShader, "lightColor"), 1.0f, 1.0f, 1.0f);
			glUniform3fv(glGetUniformLocation(PhongShader, "lightPos"), 1, &lightPos[0]);
			glUniform3f(glGetUniformLocation(PhongShader, "viewPos"), 4.0f, 4.0f, 15.0f);

			glUniform1f(glGetUniformLocation(PhongShader, "ambientStrength"), ambientStrength);
			glUniform1f(glGetUniformLocation(PhongShader, "diffuseStrength"), diffuseStrength);
			glUniform1f(glGetUniformLocation(PhongShader, "specularStrength"), specularStrength);
			glUniform1i(glGetUniformLocation(PhongShader, "reflectance"), reflectance);

			glUniformMatrix4fv(glGetUniformLocation(PhongShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(glGetUniformLocation(PhongShader, "view"), 1, GL_FALSE, &view[0][0]); 
			glUniformMatrix4fv(glGetUniformLocation(PhongShader, "projection"), 1, GL_FALSE, &projection[0][0]);
		}
		else {
			glUseProgram(GouraudShader);

			glUniform3f(glGetUniformLocation(GouraudShader, "objectColor"), 1.0f, 0.5f, 0.31f);
			glUniform3f(glGetUniformLocation(GouraudShader, "lightColor"), 1.0f, 1.0f, 1.0f);
			glUniform3fv(glGetUniformLocation(GouraudShader, "lightPos"), 1, &lightPos[0]);
			glUniform3f(glGetUniformLocation(GouraudShader, "viewPos"), 4.0f, 4.0f, 15.0f);

			glUniform1f(glGetUniformLocation(GouraudShader, "ambientStrength"), ambientStrength);
			glUniform1f(glGetUniformLocation(GouraudShader, "diffuseStrength"), diffuseStrength);
			glUniform1f(glGetUniformLocation(GouraudShader, "specularStrength"), specularStrength);
			glUniform1i(glGetUniformLocation(GouraudShader, "reflectance"), reflectance);

			glUniformMatrix4fv(glGetUniformLocation(GouraudShader, "model"), 1, GL_FALSE, &model[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(GouraudShader, "view"), 1, GL_FALSE, &view[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(GouraudShader, "projection"), 1, GL_FALSE, &projection[0][0]);
		}

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		glUseProgram(lightShader);
		glUniformMatrix4fv(glGetUniformLocation(lightShader, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
		glUniformMatrix4fv(glGetUniformLocation(lightShader, "view"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(lightShader, "projection"), 1, GL_FALSE, &projection[0][0]);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		//��Ⱦָ��
		Render();
		int view_width, view_height;
		glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &view_width, &view_height);
		glViewport(0, 0, view_width, view_height);
		ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());

		//˫����
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &lightVAO);
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



