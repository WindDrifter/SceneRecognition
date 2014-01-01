#ifndef __MDARRAY_H__
#define __MDARRAY_H__

#include <iostream>
#include <fstream>
#include <assert.h>

#include "util.h"

#define USE_DOUBLE

#ifdef USE_DOUBLE
    typedef double REAL;
#else
    typedef float REAL;
#endif

template<class T>
class Array2d
{
private:
    void IncreaseCapacity(const int newrow)
    {
        assert(newrow>nrow);

        T** newp = new T*[newrow]; assert(newp!=NULL);
        memcpy(newp,p,sizeof(T*)*nrow);
        for(int i=nrow;i<newrow;i++)
        {
            newp[i] = new T[ncol]; 
            assert(newp[i]!=NULL);
        }
        delete[] p; p = NULL;
        p = newp; newp = NULL;
        nrow = newrow;
    }

    void DecreaseCapacity(const int newrow)
    {
        assert(newrow<nrow);
        
        T** newp = new T*[newrow]; assert(newp!=NULL);
        memcpy(newp,p,sizeof(T*)*newrow);
        for(int i=newrow;i<nrow;i++)
        {
            delete[] p[i];
            p[i] = NULL;
        }
        delete[] p; p = NULL;
        p = newp; newp = NULL;
        nrow = newrow;
    }

public:
    int nrow;
    int ncol;
    T** p;
    
    Array2d():nrow(0),ncol(0),p(NULL) { }

    Array2d(const int nrow,const int ncol):nrow(0),ncol(0),p(NULL) { Create(nrow,ncol); }

    Array2d(const Array2d<T>& source):nrow(0),ncol(0),p(NULL)
    {
        if(source.p!=NULL)
        {
            Create(source.nrow,source.ncol);
            for(int i=0;i<nrow;i++) memcpy(p[i],source.p[i],sizeof(T)*ncol);
        }
    }

    Array2d<T>& operator=(const Array2d<T>& source)
    {
        if(source.p!=NULL)
        {
            Create(source.nrow,source.ncol);
            for(int i=0;i<nrow;i++) memcpy(p[i],source.p[i],sizeof(T)*ncol);
        }
        else
            Clear();
        return *this;
    }

    void Create(const int _nrow,const int _ncol)
    {
        assert(_nrow>0 && _ncol>0);
        if(ncol==_ncol) return AdjustCapacity(_nrow);
        Clear();
        nrow = _nrow; ncol = _ncol;
        p = new T*[nrow]; assert(p!=NULL);
        for(int i=0;i<nrow;i++)
        {
            p[i] = new T[ncol]; assert(p[i]!=NULL);
        }
    }

    void Swap(Array2d<T>& array2)
    {
        std::swap(nrow,array2.nrow);
        std::swap(ncol,array2.ncol);
        std::swap(p,array2.p);
    }

    void Clear()
    {
        for(int i=0;i<nrow;i++) { delete[] p[i]; p[i] = NULL; }
        delete[] p; p = NULL;
        nrow = ncol = 0;
    }

    void AdjustCapacity(const int newrow)
    {
        assert(newrow>0);
        if(newrow == nrow) 
            return;
        else if(newrow>nrow) 
            IncreaseCapacity(newrow);
        else // newrow < nrow
            DecreaseCapacity(newrow);
    }

    bool Load(const char* filename,const int height,const int width)
    {
        if(!FileExists(filename)) return false;
        if(height<=0 || width<=0) return false;
        Create(height,width);
        if(p==NULL) return false;
        std::ifstream in(filename);
        if(!in.good()) return false;
        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j++) in>>p[i][j];
            if(!in.good()) return false;
        }
        in.close();
        return true;
    }

    bool Save(const char* filename)
    {
        std::ofstream out(filename);
        for(int i=0;i<nrow;i++)
        {
            for(int j=0;j<ncol;j++) out<<p[i][j]<<" ";
            out<<std::endl;
            if(!out.good()) return false;
        }
        out.close();
        return true;
    }

    void Print(std::ostream& of = std::cout) const
    {
        of<<std::endl<<"-----------------------------------"<<std::endl;
        for(int i=0;i<nrow;i++)
        {
            for(int j=0;j<ncol;j++) of<<p[i][j]<<" ";
            of<<std::endl;
        }
        of<<"-----------------------------------"<<std::endl;
        of<<"\tHeight="<<nrow<<", Width="<<ncol<<std::endl;
    }

    void RowSum(Array2d<T>& rowsum)
    {
        rowsum.Create(1,nrow);
        for(int i=0;i<nrow;i++)
        {
            T sum = 0;
            for(int j=0;j<ncol;j++) sum += p[i][j];
            rowsum.p[0][i] = sum;
        }
    }

    void RowAverage(Array2d<T>& rowavg) // Notice that when T is an integer type, RowAverage will return an integer value
    {
        RowSum(rowavg);
        REAL t = 1.0/ncol;
        for(int i=0;i<nrow;i++) rowavg.p[0][i] *= t;
    }

    void Multiply(const Array2d<T>& B,Array2d<T>& result)
    {
        assert(ncol==B.nrow);
        result.Create(nrow,B.ncol);
        T** bp = B.p;
        T** rp = result.p;
        for(int i=0;i<nrow;i++)
            for(int j=0;j<B.ncol;j++)
            {
                REAL sum = 0;
                for(int k=0;k<ncol;k++) sum += (p[i][k]*bp[k][j]);
                rp[i][j] = sum;
            }
    }

    ~Array2d() { Clear(); }
};

