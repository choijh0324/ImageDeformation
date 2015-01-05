#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <GL/glut.h>
#include <opencv/cxcore.h>
#include <iostream>

using namespace std;
using namespace cv;

GLuint tex;

struct Image {
	unsigned long sizeX;
	unsigned long sizeY;
	char* data;
};

typedef struct Image Image;

int loadImage(Image *image){
	unsigned long size;
	IplImage *Color_Image;
	CvCapture *capture = cvCaptureFromCAM(0);
	if (capture){
		Color_Image = cvQueryFrame(capture);
	}
	else
	{
		printf("error from webcam capture");
		return 0;
	}
	image->sizeX = Color_Image->width;
	image->sizeY = Color_Image->height;
	size = image->sizeX * image->sizeY * 3;
	image->data = (char*)malloc(size);
	if (image->data == NULL)
	{
		printf("Error allocatin memory for color-corrected image data");
		return 0;
	}
	cvCvtColor(Color_Image, Color_Image, CV_BGR2RGB);
	memcpy(image->data, Color_Image->imageData, size);
	cvReleaseImage(&Color_Image);
	return 1;
}

void InitGL(void)
{
	Image *image1;
	image1 = (Image *)malloc(sizeof(Image));
	loadImage(image1);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		3,
		image1->sizeX,
		image1->sizeY,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		image1->data);
	free(image1->data);
	free(image1);
}

void ChangeSize(GLsizei w, GLsizei h)
{
	if (h == 0)
		h = 1;
	glViewport(0, 0, w, h);
	glOrtho(0, w, 0, h, 1, -1);
	cout << w << h << endl;
}

void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	InitGL();
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0, 1.0, 1.0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(10, 290);
	glTexCoord2f(1.0, 0.0);
	glVertex2f(350, 290);
	glTexCoord2f(1.0, 1.0);
	glVertex2f(350, 10);
	glTexCoord2f(0.0, 1.0);
	glVertex2f(10, 10);
	glEnd();
	glFlush();
}

void SetUpRC(void)
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

int main(int argc, char** argv)
{
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInit(&argc, argv);
	glutCreateWindow("test");
	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	SetUpRC();
	glutMainLoop();
	return 0;
}