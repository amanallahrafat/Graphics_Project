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

class Vector
{
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

// Textures
GLTexture tex_ground;
GLTexture tex_wall;
GLTexture tex_snake;

//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*******************************************************************************************//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*******************************************************************************************//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	//*******************************************************************************************//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*******************************************************************************************//

	InitLightSource();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround()
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

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

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

	// Draw Ground
	RenderGround();

	// Draw Tree Model
	glPushMatrix();
	glTranslatef(10, 0, 0);
	glScalef(0.7, 0.7, 0.7);
	model_tree.Draw();
	glPopMatrix();

	// Draw house Model
	glPushMatrix();
	glRotatef(90.f, 1, 0, 0);
	model_house.Draw();
	glPopMatrix();


	//sky box
	glPushMatrix();

	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(50, 0, 0);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 100, 100, 100);
	gluDeleteQuadric(qobj);


	glPopMatrix();



	glutSwapBuffers();
}

//=======================================================================
// Keyboard Function
//=======================================================================
void myKeyboard(unsigned char button, int x, int y)
{
	switch (button)
	{
	case 'w':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 'r':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

//=======================================================================
// Motion Function
//=======================================================================
void myMotion(int x, int y)
{
	y = HEIGHT - y;

	if (cameraZoom - y > 0)
	{
		Eye.x += -0.1;
		Eye.z += -0.1;
	}
	else
	{
		Eye.x += 0.1;
		Eye.z += 0.1;
	}

	cameraZoom = y;

	glLoadIdentity();	//Clear Model_View Matrix

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glutPostRedisplay();	//Re-draw scene 
}

//=======================================================================
// Mouse Function
//=======================================================================
void myMouse(int button, int state, int x, int y)
{
	y = HEIGHT - y;

	if (state == GLUT_DOWN)
	{
		cameraZoom = y;
	}
}

//=======================================================================
// Reshape Function
//=======================================================================
void myReshape(int w, int h)
{
	if (h == 0) {
		h = 1;
	}

	WIDTH = w;
	HEIGHT = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading Model files
	model_house.Load("Models/house/house.3DS");
	model_tree.Load("Models/tree/Tree1.3ds");
	model_apple.Load("Models/apple/apple.3DS");

	// Loading texture files
	tex_snake.Load("Textures/snake2.bmp");
	tex_ground.Load("Textures/ground2.bmp");
	tex_wall.Load("Models/tree/bark_loo.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}



//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
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

void setupLights() {
	GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 1.0f };
	GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0, 1.0f };
	GLfloat shininess[] = { 50 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

	GLfloat lightIntensity[] = { 0.7f, 0.7f, 1, 1.0f };
	GLfloat lightPosition[] = { -7.0f, 6.0f, 3.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightIntensity);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
}

void setupCamera(double fovy) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, 640 / 480, 0.001, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}

void Draw_Unit_Cube(GLuint text) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, text);    // Background texture
   // We will specify texture coordinates
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

void drawPlane() {
	glPushMatrix();
	glScaled(1.0, 0.002, 1.0);
	glutSolidCube(1);
	glPopMatrix();
}

void drawXYZ() {
	glColor3f(1, 0, 0);
	drawPlane();
	glPushMatrix();

	glColor3f(0, 1, 0);
	glRotated(90, 0, 0, 1.0);
	drawPlane();
	glPopMatrix();

	glColor3f(0, 0, 1);
	glPushMatrix();
	glRotated(-90, 1.0, 0.0, 0.0);
	drawPlane();
	glPopMatrix();
}
const int INF = 1000000000;
const int GRID_LENGTH = 15;
const int GRID_WIDTH = 15;

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
int orientation = 0;// 0-> UP, 1->RIGHT, 2->DOWN, 3->LEFT

void Mouse(int button, int state, int x, int y) {
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) orientation++;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) orientation--;
	orientation = (orientation + 4) % 4;
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
		if (x == snakeX && z == snakeZ)return true;
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
	glColor3f(1, 0, 0);
	glPushMatrix();
	glTranslatef(x, -0.15, z);
	glPushMatrix();
	glScalef(0.014, 0.014, 0.014);
	//glutSolidCube(1);
	model_apple.Draw();
	glPopMatrix();
	glPopMatrix();
}

void Draw_Wall_Unit(double x, double y, double z) {
	glColor3f(1, 1, 1);
	glPushMatrix();
	glTranslatef(x, 0.5, z);
	glPushMatrix();
	glScalef(1, 1.5, 1);
	glutSolidCube(0.98);
	Draw_Unit_Cube(tex_wall.texture[0]);
	glPopMatrix();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Disable lighting 
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
		std::cout << wall_x;
	}
}

bool Eat_Food() {
	if (snake_body_x[0] == food_x && snake_body_z[0] == food_z) {
		score++;
		Update_Food_Position();
		glutPostRedisplay();
		return true;
	}
	return false;
}

bool Is_Game_Over() {
	return GAME_OVER = GAME_OVER ||
		Is_Collided_With_Borders() ||
		Is_Collided_With_Snake_Body(snake_body_x[0], snake_body_z[0], true) ||
		Is_Collided_With_Wall(snake_body_x[0], snake_body_z[0]);
}

void Draw_Line(double x1, double y1, double z1, double x2, double y2, double z2) {
	glBegin(GL_LINE_STRIP);
	glVertex3f(x1, y1, z1);
	glVertex3f(x2, y2, z2);
	glEnd();
}

