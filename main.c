/*******************************************************************
           Multi-Part Model Construction and Manipulation
********************************************************************/
//Brandon Borg
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include "Vector3D.h"
#include "CubeMesh.h"
#include "QuadMesh.h"

const int meshSize = 16;    // Default Mesh Size
const int vWidth = 650;     // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

static int currentButton;
static unsigned char currentKey;

//Camera pos x,y,z
float cpx = 0;
float cpy = 6;
float cpz = 20;

//roatate var
float _angle = 0;
float _bAngle = 0;
float _xmovment = 0;
float _zmovment = 0;
float _height = 0;
float _rate = 0;

// Lighting/shading and material properties for submarine - upcoming lecture - just copy for now

// Light properties
static GLfloat light_position0[] = { -6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_position1[] = { 6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

// Material properties
static GLfloat submarine_mat_ambient[] = { 0.4F, 0.2F, 0.0F, 1.0F };
static GLfloat submarine_mat_specular[] = { 0.1F, 0.1F, 0.0F, 1.0F };
static GLfloat submarine_mat_diffuse[] = { 0.9F, 0.5F, 0.0F, 1.0F };
static GLfloat submarine_mat_shininess[] = { 0.0F };

// A quad mesh representing the ground / sea floor 
static QuadMesh groundMesh;

// Structure defining a bounding box, currently unused
//struct BoundingBox {
//    Vector3D min;
//    Vector3D max;
//} BBox;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
Vector3D ScreenToWorld(int x, int y);

//update controlls the movment of the sub
void update(int value)
{
	//NOTE I put a cap on how fast it can go
	{
		//finding the direction the sub is facing and moving accordingly
		_xmovment += sin(_angle * (3.14 / 180)) * _rate; 
		_zmovment += cos(_angle * (3.15 / 180)) * _rate;
		_bAngle += -100 * _rate;
		if (_bAngle > 360)
		{
			_bAngle -= 360;
		}

		glutPostRedisplay();

		glutTimerFunc(25, update, 0);
	}
}


int main(int argc, char **argv)
{
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(vWidth, vHeight);
    glutInitWindowPosition(200, 30);
    glutCreateWindow("Assignment 1");

    // Initialize GL
    initOpenGL(vWidth, vHeight);

    // Register callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotionHandler);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(functionKeys);


	glutTimerFunc(25, update, 0);
	printf("Brandon Borg \n500-678-848 \nCPS511 Assignment 1\n");
    // Start event loop, never returns
    glutMainLoop();

    return 0;
}


// Set up OpenGL. For viewport and projection setup see reshape(). */
void initOpenGL(int w, int h)
{
    // Set up and enable lighting
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    //glEnable(GL_LIGHT1);   // This light is currently off

    // Other OpenGL setup
    glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
    glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
    glClearColor(0.6F, 0.6F, 0.6F, 0.0F);  // Color and depth for glClear
    glClearDepth(1.0f);
    glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

    // Set up ground/sea floor quad mesh
    Vector3D origin = NewVector3D(-8.0f, 0.0f, 8.0f);
    Vector3D dir1v = NewVector3D(1.0f, 0.0f, 0.0f);
    Vector3D dir2v = NewVector3D(0.0f, 0.0f, -1.0f);
    groundMesh = NewQuadMesh(meshSize);
    InitMeshQM(&groundMesh, meshSize, origin, 16.0, 16.0, dir1v, dir2v);

    Vector3D ambient = NewVector3D(0.0f, 0.05f, 0.0f);
    Vector3D diffuse = NewVector3D(0.4f, 0.8f, 0.4f);
    Vector3D specular = NewVector3D(0.04f, 0.04f, 0.04f);
    SetMaterialQM(&groundMesh, ambient, diffuse, specular, 0.2);

    // Set up the bounding box of the scene
    // Currently unused. You could set up bounding boxes for your objects eventually.
    //Set(&BBox.min, -8.0f, 0.0, -8.0);
    //Set(&BBox.max, 8.0f, 6.0,  8.0);
}


// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(cpx, cpy, cpz, cpx, cpy - 6, cpz - 20, 0.0, 1.0, 0.0);

    // Draw Submarine

    // Set submarine material properties
    glMaterialfv(GL_FRONT, GL_AMBIENT, submarine_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, submarine_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, submarine_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, submarine_mat_shininess);

    // Apply transformations to move submarine
    // ...

	
	glPushMatrix();


	glPushMatrix();

	glTranslatef(_xmovment, _height, _zmovment);

	//glTranslatef(_movment, _height, _movment);

	glRotatef(_angle, 0, 1, 0);
	glRotatef(45.0, 0.0, 1.0, 0.0);
	// Apply transformations to construct submarine, modify this!
	
	//body
	glPushMatrix();
	glTranslatef(0.0, 4.0, 0.0);
	glRotatef(45.0, 0.0, 1.0, 0.0);
	glScalef(5.0, .5, .5);
	glutSolidSphere(1.0, 50, 50);
	glPopMatrix();

	//topbase
	glPushMatrix();
	glRotatef(45, 0, 1, 0);
	glTranslatef(-.3, 4.32, 0);
	glScalef(3, .4, .6);
	glutSolidCube(1);
	glPopMatrix();

	//top object
	glPushMatrix();
	glRotatef(-135, 0, 1, 0);
	glTranslatef(-.5, 4.5, -.25);
	glScalef(.6, .2, .5);
	glBegin(GL_QUADS);
	//front
	glNormal3d(0, 0, 1);
	glVertex3f(0, 0, 1);
	glVertex3f(3, 0, 1);
	glVertex3f(2.5, 2, 1);
	glVertex3f(-.5, 2, 1);

	//right
	glNormal3d(1, 0, 0);
	glVertex3f(2.5, 2, 0);
	glVertex3f(2.5, 2, 1);
	glVertex3f(3, 0, 1);
	glVertex3f(3, 0, -0);

	//back
	glNormal3d(0, 0, -1);
	glVertex3f(0, 0, 0);
	glVertex3f(3, 0, 0);
	glVertex3f(2.5, 2, 0);
	glVertex3f(-.5, 2, 0);

	//left
	glNormal3d(1, 0, 0);
	glVertex3f(-.5, 2, 0);
	glVertex3f(-.5, 2, 1);
	glVertex3f(0, 0, 1);
	glVertex3f(0, 0, -0);

	//top
	glNormal3d(0, 1, 0);
	glVertex3f(-.5, 2, 0);
	glVertex3f(2.5, 2, 0);
	glVertex3f(2.5, 2, 1);
	glVertex3f(-.5, 2, 1);

	glEnd();
	glPopMatrix();

	//Creating the quadric obj  
	GLUquadricObj *Cyl1 = gluNewQuadric();
	gluQuadricNormals(Cyl1, GLU_SMOOTH);

	//top Cylinder

	glPushMatrix();
	glRotatef(-90, 1, 0, 0);
	glScalef(.2, .2, .2);
	glTranslatef(-2, -2, 23.5);
	gluCylinder(Cyl1, 1, 1, 4, 50, 50);
	glTranslated(0, 0, 4);
	gluDisk(Cyl1, 0, 1, 50, 50);//Used disk to close off cylinder, does same transformations
	glPopMatrix();

	//scope
	glPushMatrix();
	glTranslatef(-.4, 5.3, .4);
	glRotatef(-45, 0, 1, 0);
	glScalef(.2, .2, .1);
	gluCylinder(Cyl1, .7, 1, 4, 50, 50);
	glTranslated(0, 0, 4);
	gluDisk(Cyl1, 0, 1, 50, 50);
	glPopMatrix();


	//wing1
	glPushMatrix();
	glTranslatef(2.5, 4, -2.4);
	glRotatef(45, 0, 1, 0);
	glScalef(.5, .8, .25);
	glBegin(GL_TRIANGLES);
	//front
	glNormal3d(0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(2, 1, 0);
	glVertex3f(2, -1, 0);

	//right
	glNormal3d(1, 0, 0);
	glVertex3f(2, 1, 0);
	glVertex3f(2, -1, 0);
	glVertex3f(2, -1, -.2);

	glNormal3d(1, 0, 0);
	glVertex3f(2, 1, -.2);
	glVertex3f(2, -1, -.2);
	glVertex3f(2, 1, 0);

	//back
	glNormal3d(0, 0, -1);
	glVertex3f(0, 0, -.2);
	glVertex3f(2, 1, -.2);
	glVertex3f(2, -1, -.2);

	//top
	glNormal3d(-1, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(2, 1, 0);
	glVertex3f(2, 1, -.2);

	glNormal3d(-1, 1, 0);
	glVertex3f(0, 0, -.2);
	glVertex3f(2, 1, -.2);
	glVertex3f(0, 0, 0);

	//bottom
	glNormal3d(-1, -1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(2, -1, 0);
	glVertex3f(2, -1, -.2);

	glNormal3d(-1, -1, 0);
	glVertex3f(0, 0, -.2);
	glVertex3f(2, -1, -.2);
	glVertex3f(0, 0, 0);
	glEnd();
	glPopMatrix();

	//wing2
	glPushMatrix();
	glTranslatef(2.5, 4, -2.5);
	glRotatef(45, 0, 1, 0);
	glRotatef(90, 1, 0, 0);
	glScalef(.5, .8, .25);
	glBegin(GL_TRIANGLES);
	//front
	glNormal3d(0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(2, 1, 0);
	glVertex3f(2, -1, 0);

	//right
	glNormal3d(1, 0, 0);
	glVertex3f(2, 1, 0);
	glVertex3f(2, -1, 0);
	glVertex3f(2, -1, -.2);

	glNormal3d(1, 0, 0);
	glVertex3f(2, 1, -.2);
	glVertex3f(2, -1, -.2);
	glVertex3f(2, 1, 0);

	//back
	glNormal3d(0, 0, -1);
	glVertex3f(0, 0, -.2);
	glVertex3f(2, 1, -.2);
	glVertex3f(2, -1, -.2);



	//top
	glNormal3d(-1, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(2, 1, 0);
	glVertex3f(2, 1, -.2);

	glNormal3d(-1, 1, 0);
	glVertex3f(0, 0, -.2);
	glVertex3f(2, 1, -.2);
	glVertex3f(0, 0, 0);

	//bottom
	glNormal3d(-1, -1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(2, -1, 0);
	glVertex3f(2, -1, -.2);

	glNormal3d(-1, -1, 0);
	glVertex3f(0, 0, -.2);
	glVertex3f(2, -1, -.2);
	glVertex3f(0, 0, 0);
	glEnd();
	glPopMatrix();

	//motor
	//cone
	glPushMatrix();
	glTranslatef(3.4, 4, -3.4);
	glRotatef(135, 0, 1, 0);
	glRotatef(_bAngle, 0, 0, 1);
	glScalef(.4, .4, 1);

	glutSolidCone(.36, .36, 20, 7);
	glPopMatrix();

	//blade	1
	glPushMatrix();
	glTranslatef(3.45, 4, -3.45);
	glRotatef(45, 0, 1, 0);
	glRotatef(_bAngle, 1, 0, 0);
	glScalef(.3, .3, 2);
	glRotatef(90, 1, 1, 0);

	glutSolidCone(.25, .2, 20, 7);
	glPopMatrix();

	//blade	2
	glPushMatrix();
	glTranslatef(3.45, 4, -3.45);
	glRotatef(45, 0, 1, 0);
	glRotatef(_bAngle + 90, 1, 0, 0);
	glScalef(.3, .3, 2);
	glRotatef(-90, 1, 1, 0);
	glutSolidCone(.25, .2, 20, 7);
	glPopMatrix();

	glPopMatrix();
	glPopMatrix();
	
	// Draw ground/sea floor
	DrawMeshQM(&groundMesh, meshSize);

	//random bumps on sea floor
	glPushMatrix();
	glTranslatef(0.0, 0.0, 0.0);
	glScalef(.5, .5, .5);
	glutSolidSphere(1.0, 50, 50);
	glPopMatrix();
	
	glPushMatrix();
	glTranslatef(3.0, 0.0, 0.0);
	glScalef(.5, .5, .5);
	glutSolidSphere(1.0, 50, 50);
	glPopMatrix();
	
	glPushMatrix();
	glTranslatef(-3.0, 0.0, 0.0);
	glScalef(.5, .5, .5);
	glutSolidSphere(1.0, 50, 50);
	glPopMatrix();

	glutSwapBuffers();   // Double buffering, swap buffers
}


// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
    // Set up viewport, projection, then change to modelview matrix mode - 
    // display function will then set up camera and do modeling transforms.
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLdouble)w / h, 0.2, 40.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	if (key == 'i')//moving the camera towards -z axis
	{
		cpz -= .5;
	}
	if (key == 'k')//moving the camera towards +z axis 
	{
		cpz += .5;
	}
	if (key == 'w')//accelerating sub foward
	{
		_rate += .01;
		if (_rate > 0.3) {
			_rate = .3;
		}
	}
	if (key == 's')//accelerating sub backward
	{
		_rate -= .01;
		if (_rate < -0.3) {
			_rate = -0.3;
		}
	}
	if (key == 'd') // rotating sub -y axis
	{
		_angle -= 4;
		if (_angle < 360) {
			_angle += 360;
		}
	}
	if (key == 'a') // rotating sub +y axis
	{
		_angle += 4;
		if (_angle > 360)
		{
			_angle -= 360;
		}
	}
	if (key == 'r') // move sub up vertically
	{
		_height += .1;

	}
	if (key == 'f') // move sub down vertically
	{
		_height -= .1;
	}

    glutPostRedisplay();   // Trigger a window redisplay
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
    // Help key
	if (key == GLUT_KEY_F1)
	{
		printf("\nHow to use this program \n");
		printf("Moving the Submraine: \n w - Accelerate foward \n s - Accelerate backwards \n d - Rotate on -y axis (right) \n a - Rotate on +y axis (left) \n r - Move ship up vertically \n f - Move ship down vertically \n \n");
		printf("Moving the Camera \n arrow_key_up - Move camera vertically up \n arrow_key_down - Move camera vertically down \n arrow_key_right - Move camera along the +x axis \n arrow_key_left - move camera along the -x axis \n i - Move camera along -z axis \n k - Move camera along +z axis");
	}
	if (key == GLUT_KEY_UP)//camera movment up
	{
		cpy += .5;
	}
	if (key == GLUT_KEY_DOWN)//camera movment down
	{
		cpy -= .5;
	}
	if (key == GLUT_KEY_LEFT)//camera movment left
	{
		cpx -= .5;
	}
	if (key == GLUT_KEY_RIGHT)//camera movment right
	{
		cpx += .5;
	}

    glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
    currentButton = button;

    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN)
        {
            ;

        }
        break;
    case GLUT_RIGHT_BUTTON:
        if (state == GLUT_DOWN)
        {
            ;
        }
        break;
    default:
        break;
    }

    glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
    if (currentButton == GLUT_LEFT_BUTTON)
    {
        ;
    }

    glutPostRedisplay();   // Trigger a window redisplay
}


Vector3D ScreenToWorld(int x, int y)
{
    // you will need to finish this if you use the mouse
    return NewVector3D(0, 0, 0);
}



