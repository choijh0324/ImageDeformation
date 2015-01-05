#include <opencv/cv.h> // OpenCV library
#include <opencv/highgui.h> 
#include <GL/glut.h> // GLUT framework on  Window
#include <iostream>

using namespace std;

CvCapture* capture;
IplImage* iplTexture = 0;
GLuint texture;  // array in which the generated texture names are stored
CvHaarClassifierCascade* cascade = 0;
CvMemStorage* storage = 0;

int frame = 0;
int screenW;
int screenH;
double edge[2];

void cameraInit();
void myInit();
void myIdle();
void drawGraphics();
void myKeyboard(unsigned char key, int mousex, int mousey);
void myReshape(int width, int height);

double * computePointCoordinates(int pointIndex, int xPointCount, int yPointCount, double xSpacing, double ySpacing);
void computePointCoordinates(int pointIndex, int xPointCount, int yPointCount, double xSpacing, double ySpacing, double pt[3]);
void renderTexGrid(int xPointCount, int yPointCount, double xSpacing, double ySpacing);
void renderGrid(int xPointCount, int yPointCount, double xSpacing, double ySpacing);
void detectface();

int main(int argc, char * argv[])
{
	const char *classifer = "C:\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_alt_tree.xml";

	cascade = (CvHaarClassifierCascade*)cvLoad(classifer, 0, 0, 0);
	if (!cascade){
		std::cerr << "error: cascade error!!" << std::endl;
		return -1;
	}

	storage = cvCreateMemStorage(0);
	if (!storage){
		std::cerr << "error: storage error!!" << std::endl;
		return -2;
	}
	// initialize glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	cameraInit();

	// initialize the window
	glutInitWindowSize(screenW, screenH);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("test");

	myInit();

	// set the global idle callback
	glutIdleFunc(myIdle);

	// set the display callback for the current window
	glutDisplayFunc(drawGraphics);

	// setup other callbacks
	glutKeyboardFunc(myKeyboard);
	glutReshapeFunc(myReshape);

	glutMainLoop();

	return 0;
}

void cameraInit()
{
	// initialize capture from a camera
	capture = cvCaptureFromCAM(0); // capture from video device #0
	if (!capture) {
		printf("Could not capture a camera\n\7");
		//        exit(0);
	}
	else {
		cout << endl << "capture a camera!" << endl;
	}

	// grab and return a frame from a camera
	iplTexture = cvQueryFrame(capture);
	cvCvtColor(iplTexture, iplTexture, CV_BGR2RGB);
	cvFlip(iplTexture, NULL, 0);
	cout << endl << "grab a frame!" << endl;

	screenW = iplTexture->width;
	screenH = iplTexture->height;
}


void myInit()
{
	// gluOrtho2D(0, screenW, 0, screenH);
	glTranslatef(-1.0f, -1.0f, 0);
	glViewport(0, 0, screenW, screenH);

	// create a texture for the video
	glGenTextures(1, &texture); // generate texture names
	glBindTexture(GL_TEXTURE_2D, texture); // bind a named texture to a texturing target
}


void myIdle()
{
	// grab and return a frame from a camera 
	iplTexture = cvQueryFrame(capture);
	cvCvtColor(iplTexture, iplTexture, CV_BGR2RGB);
	// flip iplTexture to fit to the OpenGL coordinate
	cvFlip(iplTexture, NULL, 0); // flip a 2D array around x-axis

	glutPostRedisplay(); // redisplay the window's normal plane
}


