#include "Header.h"

using namespace std;

int main() {
	//the following command pins the current process to the 1st core
	//otherwise, the OS tongles this process between different cores
	BOOL success = SetProcessAffinityMask(GetCurrentProcess(), 1);
	if (success == 0) {
		cout << "SetProcessAffinityMask failed" << endl;
		system("pause");
		return -1;
	}

	//--------------read the input image
	read_image(in, in_image);

	//------Gaussian Blur
	auto start = std::chrono::high_resolution_clock::now();

	for (int it = 0; it != TIMES; it++) {
		Gaussian_Blur_AVX();
	}

	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - start;
	std::cout << "Elapsed time: " << elapsed.count() << " s\n";

	//write output image
	write_image(out, filt_image);

	snprintf(message, sizeof(message) - 1, "Gaussian Blur");
	print_message(message, compare_Gaussian_images());

	//------Sobel
	start = std::chrono::high_resolution_clock::now();

	for (int it = 0; it != TIMES; it++) {
		Sobel_default();
	}

	finish = std::chrono::high_resolution_clock::now();
	elapsed = finish - start;
	std::cout << "Elapsed time: " << elapsed.count() << " s\n";

	scale_image();

	//write output image
	write_image(out2, imag);

	snprintf(message, sizeof(message) - 1, "Sobel");
	print_message(message, compare_Sobel_images());

	system("pause");
	return 0;
}

void print_message(char *s, bool outcome) {
	if (outcome == true)
		printf("\n\n\r ----- %s output is correct -----\n\r", s);
	else
		printf("\n\n\r -----%s output is INcorrect -----\n\r", s);
}

void Gaussian_Blur_AVX() {
	__m256i r0, r1, r2, r3, r4, r5, r6, r7;
	__m256i r8, r9, r10, r14, r15, const0, const1, const2, ex1, ex2, ex3;
	__m128i t0, t1, t2, t3, t4, t5,c0,c1,c2;
	short int row, col;
	int temp;

	const0 = _mm256_set_epi16(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 4, 5, 4, 2);
	const1 = _mm256_set_epi16(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 9, 12, 9, 4);
	const2 = _mm256_set_epi16(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 12, 15, 12, 5);

	for (row = 2; row < N - 2; row++) {
		for (col = 2; col < M - ?; col++) {//I have put an '?' here as you will exceed the array bounds. Although it will work this is bad practice

			r0 = _mm256_loadu_si256((__m256i *) &in_image[row - 2][col - 2]); //load 16 short ints into r0. Below, you will need to process the first 5 only. 
			//load the other elements this way too

			// use  ...=_mm256_madd_epi16(...) MORE THAN ONE TIMES

			// use ...=_mm256_add_epi32(...) MORE THAN ONE TIMES

			// use ...=_mm256_hadd_epi32(...) MORE THAN ONE TIMES

			// use temp=_mm256_cvtsi256_si32(...)
			filt_image[row][col] = temp / 159;


		}

		//padding
		for (col = ? ; col < M - 2; col++) { // modify the ? accordingly 
			temp = 0;
			for (int rowOffset = -2; rowOffset <= 2; rowOffset++) {
				for (int colOffset = -2; colOffset <= 2; colOffset++) {
					temp += in_image[row + rowOffset][col + colOffset] * gaussianMask[2 + rowOffset][2 + colOffset];
				}
			}
			filt_image[row][col] = temp / 159;
		}
	}
}

void Gaussian_Blur_default() {
	short int row, col, rowOffset, colOffset;
	short int newPixel;

	for (row = 2; row < N - 2; row++) {
		for (col = 2; col < M - 2; col++) {
			newPixel = 0;
			for (rowOffset = -2; rowOffset <= 2; rowOffset++) {
				for (colOffset = -2; colOffset <= 2; colOffset++) {
					newPixel += in_image[row + rowOffset][col + colOffset] * gaussianMask[2 + rowOffset][2 + colOffset];
				}
			}
			filt_image[row][col] = newPixel / 159;
		}
	}
}

