#ifndef OUTER_PRODUCT_DEFINED_H
#define OUTER_PRODUCT_DEFINED_H

#include <Eigen/Core>
#include <vector>
#include "CheckMatrix.h"
#include "ProductSpaceIndex.h"

int loop_pow(int x, int n){
  int r=1;
  for(int i=0; i<n; i++)r*=x;
  return r;
}


Eigen::MatrixXcd otimes(const Eigen::MatrixXcd &o1, 
                              const Eigen::MatrixXcd &o2){

  Eigen::MatrixXcd A(o1.rows()*o2.rows(), o1.cols()*o2.cols());
  for(int r1=0; r1<o1.rows(); r1++){
    for(int c1=0; c1<o1.cols(); c1++){
      for(int r2=0; r2<o2.rows(); r2++){
        for(int c2=0; c2<o2.cols(); c2++){
          A(r1*o2.rows()+r2, c1*o2.cols()+c2) = o1(r1, c1) * o2(r2, c2);
        }
      }
    }
  }
  return A;
}
Eigen::MatrixXcd otimes(const Eigen::MatrixXcd &o1, 
                              const Eigen::MatrixXcd &o2,
                              const Eigen::MatrixXcd &o3){

  return otimes(otimes(o1,o2),o3);
}

Eigen::MatrixXcd ExpandSingleOp(int dim_before, const Eigen::MatrixXcd &m, int dim_after){
  if(dim_before<=1 && dim_after<=1)return m;
  if(dim_before<=1)return otimes(m, Eigen::MatrixXcd::Identity(dim_after,dim_after));
  if(dim_after<=1)return otimes(Eigen::MatrixXcd::Identity(dim_before,dim_before),m);
  return otimes(Eigen::MatrixXcd::Identity(dim_before,dim_before), m,
                      Eigen::MatrixXcd::Identity(dim_after,dim_after));
}


Eigen::VectorXcd Vector_otimes(const Eigen::VectorXcd &o1, 
                               const Eigen::VectorXcd &o2){

  Eigen::VectorXcd A(o1.size()*o2.size());
  for(int r1=0; r1<o1.size(); r1++){
    for(int r2=0; r2<o2.size(); r2++){
      A(r1*o2.size()+r2) = o1(r1) * o2(r2);
    }
  }
  return A;
}

Eigen::VectorXcd H_Matrix_to_L_Vector(const Eigen::MatrixXcd &M){
  Eigen::VectorXcd v(M.rows()*M.cols());
  for(int i=0; i<M.rows(); i++){
    for(int j=0; j<M.cols(); j++){
      v(i*M.cols()+j)=M(i,j);
    } 
  }
  return v;
}

/* Outer product for operators in Liouville space: 
(alpha1, alpha2) otimes (beta1, beta2) -> (gamma1, gamma2)
((nu1,mu1),(nu2,mu2)) otimes ((xi1,chi1),(xi2,chi2)) 
-> ((nu1 *dim(xi1) + xi1), mu1*dim(chi1) + chi1) , ((nu2* .. 
*/
Eigen::MatrixXcd L_Op_otimes(const Eigen::MatrixXcd & A, 
                                   const Eigen::MatrixXcd & B){

  int DL1=get_dim_check_square_min(A, 1, "L_Op_otimes: A");
  int DL2=get_dim_check_square_min(B, 1, "L_Op_otimes: B");
  
  if(A.rows()==1)return A(0,0)*B;
  if(B.rows()==1)return B(0,0)*A;

  int D1=get_dim_sqrt(DL1, "L_Op_otimes: DL1");
  int D2=get_dim_sqrt(DL2, "L_Op_otimes: DL2");
  
  Eigen::MatrixXcd C=Eigen::MatrixXcd::Zero(DL1*DL2, DL1*DL2);

  for(int nu1=0; nu1<D1; nu1++){
    for(int mu1=0; mu1<D1; mu1++){
      for(int xi1=0; xi1<D2; xi1++){
        for(int chi1=0; chi1<D2; chi1++){
          for(int nu2=0; nu2<D1; nu2++){
            for(int mu2=0; mu2<D1; mu2++){
              for(int xi2=0; xi2<D2; xi2++){
                for(int chi2=0; chi2<D2; chi2++){
  C(((nu1*D2+xi1)*D1+mu1)*D2+chi1,((nu2*D2+xi2)*D1+mu2)*D2+chi2) += 
       A(nu1*D1+mu1, nu2*D1+mu2) * B(xi1*D2+chi1, xi2*D2+chi2); 
                }
              }
            }
          }
        }
      }
    }
  }
  return C;
}



/* For propagator:  Hilbert_total = Hilbert_S otimes Hilbert_E
                    Liouville_total = Hilbert_total otimes Hilbert_total
   disentangle as:  Liouville_total = Liouville_S otimes Liouville_E
*/ 
Eigen::MatrixXcd Disentangle_Propagator(const Eigen::MatrixXcd &M, int Ns){
  check_matrix_square(M, "Disentangle_Propagator: M");
  int Ntot=get_dim_sqrt(M.rows(), "Disentangle_Propagator: M.rows()");
  int Nm=Ntot/Ns;

  Eigen::MatrixXcd M2=Eigen::MatrixXcd::Zero(M.rows(), M.cols());
  for(int nu1=0; nu1<Ns; nu1++){
    for(int mu1=0; mu1<Ns; mu1++){
      for(int nu2=0; nu2<Ns; nu2++){
        for(int mu2=0; mu2<Ns; mu2++){
          for(int xi1=0; xi1<Nm; xi1++){
            for(int xi_1=0; xi_1<Nm; xi_1++){
              for(int xi2=0; xi2<Nm; xi2++){
                for(int xi_2=0; xi_2<Nm; xi_2++){
  M2(((nu1*Ns+mu1)*Nm+xi1)*Nm+xi_1,((nu2*Ns+mu2)*Nm+xi2)*Nm+xi_2)=
  M((nu1*Nm+xi1)*Nm*Ns+(mu1*Nm+xi_1),(nu2*Nm+xi2)*Nm*Ns+(mu2*Nm+xi_2));
                }
              }
            }
          }
        }
      }
    }
  }
  return M2;
}

