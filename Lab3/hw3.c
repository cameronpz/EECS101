#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define ROWS		512
#define COLUMNS		512

#define sqr(x)		((x)*(x))

void clear( unsigned char image[][COLUMNS] ) //set all pixel values to zero
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
	*(p + 7) = 0x00000000;

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

int main(int argc, char** argv)
{

	int				i, j, k;
	int				A, threshold[3] = { 138, 164,47 }; // set threshold here
	FILE* fp;
	char* ifile, * ofile;
	unsigned char	image[ROWS][COLUMNS], bimage[ROWS][COLUMNS], head[32];
	char			filename[9][50], ch;
	float				xbar, ybar;

	strcpy(filename[0], "image1.raw");
	strcpy(filename[1], "image1-b.ras");
	strcpy(filename[2], "image1.ras");
	strcpy(filename[3], "image2.raw");
	strcpy(filename[4], "image2-b.ras");
	strcpy(filename[5], "image2.ras");
	strcpy(filename[6], "image3.raw");
	strcpy(filename[7], "image3-b.ras");
	strcpy(filename[8], "image3.ras");
	header(ROWS, COLUMNS, head);

	printf("Filename: Threshold Area X Y\n");

	for (k = 0; k < 3; k++)
	{
		clear(bimage); //clear previous image
		A = 0;
		xbar = 0;
		ybar = 0;
		ifile = filename[3 * k]; //get name of just .raw files

		/* Read the image */
		if ((fp = fopen(ifile, "rb")) == NULL)
			//open ifile and assign to fp
		{
			fprintf(stderr, "error: couldn't open %s\n", ifile);
			exit(1);
		}

		for (i = 0; i < ROWS; i++)
			if (fread(image[i], 1, COLUMNS, fp) != COLUMNS)
				//read in info from fp pointer to image array
			{
				fprintf(stderr, "error: couldn't read enough stuff\n");
				exit(1);
			}
		fclose(fp);

		// Convert each image into binary image with its respective threshold value，
		for (int r = 0; r < ROWS; r++) {
			for (int c = 0; c < COLUMNS; c++) {
				if (image[r][c] <= threshold[k]) { //object is darker
					bimage[r][c] = 255; //in binary object is bright
					A++; //A counts the # of pixels of objects
				}
				else {
					bimage[r][c] = 0; //background has a value of zero, is dark in binary
				}
			}
		}

		//compute the center of image
		for (int r = 0; r < ROWS; r++) {
			for (int c = 0; c < COLUMNS; c++) {
				if (image[r][c] <= threshold[k]) {
					xbar = xbar + c; //column # = x value
					ybar = ybar + (ROWS - r - 1); //row 0 = y value of 511, row 511 = y value of 0
				}
			}
		}

		xbar = xbar / A; //divde by Area
		ybar = ybar / A; //divde by Area
		

		//mark the center of image with a small cross sign filled with pixel of value 128
		for (int r = 0; r < 5; r++) {
				bimage[ROWS - (int)ybar - 1][(int)xbar - 2 + r] = 128; //form horizontal
			}
		for (int c = 0; c < 8; c++) {
				bimage[ROWS - (int)ybar - 3 + c][(int)xbar] = 128; //form vertical
			}

		/* Save the binary image */
		ofile = filename[3 * k + 1];
		if ((fp = fopen(ofile, "wb")) == NULL)
		{
			fprintf(stderr, "error: could not open %s\n", ofile);
			exit(1);
		}
		fwrite(head, 4, 8, fp);
		for (i = 0; i < ROWS; i++) fwrite(bimage[i], 1, COLUMNS, fp);
		fclose(fp);

		printf("%s: %d %d %d %d\n", ifile, threshold[k], A, (int)xbar, (int)ybar);

		/* Save the original image as ras */
		ofile = filename[3 * k + 2];
		if ((fp = fopen(ofile, "wb")) == NULL)
		{
			fprintf(stderr, "error: could not open %s\n", ofile);
			exit(1);
		}
		fwrite(head, 4, 8, fp);
		for (i = 0; i < ROWS; i++) fwrite(image[i], 1, COLUMNS, fp);
		fclose(fp);

	}

	printf("Press any key to exit: ");
	gets(&ch);

	return 0;
}
