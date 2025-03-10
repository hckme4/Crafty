#include <GL/glut.h>
#include <math.h>
#include <stdbool.h>

#define TO_RADIANS 3.14/180.0
#define FPS 60
#define ACCEL 5.0
struct Motion
{
	bool Forward,Backward,Left,Right;
};

struct Motion motion = {false,false,false,false};

//temp 16:9 aspect ratio of window for testing. will render fullscreen later.
const int width = 16*50;
const int height = 9*50;

//TODO turn into struct
float pitch = 0.0, yaw = 0.0;
float camX=0.0,camZ=0.0;

void display();
void reshape(int w,int h);
void timer(int);
void passive_motion(int,int);
void keyboard(unsigned char key,int x,int y);
void keyboard_up(unsigned char key,int x,int y);

//init OpenGL/GLUT
void init()
{
	glutSetCursor(GLUT_CURSOR_NONE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glutWarpPointer(width/2,height/2); // currently bound to fixed width and height. to fix.	
}

int main(int argc,char**argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow("CraftyFPS");
	//later use glutFullScreen() to make window fullscreen!
	
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutTimerFunc(0,timer,0);

	glutPassiveMotionFunc(passive_motion);

	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboard_up);

	glutMainLoop();
	return 0;
}

/* draw the scene. initially we will draw only a chessboard surface to walk on to test
 * FPS camera and movement
 */
void draw()
{
	glEnable(GL_TEXTURE_2D);
	GLuint texture;
	glGenTextures(1,&texture);

	unsigned char texture_data[2][2][4] =
		{
			0,0,0,255,	255,255,255,255,
			255,255,255,255,	 0,0,0,255
		};

	glBindTexture(GL_TEXTURE_2D,texture);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,2,2,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBegin(GL_QUADS);

	glTexCoord2f(0.0,0.0);	glVertex3f(-50.0,-5.0,-50.0);
	glTexCoord2f(25.0,0.0); glVertex3f(50.0,-5.0,-50.0);
	glTexCoord2f(25.0,25.0);glVertex3f(50.0,-5.0,50.0);
	glTexCoord2f(0.0,25.0); glVertex3f(-50.0,-5.0,50.0);

	glEnd();

	glDisable(GL_TEXTURE_2D);
}

//actually show the drawn stuff
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	camera();
	draw();

	glutSwapBuffers();
}

void reshape(int w,int h)
{
	glViewport(0,0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60,16.0/9.0,1,75); //perspective is 16:9
	glMatrixMode(GL_MODELVIEW);
}

// Keep calling display function at the current FPS rate... AKA keep updating the screen
void timer(int)
{
	glutPostRedisplay();
	glutWarpPointer(width/2,height/2);
	glutTimerFunc(1000/FPS,timer,0);
}

void passive_motion(int x,int y)
{
	/*2 variables, store x/y coordinates as seen from center of window*/
	int dev_x,dev_y;
	dev_x = (width/2) - x;
	dev_y = (height/2) - y;

	/* apply changes to pitch/yaw */
	yaw+=(float)dev_x/10.0;
	pitch+=(float)dev_y/10.0;
}

//do the actual rotation as defined by passive_motion()'s supplied pitch and yaw
void camera()
{
	//Translation maths
	if(motion.Forward)
	{
		camX += cos((yaw+90)*TO_RADIANS)/ACCEL;
		camZ -= sin((yaw+90)*TO_RADIANS)/ACCEL;
	}
	if(motion.Backward)
	{
		camX += cos((yaw+90+180)*TO_RADIANS)/ACCEL;
		camZ -= sin((yaw+90+180)*TO_RADIANS)/ACCEL;
	}
	if(motion.Left)
	{
		camX += cos((yaw+90+90)*TO_RADIANS)/ACCEL;
		camZ -= sin((yaw+90+90)*TO_RADIANS)/ACCEL;
	}
	if(motion.Right)
	{
		camX += cos((yaw+90-90)*TO_RADIANS)/ACCEL;
		camZ -= sin((yaw+90-90)*TO_RADIANS)/ACCEL;
	}

	//limit pitch values between -60 and 70
	if (pitch>=70)
		pitch = 70;
	if (pitch<=-60)
		pitch = -60;

	glRotatef(-pitch,1.0,0.0,0.0); //x axis
	glRotatef(-yaw,0.0,1.0,0.0); //y axis

	glTranslatef(-camX,0.0,-camZ);
}

void keyboard(unsigned char key,int x, int y)
{
	switch(key)
	{
		case 'W':
		case 'w':
			motion.Forward = true;
			break;
		case 'A':
		case 'a':
			motion.Left = true;
			break;
		case 'S':
		case 's':
			motion.Backward = true;
			break;
		case 'D':
		case 'd':
			motion.Right = true;
			break;
		//TODO: add jumping!
	}
}

void keyboard_up(unsigned char key,int x,int y)
{
	switch(key)
	{
		case 'W':
		case 'w':
			motion.Forward = false;
			break;
		case 'A':
		case 'a':
			motion.Left = false;
			break;
		case 'S':
		case 's':
			motion.Backward = false;
			break;
		case 'D':
		case 'd':
			motion.Right = false;
			break;
		//TODO: add jumping!
	}
}
