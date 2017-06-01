/*
 Mattias P. Heinrich
 Universitaet Luebeck, 2016
 
 published and documented in:
 
 MP Heinrich, M Blendowski.
 "Multi-Organ Segmentation using Vantage Point Forests and Binary Context Features"
 Medical Image Computing and Computer Assisted Intervention (MICCAI) 2016. LNCS Springer (2016)
 (please cite if used in other works)
 
 Please see license.txt and readme.txt for more information.
 */

#include "mex.h"
#include <math.h>
#include <iostream>
#include <sys/time.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <queue>
#include <list>


using namespace std;
#define printf mexPrintf


//INPUT: masked (upsampled) probability array num_labels+1 x num_mask (float), 3D image (float), 3D mask image (uint8)
//(scalars): regularisation weighting lambda, gradient/boundary sigma
//OUTPUT: 3D segmentation after RW (int16), 3D segmentation after largest conn. comp. (int16), probabilistic (masked) output

#include "eigenlibrary/Eigen/Sparse"
#include "eigenlibrary/Eigen/Dense"
using namespace Eigen;

// Successive over-relaxation (SOR) solver for sparse linear system of equations
void sorL(float* x,SparseMatrix<double> A,float* b,int sz,int labels,int iter,double omega){
   
    double* sum=new double[labels];
    
    for(int l=0;l<iter;l++){
        for(int i=0;i<sz;i++){
            double diag=0;
            for(int lab=0;lab<labels;lab++){
                sum[lab]=b[i*labels+lab];
            }
            
            for(SparseMatrix<double>::InnerIterator it(A,i);it;++it){
                if(it.index()==i){
                    diag=it.value();
                }
                else{
                    for(int lab=0;lab<labels;lab++){
                        sum[lab]-=it.value()*x[(it.index())*labels+lab];
                    }
                }
            }
            for(int lab=0;lab<labels;lab++){
                x[i*labels+lab]=(1.0-omega)*x[i*labels+lab]+(omega*sum[lab])/diag;
            }
        }
    }
}

// variational regularisation using the multi-label random walk algorithm and SOR-solver
// only calculated within region of interest (mask)
void regularise(short* labelreg,float* probreg,float* probin,float* im,uint8_t* mask,int m,int n,int o,int num_class,float sigma,float lambda){
    
    int sz=count(mask,mask+m*n*o,true);
    
    int num_nb=6;//len*len;
    
    int dx[]={-1,1,0,0,0,0};
    int dy[]={0,0,-1,1,0,0};
    int dz[]={0,0,0,0,-1,1};
    
    vector<Triplet<double>> edges;
    edges.reserve(sz*num_nb); //nb entries per pixel
    vector<Triplet<double>> nodes;
    nodes.reserve(sz); //1 entry per pixel
    
    //inverse mask-indices
    uint32_t* maskind=new uint32_t[m*n*o];
    for(int i=0;i<m*n*o;i++){
        maskind[i]=0;
    }
    uint32_t count=0;
    for(int z=0;z<o;z++){
        for(int y=0;y<n;y++){
            for(int x=0;x<m;x++){
                int ind=x+y*m+z*m*n;
                if(mask[ind]==1){
                    maskind[ind]=count;
                    count++;
                }
            }
        }
    }
    uint32_t maxind=0;
    //set-up sparse regularisation matrix (using triplets)
    for(int z=0;z<o;z++){
        for(int y=0;y<n;y++){
            for(int x=0;x<m;x++){
                int ind=x+y*m+z*m*n;
                if(mask[ind]==1){
                    double intensity=im[ind];
                    //set individual matrix elements in four neighbourhood (nb)
                    double sumweight=0;
                    for(int nb=0;nb<num_nb;nb++){
                        if(x+dx[nb]>=0&y+dy[nb]>=0&z+dz[nb]>=0&x+dx[nb]<m&y+dy[nb]<n&z+dz[nb]<o){//check boundaries
                            int index_nb=(x+dx[nb])+(y+dy[nb])*m+(z+dz[nb])*m*n;
                            if(mask[index_nb]==1){
                                double diff=intensity-im[index_nb];
                                double weight=exp(-diff*diff/(2.0f*sigma*sigma));
                                edges.push_back(Triplet<double>(maskind[ind],maskind[index_nb],-lambda*weight));
                                sumweight+=lambda*weight;
                            }
                        }
                    }
                    nodes.push_back(Triplet<double>(maskind[ind],maskind[ind],sumweight+1.0f));
                    maxind=max(maskind[ind],maxind);
                    
                }
            }
        }
    }
    //printf("count: %d, maxind: %d\n",count,maxind);
    
    //create the actual sz x sz sparse matrix
    SparseMatrix<double> matL(sz,sz);
    matL.setFromTriplets(edges.begin(),edges.end());
    SparseMatrix<double> matD(sz,sz);
    matD.setFromTriplets(nodes.begin(),nodes.end());
    
    SparseMatrix<double> matLD=matL+matD;
    
    //printf("Starting solver\n");
    
    sorL(probreg,matLD,probin,sz,num_class,30,1.5);
    
    
    for(int z=0;z<o;z++){
        for(int y=0;y<n;y++){
            for(int x=0;x<m;x++){
                int ind=x+y*m+z*m*n;
                if(mask[ind]==1){
                    int i=maskind[ind];
                    labelreg[ind]=max_element(probreg+i*num_class,probreg+(i+1)*num_class)-(probreg+i*num_class);
                }
                else{
                    labelreg[ind]=0;
                }
                    
            }
        }
    }

    
}


