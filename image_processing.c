#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bmp_header.h"

typedef struct {

	unsigned char blue;
	unsigned char green;
	unsigned char red;
} BGR;

void makebmp(char *filename, bmp_infoheader *infoheader,
			bmp_fileheader *header, BGR **pixels){
	int padding = 4 - ((infoheader->width*3)%4);
	int i,j,k;
	unsigned char pad = 0;
	FILE *out= fopen(filename,"wb");
	fwrite(header, sizeof(bmp_fileheader), 1, out);
	fwrite(infoheader, sizeof(bmp_infoheader), 1, out);
	for(i = 0; i < infoheader->height; i++){
		for(j = 0; j < infoheader->width; j++)
		{
			fwrite(&pixels[i][j].blue,sizeof(unsigned char),1,out);
			fwrite(&pixels[i][j].green,sizeof(unsigned char),1,out);
			fwrite(&pixels[i][j].red,sizeof(unsigned char),1,out);
		}
		if(padding < 4){
			for(k = 0; k < padding; k++)
				fwrite(&pad, sizeof(unsigned char),1,out);
		}
	}
	fflush(out);
	fclose(out);
}

void blackandwhite(char *filename,bmp_infoheader *infoheader,
					bmp_fileheader *header,BGR **pixels){

	int i,j,X;
	BGR **newimg;
	newimg = malloc(infoheader->height*sizeof(unsigned char*));
	for(i = 0; i < infoheader->height; i++)
		newimg[i] = malloc(infoheader->width*sizeof(unsigned char*));
	for(i = 0; i< infoheader->height; i++)
		for(j = 0; j < infoheader->width; j++)
		{	
			X = trunc((pixels[i][j].blue + pixels[i][j].green + pixels[i][j].red)/3);
			newimg[i][j].blue = X;
			newimg[i][j].green = X;
			newimg[i][j].red = X;
		}
	filename = strtok(filename,"_");
	strcat(filename,"_black_white.bmp");
	makebmp(filename,infoheader,header,newimg);
	for(i = 0; i < infoheader->height; i++)
		free(newimg[i]);
	free(newimg);
}

void convlayers(char *filename,int **filtru,int N,bmp_infoheader *infoheader,
			bmp_fileheader *header,BGR **pixels){

	int i,j,k,m;
	const int height = infoheader->height;
	const int width = infoheader->width;
	BGR **newimg;
	newimg = malloc(infoheader->height*sizeof(unsigned char*));
	for(i = 0; i < infoheader->height; i++)
		newimg[i] = malloc(infoheader->width*sizeof(unsigned char*));
	BGR **RGB;
	RGB = malloc(infoheader->height*sizeof(unsigned char*));
	for(i = 0; i < infoheader->height; i++)
		RGB[i]=malloc(infoheader->width*sizeof(unsigned char*));
	//rasturnare matrice de pixeli
	for(i = 0; i < infoheader->height; i++){
		for(j = 0; j < infoheader->width; j++)
		{	
			RGB[i][j].red = pixels[height-1-i][j].red;
			RGB[i][j].green = pixels[height-1-i][j].green;
			RGB[i][j].blue = pixels[height-1-i][j].blue;
		}
	}
	int B,G,R;
	for(i = 0; i < height; i++){
		for(j = 0; j < width; j++)
		{
			R = B = G = 0;
			for( k = 0; k < N ; k++)
				for( m = 0; m < N; m++)
					//verifica daca este vecin valid al pixelului respectiv
					if((i-N/2+k) < 0 || (j-N/2+m) < 0 || 
						(i-N/2+k) >= height || (j-N/2+m) >= width){
						B += 0;
						G += 0;
						R += 0;
					} 
					else{
						B += RGB[i-N/2+k][j-N/2+m].blue*filtru[k][m];
						G += RGB[i-N/2+k][j-N/2+m].green*filtru[k][m];
						R += RGB[i-N/2+k][j-N/2+m].red*filtru[k][m];
					}
			(B > 255)?(B = 255):((B < 0)?(B = 0):B);
			newimg[height-1-i][j].blue=B;
			(G > 255)?(G = 255):((G < 0)?(G = 0):G);
			newimg[height-1-i][j].green=G;
			(R > 255)?(R = 255):((R < 0)?(R = 0):R);
			newimg[height-1-i][j].red=R;			
		}
	}
	filename = strtok(filename,"_");
	strcat(filename,"_filter.bmp");
	makebmp(filename,infoheader,header,newimg);	
	for(i = 0; i < infoheader->height; i++){
		free(RGB[i]);
	}
	free(RGB);
	for(i = 0; i < infoheader->height; i++){
		free(newimg[i]);
	}
	free(newimg);
}

