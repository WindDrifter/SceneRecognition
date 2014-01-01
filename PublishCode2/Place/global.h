#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <math.h>
#include <time.h>

#include <cv.h>

#include "mdarray.h"
#include "IntImage.h"
#include "util.h"

extern Array2d<double> lf,lf2;

extern bool useSobel;
extern bool useBoth;
extern int sizeCV;
extern int extra;
extern int splitlevel;
extern int splitsize[4];
extern double splitratio[3];

inline int CensusTransform(double** p,const int x,const int y)
{
    int index = 0;
    if(p[x][y]<=p[x-1][y-1]) index |= 0x80;
    if(p[x][y]<=p[x-1][y]) index |= 0x40;
    if(p[x][y]<=p[x-1][y+1]) index |= 0x20;
    if(p[x][y]<=p[x][y-1]) index |= 0x10;
    if(p[x][y]<=p[x][y+1]) index |= 0x08;
    if(p[x][y]<=p[x+1][y-1]) index |= 0x04;
    if(p[x][y]<=p[x+1][y]) index |= 0x02;
    if(p[x][y]<=p[x+1][y+1]) index |= 0x01;
    return index;
}

void GenerateFeatureForSingleChannelImage(IntImage<double>& im,double* pointer,std::ofstream* pcaout);
void PrepareScaleRatio(Array2dC<double>& scaleratio);
void NormalizeLoadFactors(Array2d<double>& lf);
void FindMinMaxValue(Array2d<double>& features,Array2dC<bool>& train,Array2dC<double>& minmax);
void ScaleFeatures(Array2d<double>& features,Array2d<double>& test);

void sceneDetection();
void Load_UIUC_Scenes();
void KTH_IDOL_Test();
void SwedenLeaf();
void SwedenLeafContour();
void Load_Princeton_Events();
