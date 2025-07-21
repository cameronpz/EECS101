#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define ROWS	480
#define COLUMNS	640
#define PI 3.14159265358979323846

#define COLS_VOT	500

#define sqr(x)	((x)*(x))

void clear( unsigned char image[][COLUMNS] );
void header( int row, int col, unsigned char head[32] );

//declarations from HW4
unsigned char	image[ROWS][COLUMNS], ximage[ROWS][COLUMNS], yimage[ROWS][COLUMNS], bimage[ROWS][COLUMNS], new_image[ROWS][COLUMNS], head[32];
int	dedy[ROWS][COLUMNS], dedx[ROWS][COLUMNS], SGM[ROWS][COLUMNS];
unsigned char	simage[ROWS][COLUMNS];
int				voting[180][COLS_VOT];
int				t_voting[180][COLS_VOT];

int main( int argc, char** argv )
{
	//general declarations
	int				i,j;
	FILE* fp;
	char			filename[50], ifilename[50];

	//declarations from HW4
	int max[3];		// max[0] is max of Gx, max[1] is max of Gy, max[2] is max of SGM

	//declarations for HW5
	// localmax: number in the three bucket of voting array corrsponding to three local maxima
	// index[3][2]: used for store rho and theta
	int	localmax[3] = {0, 0, 0}, index[3][2] = { 0, 0, 0, 0, 0, 0 };
	int				sgm_threshold, hough_threshold; //180 different possiblities for theta and 400 for rho
	float 			rho, theta, rad;
	
	//-------------SECTION I: READ IN---------------------------------------
	clear(simage);
	strcpy ( filename, "image.raw");
	memset ( voting, 0, sizeof(int) * 180 * COLS_VOT);
	header(ROWS, COLUMNS, head);

	/* Read in the image */
	if (!( fp = fopen(filename, "rb" ) ))
	{
		fprintf( stderr, "error: couldn't open %s\n", argv[1]);
		exit(1);
	}
	for ( i = 0 ; i < ROWS ; i++ )
		if (!( COLUMNS == fread( image[i], sizeof(char), COLUMNS, fp ) ))
		{
			fprintf( stderr, "error: couldn't read %s\n", argv[1] );
			exit(1);
		}
	fclose(fp);


	//-----------------SECTION II: HW4 CODE----------------------------
	max[0] = 0; //maximum of Gx, initialize
	max[1] = 0; //maximum of Gy, initialize
	max[2] = 0; //maximum of SGM, initialize

	//edges are at i = 0, i = ROWS -1, j = 0, j = COLUMNS -1...thus we don't include these values in our calculations
	for (i = 1; i < ROWS - 1; i++) {
		for (j = 1; j < COLUMNS - 1; j++) {
			dedx[i][j] = abs(image[i - 1][j + 1] + 2 * (image[i][j + 1]) + image[i + 1][j + 1] - image[i - 1][j - 1] - 2 * (image[i][j - 1]) - image[i + 1][j - 1]); //abs(dE/dx)
			if (dedx[i][j] > max[0]) {
				max[0] = dedx[i][j]; //determine max dedx
			}
			dedy[i][j] = abs(image[i - 1][j - 1] + 2 * (image[i - 1][j]) + image[i - 1][j + 1] - image[i + 1][j - 1] - 2 * (image[i + 1][j]) - image[i + 1][j + 1]); //abs(dE/dy)

			if (dedy[i][j] > max[1]) {
				max[1] = dedy[i][j]; //determine max dedy
			}
			SGM[i][j] = (dedx[i][j] * dedx[i][j]) + (dedy[i][j] * dedy[i][j]);
			if (SGM[i][j] > max[2]) {
				max[2] = SGM[i][j]; //determine max SGM
			}

		}
	}

	//Normalize the values to max of 255
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLUMNS; j++) {

			//if on the boundary set value to zero
			if (i == 0 || i == ROWS - 1 || j == 0 || j == COLUMNS - 1)
			{
				ximage[i][j] = 0;
				yimage[i][j] = 0;
				simage[i][j] = 0;
			}

			//normalize to 255 for max values
			ximage[i][j] = (((float)dedx[i][j]) / max[0]) * 255;
			yimage[i][j] = (((float)dedy[i][j]) / max[1]) * 255;
			simage[i][j] = (((float)SGM[i][j]) / max[2]) * 255;
		}
	}

	/* Save SGM to an image */
	strcpy(filename, "image");
	if (!(fp = fopen(strcat(filename, "-sgm.ras"), "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", filename);
		exit(1);
	}
	fwrite(head, 4, 8, fp);
	for (i = 0; i < ROWS; i++)
		fwrite(simage[i], sizeof(char), COLUMNS, fp);
	fclose(fp);

	//Compute the Binary Image...need to check histogram in image viewer and select proper threshold
	sgm_threshold = 60; 
	
	for (i = 0; i < ROWS; i++)
	{
		for (j = 0; j < COLUMNS; j++)
		{
			if (simage[i][j] < sgm_threshold) //if in background
			{
				bimage[i][j] = 0;
			}
			else
			{
				bimage[i][j] = 255; //for object, make object bright. 
			}
		}
	}

	/* Save the thresholded SGM (binary) to an image */
	strcpy(filename, "image");
	if (!(fp = fopen(strcat(filename, "-binary.ras"), "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", filename);
		exit(1);
	}

	fwrite(head, 4, 8, fp);
	for (i = 0; i < ROWS; i++)
		fwrite(bimage[i], sizeof(char), COLUMNS, fp);
	fclose(fp);


	//-----------------------SECTION III: DETERMINING LOCAL MAXS-------------------------------------

	int max_rho = 0;
	int min_rho = 0;
	/* build up voting array */
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLUMNS; j++)
		{
			if (bimage[i][j] == 255) //for all edge points
			{
				for (theta = 0; theta < 180; theta++) //restriction for theta
				{
					rad = 3.1415926535 / 180 * theta; //convert to radians
					rho = - j * sin(rad) + (ROWS-1-i)*cos(rad); //x = j, y = ROWS-1-i
					//p = -x*sin(rad) + y*cos(rad);
					voting[(int)theta][(int)(rho/2)+300] += 1; //quantize to integers
					//divided by 2 and added 300 to ensure always positive and in the range of 0 to 500
					if (max_rho < rho)
						max_rho = rho;
					if (min_rho > rho)
						min_rho = rho;
				}
			}
		}
	}
	//now I have found all the (theta, rho) points for all the edge points in the image
	printf("%d\n", max_rho); //379
	printf("%d\n", min_rho); //-599

	
	// Save original voting array to an image 
	strcpy(filename, "image");
	header(180, COLS_VOT, head);
	if (!(fp = fopen(strcat(filename, "-voting_array.ras"), "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", filename);
		exit(1);
	}
	fwrite(head, 4, 8, fp);
	for (i = 0; i < 180; i++)
		fwrite(voting[i], sizeof(char), COLS_VOT, fp);
	fclose(fp);


	//Determing Local Maximums
	//calculating first max
	for (i = 0; i < 180; i++) { //theta between 0 and 180
		for (j = 0; j < COLS_VOT; j++) {
			if (localmax[0] < voting[i][j])
			{
				localmax[0] = voting[i][j];
				index[0][0] = i; //theta value
				index[0][1] = j; //rho value
			}
		}
	}
	//remove local max from values
	voting[index[0][0]][index[0][1]] = 0;

	//calculating 2nd local max
	for (i = 0; i < 180; i++) { //theta between 0 and 180
		for (j = 0; j < COLS_VOT; j++) {
			if (localmax[1] < voting[i][j])
			{
				localmax[1] = voting[i][j];
				index[1][0] = i; //theta value
				index[1][1] = j; //rho value
			}
		}
	}

	//remove 2nd max from values
	voting[index[1][0]][index[1][1]] = 0;

	//calculating 3rd local max
	for (i = 0; i < 180; i++) { //theta between 0 and 180
		for (j = 0; j < COLS_VOT; j++) {
			if (localmax[2] < voting[i][j])
			{
				localmax[2] = voting[i][j];
				index[2][0] = i; //theta value
				index[2][1] = j; //rho value
			}
		}
	}

	//Conver the rho values back to originals:
	int rho1 = (index[0][1] - 300) * 2;
	int rho2 = (index[1][1] - 300) * 2;
	int rho3 = (index[2][1] - 300) * 2;

	//print out all metrics found: 
	printf("theta1 = %d rho1 = %d max1 = %d\ntheta2 = %d rho2 = %d max2 = %d\ntheta3= %d rho3 = %d max3 = %d\n",
		index[0][0], rho1, localmax[0],
		index[1][0], rho2, localmax[1],
		index[2][0], rho3, localmax[2]);

	hough_threshold = 528; 
	printf("\nHough threshold: %d\n", hough_threshold);


	//add back in the two local maxs that we got rid of prior:
	voting[index[0][0]][index[0][1]] = 700;
	voting[index[1][0]][index[1][1]] = 554;


	//-----------------SECTION IV: THRESHOLDED ARRAY----------------------------

	//Compute Thresholded Voting Array
	for (i = 0; i < 180; i++)
	{
		for (j = 0; j < COLS_VOT; j++)
		{
			if (voting[i][j] < hough_threshold)
			{
				t_voting[i][j] = 0; //get rid of other points, isolate maxs
			}
			else
			{
				t_voting[i][j] = voting[i][j]; ///show the three local maxs
			}
		}
	}


	// Reconstruct an image from the voting array 
	float rad1 = 3.1415926535 / 180 * index[0][0];
	float rad2 = 3.1415926535 / 180 * index[1][0];
	float rad3 = 3.1415926535 / 180 * index[2][0];
	//x = j, y = ROWS-1-i

	int y;
	for (j = 0; j < COLUMNS; j++) {
		 y = tan(rad1)*(float)j + rho1 / cos(rad1);
		//y = tan(rad)*x + rho/cos(rad)
		 i = ROWS - 1 - y;
		 if (i < ROWS && i >= 0)
			 new_image[i][j] = 255;
		 y = tan(rad2) * (float)j + rho2 / cos(rad2);
		 i = ROWS - 1 - y;
		 if (i < ROWS && i >= 0)
			 new_image[i][j] = 255;
		 y = tan(rad3) * (float)j + rho3 / cos(rad3);
		 i = ROWS - 1 - y;
		 if (i < ROWS && i >= 0)
			 new_image[i][j] = 255;
	}

	// Write the reconstructed figure to an image 
	strcpy(filename, "image");
	header(ROWS, COLUMNS, head);
	if (!(fp = fopen(strcat(filename, "-reconstructed_image.ras"), "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", filename);
		exit(1);
	}
	fwrite(head, 4, 8, fp);
	for (i = 0; i < ROWS; i++)
		fwrite(new_image[i], sizeof(char), COLUMNS, fp);
	fclose(fp);

	printf("Press any key to exit: ");

	return 0;
}


void clear( unsigned char image[][COLUMNS] )
{
	int	i,j;
	for ( i = 0 ; i < ROWS ; i++ )
		for ( j = 0 ; j < COLUMNS ; j++ ) image[i][j] = 0;
}

void header( int row, int col, unsigned char head[32] )
{
	int *p = (int *)head;
	char *ch;
	int num = row * col;

	/* Choose little-endian or big-endian header depending on the machine. Don't modify this */
	/* Little-endian for PC */
	
	*p = 0x956aa659;
	*(p + 3) = 0x08000000;
	*(p + 5) = 0x01000000;
	*(p + 6) = 0x0;
	*(p + 7) = 0xf8000000;

	ch = (char*)&col;
	head[7] = *ch;
	ch ++; 
	head[6] = *ch;
	ch ++;
	head[5] = *ch;
	ch ++;
	head[4] = *ch;

	ch = (char*)&row;
	head[11] = *ch;
	ch ++; 
	head[10] = *ch;
	ch ++;
	head[9] = *ch;
	ch ++;
	head[8] = *ch;
	
	ch = (char*)&num;
	head[19] = *ch;
	ch ++; 
	head[18] = *ch;
	ch ++;
	head[17] = *ch;
	ch ++;
	head[16] = *ch;
	

	/* Big-endian for unix */
	/*
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

