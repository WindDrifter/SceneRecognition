#include "global.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

void GenerateFeatureSet(cv::Mat& img, const char* dir,const int maxsize,const int label,int& start,Array2d<double>& features,Array2dC<int>& labels,const bool generatePCA,std::ofstream* pcaout=NULL)
{


// for the UIUC scene classification set // Read images from one class (i.e. one directory)
    std::ostringstream buf;
    IntImage<double> im,temp;
    for(int i=0;i<maxsize;i++)
    {
 
        im.Load2(img);
        labels.buf[start]=label;
        for(int j=0;j<features.ncol;j++) features.p[start][j]=0;

        GenerateFeatureForSingleChannelImage(im,&features.p[start][0],(generatePCA && (i<100))?pcaout:NULL);
        // For UIUC scene classification data, use first 100 images of every class to generate PCA

        start++;
    }
}



void sceneDetection2(cv::Mat img){
  bool generatePCA = false; // whether we collect training data for PCA
    bool INDOOR_OUTDOOR = true; // are we testing 15 class problem or indoor-outdoor binary problem?
    
    useSobel = false;
    useBoth = true;
    lf.Load(useSobel?"lf_UIUC_Sobel.txt":"lf_UIUC.txt",40,256); NormalizeLoadFactors(lf);
    if(useBoth)
    {
        lf2.Load("lf_UIUC_Sobel.txt",40,256);
        NormalizeLoadFactors(lf2);
    }
    sizeCV = 1;
    extra = 2;
    splitlevel = 2;
    splitratio[0] = 1.0; splitratio[1] = 1.0; splitratio[2] = 1.35;

    const int nClass = 1;
    const int imagesizes[nClass] = {1};
    const char* imagenames[nClass] = {"../Test"}; 
    const int classLabels[nClass] = { 0};
    const int labelCorrespondence[nClass] = {1}; // 1-indoor, 0-outdoor, 2-ignore
    const int totalNum = 1;

    Array2d<double> features;Array2dC<int> labels;
    features.Create(totalNum,splitlevel?(lf.nrow+extra)*splitsize[splitlevel]*(1+useBoth):(256+extra));
    labels.Create(1,features.nrow);

    std::ofstream pcaout;
    if(generatePCA) pcaout.open("pcatrain.txt"); // use first 100 images in each class to generate PCA
    int start=0;
    for(int i=0;i<nClass;i++) 
        GenerateFeatureSet(img,imagenames[i],imagesizes[i],classLabels[i],start,features,labels,generatePCA,&pcaout);
    assert(start==features.nrow);
    if(generatePCA) 
    {
        pcaout.close();
        return;
    }
    
    const int size=features.nrow;
    Array2dC<bool> train(1,size);
    Array2dC<double> minmax(2,features.ncol);
    Array2dC<double> scaleratio(1,features.ncol); PrepareScaleRatio(scaleratio);

    unsigned seed = 1194575810; // unsigned seed=(unsigned)time(NULL);  // use 1194575810 to reproduce results in paper
    std::cout<<seed<<std::endl;
    my_srand(seed);
    for(int round=0;round<sizeCV;round++)
    {
        for(int i=0;i<size;i++) train.buf[i]=false; // train/test set division

        start=0;


        std::ostringstream buf;
	
	if(!INDOOR_OUTDOOR){
	buf.str(""); buf<<"data"; buf<<round+1; buf<<".txt";	
	}
	else{
        buf.str(""); buf<<"data"; buf<<round+1; buf<<".txt";}
        std::ofstream out2(buf.str().c_str());
        for(int i=0;i<features.nrow;i++)
        {
            if(train.buf[i]==true) continue;
            if(INDOOR_OUTDOOR==false)
                out2<<labels.buf[i]<<" ";
            else
            {
                if(labels.buf[i]==3) continue;
                out2<<labelCorrespondence[labels.buf[i]]<<" ";
            }

            for(int j=0;j<features.ncol;j++) 
                out2<<j+1<<":"<<((features.p[i][j]-minmax.p[0][j])*minmax.p[1][j]-1.0)*scaleratio.buf[j]<<" ";
            out2<<std::endl;
        }
        out2.close();
        
        Array2dC<double> confusion(nClass,nClass);
      
    }
}

void sceneDetection(){
// for no pic input // will auto capture camera
//CvImagePtr toCvCopy(const sensor_msgs::ImageConstPtr& source,
  //                   const std::string& encoding = std::string());

/*
I am not sure which type of camera we will be using, hence leaving this blank

*/
cv::Mat image;
image = cvLoadImage("../Test/375.jpg", CV_LOAD_IMAGE_COLOR);


sceneDetection2(image);

}