void drawGraphics()
{
	//cout << endl << "frame #" << frame << endl;

	glClear(GL_COLOR_BUFFER_BIT); // clear the screen
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // background color

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glTranslatef(-1.0f, -1.0f, 0);
	// make a texture
	// set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// set texture environment parameters
	//    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	// specify a two-dimensional texture image
	// glTexiplTexture2D(<#GLenum target#>, <#GLint level#>, <#GLenum internalformat#>, <#GLsizei width#>, <#GLsizei height#>, <#GLint border#>, <#GLenum format#>, <#GLenum type#>, <#const GLvoid * pixels#>)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iplTexture->width, iplTexture->height, 0, GL_RGB, GL_UNSIGNED_BYTE, iplTexture->imageData);

	// draw 2D square plane to be mapped by the texture
	/*
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f); // the drawing color
	glBegin(GL_QUADS);
	{
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 0.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 0.0f);
	}
	glEnd();
	*/
	renderTexGrid(iplTexture->width / 10, iplTexture->height / 10, 0.1, 0.1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// draw a graphic object 
	glDisable(GL_TEXTURE_2D);

	//renderGrid(iplTexture->width / 10, iplTexture->height / 10, 0.1, 0.1);
	detectface();
	//glColor3f(1.0f, 1.0f, 0.0f); // the drawing color "yellow"
	//glutSolidTeapot(0.4);

	//glFlush();
	glutSwapBuffers(); // swap framebuffer 
}


void myKeyboard(unsigned char key, int mousex, int mousey)
{
	switch (key) {
	case 27: // Press ESC to exit
		exit(0);
	}
}


void myReshape(int width, int height)
{
	glTranslatef(-1.0f, -1.0f, 0.0f);
	glViewport(0, 0, width, height);
}

double * computePointCoordinates(int pointIndex, int xPointCount, int yPointCount, double xSpacing, double ySpacing)
{
	static double pt[3];
	double width = xSpacing * (xPointCount - 1) / 2;
	double Height = ySpacing * (yPointCount - 1) / 2;
	double minX = 0;
	double minY = 0;


	pt[0] = minX + xSpacing * (pointIndex % xPointCount);
	pt[1] = minY + ySpacing * (pointIndex / xPointCount);
	pt[2] = 0.0;

	return pt;
}

void computePointCoordinates(int pointIndex, int xPointCount, int yPointCount, double xSpacing, double ySpacing, double pt[3])
{
	double * tmp = computePointCoordinates(pointIndex, xPointCount, yPointCount, xSpacing, ySpacing);

	pt[0] = tmp[0];
	pt[1] = tmp[1];
	pt[2] = tmp[2];

}

//testing for git
//hahahahah
void renderTexGrid(int xPointCount, int yPointCount, double xSpacing, double ySpacing)
{
	glEnable(GL_TEXTURE_2D);
	int nrQuads = (xPointCount - 1) * (yPointCount - 1);

	double bounds[4];
	double width = (xPointCount - 1) * xSpacing;
	double height = (yPointCount - 1) * ySpacing;

	bounds[0] = -width / 2;
	bounds[1] = -height / 2;
	bounds[2] = width;
	bounds[3] = height;
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < nrQuads; i++)
	{
		int k = i + i / (xPointCount - 1);
		int a = k;
		int b = k + 1;
		int c = k + 1 + xPointCount;
		int d = k + xPointCount;
		double aPt[3], bPt[3], cPt[3], dPt[3];
		computePointCoordinates(a, xPointCount, yPointCount, xSpacing, ySpacing, aPt);
		computePointCoordinates(b, xPointCount, yPointCount, xSpacing, ySpacing, bPt);
		computePointCoordinates(c, xPointCount, yPointCount, xSpacing, ySpacing, cPt);
		computePointCoordinates(d, xPointCount, yPointCount, xSpacing, ySpacing, dPt);
		if (i == 0)
		{
			edge[0] = aPt[0];
			edge[1] = aPt[1];
		}

		//sincModify(aPt, bounds);
		//sincModify(bPt, bounds);
		//sincModify(cPt, bounds);
		//sincModify(dPt, bounds);

		glTexCoord2f(-(aPt[0]) / width, (aPt[1]) / height);
		glVertex3f(aPt[0] / width * 2, aPt[1] / height * 2, 0.0);
		glTexCoord2f(-(cPt[0]) / width, (cPt[1]) / height);
		glVertex3f(cPt[0] / width * 2, cPt[1] / height * 2, 0.0);
		glTexCoord2f(-(dPt[0]) / width, (dPt[1]) / height);
		glVertex3f(dPt[0] / width * 2, dPt[1] / height * 2, 0.0);

		glTexCoord2f(-(aPt[0]) / width, (aPt[1]) / height);
		glVertex3f(aPt[0] / width * 2, aPt[1] / height * 2, 0.0);
		glTexCoord2f(-(bPt[0]) / width, (bPt[1]) / height);
		glVertex3f(bPt[0] / width * 2, bPt[1] / height * 2, 0.0);
		glTexCoord2f(-(cPt[0]) / width, (cPt[1]) / height);
		glVertex3f(cPt[0] / width * 2, cPt[1] / height * 2, 0.0);
	}
	glEnd();

}

