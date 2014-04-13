#ifndef LCSHEURISTICS_H
#define LCSHEURISTICS_H

#include <string>
#include <vector>
#include <cassert>
#include <algorithm>
#include <cctype>
#include <sstream>

using namespace std;

template <class Atom>
struct TextWithDepth
{
  TextInfoBeta<Atom> textInfo;
  vector<int> depth;
};

template <class Atom>
vector<int> computeTextDepth(const TextInfoBeta<Atom>& info) {
  assert(false);
  return vector<int>();
}


template <>
vector<int> computeTextDepth(const TextInfoBeta<Token>& info) {
  using namespace CharacterType;
  
  vector<int> depth;
  
  int openBrackets = 0, indentLevel = 0;
  bool isLineStart = true;
  for (int i=0; i<info.size(); i++) {
    Token token = info.important_atoms[i];
    string atom = info.atomValue(i);
    depth.push_back(max(openBrackets,indentLevel));
    
    if (token.type == OPEN_BRACKET) {
      openBrackets ++;
    }
    else if (token.type == CLOSE_BRACKET) {
      openBrackets --;
    }
    
    if (isLineStart) {
      if (token.type == INDENT) indentLevel ++;
      else isLineStart = false;
    }
    if (atom == "\n") {
      isLineStart = true;
      indentLevel = 0;
    }
  }
  
  return depth;
}
    
template <class Atom>
struct WLCS
{
  struct LcsState
  {
    double maxScore;
    int matchLength;
    int fromA, fromB; // from where we came
    vector< pair<int,int> > lcsBetween;
    LcsState() {
      maxScore = -1.0;
      matchLength = -1;
      fromA = fromB = -1;
    }
  };
  
  struct LcsResult
  {
    vector< pair<int,int> > lcs;
  };
  
  TextInfoBeta<Atom> infoA, infoB;
  vector<int> depthA, depthB;
  
  double weight(const string &substr, int depthA, int depthB) {
    double space = true;
    for (char c: substr) if (!isspace(c)) space = false;
    if (space) return -0.001;
    return powf(0.1, depthA+depthB);
  }
  
  WLCS(const TextInfoBeta<Atom>& infoA, const TextInfoBeta<Atom>& infoB) {
    depthA = computeTextDepth(infoA);
    depthB = computeTextDepth(infoB);
  }
  
  LcsResult computeWeightedLCS(int startA, int endA, int startB, int endB)
  {
    vector< vector<LcsState> > stateTable(endA-startA, vector<LcsState>(endB-startB));
    for (int a=startA; a<endA; a = (infoA.states[a] == DFA::OPEN_BRACKET ? infoA.next[a] : a+1)) {
      for (int b=startB; b<endB; b = (infoB.states[b] == DFA::OPEN_BRACKET ? infoB.next[b] : b+1)) {
        string atomA = infoA.atomValue(a);
        string atomB = infoB.atomValue(b);
        int depA = depthA[a];
        int depB = depthB[b];
        
        LcsState currState;
        if (atomA == atomB) {
          double addScore = weight(atomA, depthA[a], depthB[b]);
          if (a == startA || b == startB) { // the case when we are on the border of the table
            currState.maxScore = addScore;
            currState.currLength = 1;
          }
          else { // regular case
            assert(infoA.states[a] == infoB.states[b]);
            int prevA, prevB;
            if (infoA.states[a] == DFA::END_BRACKET) { // omit substring
              prevA = infoA.prev[a];
              prevB = infoB.prev[b];
              // ...but compute lcs for it
              LcsResult subResult = computeWeightedLCS(prevA+1, a-1, prevB-1, b-1);
              currState.lcsBetween = subResult.lcs;
            }
            else {
              prevA = a-1;
              prevB = b-1;
            }
            assert(prevA >= startA);
            assert(prevB >= startB);
            currState = stateTable[prevA-startA][prevB-startB];
            currState.currLength += 1;
            currState.maxScore += addScore;
          }
          
          
        }
        else {
          ;
        } // end if atomA==atomB
      } // end for over B
    } // end for over A
  } // end function
};

