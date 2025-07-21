#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define ROWS	240
#define COLUMNS	240
#define PI 3.14159265358979323846

void clear( unsigned char image[][COLUMNS] );
void header( int row, int col, unsigned char head[32] );
unsigned char	image[ROWS][COLUMNS];
double			L[ROWS][COLUMNS];

int main( int argc, char** argv )
{
	//basic definitions
	int				i, j, k;
	FILE*			fp;
	unsigned char	head[32];
	char			filename[9][50], ch;

	//HW7 specific non-list definitions
	double			dot_product;
	double			alpha, x, y;
	double			p, q;
	double			Ls,  Ll;

	//Parameter R, M, A Lists for Each Image
	double			R[9] = { 50,50,50,10,100,50,50,50,50 }; //define r for each image
	double			M[9] = { 1,1,1,1,1,1,1,0.1,10000 };
	double			A[9] = { 0.5,0.5,0.5,0.5,0.5,0.1,1,0.5,0.5 };

	//Lists for Vectors
	double			V[] = { 0, 0, 1 }; //fixed
	double			Nhat[3], magH[3];
	double			H[3][3], Hhat[3][3], Hhat_image[9][3];
	double           Shat[3][3] = { { 0, 0, 1 }, { 1 / sqrt(3), 1 / sqrt(3), 1 / sqrt(3) }, { 1, 0, 0 } };
					//row = options, columns = componets for each option
	double			Shat_image[9][3];
	
	header(ROWS, COLUMNS, head);

	//define filenames
	strcpy(filename[0], "image_A");
	strcpy(filename[1], "image_B");
	strcpy(filename[2], "image_C");
	strcpy(filename[3], "image_D");
	strcpy(filename[4], "image_E");
	strcpy(filename[5], "image_F");
	strcpy(filename[6], "image_G");
	strcpy(filename[7], "image_H");
	strcpy(filename[8], "image_I");

	//calculating angular bisector H = V + S/mag(V + S)
	for (i = 0; i < 3; i++) {  //three options for H
		for (j = 0; j < 3; j++) { //three componets for each option
			H[i][j] = (V[j] + Shat[i][j]); //add corresponding components
		}
	}
	//Calculate the magnitude of each H
	for (i = 0; i < 3; i++) {
		magH[i] = sqrt(H[i][0] * H[i][0] + H[i][1] * H[i][1] + H[i][2] * H[i][2]);
	}

	//Normalize H to Hhat
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			Hhat[i][j] = H[i][j] / magH[i]; //divde each component by the magnitude
		}
	}
	//I now have calculat the normalize vector Hhat for all three cases
	
	//equate the three Hhat options to those deployed in the 9 images
	for (j = 0; j < 3; j++) {
		Hhat_image[0][j] = Hhat[0][j];
		Hhat_image[1][j] = Hhat[1][j];
		Hhat_image[2][j] = Hhat[2][j];
		Hhat_image[3][j] = Hhat[0][j];
		Hhat_image[4][j] = Hhat[0][j];
		Hhat_image[5][j] = Hhat[0][j];
		Hhat_image[6][j] = Hhat[0][j];
		Hhat_image[7][j] = Hhat[0][j];
		Hhat_image[8][j] = Hhat[0][j];
	}
	
	//equate the three Shat options to those deployed in the 9 images
	for (j = 0; j < 3; j++) {
		Shat_image[0][j] = Shat[0][j];
		Shat_image[1][j] = Shat[1][j];
		Shat_image[2][j] = Shat[2][j];
		Shat_image[3][j] = Shat[0][j];
		Shat_image[4][j] = Shat[0][j];
		Shat_image[5][j] = Shat[0][j];
		Shat_image[6][j] = Shat[0][j];
		Shat_image[7][j] = Shat[0][j];
		Shat_image[8][j] = Shat[0][j];
	}


	for ( k = 0; k < 9; k ++ ) //for each image
	{
		clear(image); //set image to black

		for (i = 0; i < ROWS; i++) {
			for (j = 0; j < COLUMNS; j++) {
				x = j-COLUMNS/2; //so that x and y are centered
				y = ROWS/2 - i;
				if (  (R[k] * R[k]) >= (x * x + y * y)  ) //if r^2 >= (x^2 + y^2)
				{
					if (  (R[k] * R[k]) > (x * x + y * y)  ) //if r^2 > (x^2 + y^2)
					{
						p = -x / sqrt(R[k] * R[k] - (x * x + y * y));
						q = -y / sqrt(R[k] * R[k] - (x * x + y * y));
						Nhat[0] = -p / sqrt(p * p + q * q + 1);
						Nhat[1] = -q / sqrt(p * p + q * q + 1);
						Nhat[2] = 1 / sqrt(p * p + q * q + 1);
					}

					if (  (R[k] * R[k]) == (x * x + y * y)  ) //if r^2 = (x^2 + y^2)
					{
						double Nmag = sqrt(x * x + y * y);
						Nhat[0] = x / Nmag;
						Nhat[1] = y / Nmag;
						Nhat[2] = 0;
					}

					//I have calculated the unit normal surface vector

					//calculate the dot product of N and H
					dot_product = 0; //initialize to zero
					for (int d = 0; d < 3; d++) {
						dot_product += Nhat[d] * Hhat_image[k][d];
					}
					alpha = acos(dot_product); //get alpha
					Ls = exp(-((alpha / M[k]) * (alpha / M[k])));

					dot_product = 0; //initialize to zero
					//calculate the dot product of N and S
					for (int e = 0; e < 3; e++) {
						dot_product += Nhat[e] * Shat_image[k][e];
					}
					Ll = dot_product;

					//Master Equation:
					if (Ll >= 0)
					{
						L[i][j] = A[k] * Ll + (1 - A[k]) * Ls;
						image[i][j] = L[i][j] * 255;
					}
				}
			}
		}

		//open image and write to new one
		if (!( fp = fopen(strcat(filename[k], ".ras"), "wb")))
		{
		  fprintf( stderr, "error: could not open %s\n", filename[k]);
		  exit( 1 );
		}
		fwrite( head, 4, 8, fp );
		for ( i = 0 ; i < ROWS ; i++ )
		  fwrite( image[i], sizeof(char), COLUMNS, fp );
		fclose( fp );
	}
	printf("Press any key to exit: ");
	gets ( &ch );
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