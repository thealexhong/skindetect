/**
  Simple human identification using skin colour detection
  USAGE:
  g++ skindetect.cpp -o skindetect `pkg-config --cflags --libs opencv`
  ./skindetect imagefile.jpg
*/

#include <stdio.h>
#include <iostream>
#include <fstream>
#include "cv.h"
#include "highgui.h"

/**
  Just a huge definition of colours...
*/
// Image
#define X 640 // width
#define Y 480	// height
#define BLOCKsize 1
#define BLOCK_X (X/BLOCKsize)
#define BLOCK_Y (Y/BLOCKsize)

// RGB values of interest
#define REDMAX 230
#define REDMIN 160 
#define BLUEMAX 210
#define BLUEMIN 130
#define GREENMAX 220 
#define GREENMIN 130
#define IIREDMAX 87
#define IIREDMIN 38
#define IIBLUEMAX 70
#define IIBLUEMIN 39
#define IIGREENMAX 66
#define IIGREENMIN 35
#define THRESHOLD 0.1
#define IITHRESHOLD 0.1
#define YELLOW 14
#define LIGHTGREY 7
#define DIFF 23 //15 best but changed for WOOD elimination
#define SKIN2RGMAX 24
#define SKIN2RGMIN 9
#define SKIN2RBMAX 20
#define SKIN2RBMIN 9
#define NONSKINNUM 5
#define REDMAXTWO 230
#define REDMINTWO 100
#define GREENMAXTWO 170
#define GREENMINTWO 54
#define BLUEMAXTWO 150
#define BLUEMINTWO 37
#define DIFFTWORG 20
#define DIFFTWOGB 20
// Wood-like objects
#define WOODMAXRED 191
#define WOODMINRED 165
#define WOODMAXGREEN 169
#define WOODMINGREEN 151
#define WOODMAXBLUE 162
#define WOODMINBLUE 141
#define RWOODTWOMAX 210 
#define RWOODTWOMIN 157
#define GWOODTWOMAX 183
#define GWOODTWOMIN 127
#define BWOODTWOMAX 168
#define BWOODTWOMIN 112
#define WOOD2RGMAX 34
#define WOOD2RGMIN 15
#define WOOD2RBMAX 60
#define WOOD2RBMIN 30
// Red objects
#define REDNESSMAXRED 92
#define REDNESSMINRED 70
#define REDNESSMAXGREEN 70
#define REDNESSMINGREEN 50
#define REDNESSMAXBLUE 76
#define REDNESSMINBLUE 59
#define REDTWOMAXRED 191
#define REDTWOMINRED 175
#define REDTWOMAXGREEN 112
#define REDTWOMINGREEN 107
#define REDTWOMAXBLUE 127
#define REDTWOMINBLUE 106
#define REDTWORGMAX 70
#define REDTWORGMIN 55
#define REDTWOGBMAX 0
#define REDTWOGBMIN 0
// Coat
#define RCOATMAX 255
#define RCOATMIN 177
#define GCOATMAX 248
#define GCOATMIN 177
#define BCOATMAX 208
#define BCOATMIN 125
// Bright light
#define RSHINEMAX 205
#define RSHINEMIN 199
#define GSHINEMAX 196
#define GSHINEMIN 192
#define BSHINEMAX 195
#define BSHINEMIN 177
// Cardboard
#define RCARDMAX 154
#define RCARDMIN 142
#define GCARDMAX 135
#define GCARDMIN 128
#define BCARDMAX 130
#define BCARDMIN 124
#define CARDRGMAX 17
#define CARDRGMIN 11
#define CARDRBMAX 25
#define CARDRBMIN 19
#define PSHINEREDMAX 150//255
#define PSHINEREDMIN 050//100 //182
#define PSHINEGREENMAX 110//255
#define PSHINEGREENMIN 000//100//150 //180
#define PSHINEBLUEMAX 050//255
#define PSHINEBLUEMIN 000//100//114

