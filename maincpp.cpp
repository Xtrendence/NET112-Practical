#include "Header.h"

using namespace std;

int main() {
	//the following command pins the current process to the 1st core
	//otherwise, the OS tongles this process between different cores
	BOOL success = SetProcessAffinityMask(GetCurrentProcess(), 1);
	if(success == 0) {
		cout << "SetProcessAffinityMask failed" << endl;
		system("pause");
		return -1;
	}

	//--------------read the input image
	read_image(in, in_image);

	//------Gaussian Blur
	auto start = std::chrono::high_resolution_clock::now();

	for(int it = 0; it != TIMES; it++) {
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

	for(int it = 0; it != TIMES; it++) {
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
	if(outcome == true)
		printf("\n\n\r ----- %s output is correct -----\n\r", s);
	else
		printf("\n\n\r -----%s output is INcorrect -----\n\r", s);
}

void Gaussian_Blur_AVX() {
	__m256i r0, r1, r2, r3, r4, r5, r6, r7; // Rows
	__m256i p0, p1, p2, p3, p4, p5; // Multiplied Pixels
	__m256i r8, r9, r10, r14, r15, m0, m1, m2, ex1, ex2, ex3;
	__m256i a0, a1, a2, a3, a4, a5; // Added
	__m256i h0, h1, h2, h3, h4, h5; // Hadd
	__m128i t0, t1, t2, t3, t4, t5, c0, c1, c2;
	short int row, col;
	int newPixel, n0, n1, n2, n3, n4; // Pixels

	m0 = _mm256_set_epi16(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 4, 5, 4, 2);
	m1 = _mm256_set_epi16(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 9, 12, 9, 4);
	m2 = _mm256_set_epi16(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 12, 15, 12, 5);

	for(row = 2; row < N - 2; row++) {
		for(col = 2; col < M - 2; col++) {
			r0 = _mm256_loadu_si256((__m256i *) & in_image[row - 2][col - 2]);
			r1 = _mm256_loadu_si256((__m256i *) & in_image[row - 1][col - 2]);
			r2 = _mm256_loadu_si256((__m256i *) & in_image[row][col - 2]);
			r3 = _mm256_loadu_si256((__m256i *) & in_image[row + 1][col - 2]);
			r4 = _mm256_loadu_si256((__m256i *) & in_image[row + 2][col - 2]);

			p0 = _mm256_madd_epi16(r0, m0);
			p1 = _mm256_madd_epi16(r1, m1);
			p2 = _mm256_madd_epi16(r2, m2);
			p3 = _mm256_madd_epi16(r3, m1);
			p4 = _mm256_madd_epi16(r4, m0);

			a0 = _mm256_add_epi32(p0, p1);
			a1 = _mm256_add_epi32(p2, a0);
			a2 = _mm256_add_epi32(p3, a1);

			h0 = _mm256_hadd_epi32(a2, p4);

			newPixel = (h0[0] + h0[1] + h0[2] + h0[3] + h0[4] + h0[5]);

			// Multiply row values by the appropriate mask values.
			// p0 = _mm256_madd_epi16(r0, m0);
			// p1 = _mm256_madd_epi16(r1, m1);
			// p2 = _mm256_madd_epi16(r2, m2);
			// p3 = _mm256_madd_epi16(r3, m1);
			// p4 = _mm256_madd_epi16(r4, m0);
			
			// // Add up the values for filt_image().
			// a0 = _mm256_add_epi32(p0, p1);
			// a1 = _mm256_add_epi32(a0, p2);
			// a2 = _mm256_add_epi32(a1, p3);
			// a3 = _mm256_add_epi32(a2, p4);

			// r0 = _mm256_hadd_epi32(r0, a3);
			// r1 = _mm256_hadd_epi32(r0, a3);
			// r2 = _mm256_hadd_epi32(r0, a3);
			// r3 = _mm256_hadd_epi32(r0, a3);
			// r4 = _mm256_hadd_epi32(r0, a3);

			// newPixel = a3[0] + a3[1] + a3[2] + a3[3] + a3[4];
			//newPixel += (_mm_cvtsi128_si32(_mm256_castsi256_si128(h0)));

			filt_image[row][col] = newPixel / 159;
		}

		for(col = 1008; col < M - 2; col++) {
			newPixel = 0;
			for(int rowOffset = -2; rowOffset <= 2; rowOffset++) {
				for(int colOffset = -2; colOffset <= 2; colOffset++) {
					newPixel += in_image[row + rowOffset][col + colOffset] * gaussianMask[2 + rowOffset][2 + colOffset];
				}
			}
			filt_image[row][col] = newPixel / 159;
		}
	}
}

// N = Width, M = Height (both of which are 1024).

// The algorithm works by going through each row and each column, then getting each individual pixel in a 5x5 area, and multiplying its value (0-255) by the appropriate mask value (2, 4, 5, 4, 2), (4, 9, 12, 9, 4), (5, 12, 15, 12, 5), (4, 9, 12, 9, 4), (2, 4, 5, 4, 2).

// My goal is to do these calculations using intrinsic functions.

// _mm256_loadu_si256() -> Load 256-bits of integer data from memory into "dst". mem_addr does not need to be aligned on any particular boundary.
// Use to get all 5 rows of pixels.

// _mm256_madd_epi16() -> Multiply packed signed 16-bit integers in "a" and "b", producing intermediate signed 32-bit integers. Horizontally add adjacent pairs of intermediate 32-bit integers, and pack the results in "dst".
// Use to multiply each pixel by its appropriate mask value.

// _mm256_add_epi32() -> Add packed 32-bit integers in "a" and "b", and store the results in "dst".
// 

// _mm256_hadd_epi32() -> Horizontally add adjacent pairs of 32-bit integers in "a" and "b", and pack the signed 32-bit results in "dst".
// 

// _mm256_cvtsi256_si32() -> Copy the lower 32-bit integer in "a" to "dst".
// Use to turn the array of signed integers to a 32-bit integer (i.e. newPixel).

void Gaussian_Blur_default_unrolled() {
	short int row, col;
	short int newPixel;

	for(row = 2; row < N - 2; row++) {
		for(col = 2; col < M - 2; col++) {
			newPixel = 0;

			newPixel += in_image[row - 2][col - 2] * gaussianMask[0][0];
			newPixel += in_image[row - 2][col - 1] * gaussianMask[0][1];
			newPixel += in_image[row - 2][col] *     gaussianMask[0][2];
			newPixel += in_image[row - 2][col + 1] * gaussianMask[0][3];
			newPixel += in_image[row - 2][col + 2] * gaussianMask[0][4];

			newPixel += in_image[row - 1][col - 2] * gaussianMask[1][0];
			newPixel += in_image[row - 1][col - 1] * gaussianMask[1][1];
			newPixel += in_image[row - 1][col] *	 gaussianMask[1][2];
			newPixel += in_image[row - 1][col + 1] * gaussianMask[1][3];
			newPixel += in_image[row - 1][col + 2] * gaussianMask[1][4];

			newPixel += in_image[row][col - 2] * gaussianMask[2][0];
			newPixel += in_image[row][col - 1] * gaussianMask[2][1];
			newPixel += in_image[row][col] *     gaussianMask[2][2];
			newPixel += in_image[row][col + 1] * gaussianMask[2][3];
			newPixel += in_image[row][col + 2] * gaussianMask[2][4];

			newPixel += in_image[row + 1][col - 2] * gaussianMask[3][0];
			newPixel += in_image[row + 1][col - 1] * gaussianMask[3][1];
			newPixel += in_image[row + 1][col] *     gaussianMask[3][2];
			newPixel += in_image[row + 1][col + 1] * gaussianMask[3][3];
			newPixel += in_image[row + 1][col + 2] * gaussianMask[3][4];

			newPixel += in_image[row + 2][col - 2] * gaussianMask[4][0];
			newPixel += in_image[row + 2][col - 1] * gaussianMask[4][1];
			newPixel += in_image[row + 2][col] *     gaussianMask[4][2];
			newPixel += in_image[row + 2][col + 1] * gaussianMask[4][3];
			newPixel += in_image[row + 2][col + 2] * gaussianMask[4][4];

			filt_image[row][col] = newPixel / 159;
		}
	}
}

void Gaussian_Blur_default() {
	short int row, col, rowOffset, colOffset;
	short int newPixel;

	for(row = 2; row < N - 2; row++) {
		for(col = 2; col < M - 2; col++) {
			newPixel = 0;
			for(rowOffset = -2; rowOffset <= 2; rowOffset++) {
				for(colOffset = -2; colOffset <= 2; colOffset++) {
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

	for(row = 2; row < N - 2; row++) {
		for(col = 2; col < M -2; col++) {
			newPixel = 0;
			for(rowOffset = -2; rowOffset <= 2; rowOffset++) {
				for(colOffset = -2; colOffset <= 2; colOffset++) {
					newPixel += in_image[row + rowOffset][col + colOffset] * gaussianMask[2 + rowOffset][2 + colOffset];
				}
			}
			newPixel = newPixel / 159;
			if(newPixel != filt_image[row][col]) {
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

	for(int i = 0; i < N; i++)
		for(int j = 0; j < M; j++) {
			if(gradient[i][j] <= 255) imag[i][j] = (unsigned char)gradient[i][j];
			else imag[i][j] = 255;
		}
}

void Sobel_default() {
	int row, col, rowOffset, colOffset;
	int Gx, Gy;
	float thisAngle;
	int newAngle;

	/*---------------------------- Determine edge directions and gradient strengths -------------------------------------------*/
	for(row = 1; row < N - 1; row++) {
		for(col = 1; col < M - 1; col++) {
			Gx = 0;
			Gy = 0;

			/* Calculate the sum of the Sobel mask times the nine surrounding pixels in the x and y direction */
			for(rowOffset = -1; rowOffset <= 1; rowOffset++) {
				for(colOffset = -1; colOffset <= 1; colOffset++) {
					Gx += filt_image[row + rowOffset][col + colOffset] * GxMask[rowOffset + 1][colOffset + 1];
					Gy += filt_image[row + rowOffset][col + colOffset] * GyMask[rowOffset + 1][colOffset + 1];
				}
			}

			//gradient[row][col] = sqrt(pow(Gx, 2.0) + pow(Gy, 2.0));	/* Calculate gradient strength		*/
			gradient[row][col] = abs(Gx) + abs(Gy); // this is an optimized version of the above

			thisAngle = (atan2(Gx, Gy) / 3.14159) * 180.0;		/* Calculate actual direction of edge [-180, +180]*/

			/* Convert actual edge direction to approximate value */
			if(((thisAngle >= -22.5) && (thisAngle <= 22.5)) || (thisAngle >= 157.5) || (thisAngle <= -157.5))
				newAngle = 0;
			if(((thisAngle > 22.5) && (thisAngle < 67.5)) || ((thisAngle > -157.5) && (thisAngle < -112.5)))
				newAngle = 45;
			if(((thisAngle >= 67.5) && (thisAngle <= 112.5)) || ((thisAngle >= -112.5) && (thisAngle <= -67.5)))
				newAngle = 90;
			if(((thisAngle > 112.5) && (thisAngle < 157.5)) || ((thisAngle > -67.5) && (thisAngle < -22.5)))
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
	for(row = 1; row < N - 1; row++) {
		for(col = 1; col < M - 1; col++) {
			Gx = 0;
			Gy = 0;

			/* Calculate the sum of the Sobel mask times the nine surrounding pixels in the x and y direction */
			for(rowOffset = -1; rowOffset <= 1; rowOffset++) {
				for(colOffset = -1; colOffset <= 1; colOffset++) {
					Gx += filt_image[row + rowOffset][col + colOffset] * GxMask[rowOffset + 1][colOffset + 1];
					Gy += filt_image[row + rowOffset][col + colOffset] * GyMask[rowOffset + 1][colOffset + 1];
				}
			}

			test1 = abs(Gx) + abs(Gy);		
			thisAngle = (atan2(Gx, Gy) / 3.14159) * 180.0;		/* Calculate actual direction of edge [-180, +180]*/

			/* Convert actual edge direction to approximate value */
			if(((thisAngle >= -22.5) && (thisAngle <= 22.5)) || (thisAngle >= 157.5) || (thisAngle <= -157.5))
				newAngle = 0;
			if(((thisAngle > 22.5) && (thisAngle < 67.5)) || ((thisAngle > -157.5) && (thisAngle < -112.5)))
				newAngle = 45;
			if(((thisAngle >= 67.5) && (thisAngle <= 112.5)) || ((thisAngle >= -112.5) && (thisAngle <= -67.5)))
				newAngle = 90;
			if(((thisAngle > 112.5) && (thisAngle < 157.5)) || ((thisAngle > -67.5) && (thisAngle < -22.5)))
				newAngle = 135;

			if(test1 != gradient[row][col]) {
				return false;
			}

			if(edgeDir[row][col] != newAngle)
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

	for(j = 0; j < N; j++)
		for(i = 0; i < M; i++) {
			c = getc(finput);
			image[j][i] = (unsigned short int)c;
		}

	/* for(j=0; j<N; ++j)
	   for(i=0; i<M; ++i) {
		 if(fscanf(finput, "%i", &inint)==EOF) {
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
	if((err = fopen_s(&foutput, filename, "wb")) != 0) {
		printf("Unable to open file %s forwriting\n", filename);
		exit(-1);
	}

	fprintf(foutput, "P2\n");
	fprintf(foutput, "%d %d\n", M, N);
	fprintf(foutput, "%d\n", 255);

	for(j = 0; j < N; ++j) {
		for(i = 0; i < M; ++i) {
			fprintf(foutput, "%3d ", image[j][i]);
			if(i % 32 == 31) fprintf(foutput, "\n");
		}
		if(M % 32 != 0) fprintf(foutput, "\n");
	}
	fclose(foutput);
}

void openfile(char *filename, FILE** finput) {
	int x0, y0;
	char header[255];
	int aa;

	if((err = fopen_s(finput, filename, "rb")) != 0) {
		printf("Unable to open file %s forreading\n");
		exit(-1);
	}

	aa = fscanf(*finput, "%s", header, 20);

	/*if(strcmp(header,"P2")!=0) {
	   fprintf(stderr,"\nFile %s is not a valid ascii .pgm file (type P2)\n",
			   filename);
	   exit(-1);
	 }*/

	x0 = getint(*finput);
	y0 = getint(*finput);

	if((x0 != M) || (y0 != N)) {
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
		if(c == '#') {
			/* if we're at a comment, read to end of line */
			char cmt[256], *sp;

			sp = cmt;  firstchar = 1;
			while (1) {
				c = getc(fp);
				if(firstchar && c == ' ') firstchar = 0;  /* lop off 1 sp after # */
				else {
					if(c == '\n' || c == EOF) break;
					if((sp - cmt) < 250) *sp++ = c;
				}
			}
			*sp++ = '\n';
			*sp = '\0';
		}

		if(c == EOF) return 0;
		if(c >= '0' && c <= '9') break;   /* we've found what we were looking for*/

		/* see if we are getting garbage (non-whitespace) */
		if(c != ' ' && c != '\t' && c != '\r' && c != '\n' && c != ',') garbage = 1;

		c = getc(fp);
	}

	/* we're at the start of a number, continue until we hit a non-number */
	i = 0;
	while (1) {
		i = (i * 10) + (c - '0');
		c = getc(fp);
		if(c == EOF) return i;
		if(c<'0' || c>'9') break;
	}
	return i;
}