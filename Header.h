#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pmmintrin.h>
#include <process.h>
#include <chrono>
#include <iostream>
#include <immintrin.h>
#include <Windows.h>

#define N 1024
#define M 1024
#define TIMES 1

#define P 1024


void Gaussian_Blur_AVX();
void Gaussian_Blur_default();
bool compare_Gaussian_images();

void Sobel_default();
bool compare_Sobel_images();

void scale_image();

__declspec(align(64))  unsigned short int  imag[N][M], in_image[N][M], filt_image[N][M], out_image[N][M];
__declspec(align(64))  int edgeDir[N][M];
__declspec(align(64))  int gradient[N][M];


const unsigned short int gaussianMask[5][5] = {
	{2,4,5,4,2} ,
	{4,9,12,9,4},
	{5,12,15,12,5},
	{4,9,12,9,4},
	{2,4,5,4,2}
};

const short int GxMask[3][3] = {
	{-1,0,1} ,
	{-2,0,2},
	{-1,0,1}
};

const short int GyMask[3][3] = {
	{-1,-2,-1} ,
	{0,0,0},
	{1,2,1}
};

char message[20];
void print_message(char *s, bool outcome);

//PLEASE AMEND THE DIRECTORY BELOW
char in[100] = "C:\\Users\\Xtrendence\\Documents\\GitHub\\NET112-Practical\\rec.pgm";
char out[100] = "C:\\Users\\Xtrendence\\Documents\\GitHub\\NET112-Practical\\filtered.pgm";
char out2[100] = "C:\\Users\\Xtrendence\\Documents\\GitHub\\NET112-Practical\\gradient.pgm";

FILE *fin;
errno_t err;

void read_image(char* filename, unsigned short int  image[N][M]);
void write_image(char* filename, unsigned short int  imag[N][M]);
void write_image2(char* filename, unsigned short int  imag[N][M]);

void openfile(char *filename, FILE** finput);
int getint(FILE *fp);
























