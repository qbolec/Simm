#ifndef BLOCKSTICKING_H
#define BLOCKSTICKING_H

#include "Utilities.h"
#include "Block.h"
#include "Graph.h"

unsigned long long int notMaximal = 0;
template<typename Callback>
void foreachMaximumIndependentSet(const vector<vector<int> > & originalGraph,vector<pair<int,int> > arcs,vector<vector<int> > nbrs, vector<int> take, Callback &foo,const Graph &sticks){

  //Following heuristics makes sense in a general max independent set problem:
  //if there is a node v, which is better from all it's neighbours in a sense, that 
  //the set of nbrs of v is smaller than set of any nbr of v, then we can safely take v.
  //This is perhaps easiest to see for a case where v is isolated :)
  //Or for the case when v has a single nbr.
  //But this can be generalised to degrees > 1 as described above.
  //The problem with this heuristic in our situation is that we have some additional constraints in our problem.
  //For example we want the matching brackets to be in the same component at the end of the day.
  /*
  unsigned int easy=0;
  for(bool changed=true;changed;){
    changed=false;
    for(int v=0;v<nbrs.size();++v)if(!take[v]){
      bool is_best=true;
      FOREACH(nbr,nbrs[v]){
        assert(!take[*nbr]);
        if(!isSubsetOf(nbrs[v],nbrs[*nbr],v,*nbr)){
          is_best=false;
          break;
        }
      }
      if(is_best){
        take[v]=1;
        FOREACH(nbr,nbrs[v]){
          take[*nbr]=-1;
          FOREACH(n,nbrs[*nbr])if(*n!=v){
            nbrs[*n]=erase(nbrs[*n],*nbr);
          }
        }
        easy++;
        changed=true;
        break;
      }
    }
  }
  //cerr << easy << "easy cases" << endl;
  */
  int best_v=-1;
  for(int v=0;v<nbrs.size();++v)if(!take[v]){
    if(best_v == -1 || nbrs[best_v].size() > nbrs[v].size()){
      best_v=v;
    }
  }
  if(best_v<0){
    /*
    for(int v=0;v<take.size();++v)if(take[v]==-1){
      bool hasNbr=false;
      FOREACH(n,originalGraph[v]){
        if(take[*n]==1){
          hasNbr=true;
        }
      }
      if(!hasNbr){
        //cerr << "Independent set is not maximal" << endl;
        notMaximal++;
        return;
      }
    }else{
      assert(take[v]==1);
      FOREACH(n,originalGraph[v]){
        assert(take[*n]==-1);
      }
    }
    */
    //cerr << "found sticking" << endl;
    Graph sticking=sticks;
    for(int a=0;a<arcs.size();++a)if(take[a]==1){
      //cerr << "Adding edge " << arcs[a].second << " ~> " << arcs[a].first << endl;
      sticking.addEdges(sticking.left(arcs[a].second),sticking.right(arcs[a].first));
    }
    foo(sticking);
  }else{
    vector<int> options=nbrs[best_v];
    options.push_back(best_v);
    //cerr << "Iterating over " << options.size() << " Options" << endl;
    //k-ta opcja polega na tym, że take[k]=1 i take[k+1]=take[k+2]=...=-1
    for(int k=0;k<options.size();++k){
      vector<int> newTake = take;
      vector<vector<int> > newNbrs = nbrs;
      int v= options[k];
      //cerr << "Take " << v << endl;
      newTake[v]=1;
      bool isMaximal = true;
      FOREACH(nbr,newNbrs[v]){
        if(newTake[*nbr]){
          cerr << k << ' ' << v << ' ' << *nbr << endl;;
          FOREACH(u,newNbrs[v]){
            cerr << *u << ' ';
          }
          cerr << endl;
        }
        assert(!newTake[*nbr]);
        newTake[*nbr]=-1;
        FOREACH(n,newNbrs[*nbr])if(*n!=v){
          newNbrs[*n]=erase(newNbrs[*n],*nbr);
          if(newNbrs[*n].empty() && !isOk(*n,newTake,originalGraph)){
         //   cerr << "Do not go there" << endl;
            isMaximal = false;
          }
        }
      }
      for(int i=k+1;i<options.size();++i){
        int nbr=options[i];
        assert(newTake[nbr]!=1);
        if(!newTake[nbr]){
          newTake[nbr]=-1;
          if(!isOk(nbr,newTake,originalGraph)){
       //     cerr << "Dude do not go there" <<endl;
            isMaximal = false;
          }
          FOREACH(n,newNbrs[nbr]){
            newNbrs[*n]=erase(newNbrs[*n],nbr);
            if(newNbrs[*n].empty() && !isOk(*n,newTake,originalGraph)){
        //      cerr << "Srsly, do not go there" << endl;
              isMaximal = false;
            }
          }
        }
      }
      if(isMaximal){
        foreachMaximumIndependentSet(originalGraph, arcs,newNbrs,newTake,foo,sticks);
      }

    }
  }

}