SparseMatrix<int> adjacencyMatrix(bool* mask,int m,int n,int o){
    
    //six-neigbourhood
    int num_nb=6;//len*len;
    int dx[]={-1,1,0,0,0,0};
    int dy[]={0,0,-1,1,0,0};
    int dz[]={0,0,0,0,-1,1};
    
    int sz=m*n*o;
    vector<Triplet<int>> edges;
    edges.reserve(sz*num_nb); //maximum 6 entries per pixel
    for(int z=0;z<o;z++){
        for(int y=0;y<n;y++){
            for(int x=0;x<m;x++){
                int ind=x+y*m+z*m*n;
                if(mask[ind]){ //check if in object
                    //add neighbours (nb)
                    for(int nb=0;nb<num_nb;nb++){
                        if(x+dx[nb]>=0&y+dy[nb]>=0&z+dz[nb]>=0&x+dx[nb]<m&y+dy[nb]<n&z+dz[nb]<o){//check boundaries
                            int index_nb=(x+dx[nb])+(y+dy[nb])*m+(z+dz[nb])*m*n;
                            if(mask[index_nb]){
                                edges.push_back(Triplet<int>(ind,index_nb,1));
                            }
                        }
                    }
                }
            }//end of pixel-loop
        }
    }
    //create the actual sz x sz matrix
    SparseMatrix<int> adjMat(sz,sz);
    adjMat.setFromTriplets(edges.begin(),edges.end());
    
    return adjMat;
    
}