template<class T>
class Array2dC
{
public:
    int nrow;
    int ncol;
    T** p;
    T* buf;

    Array2dC():nrow(0),ncol(0),p(NULL),buf(NULL)
    {
    }

    Array2dC(const int nrow,const int ncol):nrow(0),ncol(0),p(NULL),buf(NULL) { Create(nrow,ncol); }

    Array2dC(const Array2dC<T>& source):nrow(0),ncol(0),p(NULL),buf(NULL)
    {
        if(source.buf!=NULL)
        {
            Create(source.nrow,source.ncol);
            memcpy(buf,source.buf,sizeof(T)*nrow*ncol);
        }
    }

    Array2dC<T>& operator=(const Array2dC<T>& source)
    {
        if(source.buf!=NULL)
        {
            Create(source.nrow,source.ncol);
            memcpy(buf,source.buf,sizeof(T)*nrow*ncol);
        }
        else
            Clear();
        return *this;
    }

    void Create(const int _nrow,const int _ncol)
    {
        assert(_nrow>0 && _ncol>0);
        if(nrow==_nrow && ncol==_ncol) return;
        Clear();
        nrow = _nrow; ncol = _ncol;
        buf = new T[nrow*ncol]; assert(buf!=NULL);
        p = new T*[nrow]; assert(p!=NULL);
        for(int i=0;i<nrow;i++) p[i] = buf + i * ncol;
    }

    void Swap(Array2dC<T>& array2)
    {
        std::swap(nrow,array2.nrow);
        std::swap(ncol,array2.ncol);
        std::swap(p,array2.p);
        std::swap(buf,array2.buf);
    }

    void Clear()
    {
        delete[] buf; buf = NULL;
        delete[] p; p = NULL;
        nrow = ncol = 0;
    }

    bool Load(const char* filename,const int height,const int width)
    {
        if(!FileExists(filename)) return false;
        if(height<=0 || width<=0) return false;
        Create(height,width);
        if(p==NULL) return false;
        std::ifstream in(filename);
        if(!in.good()) return false;
        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j++) in>>p[i][j];
            if(!in.good()) return false;
        }
        in.close();
        return true;
    }

    bool Save(const char* filename)
    {
        std::ofstream out(filename);
        for(int i=0;i<nrow;i++)
        {
            for(int j=0;j<ncol;j++) out<<p[i][j];
            if(!out.good()) return false;
        }
        out.close();
        return true;
    }

    void Print(std::ostream& of = std::cout) const
    {
        of<<std::endl<<"-----------------------------------"<<std::endl;
        for(int i=0;i<nrow;i++)
        {
            for(int j=0;j<ncol;j++) of<<p[i][j]<<" ";
            of<<std::endl;
        }
        of<<"-----------------------------------"<<std::endl;
        of<<"\tHeight="<<nrow<<", Width="<<ncol<<std::endl;
    }

    void RowSum(Array2dC<T>& rowsum)
    {
        rowsum.Create(1,nrow);
        for(int i=0;i<nrow;i++)
        {
            T sum = 0;
            for(int j=0;j<ncol;j++) sum += p[i][j];
            rowsum.p[0][i] = sum;
        }
    }

    void RowAverage(Array2dC<T>& rowavg) // Notice that when T is an integer type, RowAverage will return an integer value
    {
        RowSum(rowavg);
        REAL t = 1.0/ncol;
        for(int i=0;i<nrow;i++) rowavg.p[0][i] *= t;
    }

    void Multiply(const Array2dC<T>& B,Array2dC<T>& result)
    {
        assert(ncol==B.nrow);
        result.Create(nrow,B.ncol);
        T** bp = B.p;
        T** rp = result.p;
        for(int i=0;i<nrow;i++)
            for(int j=0;j<B.ncol;j++)
            {
                REAL sum = 0;
                for(int k=0;k<ncol;k++) sum += (p[i][k]*bp[k][j]);
                rp[i][j] = sum;
            }
    }

    ~Array2dC() { Clear(); }
};

#endif // __MDARRAY_H__


