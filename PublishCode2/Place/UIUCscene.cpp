#include "global.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
void GenerateFeatureSet(const char* dir,const int maxsize,const int label,int& start,Array2d<double>& features,Array2dC<int>& labels,const bool generatePCA,std::ofstream* pcaout=NULL)
{// for the UIUC scene classification set // Read images from one class (i.e. one directory)
    std::ostringstream buf;
    IntImage<double> im,temp;
    for(int i=0;i<maxsize;i++)
    {
        buf.str(""); buf<<dir; buf<<"image_"; buf.fill('0'); buf.width(4); buf<<i+1; buf<<".jpg";
        std::cout<<buf.str()<<std::endl;
        im.Load(buf.str());
        labels.buf[start]=label;
        for(int j=0;j<features.ncol;j++) features.p[start][j]=0;

        GenerateFeatureForSingleChannelImage(im,&features.p[start][0],(generatePCA && (i<100))?pcaout:NULL);
        // For UIUC scene classification data, use first 100 images of every class to generate PCA

        start++;
    }
}

void Load_UIUC_Scenes()
{
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
    sizeCV = 3;
    extra = 2;
    splitlevel = 2;
    splitratio[0] = 1.0; splitratio[1] = 1.0; splitratio[2] = 1.35;

    const int nClass = 18;
    const int imagesizes[nClass] = {216,289,241,311,210,360,328,260,308,374,410,292,356,215,315,196,274,154};
    const char* imagenames[nClass] = {"../Data/scene/bedroom/","../Data/scene/livingroom/","../Data/scene/CALsuburb/",
                           "../Data/scene/industrial/","../Data/scene/kitchen/","../Data/scene/MITcoast/",
                           "../Data/scene/MITforest/","../Data/scene/MIThighway/","../Data/scene/MITinsidecity/",
                           "../Data/scene/MITmountain/","../Data/scene/MITopencountry/","../Data/scene/MITstreet/",
                           "../Data/scene/MITtallbuilding/","../Data/scene/PARoffice/","../Data/scene/store/","../Data/scene/bathroom/","../Data/scene/dining_room/","../Data/scene/stairscase/"}; 
    const int classLabels[nClass] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,15,16,17 };
    const int labelCorrespondence[nClass] = {1,1,0,2,1,0,0,0,0,0,0,0,0,1,1,1,1,1}; // 1-indoor, 0-outdoor, 2-ignore
    const int totalNum = 5109;

    Array2d<double> features;Array2dC<int> labels;
    features.Create(totalNum,splitlevel?(lf.nrow+extra)*splitsize[splitlevel]*(1+useBoth):(256+extra));
    labels.Create(1,features.nrow);

    std::ofstream pcaout;
    if(generatePCA) pcaout.open("pcatrain.txt"); // use first 100 images in each class to generate PCA
    int start=0;
    for(int i=0;i<nClass;i++) 
        GenerateFeatureSet(imagenames[i],imagesizes[i],classLabels[i],start,features,labels,generatePCA,&pcaout);
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
        for(int i=0;i<nClass;i++)
        {
            int end = start;
            while(end<size && labels.buf[end]==labels.buf[start]) end++;

            Array2dC<int> perm(1,end-start);
            GenerateRandomPermutation(perm.buf,end-start);
            for(int j=0;j<100;j++) train.buf[start+perm.buf[j]] = true;
            start = end;
        }
        FindMinMaxValue(features,train,minmax);

        std::ostringstream buf;
	if(INDOOR_OUTDOOR){
        buf.str(""); buf<<"train_OutIn"; buf<<round+1; buf<<".txt";}
	else{
	buf.str(""); buf<<"train_Normal"; buf<<round+1; buf<<".txt";
	}
        std::ofstream out(buf.str().c_str());
        for(int i=0;i<features.nrow;i++)
        {
            if(train.buf[i]==false) continue;
            if(INDOOR_OUTDOOR==false)
                out<<labels.buf[i]<<" ";
            else
            {
                if(labels.buf[i]==3) continue; // remove class INDUSTRIAL since it contains both indoor and outdoor images
                out<<labelCorrespondence[labels.buf[i]]<<" ";
            }

            for(int j=0;j<features.ncol;j++) 
                out<<j+1<<":"<<((features.p[i][j]-minmax.p[0][j])*minmax.p[1][j]-1.0)*scaleratio.buf[j]<<" ";
            out<<std::endl;
        }
        out.close();
	if(!INDOOR_OUTDOOR){
	buf.str(""); buf<<"test_Normal"; buf<<round+1; buf<<".txt";	
	}
	else{
        buf.str(""); buf<<"test_InOut"; buf<<round+1; buf<<".txt";}
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
        for(int i=0;i<confusion.nrow*confusion.ncol;i++) confusion.buf[i] = 0;
        KNN_FindMin vote(3);
        for(int i=0;i<features.nrow;i++)
        {
            if(train.buf[i]==true) continue;
            vote.Init();
            for(int j=0;j<features.nrow;j++) 
            {
                if(train.buf[j]==false) continue;
                double score = 0;
                for(int k=0;k<features.ncol;k++) score += (features.p[j][k]-features.p[i][k])*(features.p[j][k]-features.p[i][k]);
                vote.Examine(score,labels.buf[j],j);
            }
            int NN = vote.GetBestScoreClass(); // use 1-NN
            confusion.p[labels.buf[i]][NN]++;
        }

        double sum=0,diag=0;
        for(int i=0;i<confusion.nrow*confusion.ncol;i++) sum += confusion.buf[i];
        for(int i=0;i<confusion.nrow;i++) diag += confusion.p[i][i];
        std::cout<<"1-NN accuracy: "<<diag/sum<<std::endl;
    }
}