//Expand Hilbert space of matrix: 
//HS otimes HE -> HS otimes 1_by_dim otimes HE
//with d1=dim(HS)
Eigen::MatrixXcd ExpandMatrix(const Eigen::MatrixXcd &A, int d1, int by_dim){

  int d=get_dim_check_square_min(A, 1, "ExpandMatrix: A");
  check_matrix_dims_min(A, d1, "ExpandMatrix: A vs. d1");

  if(d1<=1){
    return otimes(Eigen::MatrixXcd::Identity(by_dim,by_dim), A);
  }
  if(d1==d){
    return otimes(A,Eigen::MatrixXcd::Identity(by_dim,by_dim));
  }
   
  int d2=d/d1;
  if(d1*d2!=d){
    std::stringstream ss; 
    ss<<"ExpandMatrix: d1*d2!=d ("<<d1<<", "<<d2<<" <-> "<<d<<")!"<<std::endl;
    throw(std::runtime_error(ss.str()));
  }

  ProductSpace p1(d1,d2);
  ProductSpace p2(d1,by_dim,d2);
  Eigen::MatrixXcd B(d*by_dim, d*by_dim);
  for(ProductSpaceIndex i(p2); !i.done(); ++i){
    for(ProductSpaceIndex j(p2); !j.done(); ++j){
      B(i.get_I(),j.get_I()) = (i[1]!=j[1]) ? 0. 
                              : A(i[0]*d2+i[2],j[0]*d2+j[2]); 
    }
  }
  return B;
}

// Same for operators in Liouville space. 
// d1 and by_dim are still Hilbert space dimensions
Eigen::MatrixXcd ExpandMatrixLiouville(const Eigen::MatrixXcd &A, int d1, int by_dim){

  int dl=get_dim_check_square_min(A, 1, "ExpandMatrixLiouville: A");
  int d=get_dim_sqrt(dl, "ExpandMatrixLiouville: dl");
  check_bounds(d1, d+1, "ExpandMatrixLiouville: A vs. d1");

  if(d1<=1){
    return otimes(Eigen::MatrixXcd::Identity(by_dim*by_dim,by_dim*by_dim), A);
  }
  if(d1==d){
    return otimes(A,Eigen::MatrixXcd::Identity(by_dim*by_dim,by_dim*by_dim));
  }
   
  int d2=d/d1;
  if(d1*d2!=d){
    std::stringstream ss; 
    ss<<"ExpandMatrix: d1*d2!=d ("<<d1<<", "<<d2<<" <-> "<<d<<")!"<<std::endl;
    throw(std::runtime_error(ss.str()));
  }

  ProductSpace p1(d1,d2);
  ProductSpace p2(d1,by_dim,d2);
  ProductSpace p3=p2.square();
  Eigen::MatrixXcd B(d*by_dim*d*by_dim, d*by_dim*d*by_dim);
  for(ProductSpaceIndex i(p3); !i.done(); ++i){
    for(ProductSpaceIndex j(p3); !j.done(); ++j){
      if( i[1]==j[1] && i[4]==j[4] ){
        ProductSpaceIndex i_red(i); 
        i_red.remove_space(4); i_red.remove_space(1);
        ProductSpaceIndex j_red(j); 
        j_red.remove_space(4); j_red.remove_space(1);
       
        B(i.get_I(),j.get_I()) = A(i_red.get_I(), j_red.get_I());
      }else{
        B(i.get_I(),j.get_I()) = 0.;
      }
    }
  }
  return B;
}



bool print_diff_from_ortho(const Eigen::MatrixXcd &A, double threshold=1e-12, const std::string &str=""){

  bool complained=false;
  for(int i=0; i<A.cols(); i++){
    for(int j=i; j<A.cols(); j++){
      if(i==j && abs(A.col(i).dot(A.col(j))-1.)>threshold){
        std::cout<<str<<"diff_from_ortho: "<<i<<","<<j<<": 1+"<<A.col(i).dot(A.col(j))-1.<<std::endl;
        complained=true;
      }
      if(i!=j && abs(A.col(i).dot(A.col(j))-0.)>threshold){
        std::cout<<str<<"diff_from_ortho: "<<i<<","<<j<<": "<<A.col(i).dot(A.col(j))<<std::endl;
        complained=true;
      }
    }
  }
  if(!complained){
    std::cout<<str<<"diff_from_ortho: orthogonal within "<<threshold<<std::endl;
  }
  return complained;
}
double max_diff_from_ortho(const Eigen::MatrixXcd &A){
  double max_diff=0.;
  for(int i=0; i<A.cols(); i++){
    for(int j=0; j<A.cols(); j++){
      double f=abs(A.col(i).dot(A.col(j)));
      if(i==j)f=abs(A.col(i).dot(A.col(j))-1.);
      if(f>max_diff)max_diff=f;
    }
  }
  return max_diff;
}

#endif