void nocrop(char *filename,bmp_infoheader *infoheader,
			bmp_fileheader *header,BGR **pixels){

	int padding = 4-((infoheader->width*3)%4);
	int i,j,k;
	const int height = infoheader->height;
	const int width = infoheader->width;
	BGR white = {255, 255, 255};
	unsigned char pad = 0;
	int dif = abs(infoheader->width - infoheader->height);
	if(dif != 0){
		if(infoheader->width < infoheader->height)
			infoheader->width = infoheader->height;
		else
			infoheader->height = infoheader->width;
	}
	filename = strtok(filename,"_");
	strcat(filename,"_nocrop.bmp");
	if(dif != 0){
		FILE *out = fopen(filename,"wb");
		fwrite(header,sizeof(bmp_fileheader),1,out);
		fwrite(infoheader,sizeof(bmp_infoheader),1,out);
		//completez coloanele daca este necesar
		if(height > width){
			int newpad = 4-((infoheader->width*3)%4);
		//daca diferenta este para, se completeaza egal in ambele parti	
		if(dif % 2 == 0){
			for(i = 0; i < infoheader->height; i++){
				for(j = 0; j < (dif/2); j++)
					fwrite(&white,sizeof(unsigned char),3,out);
				for(j = 0; j < width; j++)
				{
					fwrite(&pixels[i][j].blue,sizeof(unsigned char),1,out);
					fwrite(&pixels[i][j].green,sizeof(unsigned char),1,out);
					fwrite(&pixels[i][j].red,sizeof(unsigned char),1,out);
				}
				for(j = 0; j < (dif/2); j++)
					fwrite(&white,sizeof(unsigned char),3,out);
				if(newpad < 4)
					for(k = 0; k < newpad; k++)
						fwrite(&pad,sizeof(unsigned char),1,out);

			}
		}
		//daca este impara, se completeaza cu o coloana in plus la stanga
		else
		if(dif%2 != 0){
			for(i = 0; i < infoheader->height; i++){
				for(j=0;j<(dif/2)+1;j++)
					fwrite(&white,sizeof(unsigned char),3,out);
				for(j = 0; j < width; j++){
					fwrite(&pixels[i][j].blue,sizeof(unsigned char),1,out);
					fwrite(&pixels[i][j].green,sizeof(unsigned char),1,out);
					fwrite(&pixels[i][j].red,sizeof(unsigned char),1,out);
				}
				for(j = 0; j < (dif/2); j++){
					fwrite(&white,sizeof(unsigned char),3,out);
				}
				if(newpad < 4)
					for(k = 0; k < newpad; k++){
						fwrite(&pad,sizeof(unsigned char),1,out);
					}
			}
		}
	}
	//completez liniile daca este necesar
	if(width > height){
		//daca diferenta este para, se completeaza egal in ambele parti	
		if(dif % 2 == 0){
			for(i = 0; i < (dif/2); i++){
				for(j = 0; j < width; j++)
					fwrite(&white,sizeof(unsigned char),3,out);
				if(padding < 4){
					for(k = 0; k < padding; k++)
						fwrite(&pad,sizeof(unsigned char),1,out);
				}
			}		
			for(i = 0; i < height; i++){
				for(j = 0; j < width; j++)
				{
					fwrite(&pixels[i][j].blue,sizeof(unsigned char),1,out);
					fwrite(&pixels[i][j].green,sizeof(unsigned char),1,out);
					fwrite(&pixels[i][j].red,sizeof(unsigned char),1,out);
				}
				if(padding < 4){
					for(k = 0; k < padding; k++)
						fwrite(&pad,sizeof(unsigned char),1,out);
				}
			}
			for(i = 0; i < (dif/2); i++){
				for(j = 0; j < width; j++)
					fwrite(&white,sizeof(unsigned char),3,out);
				if(padding < 4){
					for(k = 0; k < padding; k++)
						fwrite(&pad,sizeof(unsigned char),1,out);
				}
			}	
		}	
		//daca este impara, se completeaza cu o linie in plus la sfarsit
		else
		if(dif % 2 != 0){
			for(i = 0; i < (dif/2) + 1; i++){
				for(j = 0; j < width; j++)
					fwrite(&white,sizeof(unsigned char),3,out);
				if(padding < 4){
					for(k = 0; k < padding; k++)
						fwrite(&pad,sizeof(unsigned char),1,out);
				}
			}		
			for(i = 0; i < height; i++){
				for(j = 0; j < width; j++)
				{
					fwrite(&pixels[i][j].blue,sizeof(unsigned char),1,out);
					fwrite(&pixels[i][j].green,sizeof(unsigned char),1,out);
					fwrite(&pixels[i][j].red,sizeof(unsigned char),1,out);
				}
				if(padding < 4){
					for(k = 0; k < padding; k++)
						fwrite(&pad,sizeof(unsigned char),1,out);
				}
			}
			for(i = 0; i < (dif/2); i++){
				for(j = 0; j < width; j++)
					fwrite(&white,sizeof(unsigned char),3,out);
				if(padding < 4){
					for(k = 0; k < padding; k++)
						fwrite(&pad,sizeof(unsigned char),1,out);
				}
			}
		}
	}
		fclose(out);
	}
	else
		makebmp(filename,infoheader,header,pixels);
}