//returns false/true, when the output image is incorrect/correct, respectively
bool compare_Gaussian_images() {
	int row, col, rowOffset, colOffset;
	int newPixel;

	for (row = 2; row < N - 2; row++) {
		for (col = 2; col < M -2; col++) {
			newPixel = 0;
			for (rowOffset = -2; rowOffset <= 2; rowOffset++) {
				for (colOffset = -2; colOffset <= 2; colOffset++) {
					newPixel += in_image[row + rowOffset][col + colOffset] * gaussianMask[2 + rowOffset][2 + colOffset];
				}
			}
			newPixel = newPixel / 159;
			if (newPixel != filt_image[row][col]) {
				//printf("\n %d %d - %d %d\n", row, col, newPixel, filt_image[row][col]);
				return false;
			}
		}
	}

	return true;
}

void scale_image() {
	/* the output of Sobel (gradient has values larger than 255, thus those are capped to 255
	alternatively, we can scale it, or use canny algorithm*/

	for (int i = 0; i < N; i++)
		for (int j = 0; j < M; j++) {
			if (gradient[i][j] <= 255) imag[i][j] = (unsigned char)gradient[i][j];
			else imag[i][j] = 255;
		}
}

void Sobel_default() {
	int row, col, rowOffset, colOffset;
	int Gx, Gy;
	float thisAngle;
	int newAngle;

	/*---------------------------- Determine edge directions and gradient strengths -------------------------------------------*/
	for (row = 1; row < N - 1; row++) {
		for (col = 1; col < M - 1; col++) {
			Gx = 0;
			Gy = 0;

			/* Calculate the sum of the Sobel mask times the nine surrounding pixels in the x and y direction */
			for (rowOffset = -1; rowOffset <= 1; rowOffset++) {
				for (colOffset = -1; colOffset <= 1; colOffset++) {
					Gx += filt_image[row + rowOffset][col + colOffset] * GxMask[rowOffset + 1][colOffset + 1];
					Gy += filt_image[row + rowOffset][col + colOffset] * GyMask[rowOffset + 1][colOffset + 1];
				}
			}

			//gradient[row][col] = sqrt(pow(Gx, 2.0) + pow(Gy, 2.0));	/* Calculate gradient strength		*/
			gradient[row][col] = abs(Gx) + abs(Gy); // this is an optimized version of the above

			thisAngle = (atan2(Gx, Gy) / 3.14159) * 180.0;		/* Calculate actual direction of edge [-180, +180]*/

			/* Convert actual edge direction to approximate value */
			if (((thisAngle >= -22.5) && (thisAngle <= 22.5)) || (thisAngle >= 157.5) || (thisAngle <= -157.5))
				newAngle = 0;
			if (((thisAngle > 22.5) && (thisAngle < 67.5)) || ((thisAngle > -157.5) && (thisAngle < -112.5)))
				newAngle = 45;
			if (((thisAngle >= 67.5) && (thisAngle <= 112.5)) || ((thisAngle >= -112.5) && (thisAngle <= -67.5)))
				newAngle = 90;
			if (((thisAngle > 112.5) && (thisAngle < 157.5)) || ((thisAngle > -67.5) && (thisAngle < -22.5)))
				newAngle = 135;

			edgeDir[row][col] = newAngle;
		}
	}
}

bool compare_Sobel_images() {
	int row, col, rowOffset, colOffset;
	int Gx, Gy, test1, test2;
	float thisAngle;
	int newAngle;

	/*---------------------------- Determine edge directions and gradient strengths -------------------------------------------*/
	for (row = 1; row < N - 1; row++) {
		for (col = 1; col < M - 1; col++) {
			Gx = 0;
			Gy = 0;

			/* Calculate the sum of the Sobel mask times the nine surrounding pixels in the x and y direction */
			for (rowOffset = -1; rowOffset <= 1; rowOffset++) {
				for (colOffset = -1; colOffset <= 1; colOffset++) {
					Gx += filt_image[row + rowOffset][col + colOffset] * GxMask[rowOffset + 1][colOffset + 1];
					Gy += filt_image[row + rowOffset][col + colOffset] * GyMask[rowOffset + 1][colOffset + 1];
				}
			}

			test1 = abs(Gx) + abs(Gy);		
			thisAngle = (atan2(Gx, Gy) / 3.14159) * 180.0;		/* Calculate actual direction of edge [-180, +180]*/

			/* Convert actual edge direction to approximate value */
			if (((thisAngle >= -22.5) && (thisAngle <= 22.5)) || (thisAngle >= 157.5) || (thisAngle <= -157.5))
				newAngle = 0;
			if (((thisAngle > 22.5) && (thisAngle < 67.5)) || ((thisAngle > -157.5) && (thisAngle < -112.5)))
				newAngle = 45;
			if (((thisAngle >= 67.5) && (thisAngle <= 112.5)) || ((thisAngle >= -112.5) && (thisAngle <= -67.5)))
				newAngle = 90;
			if (((thisAngle > 112.5) && (thisAngle < 157.5)) || ((thisAngle > -67.5) && (thisAngle < -22.5)))
				newAngle = 135;

			if (test1 != gradient[row][col]) {
				return false;
			}

			if (edgeDir[row][col] != newAngle)
				return false;
		}
	}
	return true;
}

