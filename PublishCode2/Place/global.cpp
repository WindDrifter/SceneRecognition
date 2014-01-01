#include "global.h"

#include <algorithm>

Array2d<double> lf,lf2;

bool useSobel = false;
bool useBoth = false; // whether use both image and Sobel
int sizeCV = 5;
int extra = 2;
int splitlevel = 2;
int splitsize[4] = {1,6,31,144};
double splitratio[3] = {1.0,1.0,1.35};

void NormalizeLoadFactors(Array2d<double>& lf)
{// normalize such that the eigenvectors have 0 mean and 1 std
    for(int i=0;i<lf.nrow;i++)
    {
        double sum;
        sum = 0; for(int j=0;j<lf.ncol;j++) sum+=lf.p[i][j]; sum/=lf.ncol;
        for(int j=0;j<lf.ncol;j++) lf.p[i][j] -= sum;
        sum = 0; for(int j=0;j<lf.ncol;j++) sum += lf.p[i][j]*lf.p[i][j]; sum /= lf.ncol;
        // What we really need is a for a feature vector x and a load factor f, calculate their correlation coefficient
        // so we need to make f 0 mean and 1 norm -- instead of 1 sigma as the code here did
        // but the difference is only a constant, i.e. equivalent if no floating point roundoff errors are considered
        // and because I do not want to run the code again on all datasets, let just don't remove the 'sum /= lf.ncol' sentence. Prentend it's not there :-)
        sum = 1.0/sqrt(sum);
        for(int j=0;j<lf.ncol;j++) lf.p[i][j] *= sum;
    }
}

void PrepareScaleRatio(Array2dC<double>& scaleratio)
{//if splitlevel==2, the level 0 (entire image) features had a different (higher) weight
    for(int i=0;i<scaleratio.ncol;i++) scaleratio.buf[i]=1.0;
    if(splitlevel==2)
    {
        for(int i=0;i<scaleratio.ncol;i++)
        {
            int index = i/(lf.nrow+extra);
            if(index<25)
                scaleratio.buf[i] = splitratio[0];
            else if(index<30)
                scaleratio.buf[i] = splitratio[1];
            else if(index<31)
                scaleratio.buf[i] = splitratio[2];
            else if(index<56)
                scaleratio.buf[i] = splitratio[0];
            else if(index<61)
                scaleratio.buf[i] = splitratio[1];
            else
                scaleratio.buf[i] = splitratio[2];
        }
    }
}

void FindMinMaxValue(Array2d<double>& features,Array2dC<bool>& train,Array2dC<double>& minmax)
{   // find min/max feature values of the training set in order to normalize features to [-1 1]
    // minmax.p[0] -- min; minmax.p[1] -- max
    for(int i=0;i<features.ncol;i++) { minmax.p[0][i]= 1E20; minmax.p[1][i]=-1E20; }
    for(int i=0;i<features.nrow;i++)
    {
        if(train.buf[i]==false) continue;
        for(int j=0;j<features.ncol;j++) 
        {
            if(features.p[i][j]<minmax.p[0][j]) minmax.p[0][j] = features.p[i][j];
            if(features.p[i][j]>minmax.p[1][j]) minmax.p[1][j] = features.p[i][j];
        }
    }
    for(int i=0;i<features.ncol;i++)
    {
        if(minmax.p[0][i]>=minmax.p[1][i])
        {
            //std::cerr<<"There exists features with equal min value and max value."<<std::endl;
            minmax.p[1][i] = 0.0;
        }
        else
            minmax.p[1][i] = 2.0/(minmax.p[1][i]-minmax.p[0][i]);
    }
}

void ScaleFeatures(Array2d<double>& features,Array2d<double>& test)
{  //similar to FindMinMaxValue, but here train/test set are provided separately
    assert(features.ncol == test.ncol); //train and test have same number of features
    for(int j=0;j<features.ncol;j++)
    {
        double fmin = 1E20, fmax = -fmin;
        for(int i=0;i<features.nrow;i++)
        {
            if(features.p[i][j]>fmax) fmax = features.p[i][j];
            if(features.p[i][j]<fmin) fmin = features.p[i][j];
        }
        double r;
        if(fmax-fmin<1e-10) r = 0; else r = 1.0/(fmax-fmin);
        for(int i=0;i<features.nrow;i++) features.p[i][j] = (features.p[i][j]-fmin)*r;
        if(features.p!=test.p) 
            for(int i=0;i<test.nrow;i++) test.p[i][j] = (test.p[i][j]-fmin)*r;
    }
}