void pooling(char *filename,char type,int N,bmp_infoheader *infoheader,
			bmp_fileheader *header,BGR **pixels){

	int i,j,k,m;
	const int height = infoheader->height;
	const int width = infoheader->width;
	BGR **copy;
	copy= malloc(infoheader->height*sizeof(unsigned char*));
	for(i = 0; i < infoheader->height; i++)
		copy[i] = malloc(infoheader->width*sizeof(unsigned char*));
	BGR min,max;
	if(type == 'M')
		for(i = 0; i < height; i++)
			for(j = 0; j < width; j++){
				max = pixels[i][j];
				for(k = 0; k < N; k++)
					for(m = 0; m < N; m++)
						//verifica daca este vecin valid
						if(!((i-N/2+k)<0 || (j-N/2+m)<0 || 
							(i-N/2+k) >= height || (j-N/2+m) >= width)){
							if(pixels[i-N/2+k][j-N/2+m].blue > max.blue)
								max.blue = pixels[i-N/2+k][j-N/2+m].blue;
							if(pixels[i-N/2+k][j-N/2+m].green > max.green)
								max.green = pixels[i-N/2+k][j-N/2+m].green;
							if(pixels[i-N/2+k][j-N/2+m].red > max.red)
								max.red = pixels[i-N/2+k][j-N/2+m].red;
						}
				copy[i][j].blue = max.blue;
				copy[i][j].green = max.green;
				copy[i][j].red = max.red;
			}
	else
		for(i = 0; i < height; i++)
			for(j = 0; j < width; j++){
				min = pixels[i][j];
				for(k = 0; k < N; k++)
					for(m = 0; m < N; m++)
						//verifica daca este vecin valid
						if(!((i-N/2+k)<0 || (j-N/2+m)<0 || 
							(i-N/2+k) >= height || (j-N/2+m) >= width)){
							if(pixels[i-N/2+k][j-N/2+m].blue < min.blue)
								min.blue = pixels[i-N/2+k][j-N/2+m].blue;
							if(pixels[i-N/2+k][j-N/2+m].green < min.green)
								min.green = pixels[i-N/2+k][j-N/2+m].green;
							if(pixels[i-N/2+k][j-N/2+m].red < min.red)
								min.red = pixels[i-N/2+k][j-N/2+m].red;
						}
					else{
						min.blue = 0;
						min.green = 0;
						min.red = 0;
					}
				copy[i][j].blue = min.blue;
				copy[i][j].green = min.green;
				copy[i][j].red = min.red;
			}
	filename = strtok(filename,"_");
	strcat(filename,"_pooling.bmp");
	makebmp(filename,infoheader,header,copy);	
	for(i = 0; i < infoheader->height; i++)
		free(copy[i]);
	free(copy);
}
void fill(int i, int j, const int x, const int y, BGR **pixels,
			bmp_infoheader *infoheader, int **zone, int threshold){
	//verifica daca este un vecin valid si daca face parte dintr-o zona
	if((i-1) >= 0 && zone[i-1][j] == 0)
		if((abs(pixels[x][y].blue - pixels[i-1][j].blue) +
			abs(pixels[x][y].green - pixels[i-1][j].green) +
			abs(pixels[x][y].red - pixels[i-1][j].red)) <= threshold){
			zone[i-1][j] = zone[x][y];
			fill(i-1,j,x,y,pixels,infoheader,zone,threshold);
		}
	if((j+1) < infoheader->width && zone[i][j+1] == 0)
		if((abs(pixels[x][y].blue - pixels[i][j+1].blue) +
			abs(pixels[x][y].green - pixels[i][j+1].green) +
			abs(pixels[x][y].red - pixels[i][j+1].red)) <= threshold){
			zone[i][j+1] = zone[x][y];
			fill(i,j+1,x,y,pixels,infoheader,zone,threshold);
		}
	if((i+1) < infoheader->height && zone[i+1][j] == 0)
		if((abs(pixels[x][y].blue - pixels[i+1][j].blue) +
			abs(pixels[x][y].green - pixels[i+1][j].green) +
			abs(pixels[x][y].red - pixels[i+1][j].red)) <= threshold){
			zone[i+1][j] = zone[x][y];
			fill(i+1,j,x,y,pixels,infoheader,zone,threshold);
		}
	if((j-1) >= 0 && zone[i][j-1] == 0)
		if((abs(pixels[x][y].blue - pixels[i][j-1].blue) +
			abs(pixels[x][y].green - pixels[i][j-1].green) +
			abs(pixels[x][y].red - pixels[i][j-1].red)) <= threshold){
			zone[i][j-1] = zone[x][y];
			fill(i,j-1,x,y,pixels,infoheader,zone,threshold);
		}	
}