void renderGrid(int xPointCount, int yPointCount, double xSpacing, double ySpacing)
{
	int nrQuads = (xPointCount - 1) * (yPointCount - 1);

	double bounds[4];
	double width = (xPointCount - 1) * xSpacing;
	double height = (yPointCount - 1) * ySpacing;

	bounds[0] = -width / 2;
	bounds[1] = -height / 2;
	bounds[2] = width;
	bounds[3] = height;
	glColor3f(0.0f, 0.0f, 0.0f);
	glPolygonMode(GL_FRONT, GL_LINE);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < nrQuads; i++)
	{
		int k = i + i / (xPointCount - 1);
		int a = k;
		int b = k + 1;
		int c = k + 1 + xPointCount;
		int d = k + xPointCount;

		double aPt[3], bPt[3], cPt[3], dPt[3];
		computePointCoordinates(a, xPointCount, yPointCount, xSpacing, ySpacing, aPt);
		computePointCoordinates(b, xPointCount, yPointCount, xSpacing, ySpacing, bPt);
		computePointCoordinates(c, xPointCount, yPointCount, xSpacing, ySpacing, cPt);
		computePointCoordinates(d, xPointCount, yPointCount, xSpacing, ySpacing, dPt);


		glVertex3f(aPt[0] / width * 2, aPt[1] / height * 2, 0.0);
		glVertex3f(cPt[0] / width * 2, cPt[1] / height * 2, 0.0);
		glVertex3f(dPt[0] / width * 2, dPt[1] / height * 2, 0.0);

		glVertex3f(aPt[0] / width * 2, aPt[1] / height * 2, 0.0);
		glVertex3f(bPt[0] / width * 2, bPt[1] / height * 2, 0.0);
		glVertex3f(cPt[0] / width * 2, cPt[1] / height * 2, 0.0);
	}
	glEnd();
}

void detectface()
{
	CvSeq *faces = 0;
	faces = cvHaarDetectObjects(iplTexture, cascade, storage, 2.0, 1, 0);

	//검출된 모든 face에 대한 반복문
	for (int i = 0; i < faces->total; i++){
		//face 영역 가져오기
		CvRect *r = 0;
		r = (CvRect*)cvGetSeqElem(faces, i);
		glColor3f(1.0f, 0.0f, 0.0f);
		glLineWidth(5);
		glBegin(GL_LINE_STRIP);
		glVertex2f(((float)iplTexture->width/1.33 - (float)r->x)/ (float)iplTexture->width * 2.0, ((float)r->y + (float)r->height) / (float)iplTexture->height* 2.0);
		glVertex2f(((float)iplTexture->width/1.33 - (float)r->x + (float)r->width) / (float)iplTexture->width * 2.0, ((float)r->y + (float)r->height) / (float)iplTexture->height * 2.0);
		glVertex2f(((float)iplTexture->width/1.33 - (float)r->x + (float)r->width) / (float)iplTexture->width * 2.0, (float)r->y / (float)iplTexture->height* 2.0);
		glVertex2f(((float)iplTexture->width/1.33 - (float)r->x) / (float)iplTexture->width * 2.0, (float)r->y / (float)iplTexture->height* 2.0);
		glVertex2f(((float)iplTexture->width/1.33 - (float)r->x) / (float)iplTexture->width * 2.0, ((float)r->y + (float)r->height) / (float)iplTexture->height* 2.0);
		glEnd();
	}

}
