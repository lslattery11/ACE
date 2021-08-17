#include "Simulation.h"
#include "MPG_Discretization.h"

/** Template to generate the two source files for the iQUAPI and TEMPO   
    binaries. 
*/

int main(int args, char** argv){

  Parameters param(args, argv, true);
  if(param.is_specified("print_param")){
    param.print(param.get_as_string("print_param"));
  }
  std::string outfile=param.get_as_string("outfile", "results.dat");


  IF_TimeGrid tgrid(param);
  if(fabs(tgrid.dt0-tgrid.dt)>1e-6*tgrid.dt){
    std::cerr<<"dt0 != dt not yet implemented for iQUAPI/TEMPO!"<<std::endl;
    exit(1);
  }
  int n_max=param.get_as_double("n_max", tgrid.n_mem);
  n_max=param.get_as_double("n_max_override", n_max);

#ifdef TEMPLATE_SET_IQUAPI
  if(n_max<1){
    std::cerr<<"iQUAPI needs parameter 'n_max' to be specified and positive!"<<std::endl; 
    exit(1);
  }
  if(n_max>=15 && !param.is_specified("n_max_override")){
    std::cerr<<"iQUAPI needs parameter 'n_max' not to be too large to fit into memory. Use a value smaller than ~15 or set 'n_max_override' explicitly!"<<std::endl; 
    exit(1);
  }
#endif

  std::string prefix=param.get_as_string("prefix","Boson");
  bool silent=param.get_as_bool("silent",false);
  bool use_bath=param.get_as_bool("use_bath", true);

  ///Bath (setup first because of possible Hilbert space rotation hs_rot).
  DiagBB diagBB(param, prefix);


  Eigen::MatrixXcd initial_rho=InitialState(param);
  if(initial_rho.rows()!=diagBB.sys_dim()){
    std::cerr<<"Mismatch in dimensions between initial state and "<<add_prefix(prefix,"SysOp")<<" for system-bath coupling!"<<std::endl;
    exit(1);
  }
  initial_rho=diagBB.hs_rot.apply(initial_rho);


  ///Bath-free propagator:
  FreePropagator fprop(param);
  if(!fprop.dim_fixed){ 
    fprop.set_dim(initial_rho.rows());
  }else{
    if(fprop.get_dim()!=initial_rho.rows()){
      std::cerr<<"Mismatch in dimensions between initial state and propagator!"<<std::endl;
      exit(1);
    }
  }

  fprop.hs_rot=diagBB.hs_rot;


  std::cout<<"Setting up influence functional"<<std::endl;

#ifdef TEMPLATE_SET_IQUAPI
  InfluenceFunctional IF(n_max, tgrid.dt, diagBB);
#endif
#ifdef TEMPLATE_SET_TEMPO
  InfluenceFunctional_Vector IF(n_max, tgrid.dt, diagBB);
#endif



#ifdef TEMPLATE_SET_IQUAPI
  Simulation sim;
#endif
#ifdef TEMPLATE_SET_TEMPO
  Simulation_TEMPO sim;
#endif
  sim.setup_output(param);
  sim.output_Op.rotate(diagBB.hs_rot);
  sim.compressor=RankCompressor_Selector(param);

  std::cout<<"Starting calculation"<<std::endl;
  if(use_bath==false){
    sim.run_nobath(fprop, tgrid.ta, tgrid.dt, tgrid.get_t_tot(), initial_rho, silent);
  }else{
    sim.run(fprop, IF, tgrid.ta, tgrid.dt, tgrid.get_t_tot(), initial_rho, silent);
  }

  sim.print_results(outfile);


  return 0;
}

