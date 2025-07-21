#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROWS		(int)480
#define COLUMNS		(int)640

void clear( unsigned char image[][COLUMNS] );
void header( int row, int col, unsigned char head[32] );

//make it a global variable 
unsigned char	image[ROWS][COLUMNS], ximage[ROWS][COLUMNS], yimage[ROWS][COLUMNS], sgmimage[ROWS][COLUMNS], bimage[ROWS][COLUMNS], head[32];
int	dedy[ROWS][COLUMNS], dedx[ROWS][COLUMNS], SGM[ROWS][COLUMNS];

int main( int argc, char **argv )
{ 
	int				i, j, k;
	int max[3];		// max[0] is max of Gx, max[1] is max of Gy, max[2] is max of SGM
	FILE			*fp;
	char			filename[6][50], ifilename[50], ch;

	strcpy( filename[0], "image1" );
	strcpy( filename[1], "image2" );
	strcpy( filename[2], "image3" );
	header ( ROWS, COLUMNS, head );

	int threshold[3] = {70,10,10}; // choose thresholds values for each image for generate binary images

	printf( "Maximum of Gx, Gy, SGM (prior to normalization) \n" );

	for ( k = 0; k < 3; k ++)
	{
		clear( ximage );
		clear( yimage );

		/* Read in the image */
		strcpy( ifilename, filename[k] );
		if (( fp = fopen( strcat(ifilename, ".raw"), "rb" )) == NULL )
		{
		  fprintf( stderr, "error: couldn't open %s\n", ifilename );
		  exit( 1 );
		}			
		for ( i = 0; i < ROWS ; i++ )
		  if ( fread( image[i], sizeof(char), COLUMNS, fp ) != COLUMNS )
		  {
			fprintf( stderr, "error: couldn't read enough stuff\n" );
			exit( 1 );
		  }
		fclose( fp );

		max[0] = 0; //maximum of Gx, initialize
		max[1] = 0; //maximum of Gy, initialize
		max[2] = 0; //maximum of SGM, initialize

		// Compute Gx, Gy, SGM, find out the maximum, use max[0], max[1], max[2] to store the maximum value
		
		//edges are at i = 0, i = ROWS -1, j = 0, j = COLUMNS -1...thus we don't include these values in our calculations
		for (i = 1; i < ROWS - 1; i++) {
			for (j = 1; j < COLUMNS - 1; j++) {
				dedx[i][j] = abs(image[i - 1][j + 1] + 2 * (image[i][j+1]) + image[i+1][j+1] - image[i-1][j-1] - 2 * (image[i][j-1]) - image[i + 1][j-1]); //abs(dE/dx)
				if (dedx[i][j] > max[0]) {
					max[0] = dedx[i][j]; //determine max dedx
				}
				dedy[i][j] = abs(image[i-1][j - 1] + 2 * (image[i-1][j]) + image[i-1][j + 1] - image[i+1][j - 1] - 2 * (image[i+1][j]) - image[i+1][j + 1]); //abs(dE/dy)

				if (dedy[i][j] > max[1]) {
					max[1] = dedy[i][j]; //determine max dedy
				}
				SGM[i][j] = (dedx[i][j] * dedx[i][j]) + (dedy[i][j] * dedy[i][j]);
				if (SGM[i][j] > max[2]) {
					max[2] = SGM[i][j]; //determine max SGM
				}

			}
		}
		
		
		for (i = 0; i < ROWS; i++) {
			for (j = 0; j < COLUMNS; j++) {
				
				//if on the boundary set value to zero
				if (i == 0 || i == ROWS - 1 || j == 0 || j == COLUMNS - 1)
				{
					ximage[i][j] = 0;
					yimage[i][j] = 0;
					sgmimage[i][j] = 0;
				}

				//normalize to 255 for max values
				ximage[i][j] = (((float)dedx[i][j]) / max[0]) * 255;
				yimage[i][j] = (((float)dedy[i][j]) / max[1]) * 255;
				sgmimage[i][j] = (((float)SGM[i][j]) / max[2]) * 255;
			
		
				/* Step 3: Compute the binary image */
				//need to check histogram in image viewer and select proper threshold, similar method in assignment 3
				if (sgmimage[i][j] < threshold[k]) //if in background
				{
					bimage[i][j] = 0;
				}
				else {
					bimage[i][j] = 255; //for object, make object bright. 
				}
			}
		}

		/* Write Gx to a new image*/
		strcpy( ifilename, filename[k] );
		if (!( fp = fopen( strcat( ifilename, "-x.ras" ), "wb" ) ))
		{
		  fprintf( stderr, "error: could not open %s\n", ifilename );
		  exit( 1 );
		}
		fwrite( head, 4, 8, fp );

		for (i = 0; i < ROWS; i++)
		{
			fwrite(ximage[i], sizeof(char), COLUMNS, fp);
		}

	    fclose( fp );
					
		/* Write Gy to a new image */
		strcpy( ifilename, filename[k] );
		if (!( fp = fopen( strcat( ifilename, "-y.ras" ), "wb" ) ))
		{
		  fprintf( stderr, "error: could not open %s\n", ifilename );
		  exit( 1 );
		}
		fwrite( head, 4, 8, fp );

		for (i = 0; i < ROWS; i++)
		{
			fwrite(yimage[i], sizeof(char), COLUMNS, fp);
		}
		fclose( fp );

		/* Write SGM to a new image */
		strcpy( ifilename, filename[k] );
		if (!( fp = fopen( strcat( ifilename, "-s.ras" ), "wb" ) ))
		{
		  fprintf( stderr, "error: could not open %s\n", ifilename );
		  exit( 1 );
		}
		fwrite( head, 4, 8, fp );
		for (i = 0; i < ROWS; i++)
		{
			fwrite(sgmimage[i], sizeof(char), COLUMNS, fp);
		}
		fclose( fp );
		
		/* Write the binary image to a new image */
		strcpy( ifilename, filename[k] );
		if (!(fp = fopen(strcat(ifilename, "-b.ras"), "wb")))
		{
			fprintf(stderr, "error: could not open %s\n", ifilename );
		  exit( 1 );
		}
		fwrite( head, 4, 8, fp );
		for (i = 0; i < ROWS; i++)
		{
			fwrite(bimage[i], sizeof(char), COLUMNS, fp);
		}
		fclose( fp );

		printf( "%d %d %d\n", max[0], max[1], max[2] );

	}

	printf( "Press any key to exit: " );
	gets( &ch );
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
