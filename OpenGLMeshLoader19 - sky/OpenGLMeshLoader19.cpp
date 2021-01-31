#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <string>
#include <string>

#include <ctime>
#include <iostream>
#include <vector>
#include <array>

#include <windows.h>
#include <mmsystem.h>
#include <Mmsystem.h>
#include <mciapi.h>
#include <iostream>
#pragma comment(lib, "Winmm.lib")

using namespace std;

#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)

int WIDTH = 1280;
int HEIGHT = 720;

GLuint tex;
char title[] = "3D Model Loader Sample";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 100;

const int INF = 1000000000;
const int GRID_LENGTH = 15;
const int GRID_WIDTH = 15;

double red = 1;
double green = 1;
double blue = 1;

vector<int> snake_body_x;
vector<int> snake_body_z;

vector<int> obstacle_wall_x;
vector<int> obstacle_wall_z;

char dir = 'U';
char prevDir = 'U';

int food_x = INF;
int food_z = INF;

int score = 0;

bool GAME_OVER = false;
bool LEVEL_TWO = true;

bool FIRST_PERSON = true;
bool SECOND_PERSON = true;

int CURRENT_TIME = 0;
int CURRENT_TIME_TICK = 0;
int orientation = 0;// 0-> UP, 1->RIGHT, 2->DOWN, 3->LEFT

int GAME_TIME_OUT = 30000;

int LEVEL_NUMBER = 1;

float lightposx;
float lightposz;

const int TARGET_GOAL_GOUNT = 5;

class Vector {
public:
	GLdouble x, y, z;
	Vector() {}
	Vector(GLdouble _x, GLdouble _y, GLdouble _z) : x(_x), y(_y), z(_z) {}
	//================================================================================================//
	// Operator Overloading; In C++ you can override the behavior of operators for you class objects. //
	// Here we are overloading the += operator to add a given value to all vector coordinates.        //
	//================================================================================================//
	void operator +=(float value)
	{
		x += value;
		y += value;
		z += value;
	}
};

Vector Eye(20, 5, 20);
Vector At(0, 0, 0);
Vector Up(0, 1, 0);

int cameraZoom = 0;

// Model Variables
Model_3DS model_house;
Model_3DS model_tree;
Model_3DS model_apple;
Model_3DS model_pyramid;
Model_3DS model_cactus;
Model_3DS model_rock;
Model_3DS model_ice;
Model_3DS model_watermelon;

// Textures
GLTexture tex_ground;
GLTexture tex_ground_snow;
GLTexture tex_wall;
GLTexture tex_snake;

