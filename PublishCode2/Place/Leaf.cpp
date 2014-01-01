#include "global.h"

void DrawLine(IntImage<double>& im,const int x,const int y,const int oldx,const int oldy)
{   // connecting consecutive contour points into a closed contour
    assert(x!=oldx || y!=oldy);
    if(abs(x-oldx)>abs(y-oldy))
    {
        int x1,y1,x2,y2;
        if(x>oldx)
        {
            x1=oldx;x2=x;y1=oldy;y2=y;
        }
        else
        {
            x1=x;x2=oldx;y1=y;y2=oldy;
        }
        for(int i=x1;i<=x2;i++) im.p[i][int(y1+(i-x1)*1.0/(x2-x1)*(y2-y1))] = 255;
    }
    else
    {
        int x1,x2,y1,y2;
        if(y>oldy)
        {
            x1=oldx;x2=x;y1=oldy;y2=y;
        }
        else
        {
            x1=x;x2=oldx;y1=y;y2=oldy;
        }
        for(int i=y1;i<=y2;i++) im.p[int(x1+(i-y1)*1.0/(y2-y1)*(x2-x1))][i] = 255;
    }
}

void SwedenLeafContour()
{
    bool generatePCA = false; // whether we collect training data for PCA
    useSobel = false;
    sizeCV = 5;
    extra = 0;
    splitlevel = 2;
    splitratio[0] = 1.0; splitratio[1] = 1.0; splitratio[2] = 1.35;
    lf.Load(useSobel?"lf_leafContour_Sobel.txt":"lf_leafContour.txt",40,256); NormalizeLoadFactors(lf);

    const int lsize = 10; for(int i=0;i<lsize;i++) lf.p[i]=lf.p[i+lf.nrow-lsize]; lf.nrow=lsize; // only use 10 eigenvectors

    Array2d<double> features(1125,splitlevel?(lf.nrow+extra)*splitsize[splitlevel]:(256+extra));
    IntImage<double> im;

    std::ifstream in("../Data/SwedenLeaf/SwedenLeafContour.txt");
    std::ofstream pcaout;
    if(generatePCA) pcaout.open("pcatrain.txt");
    for(int i=0;i<features.nrow;i++)
    {
        int h,w;
        in>>w>>h; assert(h>0 && w>0);
        im.Create(h,w);
        for(int j=0;j<h*w;j++) im.buf[j]=0;
        int n;
        in>>n; assert(n>0);
        int original_x=-1,original_y=-1; //first (x,y) pair;
        int oldx=-1,oldy=-1; //previous (x,y) pair
        for(int j=0;j<n;j++)
        {
            int x,y;
            in>>y>>x; assert(x<=h && y<=w);
            if(j==0) 
            {
                original_x=x; original_y=y;
                oldx=x;oldy=y;
            }
            else
            {
                DrawLine(im,x,y,oldx,oldy);
                oldx=x;oldy=y;
                if(j==n-1) DrawLine(im,x,y,original_x,original_y);
            }
        }
        GenerateFeatureForSingleChannelImage(im,&features.p[i][0],(generatePCA && (i%75<25))?&pcaout:NULL);
        //use first 25 leafs in each class to generate PCA
    }
    in.close();
    if(generatePCA) 
    {
        pcaout.close();
        return;
    }

    Array2dC<int> perm;
    Array2dC<bool> train(1,1125);
    Array2dC<double> minmax(2,features.ncol);
    Array2dC<double> scaleratio(1,features.ncol); PrepareScaleRatio(scaleratio);

    unsigned seed = 1194575810; // unsigned seed=(unsigned)time(NULL);  // use 1194575810 to reproduce results in paper
    std::cout<<seed<<std::endl;
    my_srand(seed);
    for(int r=0;r<sizeCV;r++)
    {
        // split train/test set
        for(int i=0;i<1125;i++) train.buf[i] = 0;
        for(int i=0;i<15;i++)
        {
            perm.Create(1,75);
            GenerateRandomPermutation(perm.buf,75);
            for(int j=0;j<25;j++) train.buf[i*75+perm.buf[j]] = true;
            // for(int j=0;j<n; ...) then there are xxx training examples, and 75 - n test examples
        }
        FindMinMaxValue(features,train,minmax);

        // save cross validation train/test set
        std::ostringstream dirname;
        dirname.str(""); dirname<<"train"; dirname<<r+1; dirname<<".txt";
        std::ofstream out1(dirname.str().c_str());
        for(int i=0;i<1125;i++)
        {
            if(train.buf[i]==false) continue;
            out1<<i/75<<" ";
            for(int j=0;j<features.ncol;j++) out1<<j+1<<":"<<((features.p[i][j]-minmax.p[0][j])*minmax.p[1][j]-1.0)*scaleratio.buf[j]<<" ";
            out1<<std::endl;
        }
        out1.close();

        dirname.str(""); dirname<<"test"; dirname<<r+1; dirname<<".txt";
        std::ofstream out2(dirname.str().c_str());
        for(int i=0;i<1125;i++)
        {
            if(train.buf[i]==true) continue;
            out2<<i/75<<" ";
            for(int j=0;j<features.ncol;j++) out2<<j+1<<":"<<((features.p[i][j]-minmax.p[0][j])*minmax.p[1][j]-1.0)*scaleratio.buf[j]<<" ";
            out2<<std::endl;
        }
        out2.close();

        Array2dC<double> confusion(15,15);
        for(int i=0;i<confusion.nrow*confusion.ncol;i++) confusion.buf[i] = 0;
        KNN_FindMin vote(3);
        for(int i=0;i<1125;i++)
        {
            if(train.buf[i]==true) continue;
            vote.Init();
            for(int j=0;j<1125;j++) 
            {
            if(train.buf[j]==false) continue;
            double score = 0;
            for(int k=0;k<features.ncol;k++) score += (features.p[j][k]-features.p[i][k])*(features.p[j][k]-features.p[i][k]);
            vote.Examine(score,j/75,j);
            }
            int NN = vote.GetBestScoreClass(); // use 1-NN
            confusion.p[i/75][NN]++;
        }

        double sum=0,diag=0;
        for(int i=0;i<confusion.nrow*confusion.ncol;i++) sum += confusion.buf[i];
        for(int i=0;i<confusion.nrow;i++) diag += confusion.p[i][i];
        std::cout<<"1-NN accuracy "<<diag/sum<<std::endl;
    }
}