template <class Atom>
struct CSLCS
{
  static constexpr double CostFullGroup = 3;
  static constexpr double CostHalfGroup = 2;
  static constexpr double CostInsert = 1;
  static constexpr double CostInfty = 1.0e50;
  
  struct CostWithTrace {
    double cost;
    int fromA, fromB;
    int fromWhich;
    CostWithTrace(double c = CostInfty) {
      fromA = fromB = 0;
      cost = c;
      fromWhich = 0;
    }
    CostWithTrace(double c, int fA, int fB, int fW) {
      cost=c; fromA=fA; fromB=fB; fromWhich=fW;
    }
    template <typename Out>
    void debug(Out& out) { out << fromWhich << (fromA==0&&fromB==0?".":fromA==0&&fromB==-1?"<":fromA==-1&&fromB==0?"^":fromA==-1&&fromB==-1?"`":"?") << cost << " "; }
  };
  
  struct LcsState {
    CostWithTrace ifEq, ifHalfA, ifHalfB, ifFull;
    // ifHalfA: cost of lcs, whose last letter is last letter of B-side
    bool isEqual;
    LcsState() { isEqual = false; }
    LcsState copyWithClose() {
      LcsState res = *this;
      /*
      res.ifEq.cost += 0.0;
      res.ifHalfA.cost += CostHalfGroup;
      res.ifHalfB.cost += CostHalfGroup;
      res.ifFull.cost += CostFullGroup;
      */
      return res;
    }
    CostWithTrace selectCost(int which) {
      return which==1?ifEq:which==2?ifHalfA:which==3?ifHalfB:ifFull;
    }
    template <typename Out>
    void debug(Out& out) {
      out << (isEqual?"E":"_");
      ifEq.debug(out);
      ifHalfA.debug(out);
      ifHalfB.debug(out);
      ifFull.debug(out);
      out << "\n";
    }
  };
  
  struct LcsResult {
    double score;
    vector< pair<int,int> > lcs;
  };
  
  TextInfoBeta<Atom> infoA, infoB;
  vector< vector<LcsState> > table;
  
  CSLCS(const TextInfoBeta<Atom>& iA, const TextInfoBeta<Atom>& iB) {
    infoA = iA;
    infoB = iB;
  }
  
  /*
  double diffCost(int diffA, int diffB) {
    double hard = CostInsert * (diffA + diffB), soft = 0.0;
    
    if (diffA == 0 && diffB == 0) soft = 0.0;
    else if (diffA == 0 || diffB == 0) soft = CostHalfGroup;
    else soft = CostFullGroup;
    
    return hard + soft;
  }
  */
  
  static pair<double,int> min4(double a, double b, double c, double d) { 
    double m = min(min(a,b),min(c,d));
    if (a==m) return {a,1};
    if (b==m) return {b,2};
    if (c==m) return {c,3};
    if (d==m) return {d,4};
    assert(false);
  }
  
  static pair<double,int> min2(double a, double b) {
    return a<b?make_pair(a,1):make_pair(b,2);
  }
  
  LcsState costFrom(int currA, int currB, int prevA, int prevB) {
    LcsState currState = table[currA][currB];
    LcsState prevState = table[prevA][prevB];
    LcsState result;
    if (currState.isEqual && prevA+1 == currA && prevB+1 == currB) {
      LcsState prevClosed = prevState.copyWithClose();
      pair<double,int> minpair = min4(prevClosed.ifEq.cost, prevClosed.ifHalfA.cost, prevClosed.ifHalfB.cost, prevClosed.ifFull.cost);
      result.ifEq.cost = minpair.first;
      result.ifEq.fromWhich = minpair.second;
    }
    double insert = currA-prevA + currB-prevB; // TODO: change into more complex insert cost (e.g. exclude indents)
    if (currB == prevB) {
      pair<double,int> minpair = min2(prevState.ifEq.cost+CostHalfGroup, prevState.ifHalfA.cost);
      result.ifHalfA.cost = minpair.first + CostInsert*insert;
      result.ifHalfA.fromWhich = minpair.second;
    }
    if (currA == prevA) {
      pair<double,int> minpair = min2(prevState.ifEq.cost+CostHalfGroup, prevState.ifHalfB.cost);
      result.ifHalfB.cost = minpair.first + CostInsert*insert;
      result.ifHalfB.fromWhich = minpair.second==1 ? 1 : 3;
    }
    pair<double,int> minpair = min4(
      prevState.ifEq.cost + ((currA==prevA || currB==prevB) ? CostHalfGroup : CostFullGroup),
      prevState.ifHalfA.cost + CostFullGroup-CostHalfGroup,
      prevState.ifHalfB.cost + CostFullGroup-CostHalfGroup,
      prevState.ifFull.cost);
     
    result.ifFull.cost = CostInsert*insert + minpair.first;
    result.ifFull.fromWhich = minpair.second;
    
    return result;
  }
    
