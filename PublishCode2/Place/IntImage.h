#ifndef __INTIMAGE_H__
#define __INTIMAGE_H__

#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#include "mdarray.h"

const REAL PI = (REAL)3.14159265358979323846;
const REAL PI4 = (REAL)(PI/4);

template<class T>
class IntImage:public Array2dC<T>
{
private:
    IntImage(const IntImage<T> &source) { } // prohibit copy constructor

public:
    IntImage():variance(0.0),label(-1) { }
    ~IntImage() { Clear(); }

    inline void Clear(void);
    inline void SetSize(const int h, const int w);
    void Print(std::ostream& of = std::cout) const;
    bool Load(const std::string& filename);
    bool Load2(cv::Mat img3);
    void Save(const std::string& filename) const;
    void Swap(IntImage<T>& image2);

    void Resize(IntImage<T> &result,const REAL ratio) const;
    void Resize(IntImage<T>& result,const int height,const int width) const;

	IntImage<T>& operator=(const IntImage<T>& source);
    void operator-=(const IntImage<T>& img2);
    void AbsoluteValue(void);
    void Thresh(const REAL thresh);

    void Sobel(IntImage<REAL>& result,const bool useSqrt,const bool normalize);
public:
    using Array2dC<T>::nrow;
    using Array2dC<T>::ncol;
    using Array2dC<T>::buf;
    using Array2dC<T>::p;
    REAL variance;
    int label;
};

template<class T>
void IntImage<T>::Clear(void)
{
    Array2dC<T>::Clear();
    variance = 0.0;
    label = -1;
}

template<class T>
bool IntImage<T>::Load(const std::string& filename)
{
    IplImage* img;
    IplImage* img2;

    img = cvLoadImage(filename.c_str());
    if(img==NULL) return false;

    if(img->nChannels>1)
    {
        img2 = cvCreateImage(cvSize(img->width,img->height),IPL_DEPTH_8U,1);
        cvCvtColor(img,img2,CV_BGR2GRAY);
        cvReleaseImage(&img);
        img = img2;
        img2 = NULL;
    }
    SetSize(img->height,img->width);
    for(int i=0,ih=img->height,iw=img->width;i<ih;i++)
    {
        T* pdata = p[i];
        unsigned char* pimg = reinterpret_cast<unsigned char*>(img->imageData+img->widthStep*i);
        for(int j=0;j<iw;j++) pdata[j] = pimg[j];
    }
    cvReleaseImage(&img);

    return true;
}

template<class T>
bool IntImage<T>::Load2(cv::Mat img3)
{
    IplImage img4 = img3; 
    IplImage* img = &img4;
    IplImage* img2;

   // img = cvLoadImage(filename.c_str());
    if(img == NULL) return false;

    if(img->nChannels>1)
    {
        img2 = cvCreateImage(cvSize(img->width,img->height),IPL_DEPTH_8U,1);
        cvCvtColor(img,img2,CV_BGR2GRAY);
        cvReleaseImage(&img);
        img = img2;
        img2 = 0;
    }
    SetSize(img->height,img->width);
    for(int i=0,ih=img->height,iw=img->width;i<ih;i++)
    {
        T* pdata = p[i];
        unsigned char* pimg = reinterpret_cast<unsigned char*>(img->imageData+img->widthStep*i);
        for(int j=0;j<iw;j++) pdata[j] = pimg[j];
    }
    cvReleaseImage(&img);

    return true;
}



template<class T>
void IntImage<T>::Save(const std::string& filename) const
{
    IplImage* img;

    img = cvCreateImage(cvSize(ncol,nrow),IPL_DEPTH_8U,1);
    for(int i=0,ih=img->height,iw=img->width;i<ih;i++)
    {
        T* pdata = p[i];
        unsigned char* pimg = reinterpret_cast<unsigned char*>(img->imageData+img->widthStep*i);
        for(int j=0;j<iw;j++) pimg[j] = (unsigned char)pdata[j];
    }
    cvSaveImage(filename.c_str(),img);
    cvReleaseImage(&img);
}

template<class T>
void IntImage<T>::SetSize(const int h,const int w)
// 'size' is the new size of the image, if necessary, memory is reallocated
// size.cx is the new height and size.cy is the new width
{
    if((h == nrow) && (w == ncol)) return;

    Clear();
    Array2dC<T>::Create(h,w);
}

template<class T>
IntImage<T>& IntImage<T>::operator=(const IntImage<T>& source)
{
        if(&source==this) return *this;
    SetSize(source.nrow,source.ncol);
    memcpy(buf,source.buf,sizeof(T)*nrow*ncol);
    label = source.label;
    variance = source.variance;

    return *this;
}

template<class T>
void IntImage<T>::Print(std::ostream& of) const
{
    Array2dC<T>::Print();
    of<<"\tvariance="<<variance<<",    label="<<label<<std::endl;
}