void RenderGround()
{
	//glDisable(GL_LIGHTING);	// Disable lighting 

	/*glColor3f(0.6, 0.6, 0.6);*/	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	if (LEVEL_NUMBER == 1) {
		glBindTexture(GL_TEXTURE_2D, tex_ground_snow.texture[0]);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture
	}

	glPushMatrix();

	glScalef(1.0 / 2.5, 1, 1.0 / 2.5);
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-20, 0, -20);
	glTexCoord2f(5, 0);
	glVertex3f(20, 0, -20);
	glTexCoord2f(5, 5);
	glVertex3f(20, 0, 20);
	glTexCoord2f(0, 5);
	glVertex3f(-20, 0, 20);
	glEnd();

	glPopMatrix();

	//glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void LoadAssets()
{
	// Loading Model files
	//model_house.Load("Models/house/house.3DS");
	//model_tree.Load("Models/tree/Tree1.3ds");
	model_apple.Load("Models/apple/apple.3DS");
	model_pyramid.Load("Models/pyramid/pyramid.3DS");
	//model_rock.Load("Models/rock/rock.3DS");
	model_cactus.Load("Models/cactus/cacutus.3DS");
	model_ice.Load("Models/ice/ice.3DS");
	model_watermelon.Load("Models/watermelon/watermelon.3DS");

	// Loading texture files
	tex_snake.Load("Textures/snake.bmp");
	tex_ground.Load("Textures/ground2.bmp");
	tex_ground_snow.Load("Textures/snow.bmp");

	//tex_wall.Load("Models/tree/bark_loo.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}

class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class Camera {
public:
	Vector3f eye, center, up;

	Camera(float eyeX = 1.0f, float eyeY = 1.0f, float eyeZ = 1.0f, float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}
	void topView() {
		eye = Vector3f(0.0f, 0.5f, 0.0001f);
		center = Vector3f(0.0f, 0.0f, 0.0f);
		up = Vector3f(0.0f, 1.0f, 0.0f);
	}

	void frontView() {
		eye = Vector3f(0.0f, 0.0f, 1.0f);
		center = Vector3f(0.0f, 0.0f, 0.0f);
		up = Vector3f(0.0f, 1.0f, 0.0f);
	}

	void sideView() {
		eye = Vector3f(1.0f, 0.0f, 0.0f);
		center = Vector3f(0.0f, 0.0f, 0.0f);
		up = Vector3f(0.0f, 1.0f, 0.0f);
	}

	void cornerView() {
		eye = Vector3f(1.0f, 1.0f, 1.0f);
		center = Vector3f(0.0f, 0.0f, 0.0f);
		up = Vector3f(0.0f, 1.0f, 0.0f);
	}

	void moveX(float d) {
		Vector3f right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void moveY(float d) {
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d) {
		Vector3f view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}

	void rotateX(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}
};

Camera camera;
void Timer(int value);

float d0R;
float d0G;
float d0B;
float a0R;
float a0G;
float a0B;
float d1R;
float d1G;
float d1B;
float a1R;
float a1G;
float a1B;

void setupCamera(double fovy) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, 640 / 480, 0.001, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}

void Draw_Unit_Cube_Mesh(GLuint text) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, text);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	glBegin(GL_QUADS);
	// Front Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
	glTexCoord2f(0.5f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
	glTexCoord2f(0.5f, 0.5f); glVertex3f(0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.0f, 0.5f); glVertex3f(-0.5f, 0.5f, 0.5f);
	// Back Face
	glTexCoord2f(0.5f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
	glTexCoord2f(0.5f, 0.5f); glVertex3f(-0.5f, 0.5f, -0.5f);
	glTexCoord2f(0.0f, 0.5f); glVertex3f(0.5f, 0.5f, -0.5f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
	// Top Face
	glTexCoord2f(0.0f, 0.5f); glVertex3f(-0.5f, 0.5f, -0.5f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.5f, 0.0f); glVertex3f(0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.5f, 0.5f); glVertex3f(0.5f, 0.5f, -0.5f);
	// Bottom Face
	glTexCoord2f(0.5f, 0.5f); glVertex3f(-0.5f, -0.5f, -0.5f);
	glTexCoord2f(0.0f, 0.5f); glVertex3f(0.5f, -0.5f, -0.5f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
	glTexCoord2f(0.5f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
	// Right face
	glTexCoord2f(0.5f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
	glTexCoord2f(0.5f, 0.5f); glVertex3f(0.5f, 0.5f, -0.5f);
	glTexCoord2f(0.0f, 0.5f); glVertex3f(0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
	// Left Face
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
	glTexCoord2f(0.5f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
	glTexCoord2f(0.5f, 0.5f); glVertex3f(-0.5f, 0.5f, 0.5f);
	glTexCoord2f(0.0f, 0.5f); glVertex3f(-0.5f, 0.5f, -0.5f);
	glEnd();
}

void Draw_Unit_Cube_AtOrigin(GLuint text) {
	glPushMatrix();
	glTranslated(0.5, 0.5, 0.5);
	Draw_Unit_Cube_Mesh(text);
	glPopMatrix();
}

int rep = 5;
void Draw_Unit_Cube(GLuint text) {
	glPushMatrix();
	glTranslated(-0.5, -0.5, -0.5);
	for (int i = 0; i < rep; i++) {
		for (int j = 0; j < rep; j++) {
			glPushMatrix();
			glTranslated(i * 1.0 / rep, 0, j * 1.0 / rep);
			glScaled(1.0 / rep, 1.0, 1.0 / rep);
			Draw_Unit_Cube_AtOrigin(text);
			glPopMatrix();
		}
	}
	glPopMatrix();
}

double randDouble(double min, double max) {
	return (max - min) * ((double)rand() / (double)RAND_MAX) + min;
}

void generateRandomColor() {
	red = randDouble(0, 1);
	green = randDouble(0, 1);
	blue = randDouble(0, 1);
}

void setupLights() {
	GLfloat shininess[] = { 50 };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

	GLfloat lightPosition0[] = { -0.5f, 2.0f, 0.0f, 0.0f };
	GLfloat diffuse0[] = { 0.98f, 0.83f, 0.25f, 1.0f };
	if (LEVEL_NUMBER == 1) {
		diffuse0[0] = 0.1f;
		diffuse0[1] = 0.1f;
		diffuse0[2] = 0.3f;
	}
	else if (LEVEL_NUMBER >= 3) {
		diffuse0[0] = red;
		diffuse0[1] = green;
		diffuse0[2] = blue;
	}

	GLfloat ambient0[] = { 0.0f, 0.0f, 0.00f, 1.0f };
	GLfloat specular0[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition0);

	GLfloat lightPosition1[] = { lightposx,0.02375f,lightposz,1.0f };
	GLfloat diffuse1[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat ambient1[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat specular1[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat direction1[] = { 0.0f, 0.0f, 0.0f };

	if (dir == 'U') {
		direction1[2] = -1.0f;
		lightposx = (snake_body_x[0] * 1.0 / GRID_LENGTH);
		lightposz = (snake_body_z[0] * 1.0 / GRID_LENGTH) + 0.02375 * 10;
	}
	if (dir == 'D') {
		direction1[2] = 1.0f;
		lightposx = (snake_body_x[0] * 1.0 / GRID_LENGTH);
		lightposz = (snake_body_z[0] * 1.0 / GRID_LENGTH) - 0.02375 * 10;
	}
	if (dir == 'L') {
		direction1[0] = -1.0f;
		lightposx = (snake_body_x[0] * 1.0 / GRID_LENGTH) + 0.02375 * 10;
		lightposz = (snake_body_z[0] * 1.0 / GRID_LENGTH);
	}
	if (dir == 'R') {
		direction1[0] = 1.0f;
		lightposx = (snake_body_x[0] * 1.0 / GRID_LENGTH) - 0.02375 * 10;
		lightposz = (snake_body_z[0] * 1.0 / GRID_LENGTH);
	}

	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse1);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular1);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, direction1);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 10.0f);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 0);
}

void Mouse(int button, int state, int x, int y) {
	if (state == GLUT_UP) {
		if (button == GLUT_RIGHT_BUTTON) {
			orientation++;
		}
		if (button == GLUT_LEFT_BUTTON) {
			orientation--;
		}
		orientation = (orientation + 4) % 4;
	}
	glutPostRedisplay();
}

void Set_Third_Camera() {
	if (dir == 'U') {
		camera.eye = Vector3f((snake_body_x[0] * 1.0) / GRID_LENGTH, 0.1f, (snake_body_z[0] * 1.0) / GRID_LENGTH);
		camera.center = camera.eye + Vector3f(0, -0.05, -0.5);
	}
	else if (dir == 'D') {
		camera.eye = Vector3f((snake_body_x[0] * 1.0) / GRID_LENGTH, 0.1f, (snake_body_z[0] * 1.0) / GRID_LENGTH);
		camera.center = camera.eye + Vector3f(0, -0.05, +0.5);
	}
	else if (dir == 'L') {
		camera.eye = Vector3f((snake_body_x[0] * 1.0) / GRID_LENGTH, 0.1f, (snake_body_z[0] * 1.0) / GRID_LENGTH);
		camera.center = camera.eye + Vector3f(-0.5, -0.05, 0);
	}
	else if (dir == 'R') {
		camera.eye = Vector3f((snake_body_x[0] * 1.0) / GRID_LENGTH, 0.1f, (snake_body_z[0] * 1.0) / GRID_LENGTH);
		camera.center = camera.eye + Vector3f(0.5, -0.05, 0);
	}
	camera.up = Vector3f(0.0f, 1.0f, 0.0f);
	camera.look();

}

void Set_First_Camera() {
	if (dir == 'U') {
		camera.eye = Vector3f((snake_body_x[0] * 1.0) / GRID_LENGTH, 0.025f, (snake_body_z[0] * 1.0) / GRID_LENGTH - 0.1);
		camera.center = camera.eye + Vector3f(0, 0, -0.5);
	}
	else if (dir == 'D') {
		camera.eye = Vector3f((snake_body_x[0] * 1.0) / GRID_LENGTH, 0.025f, (snake_body_z[0] * 1.0) / GRID_LENGTH + 0.1);
		camera.center = camera.eye + Vector3f(0, 0, +0.5);
	}
	else if (dir == 'L') {
		camera.eye = Vector3f((snake_body_x[0] * 1.0) / GRID_LENGTH - 0.1, 0.025f, (snake_body_z[0] * 1.0) / GRID_LENGTH);
		camera.center = camera.eye + Vector3f(-0.5, 0, 0);
	}
	else if (dir == 'R') {
		camera.eye = Vector3f((snake_body_x[0] * 1.0) / GRID_LENGTH + 0.1, 0.025f, (snake_body_z[0] * 1.0) / GRID_LENGTH);
		camera.center = camera.eye + Vector3f(0.5, 0, 0);
	}
	camera.up = Vector3f(0.0f, 1.0f, 0.0f);
	camera.look();
}

bool Is_Collided_With_Borders() {
	return snake_body_x[0] >= GRID_LENGTH / 2.0 ||
		snake_body_x[0] <= -GRID_LENGTH / 2.0 ||
		snake_body_z[0] >= GRID_LENGTH / 2.0 ||
		snake_body_z[0] <= -GRID_LENGTH / 2.0;
}

bool Is_Collided_With_Snake_Body(int x, int z, bool excludeHead) {
	for (int i = 0; i < snake_body_x.size(); i++) {
		if (i == 0 && excludeHead)continue;
		int snakeX = snake_body_x[i];
		int snakeZ = snake_body_z[i];
		if (x == snakeX && z == snakeZ) {
			return true;
		}
	}
	return false;
}

bool Is_Collided_With_Wall(int x, int z) {
	for (int i = 0; i < obstacle_wall_x.size(); i++) {
		int wallX = obstacle_wall_x[i];
		int wallZ = obstacle_wall_z[i];
		if (x == wallX && z == wallZ)return true;
	}
	return false;
}

void Draw_Food(double x, double y, double z) {
	glPushMatrix();
	glTranslatef(x, -0.15, z);
	glPushMatrix();
	glScalef(0.014, 0.014, 0.014);
	//glutSolidCube(1);
	if (LEVEL_NUMBER == 1) {
		model_watermelon.Draw();
	}
	else {
		//glColor3f(1, 0, 0);
		model_apple.Draw();
	}
	glPopMatrix();
	glPopMatrix();
}

void Draw_Wall_Unit(double x, double y, double z) {
	glPushMatrix();
	glTranslatef(x, 0, z);
	glPushMatrix();
	glScalef(0.01 / 2, 0.01 / 2, 0.01 / 2);
	model_pyramid.Draw();
	glPopMatrix();
	glPopMatrix();

	glEnable(GL_LIGHTING);
}

void Draw_Boarder_Unit(double x, double y, double z) {
	glPushMatrix();
	glTranslatef(x, 0, z);
	glPushMatrix();
	if (LEVEL_NUMBER == 1) {
		glScalef(0.05 / 2, 0.05 / 2, 0.05 / 2);
		model_ice.Draw();
	}

	else {
		glScalef(0.45, 0.2, 0.45);
		model_cactus.Draw();
	}
	glPopMatrix();
	glPopMatrix();

	glEnable(GL_LIGHTING);
}

void Draw_Wall() {
	for (int i = 0; i < obstacle_wall_x.size(); i++) {
		int wallX = obstacle_wall_x[i];
		int wallZ = obstacle_wall_z[i];
		Draw_Wall_Unit(wallX, 0, wallZ);
	}
}

void Update_Food_Position() {
	while (true) {
		food_x = (rand() % GRID_LENGTH) - (GRID_LENGTH / 2);
		food_z = (rand() % GRID_LENGTH) - (GRID_LENGTH / 2);
		if (!Is_Collided_With_Snake_Body(food_x, food_z, false) && !Is_Collided_With_Wall(food_x, food_z))break;
	}
}

void Generate_Obstacles() {
	obstacle_wall_x.clear();
	obstacle_wall_z.clear();
	for (int i = 0; i < 10; i++) {
		int wall_x = -1;
		int wall_z = -1;
		while (true) {
			wall_x = (rand() % GRID_LENGTH) - (GRID_LENGTH / 2);
			wall_z = (rand() % GRID_LENGTH) - (GRID_LENGTH / 2);
			if (!Is_Collided_With_Snake_Body(wall_x, wall_z, false) && wall_x != food_x && wall_z != food_z)break;
		}
		obstacle_wall_x.push_back(wall_x);
		obstacle_wall_z.push_back(wall_z);
	}
}

void Generate_Boarder() {
	for (int i = 0; i < 10; i++) {
		int x = -1;
		int z = -1;
		for (int i = 1; i <= GRID_LENGTH; i++) {

			Draw_Boarder_Unit(-(GRID_LENGTH / 2) - 1, 0, i - (GRID_LENGTH / 2) - 1);
			Draw_Boarder_Unit((GRID_LENGTH / 2) + 1, 0, i - (GRID_LENGTH / 2) - 1);
		}
		for (int i = 1; i <= GRID_LENGTH; i++) {
			glPushMatrix();
			glRotatef(90, 0, 1, 0);

			Draw_Boarder_Unit(-(GRID_LENGTH / 2) - 1, 0, i - (GRID_LENGTH / 2) - 1);
			Draw_Boarder_Unit((GRID_LENGTH / 2) + 1, 0, i - (GRID_LENGTH / 2) - 1);
			glPopMatrix();
		}
	}
}

bool Eat_Food() {
	if (snake_body_x[0] == food_x && snake_body_z[0] == food_z) {
		PlaySound(TEXT("sounds/eat.wav"), NULL, SND_ASYNC | SND_FILENAME);
		score++;
		Update_Food_Position();
		glutPostRedisplay();
		return true;
	}
	return false;
}

bool Is_Game_Over() {
	if (GAME_OVER)return true;
	GAME_OVER = CURRENT_TIME_TICK >= GAME_TIME_OUT ||
		Is_Collided_With_Borders() ||
		Is_Collided_With_Snake_Body(snake_body_x[0], snake_body_z[0], true) ||
		Is_Collided_With_Wall(snake_body_x[0], snake_body_z[0]);
	if (GAME_OVER)
		PlaySound(TEXT("sounds/dead.wav"), NULL, SND_ASYNC | SND_FILENAME);
	return GAME_OVER;
}

void Draw_Line(double x1, double y1, double z1, double x2, double y2, double z2) {
	glBegin(GL_LINE_STRIP);
	glVertex3f(x1, y1, z1);
	glVertex3f(x2, y2, z2);
	glEnd();
}

void Draw_Snake_Unit(double x, double y, double z) {
	//glColor3f(1, 248.0 / 256, 51.0 / 256);
	glPushMatrix();
	glTranslatef(x, 0.25, z);
	glPushMatrix();
	glScalef(1, 0.5, 1);
	//glutSolidCube(0.95);
	Draw_Unit_Cube(tex_snake.texture[0]);
	glPopMatrix();
	glPopMatrix();

	glEnable(GL_LIGHTING);
}

void Update_Head_Position() {
	CURRENT_TIME = 1;
	bool eatFood = Eat_Food();
	if (!eatFood && snake_body_x.size() && snake_body_z.size()) {
		snake_body_x.pop_back();
		snake_body_z.pop_back();
	}
	if (snake_body_x.size() == 0) {
		snake_body_x.push_back(0);
		snake_body_z.push_back(0);
	}

	int head_x = 0;
	if (snake_body_x.size()) {
		head_x = snake_body_x[0];
	}
	int head_z = 0;
	if (snake_body_x.size()) {
		head_z = snake_body_z[0];
	}

	if (FIRST_PERSON || SECOND_PERSON) {
		if (orientation == 0)dir = 'U';
		if (orientation == 1)dir = 'R';
		if (orientation == 2)dir = 'D';
		if (orientation == 3)dir = 'L';
	}
	else {
		if (dir == 'U')orientation = 0;
		if (dir == 'R')orientation = 1;
		if (dir == 'D')orientation = 2;
		if (dir == 'L')orientation = 3;
	}

	if (dir == 'U') {
		if (!eatFood)
			PlaySound(TEXT("sounds/up.wav"), NULL, SND_ASYNC | SND_FILENAME);
		head_z--;
	}
	else if (dir == 'D') {
		if (!eatFood)
			PlaySound(TEXT("sounds/down.wav"), NULL, SND_ASYNC | SND_FILENAME);
		head_z++;
	}
	else if (dir == 'L') {
		if (!eatFood)
			PlaySound(TEXT("sounds/left.wav"), NULL, SND_ASYNC | SND_FILENAME);
		head_x--;
	}
	else if (dir == 'R') {
		if (!eatFood)
			PlaySound(TEXT("sounds/right.wav"), NULL, SND_ASYNC | SND_FILENAME);
		head_x++;
	}

	prevDir = dir;

	snake_body_x.insert(snake_body_x.begin(), head_x);
	snake_body_z.insert(snake_body_z.begin(), head_z);
}

void Draw_Snake() {
	for (int i = 0; i < snake_body_x.size(); i++) {
		int x = snake_body_x[i];
		int z = snake_body_z[i];
		Draw_Snake_Unit(x, 0, z);
	}
}

void Draw_Grid() {
	for (double i = -GRID_LENGTH / 2.0; i <= GRID_LENGTH / 2.0; i++) {
		Draw_Line(-GRID_LENGTH / 2.0, 0, i, GRID_LENGTH / 2.0, 0, i);
		Draw_Line(i, 0, -GRID_LENGTH / 2.0, i, 0, GRID_LENGTH / 2.0);
	}
}

void drawStrokeText(char* string, int x, int y, int z) {
	char* c;
	glPushMatrix();
	glTranslatef(x, y, z);
	glScalef(0.0009f, 0.0008f, z);

	for (c = string; *c; c++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
	}
	glPopMatrix();
}

void print(double x, double y, string str) {
	int len, i;
	glRasterPos2f(x, y);
	len = str.size();
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str[i]);
	}
}

void Display() {
	if (GAME_OVER) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		setupCamera(60);

		camera.cornerView();
		glColor3f(0, 0, 0);
		if (score >= TARGET_GOAL_GOUNT) {
			print(-1.0, 0, "             WE HAVE A WINNER!");
			print(-1.0, -0.5, " Please press Space to move to Level " + to_string(LEVEL_NUMBER + 1));
		}
		else {
			print(-1.0, 0, "         Ooooooooooops! Game Over!");
			print(-1.0, -0.5, "       Please press Space to replay.");
		}
		print(-1.0, -1, "              The Score: " + to_string(score) + " / " + to_string(TARGET_GOAL_GOUNT));
	}
	else {
		float windowWidth = glutGet(GLUT_WINDOW_WIDTH);
		float windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0.0, windowWidth, 0.0, windowHeight);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_LIGHTING);
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glColor3f(1.0, 1.0, 1.0);
		print(30, windowHeight - 40, "Score: " + to_string(score) +
			+" / " + to_string(TARGET_GOAL_GOUNT));
		print(30, windowHeight - 80, "Time Left: " + to_string((GAME_TIME_OUT - CURRENT_TIME_TICK) / 1000 + 1));
		print(windowWidth / 2 - 20, 40
			, "Level " + to_string(LEVEL_NUMBER));

		glDepthMask(GL_FALSE);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_NORMALIZE);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glEnable(GL_COLOR_MATERIAL);

		glShadeModel(GL_SMOOTH);
		glDepthMask(GL_TRUE);

		if (!FIRST_PERSON && !SECOND_PERSON)
			setupCamera(60);
		else {
			setupCamera(120);
		}

		glPushMatrix();
		setupLights();
		glPopMatrix();

		glPushMatrix();
		glScalef(1.0 / GRID_LENGTH, 1.0 / GRID_WIDTH, 1.0 / GRID_WIDTH);


		if (LEVEL_NUMBER > 1 && obstacle_wall_x.size() == 0) {
			Generate_Obstacles();
		}

		glPushMatrix();
		GLUquadricObj* qobj;
		qobj = gluNewQuadric();
		glScalef(0.25, 0.25, 0.25);
		glTranslated(50, 0, 0);
		glRotated(90, 1, 0, 1);
		glBindTexture(GL_TEXTURE_2D, tex);
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		gluSphere(qobj, 100, 100, 100);
		gluDeleteQuadric(qobj);
		glPopMatrix();

		Draw_Wall();
		Draw_Grid();
		RenderGround();
		Draw_Snake();
		Draw_Food(food_x, 0, food_z);
		Generate_Boarder();

		glPopMatrix();

		if (FIRST_PERSON)
			Set_First_Camera();
		else if (SECOND_PERSON)
			Set_Third_Camera();
		else camera.cornerView();
	}
	glFlush();
}

