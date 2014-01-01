#include "global.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;
void ConvertImage(int argc,char* argv[])
{


    if(argc != 2)
    {
        std::cout<<"usage: CTimage input_image_name"<<std::endl;
        return;
    }
    std::string name = argv[1];
    if(!FileExists(name.c_str()))
    {
        std::cout<<"File not exist."<<std::endl;
        return;
    }
    IntImage<double> im,result;
    if(im.Load(name)==false)
    {
        std::cout<<"Image loading failed."<<std::endl;
        return;
    }
    result.Create(im.nrow,im.ncol);
    for(int i=0;i<result.nrow;i++) result.p[i][0] = result.p[i][result.ncol-1] = 0;
    for(int i=0;i<result.ncol;i++) result.p[0][i] = result.p[result.nrow-1][i] = 0;
    for(int i=1;i<im.nrow-1;i++)
        for(int j=1;j<im.ncol-1;j++)
            result.p[i][j]=255-CensusTransform(im.p,i,j);
    // Note that for visualization, it is different than computing, we use the complement. The visualization is conformant to the CT paper.
    // My first implementation of Census Transform is different from the paper on CT, but I do not bother to change it.
    result.Save(name+".CT.png");
}

int main(int argc,char* argv[])
{
    ConvertImage(argc,argv);
}
