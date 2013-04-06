#include "Utilities.h"
//#include "Old.h"
#include "DFA.h"
#include "CLikeDFA.h"
#include "Graph.h"
#include "Logic.h"


void getCheapest(string a,string b){
  CLikeDFA dfa;
  
  return getCheapest(a,b,dfa);
}
int main(int argc,char * args[]){
  if(argc!=3){
    cerr 
      << "Syntax is:" << endl
      << args[0] << " fileA fileB" << endl;
    return 1;
  }
  getCheapest(readfile(args[1]),readfile(args[2]));
  return 0;
}