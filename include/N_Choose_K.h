#ifndef N_CHOOSE_K_DEFINED_H
#define N_CHOOSE_K_DEFINED_H

#include <cstdlib>
#include <iostream>
#include <vector>

//Calculate n choose k efficiently by precomputing values (lazy evaluation)
class N_Choose_K{
  std::vector<std::vector<int> > precomp;
public:

  void precompute(int n){
    size_t oldsize=precomp.size();
    if(oldsize>=n+1)return;
    precomp.resize(n+1);
    if(oldsize<1){
      precomp[0].resize(1);
      precomp[0][0]=1;
      oldsize++;
    }

    for(size_t i=oldsize; i<n+1; i++){
      precomp[i].resize(i+1);
      precomp[i][0]=1;
      precomp[i][i]=1;
      for(size_t j=1; j<i; j++){
        precomp[i][j]=precomp[i-1][j-1]+precomp[i-1][j];
      }
    }

  }

  int operator()(int n, int k){
    if(k>n){
      std::cerr<<"n choose k with k>n !"<<std::endl;
      exit(1);
    }
    if(n>=precomp.size()){
      precompute(n);
    }
    return precomp[n][k];
  }
  N_Choose_K(int nmax=0){
    if(nmax>0)precompute(nmax);
  }

};



#endif
