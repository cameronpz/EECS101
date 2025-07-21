#define _CRT_SECURE_NO_DEPRECATE // add this before header
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ROWS   		 		 128
#define COLS				 128
#define LOGICAL_X_MIN			-4.0
#define LOGICAL_X_MAX			 4.0
#define LOGICAL_Y_MIN			-4.0
#define LOGICAL_Y_MAX			 4.0

//function declarations
void clear(unsigned char image[][COLS]);
int plot_logical_point(float x, float y, unsigned char image[][COLS]); // maps image
int plot_physical_point(int x, int y, unsigned char image[][COLS]); //maps image
int in_range(int x, int y);
void header(int row, int col, unsigned char head[32]);/*write header for the output images, do not modify this*/

//main function
int main(int argc, char **argv)
{
	int				i;
	float			t, x, y, z, xprime, yprime;
	float			xh, yh, xhprime, yhprime; //z=zh so I won't define zh
	float			x1, y1, zo, x2, y2;
	float			a, b, f;
	FILE			*fp;
	char			 *ofile;
	unsigned char	image[ROWS][COLS]; // 0 - 255
	//there are 16384 total elements (pixels)
	unsigned char head[32];

	//this is all just varaible declarations. 

	clear(image); //reset image

	//q2 parameters
	x1 = 0.5;
	x2 = -0.5;
	y1 = -1;
	y2 = -1;
	zo = -1; //will vary this manually to -2 and -3
	a = 1;
	b = 1;
	f = 1; 

	for (t = 0.01; t < 10000.01; t += .01)
	//Directions explicity say to use t from 0.01 to 10000
	{
		
		x = x1 + t*a;
		xh = x2 + t*a;
		y = y1 + t*b;
		yh = y2 + t*b;
		z = zo;

		//perspective projection
		//xprime = (f / z) * x;
		//yprime = (f / z) * y;
		//xhprime = (f / z) * xh;
		//yhprime = (f / z) * yh;

		//orthographic projection
		xprime = x;			
		yprime = y;
		xhprime = xh;
		yhprime = yh;			

		plot_logical_point(xprime, yprime, image);
		plot_logical_point(xhprime, yhprime, image);
	}


	/* Starting write an image */

	/* Create the header */ 
	header(ROWS, COLS, head);

	ofile = "hw1q2o_-1.ras";// Define the name of your output file, modify the name according to the requirements
	//Ex. p3_b=0_c=-1_ortho.ras
	//Since no path is specified it will go to the working directory

	if (!(fp = fopen(ofile, "wb"))) //open the file in order to write to it. set as fp for code. 
		fprintf(stderr, "error: could not open %s\n", ofile), exit(1); //if file doesn't open print error. 

	/* Write the header */
	fwrite(head, 4, 8, fp); //write header to file

	for (i = 0; i < ROWS; i++) fwrite(image[i], 1, COLS, fp); //copying element values of image to external file 
	fclose(fp); //close the file 

	return 0;
}

/* No need to modify the methods below */


//function definitions
void clear(unsigned char image[][COLS])
{
	int	i, j;
	for (i = 0; i < ROWS; i++)
		for (j = 0; j < COLS; j++) image[i][j] = 0; //clear image by setting all array values to zero. 
}

int plot_logical_point(float x, float y, unsigned char image[][COLS])
{
	int	nx, ny;
	float	xc, yc;
	xc = COLS / ((float)LOGICAL_X_MAX - LOGICAL_X_MIN); // xc = 128/8 = 16
	yc = ROWS / ((float)LOGICAL_Y_MAX - LOGICAL_Y_MIN); //xc = 128/8 = 16
	nx = (x - LOGICAL_X_MIN) * xc; // nx = (x+4)/16
	ny = (y - LOGICAL_Y_MIN) * yc; // ny = (y+4)/16 

	return plot_physical_point(nx, ny, image); //nothing in here modifies the image 
	
}

int plot_physical_point(int x, int y, unsigned char image[][COLS]) //given an x,y...given a pt, if it is out of range, then set the value to 255
{
	if (in_range(x, y)) //return 0;
	return image[y][x] = 255;
}

int in_range(int x, int y)
{
	return x >= 0 && x < COLS && y >= 0 && y < ROWS; 
	//returns 1 (true) if the point is in the array
	//returns o (false) if the point is out of the array
}

void header(int row, int col, unsigned char head[32])
{
	int *p = (int *)head;
	char *ch;
	int num = row * col;

	/* Choose little-endian or big-endian header depending on the machine. Don't modify the contents */
	
	/* Little-endian for PC */
	*p = 0x956aa659;
	*(p + 3) = 0x08000000;
	*(p + 5) = 0x01000000;
	*(p + 6) = 0x0;
	*(p + 7) = 0xf8000000;

	ch = (char*)&col;
	head[7] = *ch;
	ch++;
	head[6] = *ch;
	ch++;
	head[5] = *ch;
	ch++;
	head[4] = *ch;

	ch = (char*)&row;
	head[11] = *ch;
	ch++;
	head[10] = *ch;
	ch++;
	head[9] = *ch;
	ch++;
	head[8] = *ch;

	ch = (char*)&num;
	head[19] = *ch;
	ch++;
	head[18] = *ch;
	ch++;
	head[17] = *ch;
	ch++;
	head[16] = *ch;

	/*
	// Big-endian for unix
	*p = 0x59a66a95;
	*(p + 1) = col;
	*(p + 2) = row;
	*(p + 3) = 0x8;
	*(p + 4) = num;
	*(p + 5) = 0x1;
	*(p + 6) = 0x0;
	*(p + 7) = 0xf8;
	*/
}