int getBBlock(unsigned int bp,const vector<Block> &blocks,const TextInfo & a){
  for(unsigned int b=0;b<blocks.size();++b){
    if(blocks[b].containsDest(bp+1+a.next.size())){
      return b;
    }
  }
  return -1;
}
int getABlock(unsigned int ap,const vector<Block> &blocks){
  for(unsigned int b=0;b<blocks.size();++b){
    if(blocks[b].containsSrc(ap+1)){
      return b;
    }
  }
  return -1;
}

template<typename Callback>
void foreachBlocksSticking(vector<Block> blocks,Callback &foo,TextInfo &a,TextInfo &b){
  vector<pair<int,int> > implicitArcs;
  for(unsigned int i=0;i<a.next.size();++i){
    if(a.next[i]<a.next.size()){
      int pb=getABlock(i,blocks);
      int nb=getABlock(a.next[i],blocks);
      if(pb!=nb){
        assert(pb<nb);
        assert(blocks[pb].startSrc < blocks[nb].startSrc);
        if(areAligned(blocks[pb],blocks[nb])){
          implicitArcs.push_back(make_pair(pb,nb));
        }else{
          cerr << "The whole matching has wrongly matched (unaligned) brackets" << endl;
          assert(false);//This should be checked one level up
          return;
        }
      }
    }
  }
  for(unsigned int i=0;i<b.next.size();++i){
    if(b.next[i]<b.next.size()){
      int pb=getBBlock(i,blocks,a);
      int nb=getBBlock(b.next[i],blocks,a);
      if(pb!=nb){
        assert(pb<nb);
        assert(blocks[pb].startDest < blocks[nb].startDest);
        if(areAligned(blocks[pb],blocks[nb])){
          implicitArcs.push_back(make_pair(pb,nb));
        }else{
          cerr << "The whole matching has wrongly matched (unaligned) brackets" << endl;
          assert(false);//This should be checked one level up
          return;
        }
      }
    }
  }
  vector<pair<int,int> > arcs;
  for(int i=blocks.size();i--;){
    for(int j=blocks.size();j--;)if(i<j){
      assert(blocks[i].startSrc < blocks[j].startSrc);
      if(areAligned(blocks[i],blocks[j])){
        bool bad=false;
        FOREACH(imp,implicitArcs){
          if(conflict( blocks[imp->second],blocks[imp->first], blocks[j],blocks[i])){
            bad=true;
          }
        }
        if(!bad){
          arcs.push_back(make_pair(i,j));
        }
      }
    }
  }
  vector<vector<int> > nbrs(arcs.size());
  int edges=0;
  for(int i=arcs.size();i--;){
    for(int j=i;j--;){
      if(conflict( blocks[arcs[i].second],blocks[arcs[i].first],blocks[arcs[j].second],blocks[arcs[j].first]) || arcs[i].second==arcs[j].second || arcs[i].first==arcs[j].first){
        edges++;
        nbrs[i].push_back(j);
        nbrs[j].push_back(i);
      }
    }
  }
  FOREACH(n,nbrs){
    sort(n->begin(),n->end());
  }
  notMaximal = 0;
  //cerr << "arcs " << arcs.size() << " edges " << edges << endl;
  Graph sticks(blocks.size(),blocks.size());
  foreachMaximumIndependentSet(nbrs,arcs,nbrs,vector<int> (arcs.size(),0),foo,sticks);
  //cerr << "Not maximal sets " << notMaximal << endl;
  //foreachBlocksSticking(blocks,foo,0,sticks);
}


