//	CSCI 420 - Assignment 2: Simulating a Roller Coaster
//	Avishkar Kolahalu

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <GL/glu.h>
#include <GL/glut.h>

#include "opencv2/core/core.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"

/* Object where you can load an image */
cv::Mat3b imageBGR;

/* represents one control point along the spline */
struct point {
	double x;
	double y;
	double z;
};

/* spline struct which contains how many control points, and an array of control points */
struct spline {
	int numControlPoints;
	struct point *points;
};

/* the spline array */
struct spline *g_Splines;

/* total number of splines */
int g_iNumOfSplines;

int loadSplines(char *argv) {
	char *cName = (char *)malloc(128 * sizeof(char));
	FILE *fileList;
	FILE *fileSpline;
	int iType, i = 0, j, iLength;

	/* load the track file */
	fileList = fopen(argv, "r");
	if (fileList == NULL) {
		printf("can't open file\n");
		exit(1);
	}

	/* stores the number of splines in a global variable */
	fscanf(fileList, "%d", &g_iNumOfSplines);
	printf("%d\n", g_iNumOfSplines);
	g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

	/* reads through the spline files */
	for (j = 0; j < g_iNumOfSplines; j++) {
		i = 0;
		fscanf(fileList, "%s", cName);
		fileSpline = fopen(cName, "r");

		if (fileSpline == NULL) {
			printf("can't open file\n");
			exit(1);
		}

		/* gets length for spline file */
		fscanf(fileSpline, "%d %d", &iLength, &iType);

		/* allocate memory for all the points */
		g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
		g_Splines[j].numControlPoints = iLength;

		/* saves the data to the struct */
		while (fscanf(fileSpline, "%lf %lf %lf",
			&g_Splines[j].points[i].x,
			&g_Splines[j].points[i].y,
			&g_Splines[j].points[i].z) != EOF) {
			i++;
		}
	}

	free(cName);

	return 0;
}

/* Write a screenshot to the specified filename */
void saveScreenshot(char *filename)
{
	if (filename == NULL)
		return;

	// Allocate a picture buffer // 
	cv::Mat3b bufferRGB = cv::Mat::zeros(480, 640, CV_8UC3); //rows, cols, 3-channel 8-bit.
	printf("File to save to: %s\n", filename);

	//use fast 4-byte alignment (default anyway) if possible
	glPixelStorei(GL_PACK_ALIGNMENT, (bufferRGB.step & 3) ? 1 : 4);
	//set length of one complete row in destination data (doesn't need to equal img.cols)
	glPixelStorei(GL_PACK_ROW_LENGTH, bufferRGB.step / bufferRGB.elemSize());
	glReadPixels(0, 0, bufferRGB.cols, bufferRGB.rows, GL_RGB, GL_UNSIGNED_BYTE, bufferRGB.data);
	//flip to account for GL 0,0 at lower left
	cv::flip(bufferRGB, bufferRGB, 0);
	//convert RGB to BGR
	cv::Mat3b bufferBGR(bufferRGB.rows, bufferRGB.cols, CV_8UC3);
	cv::Mat3b out[] = { bufferBGR };
	// rgb[0] -> bgr[2], rgba[1] -> bgr[1], rgb[2] -> bgr[0]
	int from_to[] = { 0,2, 1,1, 2,0 };
	mixChannels(&bufferRGB, 1, out, 1, from_to, 3);

	if (cv::imwrite(filename, bufferBGR)) {
		printf("File saved Successfully\n");
	}
	else {
		printf("Error in Saving\n");
	}
}

/* Function to get a pixel value. Use like PIC_PIXEL macro.
Note: OpenCV images are in channel order BGR.
This means that:
chan = 0 returns BLUE,
chan = 1 returns GREEN,
chan = 2 returns RED. */
unsigned char getPixelValue(cv::Mat3b& image, int x, int y, int chan)
{
	return image.at<cv::Vec3b>(y, x)[chan];
}

/* Function that does nothing but demonstrates looping through image coordinates.*/
void loopImage(cv::Mat3b& image)
{
	for (int r = 0; r < image.rows; r++) { // y-coordinate
		for (int c = 0; c < image.cols; c++) { // x-coordinate
			for (int channel = 0; channel < 3; channel++) {
				// DO SOMETHING... example usage
				// unsigned char blue = getPixelValue(image, c, r, 0);
				// unsigned char green = getPixelValue(image, c, r, 1); 
				// unsigned char red = getPixelValue(image, c, r, 2); 
			}
		}
	}
}

/* Read an image into memory.
Set argument displayOn to true to make sure images are loaded correctly.
One image loaded, set to false so it doesn't interfere with OpenGL window.*/
int readImage(char *filename, cv::Mat3b& image, bool displayOn)
{
	std::cout << "reading image: " << filename << std::endl;
	image = cv::imread(filename);
	if (!image.data) // Check for invalid input                    
	{
		std::cout << "Could not open or find the image." << std::endl;
		return 1;
	}

	if (displayOn)
	{
		cv::imshow("TestWindow", image);
		cv::waitKey(0); // Press any key to enter. 
	}
	return 0;
}

/* OpenCV help:
Access number of rows of image (height): image.rows;
Access number of columns of image (width): image.cols;
Pixel 0,0 is the upper left corner. Byte order for 3-channel images is BGR.
*/

int main(int argc, char *argv[])
{
	// I've set the argv[1] to track.txt.
	// To change it, on the "Solution Explorer",
	// right click "assign1", choose "Properties",
	// go to "Configuration Properties", click "Debugging",
	// then type your track file name for the "Command Arguments"
	if (argc < 2)
	{
		printf("usage: %s <trackfile>\n", argv[0]);
		exit(0);
	}

	loadSplines(argv[1]);

	// If you need to load textures use below instead of pic library:
	// readImage("spiral.jpg", imageBGR, true);

	// Demonstrates to loop across image and access pixel values:
	// Note this function doesn't do anything, but you may find it useful:
	// loopImage(imageBGR);

	// Rebuilt save screenshot function, but will seg-fault unless you have
	// OpenGL framebuffers initialized. You can use this new function as before:
	// saveScreenshot("test_screenshot.jpg");

	return 0;
}
