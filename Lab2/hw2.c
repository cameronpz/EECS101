
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define ROWS		100
#define COLUMNS		100


int main( int argc, char **argv )
{
	//declaration section
	int			i;
	FILE			*fp;
	float			u, var;
	unsigned char	image[ROWS][COLUMNS];
  	char			*ifile, *ofile, ch;
	char			filename[4][50];

	//dest, source
	strcpy(filename[0], "image1.raw"); //copy image#.raw to filename array"
	strcpy(filename[1], "image2.raw");
	strcpy(filename[2], "image3.raw");
	strcpy(filename[3], "image4.raw");
	//all of the names of my images are stored in the filename array

	for (int k = 0; k < 4; k++)
	{
		ifile = filename[k]; //assign each image name to ifile variable
		fp = fopen(ifile, "rb"); //open image file and assign to fp

		if (fp == NULL) //rb = read binary...if the file is empty print:
		{
			fprintf(stderr, "error: couldn't open %s\n", ifile);
			exit(1);
		}

		for (i = 0; i < ROWS; i++)
		{

			if (fread(image[i], 1, COLUMNS, fp) != COLUMNS) //if I don't read all the columns then:
				//fread(store data in,sizeof each ele, total # of ele to be read, read from)
			{
				fprintf(stderr, "error: couldn't read enough stuff\n");
				exit(1);
			}
		}

		fclose(fp);

		//Calculate Mean for each image here: mean = u
		float sum = 0;
		for (int r = 0; r < ROWS; r++)
		{
			for (int c = 0; c < COLUMNS; c++)
			{
				sum = sum + image[r][c]; //sum = past value + image
			}
		}
		u = sum / 10000; // sum/N^2

		//Calculate Variance for each image here; variance = var
		float sumvar = 0;
		for (int r = 0; r < ROWS; r++)
		{
			for (int c = 0; c < COLUMNS; c++)
			{
				sumvar = sumvar + pow(image[r][c] - u,2); //sum = past value + image
			}
		}
		var = sumvar / (10000 - 1); // sum/(N^2 - 1)

		//Print mean and variance for each image
		printf("%s: %f %f\n", ifile, u, var);

	}
	printf("Press any key to exit: ");
	gets(&ch);

	return 0;
}