void read_image(char* filename, unsigned short int image[N][M]) {
	int inint = -1;
	int c;
	FILE *finput;
	int i, j;

	printf("  Reading image from disk (%s)...\n", filename);
	//finput = NULL;
	openfile(filename, &finput);

	for (j = 0; j < N; j++)
		for (i = 0; i < M; i++) {
			c = getc(finput);
			image[j][i] = (unsigned short int)c;
		}

	/* for (j=0; j<N; ++j)
	   for (i=0; i<M; ++i) {
		 if (fscanf(finput, "%i", &inint)==EOF) {
		   fprintf(stderr,"Premature EOF\n");
		   exit(-1);
		 } else {
		   image[j][i]= (unsigned char) inint; //printf("\n%d",inint);
		 }
	   }*/

	fclose(finput);
}

void write_image(char* filename, unsigned short int image[N][M]) {
	FILE* foutput;
	int i, j;

	printf("  Writing result to disk (%s)...\n", filename);
	if ((err = fopen_s(&foutput, filename, "wb")) != 0) {
		printf("Unable to open file %s for writing\n", filename);
		exit(-1);
	}

	fprintf(foutput, "P2\n");
	fprintf(foutput, "%d %d\n", M, N);
	fprintf(foutput, "%d\n", 255);

	for (j = 0; j < N; ++j) {
		for (i = 0; i < M; ++i) {
			fprintf(foutput, "%3d ", image[j][i]);
			if (i % 32 == 31) fprintf(foutput, "\n");
		}
		if (M % 32 != 0) fprintf(foutput, "\n");
	}
	fclose(foutput);
}

void openfile(char *filename, FILE** finput) {
	int x0, y0;
	char header[255];
	int aa;

	if ((err = fopen_s(finput, filename, "rb")) != 0) {
		printf("Unable to open file %s for reading\n");
		exit(-1);
	}

	aa = fscanf(*finput, "%s", header, 20);

	/*if (strcmp(header,"P2")!=0) {
	   fprintf(stderr,"\nFile %s is not a valid ascii .pgm file (type P2)\n",
			   filename);
	   exit(-1);
	 }*/

	x0 = getint(*finput);
	y0 = getint(*finput);

	if ((x0 != M) || (y0 != N)) {
		printf("Image dimensions do not match: %ix%i expected\n", N, M);
		exit(-1);
	}

	x0 = getint(*finput); /* read and throw away the range info */
}

int getint(FILE *fp) /* adapted from "xv" source code */ {
	int c, i, firstchar, garbage;

	/* note:  if it sees a '#' character, all characters from there to end of
	   line are appended to the comment string */

	   /* skip forward to start of next number */
	c = getc(fp);
	while (1) {
		/* eat comments */
		if (c == '#') {
			/* if we're at a comment, read to end of line */
			char cmt[256], *sp;

			sp = cmt;  firstchar = 1;
			while (1) {
				c = getc(fp);
				if (firstchar && c == ' ') firstchar = 0;  /* lop off 1 sp after # */
				else {
					if (c == '\n' || c == EOF) break;
					if ((sp - cmt) < 250) *sp++ = c;
				}
			}
			*sp++ = '\n';
			*sp = '\0';
		}

		if (c == EOF) return 0;
		if (c >= '0' && c <= '9') break;   /* we've found what we were looking for */

		/* see if we are getting garbage (non-whitespace) */
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n' && c != ',') garbage = 1;

		c = getc(fp);
	}

	/* we're at the start of a number, continue until we hit a non-number */
	i = 0;
	while (1) {
		i = (i * 10) + (c - '0');
		c = getc(fp);
		if (c == EOF) return i;
		if (c<'0' || c>'9') break;
	}
	return i;
}