#ifndef PARAMETERS_DEFINED_H
#define PARAMETERS_DEFINED_H

#include <map>
#include <set>
#include <cmath>
#include "Reader.h"
#include "ReadExpression.h"


typedef std::vector<std::vector<std::string> > Parameters_Entry;

class Parameters{
public:
  std::map<std::string, Parameters_Entry> map;
  std::set<std::string> requested;
  bool register_requested;

  typedef std::map<std::string, Parameters_Entry>::iterator Iterator;
  typedef std::map<std::string, Parameters_Entry>::const_iterator cIterator;
  
  Parameters_Entry dummy; //<-Empty entry for function "get"

  //append string to Parameters_Entry of key; creates key if it doesn't exist
  void add_to(const std::string & key, const std::vector<std::string> &arg){
    Iterator it=map.find(key);
    if(it==map.end()){
      map.insert(std::make_pair(key,Parameters_Entry(1,arg)));
    }else{
      it->second.push_back(arg);
    }
  }  
  void add_to(const std::string & key, const std::string &arg){
    add_to(key, std::vector<std::string>(1, arg));
  }  
  void add_to(const std::string & key, const double &d){
    std::stringstream ss; ss<<d; 
    add_to(key, std::vector<std::string>(1, ss.str()));
  }
  void erase(const std::string & key){
    map.erase(key);
  }  
  void override_param(const std::string & key, const std::vector<std::string> &arg){
    erase(key);
    add_to(key, arg);
  }
  void override_param(const std::string & key, const std::string &arg){
    override_param(key, std::vector<std::string>(1, arg));
  }
 
  void set_requested(const std::string &key){
    if(requested.find(key)==requested.end()){
      requested.insert(key);
    }
  }
  const Parameters_Entry &get(const std::string &key){
    Iterator it=map.find(key);
    if(it==map.end()){
      return dummy;
    }else{
      set_requested(key);
      return it->second;
    }
  }
  bool is_specified(const std::string &key)const{
    return map.find(key)!=map.end();
  }
  void complain_if_not_specified(const std::string &key)const{
    if(!is_specified(key)){
      std::cerr<<"Please specify parameter '"<<key<<"'!"<<std::endl;
    }
  }
  int get_nr_rows(const std::string &key)const{
    cIterator it=map.find(key);
    if(it==map.end()){
      return 0;
    }else{
      return it->second.size();
    }
  }
  std::vector<std::string> get_row(const std::string &key, int row){
    Parameters_Entry pe=get(key);
    if(row>=pe.size()){
      std::cerr<<"Parameters: Error: get_row: row>=get(\""<<key<<"\").size()!"<<std::endl;
      exit(1);
    }
    return pe[row];
  }
  int get_nr_cols(const std::string &key, int row)const{
    cIterator it=map.find(key);
    int nr_rows;
    if(it==map.end()){
      return 0;
    }else{
      nr_rows=it->second.size();
    }
    if(row>=nr_rows){
      std::cerr<<"Parameters: Error: get_nr_cols: row>=nr_rows!"<<std::endl;
      exit(1);
    }
    return it->second[row].size();
  }
 
  std::vector<std::string> get_all_strings(const std::string &key){
    std::vector<std::string> sv;
    Parameters_Entry got=get(key);
    for(size_t i=0; i<got.size(); i++){
      for(size_t j=0; j<got[i].size(); j++){
        sv.push_back(got[i][j]);
      }
    }
    return sv;
  }
  std::string get_as_single_string(const std::string &key, int row=0){
    Parameters_Entry pe=get(key);
    if(row>=pe.size()){
      std::cerr<<"Parameters: Error: get_as_single_string: row>=get(\""<<key<<"\").size()!"<<std::endl;
      exit(1);
    }
    std::stringstream ss;
    for(size_t i=0; i<pe[row].size(); i++){
      if(i>0)ss<<" ";
      ss<<pe[row][i];
    }
    return ss.str();
  }

  std::vector<double> get_row_doubles(const std::string &key, int row=0, int min=0){
    std::vector<std::string> row_s=get_row(key, row);
    std::vector<double> ret;
    double d;
    for(size_t i=0; i<row_s.size(); i++){
     if(!Reader::canReadDouble(row_s[i], d))break;
     ret.push_back(d);
    }
    if((int)ret.size()<min){
      std::cerr<<"Reading parameter '"<<key<<"': "<<ret.size()<<" doubles found where "<<min<<" are required!"<<std::endl;
      exit(1);
    }
    return ret;
  } 

