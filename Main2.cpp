#include "Utilities.h"
#include "DFA.h"
#include "CLikeDFA.h"
#include "Graph.h"
#include "Logic2.h"


void getCheapest(string a,string b){
  CLikeDFA dfa;
  FileWriter fw("output.html", true);
  return getCheapest(a,b,dfa, fw);
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
