#include <iostream>
#include "GL\glew.h" //OpenGL
#include "GL\glut.h" //OpenGL

#include "glsl.h" //shaders
#include "SOIL.h" //загрузка изображений

using namespace cwc;
using namespace std;

struct Light  {
	bool isActive = false;
	float *position, *ambient, *diffuse, *specular;
	~Light() {
		delete position;
		delete ambient;
		delete diffuse;
		delete specular;
	}
};

struct Material {
	float *ambient, *diffuse, *specular, *emission, shininess;
	~Material() {
		delete ambient;
		delete diffuse;
		delete specular;
		delete emission;
	}
};

const int lights_count = 3;
Light lights[lights_count];
Material material;

GLuint   texture[1];

glShaderManager shader_manager;
glShader *shader;
char *vertexShader = "vertexShader.txt", *fragmentShader = "fragmentShader.txt";

float rotateX = 0, rotateY = 0;

int LoadGLTextures()
{
	texture[0] = SOIL_load_OGL_texture
		(
			"1.jpg",
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_INVERT_Y
			);
	if (texture[0] == 0)
		return false;
	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}
void DrawCube(float sz)
{
	glBegin(GL_QUADS);
	// передн€€ грань
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-sz, -sz, sz);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(sz, -sz, sz);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(sz, sz, sz);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-sz, sz, sz);

	// задн€€ грань
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-sz, -sz, -sz);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-sz, sz, -sz);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(sz, sz, -sz);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(sz, -sz, -sz);

	// верхн€€ грань
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-sz, sz, -sz);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-sz, sz, sz);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(sz, sz, sz);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(sz, sz, -sz);

	// нижн€€ грань
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-sz, -sz, -sz);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(sz, -sz, -sz);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(sz, -sz, sz);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-sz, -sz, sz);

	// парва€ грань
	glTexCoord2f(1.0f, 0.0f); glVertex3f(sz, -sz, -sz);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(sz, sz, -sz);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(sz, sz, sz);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(sz, -sz, sz);

	// лева€ грань
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-sz, -sz, -sz);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-sz, -sz, sz);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-sz, sz, sz);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-sz, sz, -sz);
	glEnd();
}


void Init() {
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glEnable(GL_DEPTH_TEST);
	LoadGLTextures();

	shader->setUniform1ui("my_color_texture", 0);

	lights[0].position = new float[4]{ 0.0f, -100.0f, 200.0f, 1.0f };
	lights[1].position = new float[4]{ -150.0f, 0.0f, 200.0f, 1.0f };
	lights[2].position = new float[4]{ 150.0f, 0.0f, 200.0f, 1.0f };

	lights[0].isActive = true;
	for (int i = 0; i < lights_count; i++) {
		lights[i].ambient = new float[4]{ 0.0f, 0.0f, 0.0f, 1.0f };
		lights[i].diffuse = new float[4]{ 191.0f / 256, 249.0f / 256, 253.0 / 256, 1.0f };
		lights[i].specular = new float[4]{ 0.8f, 1.0f, 1.0f, 1.0f };
	}

	material.ambient = new float[4]{ 190.0 / 256, 190.0 / 256, 190.0 / 256, 1.0 };
	material.diffuse = new float[4]{ 190.0 / 256, 190.0 / 256, 190.0 / 256, 1.0 };
	material.specular = new float[4]{ 190.0 / 256, 190.0 / 256, 190.0 / 256, 1.0 };
	material.emission = new float[4]{ 0.0f, 0.0f, 0.0f, 0.0f };
	material.shininess = 0.5f * 128;

	shader = shader_manager.loadfromFile(vertexShader, fragmentShader);
	if (shader == 0)
		std::cout << "Error (shaders are not loaded)\n";
}

void RecalcLight() {
	for (int i = 0; i < lights_count; i++) {
		if (lights[i].isActive) {
			unsigned light_ind = GL_LIGHT0 + i;
			glEnable(light_ind);
			glLightfv(light_ind, GL_POSITION, lights[i].position);
			glLightfv(light_ind, GL_AMBIENT, lights[i].ambient);
			glLightfv(light_ind, GL_DIFFUSE, lights[i].diffuse);
			glLightfv(light_ind, GL_SPECULAR, lights[i].specular);
		}
	}

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   material.ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   material.diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  material.specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  material.emission);
	glMaterialf(GL_FRONT_AND_BACK,  GL_SHININESS, material.shininess);
}

void Resize(int w, int h)
{
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60, (GLdouble)w / (GLdouble)h, 1, 1000);
	gluLookAt(0, 0, 0, 0, 0, -400, 0, 1, 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslated(0, 0, -500);
}

void Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();

	if (!shader) 
		exit(1);

	shader->begin();

		glPushMatrix();
		glRotatef(rotateX, 1, 0, 0);
		glRotatef(rotateY, 0, 1, 0);
		DrawCube(70);
		glPopMatrix();

		for (int i = 0; i < lights_count; i++) {
			glPushMatrix();
			if (lights[i].isActive)
				glTranslatef(lights[i].position[0], lights[i].position[1], lights[i].position[2]);
			glutSolidSphere(15, 10, 10);
			glPopMatrix();
		}

	shader->end();

	RecalcLight();
	glutSwapBuffers();

	glPopMatrix();

	glutPostRedisplay();
}

void specialKey(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		rotateX += 5.0f;
		break;
	case GLUT_KEY_DOWN:
		rotateX -= 5.0f;
		break;
	case GLUT_KEY_RIGHT:
		rotateY += 5.0f;
		break;
	case GLUT_KEY_LEFT:
		rotateY -= 5.0f;
		break;
	case GLUT_KEY_F1:
		lights[0].isActive = lights[0].isActive ? false : true;
		break;
	case GLUT_KEY_F2:
		lights[1].isActive = lights[1].isActive ? false : true;
		break;
	case GLUT_KEY_F3:
		lights[2].isActive = lights[2].isActive ? false : true;
		break;
	}
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'w':
		lights[0].position[1] += 5;
		break;
	case 's':
		lights[0].position[1] -= 5;
		break;
	case 'a':
		lights[0].position[0] -= 5;
		break;
	case 'd':
		lights[0].position[0] += 5;
		break;

	case 't':
		lights[1].position[1] += 5;
		break;
	case 'g':
		lights[1].position[1] -= 5;
		break;
	case 'f':
		lights[1].position[0] -= 5;
		break;
	case 'h':
		lights[1].position[0] += 5;
		break;

	case 'i':
		lights[2].position[1] += 5;
		break;
	case 'k':
		lights[2].position[1] -= 5;
		break;
	case 'j':
		lights[2].position[0] -= 5;
		break;
	case 'l':
		lights[2].position[0] += 5;
		break;
	}
	glutPostRedisplay();
}

int main(int argc, char ** argv) {
	glutInit(&argc, argv);
	
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Lightning Model");

	Init();
	glutDisplayFunc(Render);
	glutReshapeFunc(Resize);
	glutSpecialFunc(specialKey);
	glutKeyboardFunc(keyboard);

	glutMainLoop();
	return 0;
}