struct JudgeSticking{
  vector<Block> blocks;
  TextInfo a;
  TextInfo b;
  Graph bestSticks;
  unsigned int bestSticksCount;
  unsigned long long int stickingsTested;
  unsigned long long int validBrackets;
  JudgeSticking(vector<Block> blocks,TextInfo a,TextInfo b):blocks(blocks),a(a),b(b),bestSticks(blocks.size(),blocks.size()),bestSticksCount(0),stickingsTested(0),validBrackets(0){
  }
  int getBBlock(unsigned int bp){
    for(unsigned int b=0;b<blocks.size();++b){
      if(blocks[b].containsDest(bp+1+a.next.size())){
        return b;
      }
    }
    return -1;
  }
  int getABlock(unsigned int ap){
    for(unsigned int b=0;b<blocks.size();++b){
      if(blocks[b].containsSrc(ap+1)){
        return b;
      }
    }
    return -1;
  }
  bool areBlocksConnected(unsigned int ab,unsigned int bb,Graph sticks){
    if(ab==bb){
      return true;
    }
    if(bb<ab){
      return false;
    }
    if(!sticks.getOutDegree(sticks.right(ab))){
      return false;
    }
    assert(sticks.getOutDegree(sticks.right(ab))==1);
    return areBlocksConnected(sticks.whichLeft(sticks.getOutEdgeEnd(sticks.right(ab),0)),bb,sticks);
  }
  void operator()(Graph sticks){
    stickingsTested++;
    for(unsigned int i=0;i<a.next.size();++i){
      if(a.next[i]<a.next.size()){
        int pb=getABlock(i);
        int nb=getABlock(a.next[i]);
        //cerr << "Verifying " << i << " (" << pb << ") paired with " << a.next[i] << " (" << nb << ")" << endl;
        
        if((-1<=pb) != (-1<=nb)){
          return;
        }
        if(-1<=pb && -1<=nb){
          //cerr << "Both are matched" << endl;
          if(!areBlocksConnected(pb,nb,sticks)){
            //cerr << "Brackets are not correctly connected" << endl;
            return;
          }
        }
      }
    }
    for(unsigned int i=0;i<b.next.size();++i){
      if(b.next[i]<b.next.size()){
        int pb=getBBlock(i);
        int nb=getBBlock(b.next[i]);
        if((-1<=pb) != (-1<=nb)){
          return;
        }
        if(-1<=pb && -1<=nb){
          if(!areBlocksConnected(pb,nb,sticks)){
            return;
          }
        }
      }
    }
    validBrackets ++;

    //debugDumpAsHtml(sticks,"sticks");
    unsigned int edgesCount = sticks.getEdgesCount();
    //cerr << "edgesCount is " << edgesCount << endl;
    assert(edgesCount%2==0);
    if(bestSticksCount < edgesCount){
      bestSticksCount = edgesCount;
      bestSticks = sticks;
    }
  }
  Graph getBestSticks(){
    return bestSticks;
  }
};
vector<Block> getBlocks(Graph matching){
  unsigned int lastLeft=-1;
  unsigned int lastRight=-1;
  vector<Block> blocks;
  for(unsigned int i=0;i<matching.getLeftSize();++i){
    unsigned int id=matching.left(i);
    for(unsigned int j=0;j<matching.getOutDegree(id);++j){
      unsigned int endId = matching.getOutEdgeEnd(id,j);
      if(id==lastLeft+1 && endId ==lastRight+1){
        ++blocks.back().length;
      }else{
        blocks.push_back(Block(id,endId,1));
      }
      lastLeft = id;
      lastRight = endId;
    }
  }  
  return blocks;
}
Graph getBestSticksForBlocks(vector<Block> blocks,TextInfo a,TextInfo b ){
  //debugDumpAsHtml(blocks);
  JudgeSticking judge(blocks,a,b);
  foreachBlocksSticking(blocks,judge,a,b);
  //cerr << "Blocks count " << blocks.size() << ", stickings tested " << judge.stickingsTested << " of which " << judge.validBrackets << " have valid brackets" << endl ;
  return judge.getBestSticks();
}
Graph getBestSticksForMatching(Graph matching,TextInfo a,TextInfo b){
  return getBestSticksForBlocks(getBlocks(matching),a,b);
}



#endif