//!!!!!!!!!! PACT
//!!!!!!!!!! comment out the 2 functions for orientation histogram below, and uncomment these 2 functions for PACT to use PACT as features
void GenerateHistForOneRect(double** p,const int x1,const int x2,const int y1,const int y2,double* pointer,double* temp)
{   // generate CT histogram for a given rectangle: pixels in [x1 y1]-(x2,y2) -- including (x1,y1) but excluding (y1,y2)
    for(int i=0;i<256;i++) temp[i]=0;
    double pixelSum = 0, pixelSq = 0;
    for(int x=x1;x<x2;x++)
    {
        for(int y=y1;y<y2;y++)
        {
            pixelSum += p[x][y];
            pixelSq += p[x][y]*p[x][y];
            temp[CensusTransform(p,x,y)]++;
        }
    }

    const double size = (x2-x1)*(y2-y1);
    if(extra)
    {
        pixelSum /= ( size*255.0 );
        pixelSq /= ( size*255.0*255.0 );
        pixelSq -= pixelSum*pixelSum; if(pixelSq<1E-6) pixelSq=0; else pixelSq = sqrt(pixelSq);
        pointer[0] = pixelSum;
        if(pixelSum<1E-6)
        {
            if(extra>=2) pointer[1]=0;
        }
        else 
        {
            if(extra>=2) pointer[1]=pixelSq/pixelSum;
        }
    }
    // normalize histogram to have 0 mean, remove the first and last entry, and normalize to have unit norm
    double sum = 0; 
    for(int j=0;j<256;j++) sum += temp[j]; sum/=256;
    for(int j=0;j<256;j++) temp[j] -= sum;
    temp[0] = temp[255] = 0;
    double sq = 0; 
    for(int j=0;j<256;j++) sq += temp[j]*temp[j]; 
    sq = (sq>100)?1.0/sqrt(sq):0; 
    for(int j=0;j<256;j++) temp[j] *= sq;
}

void GenerateFeatureForOneRect(double** p,const int x1,const int x2,const int y1,const int y2,double* pointer,Array2d<double>& lf,std::ofstream* pcaout)
{   // generate PCA of histogram
    double temp[256];

    GenerateHistForOneRect(p,x1,x2,y1,y2,pointer,temp);
    if(pcaout!=NULL) // save the histogram for generating PCA eigenvectors if 'pcaout' file handler is not NULL
    {
        for(int j=0;j<256;j++) (*pcaout)<<temp[j]<<" ";
        (*pcaout)<<std::endl;
    }

    for(int j=0;j<lf.nrow;j++) 
    { 
        double sum=0; 
        for(int k=0;k<256;k++) sum+=lf.p[j][k]*temp[k];
        pointer[j+extra] = sum; 
    }
}

//!!!!!!!!! Orientation histogram
//!!!!!!!!! comment out the above 2 functions, and uncomment the below 2, to change from PACT to orientation histogram
// void GenerateHistForOneRect(double** p,const int x1,const int x2,const int y1,const int y2,double* pointer,double* temp)
// {   // generate CT histogram for a given rectangle: pixels in [x1 y1]-(x2,y2) -- including (x1,y1) but excluding (y1,y2)
//  for(int i=0;i<40;i++) temp[i]=0;
//  double pixelSum = 0, pixelSq = 0;
//  for(int x=x1;x<x2;x++)
//  {
//      for(int y=y1;y<y2;y++)
//      {
//          pixelSum += p[x][y];
//          pixelSq += p[x][y]*p[x][y];
//          temp[int((atan2(p[x][y]-p[x+1][y],p[x][y]-p[x][y+1])+PI)/(2.0*PI/39.9999))]++;
//      }
//  }

//  const double size = (x2-x1)*(y2-y1);
//  if(extra)
//  {
//      pixelSum /= ( size*255.0 );
//      pixelSq /= ( size*255.0*255.0 );
//      pixelSq -= pixelSum*pixelSum; if(pixelSq<1E-6) pixelSq=0; else pixelSq = sqrt(pixelSq);
//      pointer[0] = pixelSum;
//      if(pixelSum<1E-6)
//      {
//          if(extra>=2) pointer[1]=0;
//      }
//      else 
//      {
//          if(extra>=2) pointer[1]=pixelSq/pixelSum;
//      }
//  }
//  // normalize histogram to have 0 mean, and normalize to have unit norm
//  double sum = 0; 
//  for(int j=0;j<40;j++) sum += temp[j]; sum/=40;
//  for(int j=0;j<40;j++) temp[j] -= sum;
//  double sq = 0; 
//  for(int j=0;j<40;j++) sq += temp[j]*temp[j]; 
//  sq = (sq>100)?1.0/sqrt(sq):0; 
//  for(int j=0;j<40;j++) temp[j] *= sq;
// }

// void GenerateFeatureForOneRect(double** p,const int x1,const int x2,const int y1,const int y2,double* pointer,Array2d<double>& lf,std::ofstream* /*pcaout*/)
// {   // generate PCA of histogram
//  double temp[40];

//  GenerateHistForOneRect(p,x1,x2,y1,y2,pointer,temp);