void init() {
	snake_body_x.clear();
	snake_body_z.clear();
	snake_body_x.push_back(0);
	snake_body_z.push_back(0);

	obstacle_wall_x.clear();
	obstacle_wall_z.clear();

	dir = 'U';
	prevDir = 'U';

	food_x = INF;
	food_z = INF;

	score = 0;

	GAME_OVER = false;

	FIRST_PERSON = true;
	SECOND_PERSON = true;

	CURRENT_TIME = 0;
	CURRENT_TIME_TICK = 0;

	orientation = 0;// 0->UP, 1->RIGHT, 2->DOWN, 3->LEFT

	Update_Food_Position();
}

void Keyboard(unsigned char key, int x, int y) {
	float d = 0.01;
	switch (key) {
	case 'm':
		orientation++;
		orientation %= 4;
		break;
	case 'n':
		orientation--;
		orientation = (orientation + 4) % 4;
		break;
	case 'z':
		FIRST_PERSON = true;
		SECOND_PERSON = false;
		break;
	case 'x':
		FIRST_PERSON = false;
		SECOND_PERSON = true;
		break;
	case 'c':
		FIRST_PERSON = false;
		SECOND_PERSON = false;
		break;
	case ' ':
		if (GAME_OVER) {
			if (score >= TARGET_GOAL_GOUNT) {
				LEVEL_NUMBER++;
			}
			if (LEVEL_NUMBER >= 3 && score >= TARGET_GOAL_GOUNT) {
				generateRandomColor();
			}
			init();
		}
		break;
	case GLUT_KEY_ESCAPE:
		exit(EXIT_SUCCESS);
	}
	glutPostRedisplay();
}