  std::string get_as_string(const std::string &key,const std::string &def="",
                            int row=0, int col=0){
    set_requested(key);
    Iterator it=map.find(key);
    if(it==map.end()){
      return def;
    }else{
      if(row>=it->second.size()){
        std::cerr<<"Parameters::get: '"<<key<<"' row>=it->second.size()!"<<std::endl;
        exit(1);
      }
      if(col>=it->second[row].size()){
        std::cerr<<"Parameters::get: '"<<key<<"' col>=it->second[row].size()!"<<std::endl;
        exit(1);
      }
      return it->second[row][col];
    }
  }
  Eigen::MatrixXcd get_as_operator(const std::string &key, Eigen::MatrixXcd def=Eigen::MatrixXcd::Zero(1,1), int row=0, int col=0){
    std::string str=get_as_string(key, "", row, col);
#ifdef DEBUG_EXPRESSIONS
    std::cout<<"get_as_operator: key: '"<<key<<"' string: '"<<str<<"'"<<std::endl;
#endif
    if(str=="")return def;
    return ReadExpression(str); //Reader::readDouble(str, key);
  }



  double get_as_double(const std::string &key, double def=0., int row=0, int col=0){
    std::string str=get_as_string(key, "", row, col);
    if(str=="")return def;
    return Reader::readDouble(str, key);
  }
  int get_as_int(const std::string &key, int def=0., int row=0, int col=0){
    return round(get_as_double(key, def, row, col));
  }
  size_t get_as_size_t(const std::string &key, int def=0., int row=0, int col=0){
    int i=get_as_int(key, def, row, col);
    if(i<0){ std::cerr<<"Parameter '"<<key<<"' must not be smaller than zero!"<<std::endl; exit(1); }
    return (size_t)i;
  }
  bool get_as_bool(const std::string &key, bool def=false, int row=0, int col=0){
    std::string str=get_as_string(key, "", row, col);
    if(str=="")return def;
    if(str=="true"||str=="TRUE"||str=="True")return true;
    if(str=="false"||str=="FALSE"||str=="False")return false;
    std::cerr<<"Parameters::get_as_bool: Cannot interpret '"<<str<<"' as Boolean value!"<<std::endl;
    exit(1);
    return def;
  }

  void add_from_stringvec(const std::string &key, const std::vector<std::string> &toks){
    //take argument in { } as a single parameter, (also save {})
    std::vector<std::string> svec;
    for(int i=0; i<(int)toks.size(); i++){
      if(toks[i].size()<1){
        std::cerr<<"Parameters::add_from_stringvec: toks[i].size()<1!"<<std::endl;
        exit(1);
      }
      if(toks[i][0]!='{'){svec.push_back(toks[i]); continue;}

      int nr_open=1;
      std::stringstream ss;
      for(; i<toks.size(); i++){
        for(int j=0; j<toks[i].length(); j++){
          if(toks[i][j]=='{')nr_open++;
          if(toks[i][j]=='}'){nr_open--;}//if(nr_open<=1) break;} <- let this be handled when accessing the parameter
          ss<<toks[i][j];
        }
        if(nr_open==1)break;
        ss<<' ';
      }
      if(nr_open!=1){
        std::cerr<<"Error: Parameters: can't find closing '}'!"<<std::endl;
        exit(1);
      }
      svec.push_back(ss.str());
    }

    add_to(key, svec);
  }
  void add_from_file(const std::string &fname){
    std::ifstream ifs(fname.c_str());
    std::vector<std::string> toks;
    while(Reader::getRelevantLineTokens(ifs,toks)){
      if(toks.size()<1)continue;
      std::string key=toks[0];
      toks.erase(toks.begin());
      add_from_stringvec(key, toks);
    }
  }
  void setup(int args, char** argv){
    std::string key="";
    std::vector<std::string> svec;
    for(int i=1; i<args; i++){
      //check if argument is a key (starts with "-" and is not a number)
      std::string arg(argv[i]);

      if(arg.length()<2 || arg[0]!='-' || (arg[1]>='0' && arg[1] <= '9') 
         || arg[1]=='.'){
        if(key==""){
//          std::cerr<<"Error processing parameters: first argument not a key, i.e., does not start with '-'!"<<std::endl;
//          exit(1);
          key=std::string("driver");
        }   
        svec.push_back(arg);
      }else{
        if(svec.size()>0){
          add_from_stringvec(key, svec);
          svec.clear();
        }
        key=arg.substr(1);
      }
    }
    if(svec.size()>0)add_from_stringvec(key,svec);
   
    Iterator it=map.find("driver");
    if(it!=map.end()){
      for(size_t i=0; i<it->second.size(); i++){
        for(size_t j=0; j<it->second[i].size(); j++){
          add_from_file(it->second[i][j]);
        }
      }
    } 
  }
  void print(const std::string &fname)const{
    std::ofstream ofs(fname.c_str());
    for(cIterator it=map.begin(); it!=map.end(); ++it){
      if(it->first=="driver")continue;
      if(it->first=="print_param")continue;
//std::cout<<"PRINT: '"<<it->first<<"'"<<std::endl;
      for(size_t row=0; row<it->second.size(); row++){
        ofs<<it->first;
        for(size_t col=0; col<it->second[row].size(); col++){
          ofs<<" "<<it->second[row][col];
        }
        ofs<<std::endl;
      }
    }
  }
  Parameters(int args, char** argv, bool reg_req=false){
    setup(args, argv);
    register_requested=reg_req;
  }
  Parameters(){ 
  }
  
};

#endif