  /*
  void sanityCheck(int a, int b) {
    LcsState state = table[a][b];
    if (state.isEqual) 
      assert(infoA.atomValue(a) == infoB.atomValue(b));
    else {
      if (state.fromA == -1) assert(state.fromB == -1);
      else assert(infoA.atomValue(state.fromA) == infoB.atomValue(state.fromB));
    }
  }
  */
  
  static CostWithTrace selectMin3CostWithTrace(CostWithTrace cwtA, CostWithTrace cwtB, CostWithTrace cwtAB) {
    if (cwtA.cost < cwtB.cost && cwtA.cost < cwtAB.cost)
      return {cwtA.cost, -1, 0, cwtA.fromWhich};
    else if (cwtB.cost < cwtA.cost && cwtB.cost < cwtAB.cost)
      return {cwtB.cost, 0, -1, cwtB.fromWhich};
    else 
      return {cwtAB.cost, -1, -1, cwtAB.fromWhich};
  }
  
  LcsResult computeLCS() {
    std::cerr << std::setprecision(6);
    cerr << "Start computing LCS, text sizes " << infoA.size() << " " << infoB.size() << "\n";
    table.resize(infoA.size(), vector<LcsState>(infoB.size()));
    
    LcsState& state0 = table[0][0];
    state0.ifFull.cost = CostInsert*2.0 + CostFullGroup; // we pay for two unmatched letters plus a full group :(
    if (infoA.atomValue(0) == infoB.atomValue(0)) {
      // state0.ifHalfA = state0.ifHalfB  remain infty
      state0.ifEq.cost = 0.0; // cost is perfectly zero
    }
    
    for (int a = 0; a < infoA.size(); a ++) {
      for (int b = 0; b < infoB.size(); b ++) {
        if (a == 0 && b == 0) continue;
        string atomA = infoA.atomValue(a);
        string atomB = infoB.atomValue(b);
        LcsState& currState = table[a][b];
        
        if (atomA == atomB) {
          currState.isEqual = true;
        }
        
        LcsState costFromA  = a==0 ? LcsState() : costFrom(a, b, a-1, b);
        LcsState costFromB  = b==0 ? LcsState() : costFrom(a, b, a, b-1);
        LcsState costFromAB = (a==0 || b==0) ? LcsState() : costFrom(a, b, a-1, b-1);
        
        if (currState.isEqual && (a==0 || b==0)) {
          costFromAB.ifEq.cost = CostHalfGroup * (double)(a>0 || b>0) + CostInsert*(a+b); 
          // cerr << " equal on border! Special cost = " << costFromAB.ifEq.cost << "\n";
        }
        
        currState.ifEq =    selectMin3CostWithTrace(costFromA.ifEq,    costFromB.ifEq,    costFromAB.ifEq);
        currState.ifHalfA = selectMin3CostWithTrace(costFromA.ifHalfA, costFromB.ifHalfA, costFromAB.ifHalfA);
        currState.ifHalfB = selectMin3CostWithTrace(costFromA.ifHalfB, costFromB.ifHalfB, costFromAB.ifHalfB);
        currState.ifFull =  selectMin3CostWithTrace(costFromA.ifFull,  costFromB.ifFull,  costFromAB.ifFull);
        
        // cerr << "At " << a << " " << atomA << " " << b << " " << atomB << ", cost: ";
        // currState.debug(cerr);
        
        // sanityCheck(a,b);
      } // end for over b
    } // end for over a
    
    LcsResult result;
    int iterA = infoA.size()-1;
    int iterB = infoB.size()-1;
    LcsState currState = table[iterA][iterB];
    currState.copyWithClose();
    pair<double,int> minp = min4(currState.ifEq.cost, currState.ifHalfA.cost, currState.ifHalfB.cost, currState.ifFull.cost);
    int which = minp.second;
    
    vector< pair<int,int> > lcs;
    while (true) {
      // cerr << "Backtrace at: " << iterA << ", " << iterB << ": " << infoA.atomValue(iterA) << ", " << infoB.atomValue(iterB) << ": " << which << "\n";
      if (which == 1) {
        assert(infoA.atomValue(iterA) == infoB.atomValue(iterB));
        lcs.push_back(make_pair(iterA, iterB));
        cerr << "  Add " << infoA.atomValue(iterA) << "\n";
      }
      CostWithTrace cwt = currState.selectCost(which);
      which = cwt.fromWhich;
      if (which == 0 || (cwt.fromA == 0 && cwt.fromB == 0)) break;
      iterA += cwt.fromA;
      iterB += cwt.fromB;
      cerr << "  Go " << cwt.fromA << " " << cwt.fromB << " " << which << "\n";
      currState = table[iterA][iterB];
    }
    reverse(lcs.begin(), lcs.end());
    result.lcs = lcs;
    
    return result;
  } // end function
};