void Special(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		if (prevDir != 'D') {
			dir = 'U';
		}
		break;
	case GLUT_KEY_DOWN:
		if (prevDir != 'U') {
			dir = 'D';
		}
		break;
	case GLUT_KEY_LEFT:
		if (prevDir != 'R') {
			dir = 'L';
		}
		break;
	case GLUT_KEY_RIGHT:
		if (prevDir != 'L') {
			dir = 'R';
		}
		break;
	}
	glutPostRedisplay();
}

int UPDATE_FOOD_POSITION_TIME = 0;

void Timer(int value) {
	Is_Game_Over();
	if (LEVEL_NUMBER == 1) {
		if (CURRENT_TIME % 150 == 0) {
			UPDATE_FOOD_POSITION_TIME++;
			if (!Is_Game_Over()) {
				Update_Head_Position();
				if (UPDATE_FOOD_POSITION_TIME % 20 == 0) {
					PlaySound(TEXT("sounds/woosh.wav"), NULL, SND_ASYNC | SND_FILENAME);
					Update_Food_Position();
				}
			}
			glutPostRedisplay();
		}
	}
	else if (LEVEL_NUMBER == 2) {
		if (CURRENT_TIME % 250 == 0) {
			UPDATE_FOOD_POSITION_TIME++;
			if (!Is_Game_Over()) {
				Update_Head_Position();
				if (UPDATE_FOOD_POSITION_TIME % 30 == 0) {
					PlaySound(TEXT("sounds/woosh.wav"), NULL, SND_ASYNC | SND_FILENAME);
					Update_Food_Position();
				}
			}
			glutPostRedisplay();
		}
	}
	else {
		if (CURRENT_TIME % (300 - (min((LEVEL_NUMBER - 3) * 50, 299))) == 0) {
			UPDATE_FOOD_POSITION_TIME++;
			if (!Is_Game_Over()) {
				Update_Head_Position();
				if (UPDATE_FOOD_POSITION_TIME % (50 + (10 * (LEVEL_NUMBER - 3))) == 0) {
					PlaySound(TEXT("sounds/woosh.wav"), NULL, SND_ASYNC | SND_FILENAME);
					Update_Food_Position();
				}
				if (UPDATE_FOOD_POSITION_TIME % (10 + (5 * (LEVEL_NUMBER - 3))) == 0) {
					PlaySound(TEXT("sounds/maze.wav"), NULL, SND_ASYNC | SND_FILENAME);
					Generate_Obstacles();
				}
			}
			glutPostRedisplay();
		}
	}

	CURRENT_TIME++;
	CURRENT_TIME_TICK++;
	glutTimerFunc(1, Timer, 0);
}