void clustering(char *filename,int threshold,bmp_infoheader *infoheader,
			bmp_fileheader *header,BGR **pixels){

	int nr = 0;
	int i, j, k;
	BGR **newimg;
	newimg = malloc(infoheader->height*sizeof(unsigned char*));
	for(i = 0; i < infoheader->height; i++)
		newimg[i] = malloc(infoheader->width*sizeof(unsigned char*));
	const int width = infoheader->width;
	const int height = infoheader->height;
	BGR **reverse;
	reverse = (BGR**)malloc(height*sizeof(BGR*));
	for(i = 0;i < height; i++)
		reverse[i] = (BGR*)malloc(width*sizeof(BGR));
	//rasturnare matrice de pixeli
	for(i = 0; i < height; i++)
		for(j = 0;j < width; j++){
			reverse[i][j].blue = pixels[height-1-i][j].blue;
			reverse[i][j].green = pixels[height-1-i][j].green;
			reverse[i][j].red = pixels[height-1-i][j].red;
		}
	if(threshold > 0){
		int **zone;
		zone = (int**)malloc(height*sizeof(int*));
		for(i = 0; i < height; i++)
			zone[i] = (int*)malloc(width*sizeof(int));
		for(i = 0; i < height; i++)
			for(j = 0;j < width; j++)
				zone[i][j] = 0;
		//alcatuirea zonelor de pixeli
		for(i = 0; i < height; i++)
			for(j = 0; j<width; j++){
				if(zone[i][j] == 0){
					nr++;
					zone[i][j] = nr;
					fill(i,j,i,j,reverse,infoheader,zone,threshold);
				}
			}
		//determinare suma valorilor pixelilor pentru fiecare zona		
		int sumB, sumG, sumR, N;
		for(k = 1; k <= nr; k++){
			sumB = 0;
			sumG = 0;
			sumR = 0;
			N = 0;
			for(i = 0; i < height; i++)
				for(j = 0;j < width; j++)
					if(zone[i][j] == k){
						sumR = sumR + reverse[i][j].red;
						sumG = sumG + reverse[i][j].green;
						sumB = sumB + reverse[i][j].blue;
						N++;
					}
			//atribuire noua valoare a pixelilor si rasturnarea acestora		
			for(i = 0; i < height; i++)
				for(j = 0; j < width; j++)
					if(zone[i][j] == k){
						newimg[height-1-i][j].red = sumR/N;
						newimg[height-1-i][j].green = sumG/N;
						newimg[height-1-i][j].blue = sumB/N;
					}	
		}
		filename = strtok(filename,".");
		strcat(filename,"_clustered.bmp");
		makebmp(filename,infoheader,header,newimg);
		for(i = 0;i < height; i++)
		free(zone[i]);
		free(zone);
	}
	else{
		filename = strtok(filename,".");
		strcat(filename,"_clustered.bmp");
		makebmp(filename,infoheader,header,pixels);
	}
	for(i = 0; i < height; i++)
		free(reverse[i]);
	free(reverse);
	for(i = 0; i < infoheader->height; i++)
		free(newimg[i]);
	free(newimg);
}