/*
 i f* (atomA == atomB) {
   currState.isEqual = true;
   if (a == 0 || b == 0) {
     currState.cost = Cost(0.0, CostInsert*(a+b));
     currState.fromA = currState.fromB = -1;
     currState.length = 1;
     }
     else {
       LcsState prevState = table[a-1][b-1];
       if (prevState.isEqual) {
         currState.fromA = a-1;
         currState.fromB = b-1;
         }
         else {
           currState.fromA = prevState.fromA;
           currState.fromB = prevState.fromB;
           }
           Cost addCost = diffCost(a-currState.fromA-1, b-currState.fromB-1);
           addCost.hard += prevState.cost.hard;
           addCost.hard += addCost.soft; addCost.soft = 0.0;
           currState.length = prevState.length+1;
           currState.cost = addCost;
           } // end if on the border
           
           if (a+b < 10000)
             cerr << "Go a=" << a << ",b=" << b << "(" << atomA << " " << atomB << "), cost: " << (string)currState.cost << "\n";
           }
           else { // not the same atoms
             currState.isEqual = false;
             if (a == 0 && b == 0) {
               currState.length = 0;
               currState.fromA = currState.fromB = -1;
               currState.cost = Cost(0.0, CostInsert + CostFullGroup);
               currState.isEqual = false;
               continue;
             }
             
             Cost costFromA = a == 0 ? Cost::Inf() :
             costFrom(a, b, table[a-1][b]);
             Cost costFromB = b == 0 ? Cost::Inf() :
             costFrom(a, b, table[a][b-1]);
             
             if (a+b < 10000)
               cerr << "Go a=" << a << ",b=" << b << "(" << atomA << " " << atomB << "), costs: " << (string)costFromA << ", " << (string)costFromB << "\n";
             
             // take direction with smaller cost
             int prevA=a, prevB=b;
             Cost costFrom;
             LcsState prevState;
             if (costFromA < costFromB) {
               prevState = table[a-1][b];
               prevA = a-1;
               costFrom = costFromA;
             }
             else {
               prevState = table[a][b-1];
               prevB = b-1;
               costFrom = costFromB;
             }
             
             if (prevState.isEqual) {
               currState.fromA = prevA;
               currState.fromB = prevB;
             }
             else {
               currState.fromA = prevState.fromA;
               currState.fromB = prevState.fromB;
             }
             
             currState.length = prevState.length;
             currState.cost = costFrom;
             } // end if atoms are the same
             */


#endif