void main(int argc, char** argv) {
	srand(time(NULL));
	// LIGHT VARIABLES
	d0R = rand() % 256;
	d0R = d0R / 255;
	d0G = rand() % 256;
	d0G = d0G / 255;
	d0B = rand() % 256;
	d0B = d0B / 255;
	d1R = rand() % 256;
	d1R = d1R / 255;
	d1G = rand() % 256;
	d1G = d0G / 255;
	d1B = rand() % 256;
	d1B = d0B / 255;
	a0R = 1 - d0R;
	a0G = 1 - d0G;
	a0B = 1 - d0B;
	a1R = 1 - d1R;
	a1G = 1 - d1G;
	a1B = 1 - d1B;
	// START GLOBALE VARIABLES INIT
	snake_body_x.push_back(0);
	snake_body_z.push_back(0);
	// END GLOBAL VARIABLES INIT
	glutInit(&argc, argv);

	glutInitWindowSize(800, 700);
	glutInitWindowPosition(50, 50);

	glutCreateWindow("ts ts 2na 2l so3ban");
	glutDisplayFunc(Display);
	glutTimerFunc(0, Timer, 0);

	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(Special);
	glutMouseFunc(Mouse);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	LoadAssets();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);
	int current_time = std::time(NULL);
	std::srand(current_time);

	Update_Food_Position();
	glutMainLoop();
}
