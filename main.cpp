#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <stddef.h> /*for function: offsetof */
#include <math.h>
#include <string.h>
#include "GL/glew.h"
#include "GL/glut.h"
#include "shader_lib/shader.h"
#include "glm/glm.h"

extern "C"
{
	#include "glm_helper.h"
}

struct Vertex
{
	GLfloat position[3];
	GLfloat normal[3];
	GLfloat texcoord[2];
};
typedef struct Vertex Vertex;

void init(void);
void shaderInit(char*,char*);
void display(void);
void reshape(int width, int height);
void idle(void);
char* readShader(char*);
GLuint loadTexture(char* name, GLfloat width, GLfloat height);

char* obj_file_dir = "Resources/Teapot.obj";
char* main_tex_dir = "Resources/WoodFine.ppm";

GLuint mainTextureID;

GLMmodel *model;
GLuint vboIds;
char* vertShaderPath = "Shaders/Phong.vert";
char* fragShaderPath = "Shaders/Phong.frag";

int shaderProgram;
float eyex = 0.0;
float eyey = 0.0;
float eyez = 15;
int time = 0;

GLfloat light_pos[] = { 1.1, 1.0, 1.3 };

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	
	// remember to replace "YourStudentID" with your own student ID
	glutCreateWindow("RTR_HW1_0856631");
	glutReshapeWindow(512, 512);

	glewInit();

	init();
	shaderInit(vertShaderPath, fragShaderPath);

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(idle);

	glutMainLoop();

	glmDelete(model);
	return 0;
}

void init(void)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glEnable(GL_CULL_FACE);				//清除看不到的面

	model = glmReadOBJ(obj_file_dir);

	mainTextureID = loadTexture(main_tex_dir, 1024, 1024);
	
	glmUnitize(model);										//Normalize the model to the 3D space which is centered on the origin
	glmFacetNormals(model);
	glmVertexNormals(model, 90.0, GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	print_model_info(model);

	const int vertex_count = model->numvertices;			//362
	int triangles_count = model->numtriangles;				//720

	Vertex* v = new Vertex[triangles_count * 3];
	GLMtriangle* triangles = model->triangles;

	for (int i = 0; i < triangles_count; i++) {
		int vindex[3] = { model->triangles[i].vindices[0], model->triangles[i].vindices[1], model->triangles[i].vindices[2] };
		int nindex[3] = { model->triangles[i].nindices[0], model->triangles[i].nindices[1], model->triangles[i].nindices[2] };
		int tindex[3] = { model->triangles[i].tindices[0], model->triangles[i].tindices[1], model->triangles[i].tindices[2] };


		for (int j = 0; j < 3;j++) {
			//Position
			v[i * 3 + j].position[0] = model->vertices[vindex[j]*3 + 0];		//X軸座標
			v[i * 3 + j].position[1] = model->vertices[vindex[j]*3 + 1];		//Y軸座標
			v[i * 3 + j].position[2] = model->vertices[vindex[j]*3 + 2];		//Z軸座標

			//Normal
			v[i * 3 + j].normal[0] = model->normals[nindex[j]*3 + 0];
			v[i * 3 + j].normal[1] = model->normals[nindex[j]*3 + 1];
			v[i * 3 + j].normal[2] = model->normals[nindex[j]*3 + 2];
		}
	}

	glGenBuffers(1, &vboIds);
	glBindBuffer(GL_ARRAY_BUFFER, vboIds);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * triangles_count * 3 , v, GL_STATIC_DRAW);
	
	// position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); // stride 0 for tightly packed

	// normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position))); // stride 0 for tightly packed
	
	// texture coordinates
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal))); // stride 0 for tightly packed

	glBindBuffer(GL_ARRAY_BUFFER, 0);				//解bind

	delete []v;
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);								//開始操作MV
	glLoadIdentity();
	gluLookAt(
		eyex, 
		eyey, 
		eyez,
		0.0, 
		0.0, 
		0.0,
		0.0,
		1.0,
		0.0);

	float viewMatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX,viewMatrix);

	GLint v = glGetUniformLocation(shaderProgram, "viewMatrix");
	glUniformMatrix4fv(v, 1, GL_FALSE, viewMatrix);

	glUseProgram(NULL);
	glPushMatrix();
		glTranslatef(0, 0, 0);
		glRotatef(0, 1, 0, 0);
		glRotatef(0, 0, 1, 0);
		glRotatef(0, 0, 0, 1);
	
	glEnable(GL_TEXTURE_2D);

	float modelViewMatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMatrix);

	float projectMatrix[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projectMatrix);

	float translate[16];
	glLoadIdentity();
	glPushMatrix();
	glTranslatef(5.0, 0, 0);
	glGetFloatv(GL_MODELVIEW_MATRIX, translate);

	glUseProgram(shaderProgram);

	GLint mv = glGetUniformLocation(shaderProgram, "modelView");
	glUniformMatrix4fv(mv, 1, GL_FALSE, modelViewMatrix);

	GLint p = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(p, 1, GL_FALSE, projectMatrix);

	GLint t = glGetUniformLocation(shaderProgram, "translation");
	glUniformMatrix4fv(t, 1, GL_FALSE, translate);

	GLint tex = glGetUniformLocation(shaderProgram, "myTexture");
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, mainTextureID);
	glUniform1i(tex, 0);

	time++;
	GLint Time = glGetUniformLocation(shaderProgram, "Time");
	glUniform1i(Time, time);

	glBindVertexArray(vboIds);
	glDrawArrays(GL_TRIANGLES, 0, model->numtriangles * 3);
	glBindTexture(GL_TEXTURE_2D, NULL);

	glPopMatrix();

	glutSwapBuffers();
}

void shaderInit(char* vShader, char* fShader) {
	int  Success;												//檢查是否Shader讀取成功
	char infoLog[512];											//接錯誤訊息
	unsigned int vertexShader;
	unsigned int fragmentShader;

	char* vertexShaderSource = readShader(vShader);
	char* fragmentShaderSource = readShader(fShader);

	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	//Check the correctness of VertexShader Compilation
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &Success);
	if (!Success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	//Check the correctness of FragmentShader Compilation
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &Success);
	if (!Success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

}

char* readShader(char* shaderPath) {
	FILE *fp;
	fp = fopen(shaderPath, "r");
	char *buffer = (char*)malloc(sizeof(char) * 4096);
	char *data = (char*)malloc(sizeof(char) * 4096);
	buffer[0] = '\0';
	data[0] = '\0';

	if (fp == NULL) {
		std::cout << "Error" << std::endl;
	}

	while (fgets(buffer, 4096, fp) != NULL) {
		strcat(data, buffer);
	}
	free(buffer);
	fclose(fp);

	return data;
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.001f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

void idle(void)
{
	glutPostRedisplay();
}

GLuint loadTexture(char* name, GLfloat width, GLfloat height)
{
	return glmLoadTexture(name, false, true, true, true, &width, &height);
}