template<class T>
void IntImage<T>::Resize(IntImage<T> &result,const REAL ratio) const
{
    Resize(result,int(nrow*ratio),int(ncol*ratio));
}

template<class T>
void IntImage<T>::Resize(IntImage<T>& result,const int height,const int width) const
{
    assert(height>0 && width>0);
    result.SetSize(height,width);
    REAL ixratio = nrow*1.0/height, iyratio = ncol*1.0/width;

    REAL* p_y = new REAL[result.ncol]; assert(p_y!=NULL);
    int* p_y0 = new int[result.ncol]; assert(p_y0!=NULL);
    for(int i=0;i<width;i++)
    {
        p_y[i] = i*iyratio;
        p_y0[i] = (int)p_y[i];
        if(p_y0[i]==ncol-1) p_y0[i]--;
        p_y[i] -= p_y0[i];
    }

    for(int i=0;i<height;i++)
    {
        int x0; REAL x;
        x = i*ixratio;
        x0 = (int)x;
        if(x0==nrow-1) x0--;
        x -= x0;
        for(int j=0;j<width;j++)
        {
            int y0=p_y0[j];
            REAL y=p_y[j],fx0,fx1;

            fx0 = REAL(p[x0][y0] + y*(p[x0][y0+1]-p[x0][y0]));
            fx1 = REAL(p[x0+1][y0] + y*(p[x0+1][y0+1]-p[x0+1][y0]));

            result.p[i][j] = T(fx0 + x*(fx1-fx0));
        }
    }

    delete[] p_y; p_y=NULL;
    delete[] p_y0; p_y0=NULL;
}

template<class T>
void IntImage<T>::Swap(IntImage<T>& image2)
{
    Array2dC<T>::Swap(image2);
    std::swap(variance,image2.variance);
    std::swap(label,image2.label);
}

template<class T>
void IntImage<T>::operator-=(const IntImage<T>& img2)
{
    assert(nrow==img2.nrow && ncol==img2.ncol);
    for(int i=0,size=nrow*ncol;i<size;i++) p[i] -= img2.p[i];
}

template<class T>
void IntImage<T>::AbsoluteValue(void)
{
    for(int i=0,size=nrow*ncol;i<size;i++)  if(buf[i]<0) buf[i] = -buf[i];
}

template<class T>
void IntImage<T>::Thresh(const REAL thresh)
{
    for(int i=0,size=nrow*ncol;i<size;i++) buf[i]=(buf[i]>thresh)?REAL(1.0):0;
}

template<class T>
void IntImage<T>::Sobel(IntImage<REAL>& result,const bool useSqrt,const bool normalize)
{// compute the Sobel gradient. For now, we just use the very inefficient way. Optimization can be done later
// if useSqrt = true, we compute the real Sobel gradient; otherwise, the square of it
// if normalize = true, the numbers are normalized to be in 0..255
    result.Create(nrow,ncol);
    for(int i=0;i<nrow;i++) result.p[i][0] = result.p[i][ncol-1] = 0;
    for(int i=0;i<ncol;i++) result.p[0][i] = result.p[nrow-1][i] = 0;
    for(int i=1;i<nrow-1;i++)
    {
        for(int j=1;j<ncol-1;j++)
        {
            REAL gx =     p[i-1][j-1] - p[i-1][j+1]
                     + 2*(p[i][j-1]   - p[i][j+1])
                     +    p[i+1][j-1] - p[i+1][j+1];
            REAL gy =     p[i-1][j-1] - p[i+1][j-1]
                     + 2*(p[i-1][j]   - p[i+1][j])
                     +    p[i-1][j+1] - p[i+1][j+1];
           if(useSqrt || normalize ) // if we want to normalize the result image, we'd better use the true Sobel gradient
                result.p[i][j] = sqrt(gx*gx+gy*gy);
           else
                result.p[i][j] = gx*gx+gy*gy;
        }
    }
    if(normalize)
    {
        REAL minv = 1e20, maxv = -minv;
        for(int i=1;i<nrow-1;i++)
        {
            for(int j=1;j<ncol-1;j++)
            {
                if(result.p[i][j]<minv)
                    minv = result.p[i][j];
                else if(result.p[i][j]>maxv)
                    maxv = result.p[i][j];
            }
        }
        for(int i=0;i<nrow;i++) result.p[i][0] = result.p[i][ncol-1] = minv;
        for(int i=0;i<ncol;i++) result.p[0][i] = result.p[nrow-1][i] = minv;
        REAL s = 255.0/(maxv-minv);
        for(int i=0;i<nrow*ncol;i++) result.buf[i] = (result.buf[i]-minv)*s;
    }
}

#endif //__INTIMAGE_H__