//  for(int j=0;j<lf.nrow;j++) pointer[j+extra] = temp[j]; 
// }

void GenerateFeatureForOneSplit(IntImage<double>& im,const int splitX,const int splitY,double* pointer,Array2d<double>& lf,std::ofstream* pcaout)
{   // generate features for a split level, which divide the image into splitX*splitY blocks
    for(int splitx=0;splitx<splitX;splitx++) 
    {
        for(int splity=0;splity<splitY;splity++) 
        {
            int x1=1+im.nrow/splitX*splitx, x2=im.nrow/splitX*(splitx+1)-1;
            int y1=1+im.ncol/splitY*splity, y2=im.ncol/splitY*(splity+1)-1;
            if(useSobel)
            {
                if(x1==1) x1++;
                if(x2==im.nrow-1) x2--; 
                if(y1==1) y1++;
                if(y2==im.ncol-1) y2--;
            }
            GenerateFeatureForOneRect(im.p,x1,x2,y1,y2,&pointer[(splitx*splitY+splity)*(lf.nrow+extra)],lf,pcaout);
        }
    }
    // we also generate features with shifted blocks (splitX-1)*(splitY-1)
    for(int splitx=0;splitx<splitX-1;splitx++) 
    { // all rectangles will not be at borders, so useSobel=true/false does not affect this part
        for(int splity=0;splity<splitY-1;splity++) 
        {
            int x1=1+im.nrow/splitX*splitx+im.nrow/(2*splitX), x2=im.nrow/splitX*(splitx+1)-1+im.nrow/(2*splitX);
            int y1=1+im.ncol/splitY*splity+im.ncol/(2*splitY), y2=im.ncol/splitY*(splity+1)-1+im.ncol/(2*splitY);
            GenerateFeatureForOneRect(im.p,x1,x2,y1,y2,&pointer[(lf.nrow+extra)*(splitX*splitY+(splitx*(splitY-1)+splity))],lf,pcaout);
        }
    }
}

void GenerateFeatureForSingleChannelImage(IntImage<double>& im,double* pointer,std::ofstream* pcaout,Array2d<double>& lf)
{   // single channel image (grey, or R, G, or B channel);
    IntImage<double> resized;
    if(splitlevel==0)
    {
        if(useSobel)
        {
            im.Sobel(resized,true,true);
            GenerateHistForOneRect(resized.p,2,resized.nrow-2,2,resized.ncol-2,pointer,pointer+extra);
        }
        else
            GenerateHistForOneRect(im.p,1,im.nrow-1,1,im.ncol-1,pointer,pointer+extra);
        return;
    }

    int baseoffset = 0;
    assert(splitlevel>=1 && splitlevel<=3);

    if(splitlevel==3)
    {
        if(useSobel)
        {
            im.Sobel(resized,true,true);
            GenerateFeatureForOneSplit(resized,8,8,pointer,lf,pcaout);
        }
        else
            GenerateFeatureForOneSplit(im,8,8,pointer,lf,pcaout);
        baseoffset = (lf.nrow+extra)*(8*8+7*7);
    }
    if(splitlevel>=2)
    {
        if(useSobel)
        {
            im.Sobel(resized,true,true);
            GenerateFeatureForOneSplit(resized,4,4,pointer+baseoffset,lf,pcaout);
        }
        else
            GenerateFeatureForOneSplit(im,4,4,pointer+baseoffset,lf,pcaout);
        im.Resize(resized,0.5); im = resized;
        baseoffset += (lf.nrow+extra)*(4*4+3*3);
    }
    if(splitlevel>=1)
    {
        if(useSobel)
        {
            im.Sobel(resized,true,true);
            GenerateFeatureForOneSplit(resized,2,2,pointer+baseoffset,lf,pcaout);
        }
        else    
            GenerateFeatureForOneSplit(im,2,2,pointer+baseoffset,lf,pcaout);
        im.Resize(resized,0.5); im = resized;
        baseoffset += (lf.nrow+extra)*(2*2+1*1);
    }
    if(useSobel)
    {
        im.Sobel(resized,true,true);
        GenerateFeatureForOneSplit(resized,1,1,pointer+baseoffset,lf,pcaout);
    }
    else
        GenerateFeatureForOneSplit(im,1,1,pointer+baseoffset,lf,pcaout);
}

void GenerateFeatureForSingleChannelImage(IntImage<double>& im,double* pointer,std::ofstream* pcaout)
{
    IntImage<double> im2;
    im2 = im;
    GenerateFeatureForSingleChannelImage(im2,pointer,pcaout,lf);
    if(useBoth)
    {
        bool oldSobel = useSobel;
        im2 = im;
        useSobel = true;
        GenerateFeatureForSingleChannelImage(im2,pointer+splitsize[splitlevel]*(lf.nrow+extra),pcaout,lf2);
        useSobel = oldSobel;
    }
}