// image -> pixels
CvScalar** getPixelColor(IplImage *img)
{ 
	CvScalar** data = new CvScalar*[X];
	for(int i = 0; i < X; i++)
		data[i] = new CvScalar[Y];

  // RGB values of each pixel in the image
  for (int x = 0; x < img->width; x++)
  {
    for (int y = 0; y < img->height; y++)
    {
      data[x][y].val[0] = 
        ((uchar*)(img->imageData + img->widthStep * y))[x * 3]; // blue
		  data[x][y].val[1] = 
        ((uchar*)(img->imageData + img->widthStep * y))[x * 3 + 1]; // green
		  data[x][y].val[2] = 
        ((uchar*)(img->imageData + img->widthStep * y))[x * 3 + 2]; // red
		}
  }
  return data;
}

int main(int argc, char** argv)
{
  IplImage* img;
  
  if(argc == 2 && (img = cvLoadImage(argv[1], CV_LOAD_IMAGE_UNCHANGED)) != 0)
  {
  	CvScalar** data = getPixelColor(img); // pixel values

    // Some counter variables
  	int sumRED = 0,
        sumGREEN = 0,
        sumBLUE = 0;	
    int counterRED = 0,
        counterGREEN = 0,
        counterBLUE = 0;
  	float avgR[BLOCK_X][BLOCK_Y],
          avgG[BLOCK_X][BLOCK_Y],
          avgB[BLOCK_X][BLOCK_Y];

	// Looks at each RGB values and see if it passes the skin filters
	int a = 0, i = 0, b, j; // loop counters
  while (a < BLOCK_X && i < X)
  {
    b = 0, j = 0;
    while (b < BLOCK_Y && j < Y)
    {
			counterRED = 0; counterGREEN = 0; counterBLUE = 0;
			sumRED = 0; sumGREEN = 0; sumBLUE = 0;
      
      // If passs, counter is incremented
			for (int n = i; n < i + BLOCKsize; n++) {
				for (int m = j; m < j + BLOCKsize; m++) {
					if (data[n][m].val[2] <= REDMAX && data[n][m].val[2] >= REDMIN) {
						sumRED += data[n][m].val[2];
						counterRED++;
					}
					if (data[n][m].val[1] <= GREENMAX && data[n][m].val[1] >= GREENMIN) {
						sumGREEN += data[n][m].val[1];
						counterGREEN++;
					}
					if (data[n][m].val[0] <= BLUEMAX && data[n][m].val[0] >= BLUEMIN) {
						sumBLUE += data[n][m].val[0];
						counterBLUE++;
					}
				}
			}
      
      /*
        For a block of [BLOCKsize x BLOCKsize] to be called skin, threshold
        must be passed. Average of RGB are taken of the block.
      */
			if ((counterRED / (BLOCKsize * BLOCKsize)) >= THRESHOLD) {
				avgR[a][b] = (sumRED / counterRED);
			}
			else {
				avgR[a][b] = 0;
			}

			if ((counterGREEN / (BLOCKsize * BLOCKsize)) >= THRESHOLD) {
				avgG[a][b] = (sumGREEN / counterGREEN);
			}
			else {
				avgG[a][b] = 0;
			}

			if ((counterBLUE / (BLOCKsize * BLOCKsize)) >= THRESHOLD) {
				avgB[a][b] = (sumBLUE / counterBLUE);
			}
			else {
				avgB[a][b] = 0;
			}
      
      b++;
      j+= BLOCKsize;
    }
    a++;
    i+= BLOCKsize;
  }
  
	
  int SKIN[(X / BLOCKsize)][(Y / BLOCKsize)]; // skin matrix
	/*
    The multiple filters that the averages of RGB must pass through
  */
	for (int v = 0; v < (X / BLOCKsize); v++) {
    for (int w = 0; w < (Y / BLOCKsize); w++) {
				if (avgR[v][w] < REDMAX && 
            (avgR[v][w] - avgG[v][w]) >= DIFF &&
            (avgR[v][w] - avgB[v][w]) >= DIFF &&
            avgR[v][w] > REDMIN &&
            avgG[v][w] < GREENMAX &&
            avgG[v][w] > GREENMIN &&
            avgB[v][w] < BLUEMAX &&
            avgB[v][w] > BLUEMIN)
					SKIN[v][w]=1;
        
        if ((!((avgR[v][w] < REDMAX &&
              (avgR[v][w] - avgG[v][w]) >= DIFF &&
              (avgR[v][w] - avgB[v][w]) >= DIFF &&
              avgR[v][w] > REDMIN &&
              avgG[v][w] < GREENMAX &&
              avgG[v][w] > GREENMIN &&
              avgB[v][w] < BLUEMAX &&
              avgB[v][w] > BLUEMIN) ||
            (avgR[v][w] < IIREDMAX &&
            (avgR[v][w] - avgG[v][w]) <= SKIN2RGMAX &&
            (avgR[v][w] - avgG[v][w]) >= SKIN2RGMIN &&
            (avgR[v][w] - avgB[v][w]) <= SKIN2RBMAX &&
            (avgR[v][w] - avgB[v][w]) >= SKIN2RBMIN &&
            avgR[v][w] > IIREDMIN &&
            avgG[v][w] < IIGREENMAX &&
            avgG[v][w] > IIGREENMIN &&
            avgB[v][w] < IIBLUEMAX &&
            avgB[v][w] > IIBLUEMIN))) ||
                
            ((avgR[v][w] - avgG[v][w]) <= NONSKINNUM &&
            (avgR[v][w] - avgB[v][w]) <= NONSKINNUM &&
            (avgG[v][w] - avgB[v][w]) <= NONSKINNUM) ||
            
            // Eliminate one shade of wood
            (avgR[v][w] <= WOODMAXRED &&
            avgR[v][w] >= WOODMINRED &&
            avgG[v][w] <= WOODMAXGREEN &&
            avgG[v][w] >= WOODMINGREEN &&
            avgB[v][w] <= WOODMAXBLUE &&
            avgB[v][w] >= WOODMINBLUE) ||
            
            // Eliminate one shade of red
            (avgR[v][w] <= REDNESSMAXRED &&
            avgR[v][w] >= REDNESSMINRED &&
            avgG[v][w] <= REDNESSMAXGREEN &&
            avgG[v][w] >= REDNESSMINGREEN &&
            avgB[v][w] <= REDNESSMAXBLUE &&
            avgB[v][w] >= REDNESSMINBLUE) ||
            
            (avgR[v][w] < RCOATMAX &&
            (avgR[v][w] - avgG[v][w]) >= 0 &&
            (avgG[v][w] - avgB[v][w]) >= 0 &&
            avgR[v][w] > RCOATMIN &&
            avgG[v][w] < GCOATMAX &&
            avgG[v][w] > GCOATMIN &&
            avgB[v][w] < BCOATMAX &&
            avgB[v][w] > BCOATMIN))
					SKIN[v][w]=0;																							
        
        
				if (avgR[v][w] <= RSHINEMAX &&
            (avgR[v][w] - avgG[v][w]) <= 5 &&
            (avgR[v][w] - avgB[v][w]) <= 6 &&
            avgR[v][w] >= RSHINEMIN &&
            avgG[v][w] <= GSHINEMAX &&
            avgG[v][w] >= GSHINEMIN &&
            avgB[v][w] <= BSHINEMAX &&
            avgB[v][w] >= BSHINEMIN)
					SKIN[v][w]=1;
		  }
	  }
      
    // Write results to file 
    std::ofstream myfile;
    myfile.open ("skin.txt");
	  for (int i=0; i < X / BLOCKsize; i++) {
	    for (int j=0; j < Y / BLOCKsize; j++)
		    myfile << SKIN[i][j];
      myfile << "\n";
	  }
    myfile.close();
	  return 0;
  }
  return -1;
}