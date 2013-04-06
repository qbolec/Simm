#include "Utilities.h"
#include "Node.h"
#include "State.h"
//#include "Old.h"
#include "DFA.h"
#include "CLikeDFA.h"
#include "Graph.h"
#include "Logic.h"


State getCheapest(string a,string b){
  State s(Node::fromString(a),Node::fromString(b),1);
//  cap = cost(s);
  CLikeDFA dfa;
  
  return getCheapest(s,dfa);
}
int main(int argc,char * args[]){
  if(argc!=3){
    cerr 
      << "Syntax is:" << endl
      << args[0] << " fileA fileB" << endl;
    return 1;
  }
  State s=getCheapest(readfile(args[1]),readfile(args[2]));
  cerr << "Penalty:" << cost(s) << endl;
  cout << s<< endl;
  return 0;
}