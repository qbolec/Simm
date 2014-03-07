#ifndef FOREACHMATCHING_H
#define FOREACHMATCHING_H

#include "Utilities.h"
#include "Graph.h"
#include <vector>
#include <iostream>

using namespace std;

template<typename Callback>
void foreachMatching(Graph &g,Callback &foo,unsigned int leftPos,Graph matching){
  if(leftPos == g.getLeftSize()){
    foo(matching);
  }else{
    //be consequent:
    int suggestedEndId = -1;
    if(leftPos>0){
      unsigned int prevNodeId = g.left(leftPos-1);
      unsigned int myNodeId = g.left(leftPos);
      unsigned int hisDegree =g.getOutDegree(prevNodeId);
      unsigned int myDegree = g.getOutDegree(myNodeId);
      if(0<matching.getOutDegree(prevNodeId) &&  myDegree==hisDegree){
        vector<unsigned int> hisOptions;
        for(unsigned int i=0;i<hisDegree;++i){
          hisOptions.push_back(g.getOutEdgeEnd(prevNodeId,i)+1);
        }
        sort(hisOptions.begin(),hisOptions.end());
        vector<unsigned int> myOptions;
        for(unsigned int i=0;i<myDegree;++i){
          myOptions.push_back(g.getOutEdgeEnd(myNodeId,i));
        }
        sort(myOptions.begin(),myOptions.end());
        if(myOptions==hisOptions){
          //be consequent, choose the same option.
          suggestedEndId = matching.getOutEdgeEnd(prevNodeId,0)+1;
        }        
      }
      
    }
    unsigned int id=g.left(leftPos);
    bool canSkip = suggestedEndId==-1;
    for(unsigned int i=0;i<g.getOutDegree(id);++i){
      unsigned int endId = g.getOutEdgeEnd(id,i);
      if(suggestedEndId!=-1 && endId!=(unsigned int)suggestedEndId){
        continue;
      }
      if(0<matching.getOutDegree(endId)){
        unsigned int saveFromId = matching.getOutEdgeEnd(endId,0);
        matching.isolate(endId);
          
        matching.addEdge(id,endId);
        matching.addEdge(endId,id);
        foreachMatching(g,foo,leftPos+1,matching);

        matching.isolate(endId);
        matching.addEdge(saveFromId,endId);
        matching.addEdge(endId,saveFromId);
          
      }else{
        canSkip = false;
          
        matching.addEdge(id,endId);
        matching.addEdge(endId,id);
        foreachMatching(g,foo,leftPos+1,matching);

        matching.isolate(endId);
          
      }
    }
    if(canSkip){
      foreachMatching(g,foo,leftPos+1,matching);
    }
    
  }
}
template<typename Callback>
void foreachMatching(Graph g,Callback &foo){
  Graph matching(g.getLeftSize(),g.getRightSize());
  foreachMatching(g,foo,0,matching);
}

#endif // FOREACHMATCHING_H