int main()
{			
	char image[10];
	char filtre[26];
	char poolinfo[27];
	char cluster[30];
	FILE *in = fopen("input.txt","r");
	if(in == NULL)
		printf("Nu s-a putut deschide fisierul\n");
	fgets(image,10,in);
	fseek(in,1,SEEK_CUR);
	fgets(filtre,26,in);
	fseek(in,1,SEEK_CUR);
	fgets(poolinfo,27,in);
	fseek(in,1,SEEK_CUR);
	fgets(cluster,30,in);
	fclose(in);
	int i,j;
	//citire filtru
	in = fopen(filtre,"r");
	int N, **filtru;
	fscanf(in,"%d", &N);
	filtru = malloc(N*sizeof(int*));
	for(i = 0; i < N; i++)
		filtru[i] = malloc(N*sizeof(int*));
	for(i = 0; i < N; i++)
		for(j = 0; j < N; j++)
			fscanf(in,"%d",&filtru[i][j]);
	fclose(in);
	//citire informatii pooling
	int size;
	char type;
	in = fopen(poolinfo,"r");
	fscanf(in,"%c",&type);
	fscanf(in,"%d",&size);
	fclose(in);
	//citire threshold
	int threshold;
	in = fopen(cluster,"r");
	fscanf(in,"%d",&threshold);
	fclose(in);
	bmp_fileheader *header;
	header = malloc(sizeof(bmp_fileheader));
	bmp_infoheader *infoheader;
	infoheader = malloc (sizeof(bmp_infoheader));
	in = fopen(image,"rb");
	if(in == NULL)
		printf("Nu s-a putut deschide fisierul\n");
	//citire header
	fread(header,sizeof(bmp_fileheader),1,in);
	//citire infoheader
	fread(infoheader,sizeof(bmp_infoheader),1,in);
	const int height = infoheader->height;
	//citire pixeli
	BGR **pixels;
	pixels = malloc(infoheader->height*sizeof(unsigned char*));
	for(i = 0; i < infoheader->height; i++)
		pixels[i] = malloc(infoheader->width*sizeof(unsigned char*));
	int padding = 4 - ((infoheader->width*3)%4);
	for(i = 0; i < infoheader->height; i++){
		for(j = 0; j < infoheader->width; j++)
		{
			fread(&pixels[i][j].blue,sizeof(unsigned char),1,in);
			fread(&pixels[i][j].green,sizeof(unsigned char),1,in);
			fread(&pixels[i][j].red,sizeof(unsigned char),1,in);
		}
		if(padding < 4)
			fseek(in,padding,SEEK_CUR);
	}
	fclose(in);
	clustering(image,threshold,infoheader,header,pixels);
	pooling(image,type,size,infoheader,header,pixels);
	convlayers(image,filtru,N,infoheader,header,pixels);
	blackandwhite(image,infoheader,header,pixels);
	nocrop(image,infoheader,header,pixels);
	//eliberare memorie
	for(i = 0; i < height; i++)
		free(pixels[i]);
	free(pixels);
	free(infoheader);
	free(header);
	for(i = 0; i < N; i++)
		free(filtru[i]);
	free(filtru);
}