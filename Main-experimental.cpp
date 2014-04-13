#include "Utilities.h"
#include "DFA.h"
#include "CLikeDFA.h"
#include "Graph.h"
#include "Logic2.h"
#include "EvaluateMatchings.h"
#include "OptimizeRelaxedMatchings.h"
#include "LcsHeuristics.h"

template <class Atom>
void getBestMatching(string aText, string bText) {
  CLikeDFA dfa;
  TextInfoBeta<Atom> a = analyzeTextBeta<Atom>(aText,dfa);
  TextInfoBeta<Atom> b = analyzeTextBeta<Atom>(bText,dfa);
  
  MatchingEvaluator<Atom> evaluator(a, b);
  evaluator.findBest();
}

template <class Atom>
Graph optimizeRelaxed(string aText, string bText)
{
  CLikeDFA dfa;
  TextInfoBeta<Atom> a = analyzeTextBeta<Atom>(aText,dfa);
  TextInfoBeta<Atom> b = analyzeTextBeta<Atom>(bText,dfa);
  
  Optimizer<Atom> optimizer(a, b);
  optimizer.optimize(2, 10);
  Graph graph = optimizer.retrieveMatching();
  Logic2<Atom> logic;
  FileWriter fw("output.html");
  logic.dealWithMatching(a, b, graph, true, fw);
  return graph;
}

template <class Atom>
void FindCS_LCS(string aText, string bText)
{
  CLikeDFA dfa;
  TextInfoBeta<Atom> a = analyzeTextBeta<Atom>(aText,dfa);
  TextInfoBeta<Atom> b = analyzeTextBeta<Atom>(bText,dfa);
  
  CSLCS<Atom> cslcs(a,b);
  
  typename CSLCS<Atom>::LcsResult result = cslcs.computeLCS();
  
  FileWriter fw("cs-lcs.html", true);
  printLCS(fw, a, b, result.lcs);
}




int main(int argc,char * args[])
{
  if(argc!=3)
  {
    cerr
    << "Syntax is:" << endl
    << args[0] << " fileA fileB" << endl;
    return 1;
  }
  string file1 = readfile(args[1]);
  string file2 = readfile(args[2]);
  
  FindCS_LCS<Token>(file1, file2);
  return 0;
  
  optimizeRelaxed<Token>(file1, file2);
  return 0;
  
  getBestMatching<Token>(file1, file2);
  return 0;
}