void Draw_Snake_Unit(double x, double y, double z) {
	glColor3f(1, 248.0/256, 51.0/256);
	glPushMatrix();
	glTranslatef(x, 0.5, z);
	glPushMatrix();
	glScalef(1, 0.5, 1);
	glutSolidCube(0.95);
	Draw_Unit_Cube(tex_snake.texture[0]);
	glPopMatrix();
	glPopMatrix();

	glEnable(GL_LIGHTING);
}

void Update_Head_Position() {
	CURRENT_TIME = 1;

	if (!Eat_Food() && snake_body_x.size() && snake_body_z.size()) {
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

	if (dir == 'U')head_z--;
	else if (dir == 'D')head_z++;
	else if (dir == 'L')head_x--;
	else if (dir == 'R')head_x++;

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

void Display() {
	if (!FIRST_PERSON && !SECOND_PERSON)
		setupCamera(60);
	else {
		setupCamera(120);
	}
	setupLights();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glScalef(1.0 / GRID_LENGTH, 1.0 / GRID_WIDTH, 1.0 / GRID_WIDTH);

	if (LEVEL_TWO) {
		LEVEL_TWO = false;
		Generate_Obstacles();
	}
	Draw_Wall();
	Draw_Grid();
	RenderGround();
	Draw_Snake();
	Draw_Food(food_x, 0, food_z);
	glPopMatrix();

	if (FIRST_PERSON)
		Set_First_Camera();
	else if (SECOND_PERSON)
		Set_Third_Camera();
	else camera.cornerView();

	glFlush();
}

void Keyboard(unsigned char key, int x, int y) {
	float d = 0.01;
	switch (key) {
	case 'w':
		camera.moveY(d);
		break;
	case 's':
		camera.moveY(-d);
		break;
	case 'a':
		camera.moveX(d);
		break;
	case 'd':
		camera.moveX(-d);
		break;
	case 'q':
		camera.moveZ(d);
		break;
	case 'e':
		camera.moveZ(-d);
		break;
	case 't':
		camera.topView();
		camera.moveZ(-0.4);
		break;
	case 'h':
		camera.sideView();
		camera.moveZ(-0.4);
		break;
	case 'f':
		camera.frontView();
		camera.moveZ(-0.4);
		break;
	case 'g':
		camera.cornerView();
		camera.moveZ(0);
		break;
	case 'i': // UP
		if (prevDir != 'D') {
			dir = 'U';
		}
		break;
	case 'k': // DOWN
		if (prevDir != 'U') {
			dir = 'D';
		}
		break;
	case 'j': // LEFT
		if (prevDir != 'R') {
			dir = 'L';
		}
		break;
	case 'l': // RIGHT
		if (prevDir != 'L') {
			dir = 'R';
		}
		break;
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


		break;
	case GLUT_KEY_ESCAPE:
		exit(EXIT_SUCCESS);
	}
	glutPostRedisplay();
}

void Special(int key, int x, int y) {
	float a = 1.0;
	switch (key) {
	case GLUT_KEY_UP:
		camera.rotateX(a);
		break;
	case GLUT_KEY_DOWN:
		camera.rotateX(-a);
		break;
	case GLUT_KEY_LEFT:
		camera.rotateY(a);
		break;
	case GLUT_KEY_RIGHT:
		camera.rotateY(-a);
		break;
	}
	glutPostRedisplay();
}

void Timer(int value) {
	Is_Game_Over();
	if (CURRENT_TIME % 30 == 0) {
		if (!Is_Game_Over()) {
			Update_Head_Position();
			glutPostRedisplay();
		}
	}
	CURRENT_TIME++;
	glutTimerFunc(1, Timer, 0);
}

void main(int argc, char** argv) {
	// START GLOBALE VARIABLES INIT
	snake_body_x.push_back(0);
	snake_body_z.push_back(0);
	// END GLOBAL VARIABLES INIT
	glutInit(&argc, argv);

	glutInitWindowSize(800, 700);
	glutInitWindowPosition(50, 50);

	glutCreateWindow("Assignment 2");
	glutDisplayFunc(Display);
	glutTimerFunc(0, Timer, 0); // sets the Timer handler function; which runs every `Threshold` milliseconds (1st argument)

	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(Special);
	glutMouseFunc(Mouse);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);
	int current_time = std::time(NULL);
	std::srand(current_time);

	Update_Food_Position();

	glutMainLoop();
}

//=======================================================================
// Main Function
//=======================================================================
//void main(int argc, char** argv)
//{
//	glutInit(&argc, argv);
//
//	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
//
//	glutInitWindowSize(WIDTH, HEIGHT);
//
//	glutInitWindowPosition(100, 150);
//
//	glutCreateWindow(title);
//
//	glutDisplayFunc(Display);
//
//	glutKeyboardFunc(myKeyboard);
//
//	glutMotionFunc(myMotion);
//
//	glutMouseFunc(myMouse);
//
//	glutReshapeFunc(myReshape);
//
//	myInit();
//
//	LoadAssets();
//	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_LIGHTING);
//	glEnable(GL_LIGHT0);
//	glEnable(GL_NORMALIZE);
//	glEnable(GL_COLOR_MATERIAL);
//
//	glShadeModel(GL_SMOOTH);
//
//	glutMainLoop();
//}