int getNumberOfConnectedComponents(short* labelsout,bool* mask,int m,int n,int o,int lab){
    int components=1;
    int sz=m*n*o;
    
    SparseMatrix<int> adjMat=adjacencyMatrix(mask,m,n,o);
    //printf("adjacency elements: %d\n",(int)adjMat.sum());
    
    queue<int> S;
    
    int* labelcomp=new int[sz];
    int num=count(mask,mask+sz,true);
    //printf("%d voxels in label\n",num);
    bool* visited=new bool[sz];
    for(int i=0;i<sz;i++){
        visited[i]=false;
        labelcomp[i]=0;
    }
    
    int firstind=find(mask,mask+sz,true)-mask;
    
    visited[firstind]=true;
    S.push(firstind);
    while(!S.empty())
    {
        int current=S.front();
        S.pop();
        for(SparseMatrix<int>::InnerIterator it(adjMat,current);it;++it){
            if(not(visited[it.index()])){
                S.push(it.index());
                visited[it.index()]=true;
                labelcomp[it.index()]=components;
            }
        }
        
        if(S.empty()){
            components++;
            for(int i=0;i<sz;i++){
                if(mask[i]&&not(visited[i])){
                    S.push(i);
                    visited[i]=true;
                    break;
                }
            }
        }
        
        
    }
    // printf("%d components found\n",components-1);
    
    //find and label maximum component
    int maxcount=-1; int maxcomp=0;
    for(int c=1;c<components;c++){
        int countc=count(labelcomp,labelcomp+sz,c);
        if(countc>maxcount){
            maxcount=countc;
            maxcomp=c;
        }
    }
    //  printf("largest component (c=%d) of label (lab=%d) has size %d\n",maxcomp,lab,maxcount);
    for(int i=0;i<sz;i++){
        if(labelcomp[i]==maxcomp){
            labelsout[i]=lab;
        }
    }
    delete labelcomp;
    delete visited;
    
    return components;
}

void largestComponents(short* labelsout,short* labelsin,int m,int n,int o,int num_class){
    int sz=m*n*o;
    
    bool* mask=new bool[sz];
    
    //int maxlabel=*max_element(labelsin,labelsin+sz);
    for(int i=0;i<sz;i++){
        labelsout[i]=0;
    }
    
    for(int lab=1;lab<num_class;lab++){
        bool exist=false;
        for(int i=0;i<sz;i++){
            mask[i]=labelsin[i]==lab;
            exist|=labelsin[i]==lab;
        }
        if(not(exist)){
            printf("label %d does not exist\n",lab);
        }
        if(exist){
            
            int numComp=getNumberOfConnectedComponents(labelsout,mask,m,n,o,lab);
            
        }
    }
    delete mask;
    
}


/* the gateway function */
void mexFunction( int nlhs, mxArray *plhs[],int nrhs, const mxArray *prhs[])
{
    timeval time1,time2;
    
    
    float* probvol=(float*)mxGetData(prhs[0]);
    float* im1=(float*)mxGetData(prhs[1]);
    uint8_t* mask=(uint8_t*)mxGetData(prhs[2]);

    float lambda=(float)mxGetScalar(prhs[3]);
    float sigma=(float)mxGetScalar(prhs[4]);

    const mwSize* dims0=mxGetDimensions(prhs[0]);
    

    const mwSize* dims1=mxGetDimensions(prhs[1]);
    int m=dims1[0]; int n=dims1[1]; int o=dims1[2];
    
    int sz=count(mask,mask+m*n*o,1);


    int num_class=dims0[0];

    int dimsout[]={num_class,sz};

    plhs[0]=mxCreateNumericArray(3,dims1,mxINT16_CLASS,mxREAL);
    plhs[1]=mxCreateNumericArray(3,dims1,mxINT16_CLASS,mxREAL);
    short* labelreg=(short*)mxGetData(plhs[0]);
    short* labellcc=(short*)mxGetData(plhs[1]);

    plhs[2]=mxCreateNumericArray(2,dimsout,mxSINGLE_CLASS,mxREAL);
    
    float* probreg=(float*)mxGetData(plhs[2]);
    
    
    gettimeofday(&time1, NULL);

    regularise(labelreg,probreg,probvol,im1,mask,m,n,o,num_class,sigma,lambda);

    gettimeofday(&time2, NULL);
    
    float timeP2=time2.tv_sec+time2.tv_usec/1e6-(time1.tv_sec+time1.tv_usec/1e6);
    printf("Computation time: %4.2f secs.\n",timeP2);
    
   // largestComponents(labellcc,labelreg,m,n,o,num_class);
   
    
    
}
