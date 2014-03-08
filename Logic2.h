#include<cmath>
#include "Output.h"
#include "Utilities.h"
#include "TextInfo.h"
#include "BlockSticking.h"
#include "AnalyzeText.h"
#include "Graph.h"
#include "Match.h"

const int VARIANTS = 2;

template <class Atom>
struct Logic2 {
  unsigned int cost(Graph matching,const TextInfoBeta<Atom>& a, const TextInfoBeta<Atom>& b){
    Graph sticks = getBestSticksForMatching(matching,a,b);
    unsigned int numberOfLetters = matching.getLeftSize()+matching.getRightSize();
    unsigned int matchedLetters = matching.getEdgesCount();
    assert(matchedLetters%2 == 0);
    assert(matchedLetters <= numberOfLetters);
    unsigned int unmatchedGroups = 0;
    for(unsigned int i=0;i<matching.getLeftSize();++i){
      unsigned int id = matching.left(i);
      if(matching.getOutDegree(id)==0){
        if(i==0 || matching.getOutDegree(id-1)!=0){
          unmatchedGroups++;
        }
      }
    }
    for(unsigned int i=0;i<matching.getRightSize();++i){
      unsigned int id = matching.right(i);
      if(matching.getOutDegree(id)==0){
        if(i==0 || matching.getOutDegree(id-1)!=0){
          unmatchedGroups++;
        }
      }
    }
    unsigned int unmatchedLetters = numberOfLetters-matchedLetters;
    unsigned int matches = sticks.getLeftSize();
    assert(matches == sticks.getRightSize());
    unsigned int sticked = sticks.getEdgesCount();
    assert(sticked%2 == 0);
    unsigned int sticksCount = sticked/2;
    assert(sticksCount==0 || sticksCount< matches);
    unsigned int cost = (unmatchedLetters + unmatchedGroups + matches-sticksCount)*10+matches;
    return cost;
  }

  void storeMatch(int from,int len,int continuationOf,int matchId,vector<pair<int,int> > &matchInfo){
    cerr << "storeMatch with id " << matchId << endl;
    for(int i=from;i<from+len;++i){
      matchInfo[i].first = matchId;
      matchInfo[i].second = (i==from)?continuationOf:i-1;
    }
  }

  void naiveInflate(TextInfoBeta<Atom>& info){
    info.fullMatchInfo = vector<pair<int,int> >(info.origSize(), make_pair(-1,-1));
    for(unsigned int i=0;i<info.matchInfo.size();++i){
      if(info.matchInfo[i].first == -1){      
      }else{
        info.fullMatchInfo[ info.positions[i] ].first = info.matchInfo[i].first;
        if(info.matchInfo[i].second == -1){
          info.fullMatchInfo[info.positions[i]].second = -1;
        }else{
          info.fullMatchInfo[info.positions[i]].second = info.positions[info.matchInfo[i].second];
        }
      }
    }
  }

  void inflateWhitespaces(TextInfoBeta<Atom> &a,TextInfoBeta<Atom> &b,Graph matching){
    naiveInflate(a);
    naiveInflate(b);
    
    int expected =0;
    for(unsigned int i=0;i<a.size();++i){
      if(a.positions[i]!=expected){
        unsigned int start =expected;
        unsigned int end=a.positions[i];
        if(0<matching.getOutDegree(matching.left(i))){
          unsigned int rightEnd= b.positions[matching.whichRight(matching.getOutEdgeEnd(matching.left(i),0))];
          unsigned int rightStart=rightEnd;
          while(0<rightStart && b.states[rightStart-1]==DFA::IGNORABLE && b.fullMatchInfo[rightStart-1].first==-1){
            --rightStart;
          }
          string leftString = a.atomOrigSeqValue(start, end-start); // a.original_text.substr(start,end-start);
          string rightString = b.atomOrigSeqValue(rightStart,rightEnd-rightStart); // b.original_text.substr(rightStart,rightEnd-rightStart);
          cerr << "Matching " << start << "-:-" << end << " vs. " << rightStart << "-:-" << rightEnd << endl;
          vector<pair<int,int> > path = getAnyLCS(lcs(leftString,rightString));
          cerr << "Found " << path.size() << " path" << endl;
          int lastLeft = a.fullMatchInfo[end].second;
          int lastRight = b.fullMatchInfo[rightEnd].second;
          int freeMatchId = a.fullMatchInfo[end].first;
          assert(freeMatchId == b.fullMatchInfo[rightEnd].first);
          
          FOREACH(p,path){
            a.fullMatchInfo[start+p->first].first = freeMatchId;
            a.fullMatchInfo[start+p->first].second = lastLeft;
            lastLeft = start+p->first;
            b.fullMatchInfo[rightStart+p->second].first = freeMatchId;
            b.fullMatchInfo[rightStart+p->second].second = lastRight;
            lastRight = rightStart+p->second;
          }
          a.fullMatchInfo[end].second=lastLeft;
          b.fullMatchInfo[rightEnd].second=lastRight;
          
        }      
      }    
      expected=a.positions[i]+1;
    }
  }

  void dfs(const TextInfoBeta<Atom> &a,const TextInfoBeta<Atom> &b,unsigned int ai,unsigned int bi,int color,vector<vector<int> > &blockId){
    assert(ai < a.size() && bi < b.size());
    assert(a.atomValue(ai)==b.atomValue(bi));
    if(blockId[ai][bi]==color){
      return;
    }
    assert(blockId[ai][bi]==-1);
    blockId[ai][bi]=color;
    if(0<ai && 0<bi && a.atomValue(ai-1)==b.atomValue(bi-1)){
      dfs(a,b,ai-1,bi-1,color,blockId);
    }
    if(ai+1<a.size() && bi+1<b.size() && a.atomValue(ai+1)==b.atomValue(bi+1)){
      dfs(a,b,ai+1,bi+1,color,blockId);
    }
    if(a.next[ai]<a.size() && b.next[bi]<b.size()){
      assert(a.atomValue(a.next[ai]) == b.atomValue(b.next[bi]));
      dfs(a,b,a.next[ai],b.next[bi],color,blockId);
    }
    if(a.prev[ai]<a.size() && b.prev[bi]<b.size()){
      assert(a.atomValue(a.prev[ai]) == b.atomValue(b.prev[bi]));
      dfs(a,b,a.prev[ai],b.prev[bi],color,blockId);
    }
  }

  vector<vector<pair<int,int> > > getDominantBlocks(const TextInfoBeta<Atom>& a,const TextInfoBeta<Atom>& b){
    vector<vector<int> > blockId(a.size(),vector<int>(b.size(),-1));
    vector<vector<pair<int,int> > > blocks;
    for(unsigned int ai=0;ai<a.size();++ai){
      for(unsigned int bi=0;bi<b.size();++bi){
        if(blockId[ai][bi]<0 && a.atomValue(ai)==b.atomValue(bi)){
          dfs(a,b,ai,bi,blocks.size(),blockId);
          blocks.push_back(vector<pair<int,int> >());
        }
      }
    }
    for(unsigned int ai=0;ai<a.size();++ai){
      for(unsigned int bi=0;bi<b.size();++bi){
        if(0<=blockId[ai][bi]){
          blocks[blockId[ai][bi]].push_back(make_pair(ai,bi));
        }
      }
    }
    vector<bool> isBlockDominant(blocks.size(),false);
    for(unsigned int ai=0;ai<a.size();++ai){
      size_t longest_length = 0;
      for(unsigned int bi=0;bi<b.size();++bi){
        if(0<=blockId[ai][bi]){
          longest_length = max(longest_length,blocks[blockId[ai][bi]].size());
        }
      }
      for(unsigned int bi=0;bi<b.size();++bi){
        if(0<=blockId[ai][bi] && longest_length == blocks[blockId[ai][bi]].size()){
          isBlockDominant[blockId[ai][bi]]=true;
        }
      }
    }
    for(unsigned int bi=0;bi<b.size();++bi){
      size_t longest_length = 0;
      for(unsigned int ai=0;ai<a.size();++ai){
        if(0<=blockId[ai][bi]){
          longest_length = max(longest_length,blocks[blockId[ai][bi]].size());
        }
      }
      for(unsigned int ai=0;ai<a.size();++ai){
        if(0<=blockId[ai][bi] && longest_length == blocks[blockId[ai][bi]].size()){
          isBlockDominant[blockId[ai][bi]]=true;
        }
      }
    }
    vector<vector<pair<int,int> > > dominantBlocks;
    for(unsigned int i=0;i<blocks.size();++i){
      if(isBlockDominant[i]){
        dominantBlocks.push_back(blocks[i]);
      }
    }
    for(unsigned int i=0;i<dominantBlocks.size();++i){
      cerr << "Dominant block #" << i << " is:" << endl;
      for(unsigned int j=0;j<dominantBlocks[i].size();++j){
        cerr << " " << dominantBlocks[i][j].first << ": " << a.atomValue(dominantBlocks[i][j].first) << "  -- " << dominantBlocks[i][j].second << ": " << b.atomValue(dominantBlocks[i][j].second) << endl;
      }
    }
    vector<vector<pair<int,int> > > shortBlocks;
    for(unsigned int i=0;i<dominantBlocks.size();++i){
      map<int,int> diffToBlockId;
      for(unsigned int j=0;j<dominantBlocks[i].size();++j){
        int diff = dominantBlocks[i][j].first-dominantBlocks[i][j].second;
        if(diffToBlockId.find(diff)==diffToBlockId.end() || shortBlocks[diffToBlockId[diff]].back().first!= dominantBlocks[i][j].first-1){
          diffToBlockId[diff] = shortBlocks.size();
          shortBlocks.push_back(vector<pair<int,int> >());
        }
  //      cerr << "Pushing " << dominantBlocks[i][j].first << " --> " << dominantBlocks[i][j].second << " with diff " << diff << " at " << diffToBlockId[diff] <<  endl ;
        shortBlocks[diffToBlockId[diff]].push_back(dominantBlocks[i][j]);
      }
    }
    
    cerr << "There are " << shortBlocks.size() << " potential short blocks" << endl;
    for(unsigned int i=0;i<shortBlocks.size();++i){
      cerr << "Short block #" << i << " is:" << endl;
      for(unsigned int j=0;j<shortBlocks[i].size();++j){
        if(a.size() <= shortBlocks[i][j].first || b.size() <= shortBlocks[i][j].second){
          cerr << "WTF is this: " << shortBlocks[i][j].first << " ---> " << shortBlocks[i][j].second << endl;
        }
        cerr << " " << shortBlocks[i][j].first << ": " << a.atomValue(shortBlocks[i][j].first) << "  -- " << shortBlocks[i][j].second << ": " << b.atomValue(shortBlocks[i][j].second) << endl;
      }
    }
  
    //If both endpoints of an edge have degree=1, then this edge must be in the matching.
    //If in a block two edges must be in the matching (due to above rule), then we want (for it is "natural") all edges "in between" these two edges to also be in the matching.
    //Forcing these "inner" edges to be in the matching, perhaps means that some other edges (from other blocks) can not be in the matching.
    //If you think about it for a while (a picture might help) if there is at least one edge in a block that can not be in the maching (due to above rule) then the whole block can not be in the matching.
    //So, in short: one block can dominate another block, if the other block lays "inside" (either in text A or in text B) the first block, between two edges which must be in the matching.
    //We can remove such dominated block, with all it edges, which in turn may lead to new nodes of degree=1, and recursively cause some new dominations to occur.
    //This is a fixed point loop, which removes dominated blocks as long as there are any.
    for(bool changed=true;changed;){
      changed=false;
      vector<int> aCoverage(a.size(),0);
      vector<int> bCoverage(b.size(),0);
      for(unsigned int i=0;i<shortBlocks.size();++i){
        for(unsigned int j=0;j<shortBlocks[i].size();++j){
          aCoverage[shortBlocks[i][j].first]++;
          bCoverage[shortBlocks[i][j].second]++;
        }
      }
      for(unsigned int i=0;i<shortBlocks.size();++i){
        int first_j = -1;
        int last_j = -1;
        for(unsigned int j=0;j<shortBlocks[i].size();++j){
          assert(aCoverage[shortBlocks[i][j].first]>=1 && bCoverage[shortBlocks[i][j].second]>=1);

          if(aCoverage[shortBlocks[i][j].first]==1 && bCoverage[shortBlocks[i][j].second]==1){
            if(first_j==-1){
              first_j=j;
            }
            last_j = j;
          }
        }
        cerr << "short block #" << i << " first_j" << first_j << " last_j " << last_j << endl;
        if(first_j < last_j){
          bool gotSomething=false;
          for(unsigned int j=first_j;j<=last_j;++j){
            if(aCoverage[shortBlocks[i][j].first]>1 || bCoverage[shortBlocks[i][j].second]>1){
              gotSomething = true;
            }
          }
          if(gotSomething){
            cerr << "got something" << endl;
            changed = true;
            vector<vector<pair<int,int> > > aliveBlocks;
            for(unsigned int k=0;k<shortBlocks.size();++k){
              if(k!=i && (
                      (shortBlocks[i][first_j].first <= shortBlocks[k].back().first && shortBlocks[k].back().first <= shortBlocks[i][last_j].first) ||
                      (shortBlocks[i][first_j].second <= shortBlocks[k].back().second && shortBlocks[k].back().second <= shortBlocks[i][last_j].second)
                    )){
                assert((
                    shortBlocks[i][first_j].first < shortBlocks[k].back().first && 
                    shortBlocks[k].back().first < shortBlocks[i][last_j].first &&
                    shortBlocks[i][first_j].first < shortBlocks[k].front().first && 
                    shortBlocks[k].front().first < shortBlocks[i][last_j].first
                ) || (
                    shortBlocks[i][first_j].second < shortBlocks[k].back().second && 
                    shortBlocks[k].back().second < shortBlocks[i][last_j].second &&
                    shortBlocks[i][first_j].second < shortBlocks[k].front().second && 
                    shortBlocks[k].front().second < shortBlocks[i][last_j].second
                ));
                cerr << "short block #" << i << " dominated short block #" << k << endl;
              }else{
                aliveBlocks.push_back(shortBlocks[k]);
              }
            }
            assert(aliveBlocks.size() < shortBlocks.size());
            swap(aliveBlocks,shortBlocks);
            break;
          }
        }

      }

    }
    /*
    //For performance reasons let us remove all single-char blocks
    {
      unsigned int j=0;
      for(unsigned int i=0;i<shortBlocks.size();++i){
        if(shortBlocks[i].size()>1){
          shortBlocks[j++]=shortBlocks[i];
        }
      }
      shortBlocks.resize(j);
    }
    */
    cerr << "There are " << shortBlocks.size() << " final short blocks" << endl;
    for(unsigned int i=0;i<shortBlocks.size();++i){
      cerr << "Short block #" << i << " is:" << endl;
      for(unsigned int j=0;j<shortBlocks[i].size();++j){
        if(a.size() <= shortBlocks[i][j].first || b.size() <= shortBlocks[i][j].second){
          cerr << "WTF is this: " << shortBlocks[i][j].first << " ---> " << shortBlocks[i][j].second << endl;
        }
        cerr << " " << shortBlocks[i][j].first << ": " << a.atomValue(shortBlocks[i][j].first) << "  -- " << shortBlocks[i][j].second << ": " << b.atomValue(shortBlocks[i][j].second) << endl;
      }
    }
    return shortBlocks;
  }
  
  bool blocksConflict(const vector<pair<int,int> > & blockX,const vector<pair<int,int> > & blockY){
    set<int> left;
    set<int> right;
    for(unsigned int i=0;i<blockX.size();++i){
      left.insert(blockX[i].first);
      right.insert(blockX[i].second);
    }
    for(unsigned int i=0;i<blockY.size();++i){
      if(left.count(blockY[i].first) || right.count(blockY[i].second)){
        return true;
      }
    }
    return false;
  }
  
  vector<pair<int,int> > getConflictingBlocks(const vector<vector<pair<int,int> > > & dominantBlocks){
    vector<pair<int,int> > conflicts;
    for(unsigned int i=0;i<dominantBlocks.size();++i){
      for(unsigned int j=0;j<i;++j){
        if(blocksConflict(dominantBlocks[i],dominantBlocks[j])){
          conflicts.push_back(make_pair(j,i));
        }
      }
    }
    cerr << "There is " << conflicts.size() << " conflicts among blocks:" << endl;
    for(unsigned int i=0;i<conflicts.size();++i){
      cerr << " " << conflicts[i].first << " ~ " << conflicts[i].second << endl;
    }
    return conflicts;
  }

  struct OnEachBlocksSequence{
    Logic2& logic;
    const TextInfoBeta<Atom> &a;
    const TextInfoBeta<Atom> &b;
    Graph bestMatching;
    unsigned int bestCost;
    unsigned long long testedSequences;
    unsigned long long validBrackets;
    OnEachBlocksSequence(Logic2& log, const TextInfoBeta<Atom> &a, const TextInfoBeta<Atom> &b):
      logic(log),a(a),b(b),bestMatching(a.size(),b.size()),bestCost(1e9),testedSequences(0),validBrackets(0){
    }
    void onBlocksSequence(const vector<vector<pair<int,int> > > &blocks){
      testedSequences++;
      Graph matching(a.size(),b.size());
      for(unsigned int i=0;i<blocks.size();++i){
        for(unsigned int j=0;j<blocks[i].size();++j){
          unsigned int left=matching.left(blocks[i][j].first);
          unsigned int right=matching.right(blocks[i][j].second);
          //assert(0==matching.getOutDegree(left) && 0==matching.getOutDegree(right));//for new algorithm
          if(0==matching.getOutDegree(left) && 0==matching.getOutDegree(right)){
            matching.addEdges(left,right);
          }
        }
      }
      for(unsigned int i=0;i<a.size();++i){
        if(a.next[i] < a.size()){
          if(matching.getOutDegree(matching.left(i)) != matching.getOutDegree(matching.left(a.next[i]))){
            return;
          }
          if(matching.getOutDegree(matching.left(i))){
            if(b.next[matching.whichRight(matching.getOutEdgeEnd(matching.left(i),0))] != matching.whichRight(matching.getOutEdgeEnd(matching.left(a.next[i]),0))){
              return;
            }
          }
        }
      }
      validBrackets++;
      //debugDumpAsHtml(matching,"Candidate matching");
      unsigned int currentCost = logic.cost(matching,a,b);
      //cerr << "cost of this matching is " << currentCost << endl;
      if(currentCost < bestCost){
        bestCost = currentCost;
        bestMatching = matching;
      }
    }
  };

  bool dagDfs(const vector<vector<int> > & dag,int src,int dest,set<int> &visited){
    if(src==dest){
      return true;
    }
    if(!visited.insert(src).second){
      return false;
    }
    for(unsigned int j=dag[src].size();j--;){
      if(dagDfs(dag,dag[src][j],dest,visited)){
        return true;
      }
    }
    return false;
  }
  bool dagDfs(const vector<vector<int> > & dag,int src,int dest){
    set<int> visited;
    return dagDfs(dag,src,dest,visited);
  }

  void allConflicts(
      int conflictId,
      vector<vector<int> > dag,
      vector<vector<pair<int,int> > > dominantBlocks, 
      vector<pair<int,int> > conflictingBlocks,
      OnEachBlocksSequence & visitor,
      unsigned long long int progress=0){
    if(conflictId == min(20U,(unsigned int)conflictingBlocks.size())){
      cerr << "Progress " << progress << " out of " << (1ULL << conflictId) << "\r";
    }
    {
      for(unsigned int i=conflictId+1;i<conflictingBlocks.size();++i){
        unsigned int best=dag[conflictingBlocks[conflictId].first].size() + dag[conflictingBlocks[conflictId].second].size();
        unsigned int current=dag[conflictingBlocks[i].first].size() + dag[conflictingBlocks[i].second].size();
        if(current>best){
          swap(conflictingBlocks[i],conflictingBlocks[conflictId]);
  //         cerr << "swap conflict " << i << " with conflict " << conflictId << endl;
        }
      }
    }
    for(bool change=true;change;){
      change=false;
      for(unsigned int i=0;i<dominantBlocks.size();++i){
        for(unsigned int j=0;j<dominantBlocks[i].size();++j){
          int a=dominantBlocks[i][j].first;
          int b=dominantBlocks[i][j].second;
          vector<int> worse;
          bool might_be_bad=false;
          bool a_conflicts = false;
          bool b_conflicts = false;
          for(unsigned int k=0;k<dominantBlocks.size();++k)if(k!=i && !dominantBlocks[k].empty()){
            if((dominantBlocks[k].front().first <= a && a <= dominantBlocks[k].back().first) || 
                (dominantBlocks[k].front().second <= b && b <=dominantBlocks[k].back().second)){
              for(unsigned int p=0;p<dominantBlocks[k].size();++p){
                if(dominantBlocks[k][p].first == a){
                  a_conflicts = true;
                }
                if(dominantBlocks[k][p].second == b){
                  b_conflicts = true;
                }
                if(dominantBlocks[k][p].first == a || dominantBlocks[k][p].second == b){
                  if(dagDfs(dag,i,k)){
                    assert(worse.empty() || worse.back()!=k);
                    worse.push_back(k);
                  }else{
                    might_be_bad = true;
                  }
                }
              }
            }
          }
          bool kill_conflicts = false;
          if(!worse.empty() && (!a_conflicts || !b_conflicts)){
            //IDEA: if one of ends of the edge a-->b is not conflicting with any other edge,
            //then a-->b will be in the matching, unless something covers the other end.
            //But if something covers the other end, then it has to be better than a-->b,
            //and (by transitivity) better than everything which is worse than a-->b,
            //and so everything that is worse than a-->b and conflicts with the other end
            //will not be in the matching.
            kill_conflicts = true;
          //  cerr << "Kill in the name of single end" << endl;
          }else if(!might_be_bad && !worse.empty()){
            //IDEA: when all conflicting blocks are worse, then edge a-->b can not be covered
            //by any of these worse blocks, so edge a-->b will be in the matching, and thus
            //all conflicting edges can be removed
            kill_conflicts = true;
          //  cerr << "Kill in the name of domination" << endl;
          }
          if(kill_conflicts){
            change = true;
            FOREACH(kp,worse){
              const unsigned int k=*kp;
              unsigned int r=0;
              for(unsigned int p=0;p<dominantBlocks[k].size();++p){
                if(dominantBlocks[k][p].first == a || dominantBlocks[k][p].second == b){
  //                cerr << "Eliminating edge " << dominantBlocks[k][p].first << " --> " << dominantBlocks[k][p].second << " from block #" << k << " because block #" << i << " dominated" << endl;
                }else{
                  dominantBlocks[k][r++] = dominantBlocks[k][p];
                }
              }
              dominantBlocks[k].resize(r);
              //IDEA: when a block is eliminated (has no edges) then it should not dominate anything (should have no outgoing edges in the dag).
              //We try to make sure that it holds "deeper" in the recursion, by marking such conflicts as irrelevant and do not add them to the dag (nor their reflections).
              //However "shalower" in the recursion we might have already added some edges.
              //We want to kill the whole recursion and back track in such situations.
              //However this might lead to a severe bug: if there are two conflicting blocks, both of them become empty deeper in the recursion, then one of them will always have an edge in the dag.
              //So, to allow that, we must break ties somehow for empty blocks, and allow edges in the dag which follow the rule of tiebreaking.
              //Here we use the block id as a tiebreaker.
              //We allow an empty block X to dominate block Y only if #Y<#X.
              if(dominantBlocks[k].empty()){
  //              cerr << "Eliminated block #" << k << endl;
                FOREACH(z,dag[k]){
                  if(k<*z){
  //                  cerr << "Which is inconsistent with it dominating block " << *z << " which has higher number" << endl;
                    return;
                  }
                }
              }
            }
          }
        }
      }
    }
  //  cerr << "domination ends" << endl;
    if(conflictId == conflictingBlocks.size()){
    
      vector<int> depsCnt(dominantBlocks.size(),0);
      for(unsigned int i=dag.size();i--;){
        for(unsigned int j=dag[i].size();j--;){
          depsCnt[dag[i][j]]++;
        }
      }

      vector<unsigned int> q;
      for(unsigned int i=depsCnt.size();i--;){
        if(!depsCnt[i]){
          q.push_back(i);
        }
      }

      vector<int> permutation;
      while(!q.empty()){
        unsigned int v=q.back();
        permutation.push_back(v);
        q.pop_back();
        for(unsigned int i=dag[v].size();i--;){
          if(!--depsCnt[dag[v][i]]){
            q.push_back(dag[v][i]);
          }
        }
      }
      if(permutation.size()!=dominantBlocks.size()){
        for(unsigned int i=dag.size();i--;){
          for(unsigned int j=dag[i].size();j--;){
            cerr << "EDGE " << i << " -> " << dag[i][j] << endl;
          }
        }
        for(unsigned int i=depsCnt.size();i--;){
          if(depsCnt[i]){
            cerr << "UNREACHABLE " << i << endl;
          }
        }
      }
      assert(permutation.size()==dominantBlocks.size());
      vector<vector<pair<int,int> > > blocks;
  //    cerr << "Consider permutation of blocks [";
      for(unsigned i=permutation.size();i--;){
  //      cerr << permutation[i] << ",";
        if(dominantBlocks[permutation[i]].empty()){
  //        cerr << "skiping now empty block #" << permutation[i] << endl;
        }else{
          blocks.push_back(dominantBlocks[permutation[i]]);
        }
      }
  //    cerr << "]" << endl;
      visitor.onBlocksSequence(blocks);
  //    cerr << "updated" << endl;
    }else{
      int a=conflictingBlocks[conflictId].first;
      int b=conflictingBlocks[conflictId].second;
      bool irrelevant = dominantBlocks[a].empty() || dominantBlocks[b].empty() ||  dagDfs(dag,a,b) ||dagDfs(dag,b,a);
      /*
      //THIS WHOLE optimization does not seem to work actually (either it has a bug, or tests do not contain such situations, or perhaps this is impossible anyway)
      if(!irrelevant){
        bool reallyCollides = false;
        FOREACH(x,dominantBlocks[a]){
          FOREACH(y,dominantBlocks[b]){
            if(x->first == y->first || x->second==y->second){
              reallyCollides = true;
  //            cerr << "Yes, there still is a collision between " <<  x->first << " --> " << x->second << " and " << y->first << " --> " << y->second << endl;
            }
          }
        }
        if(!reallyCollides){
          cerr << "Ha! blocks #" << a << " and #" << b << " do not really collide any more." << endl;
          irrelevant = true;
        }
      }
      */
      if(irrelevant){
  //      cerr << "Free drink at " << a << " vs " << b << endl;
        allConflicts(conflictId+1,dag,dominantBlocks,conflictingBlocks,visitor, progress*2+1);
      }else{
  //      cerr << "Split at " << a << " vs " << b << endl;
        dag[a].push_back(b);
        allConflicts(conflictId+1,dag,dominantBlocks,conflictingBlocks,visitor, progress*2);
        assert(dag[a].back()==b);
        dag[a].pop_back();
        dag[b].push_back(a);
  //      cerr << "Other option of " << a << " vs " << b << endl;
        allConflicts(conflictId+1,dag,dominantBlocks,conflictingBlocks,visitor, progress*2+1);
      }
  //    cerr << "level up" << endl;
    }
  }
  void dfs(const vector<vector<int> > & g,vector<int> &component,int color,int node){
    if(component[node]==color){
      return;
    }
    component[node]=color;
    FOREACH(nbr,g[node]){
      dfs(g,component,color,*nbr);
    }
  }
  int block_by_edge(const vector<vector<pair<int,int> > > & dominantBlocks,int l,int r){
    for(int i=0;i<dominantBlocks.size();++i){
      if(find(dominantBlocks[i].begin(),dominantBlocks[i].end(),make_pair(l,r))!=dominantBlocks[i].end()){
        return i;
      }
    }
    assert(false);
  }
  unsigned long long pow(unsigned long long x,int k){
    if(k==0)return 1;
    unsigned long long h=pow(x,k/2);
    if(k&1){
      return h*h*x;
    }
    return h*h;
  }
  vector<int> levels_visits;

  bool rumba(
      vector<int> chosenMatchings,
      unsigned long long int &progress,
      int levelId,
      vector<vector<int> > dag,
      const vector<vector<pair<int,int> > > & dominantBlocks,
      const vector< vector<pair< vector<pair<int,int> >, vector<pair< pair<int,int>, pair<int,int> > > > > > levels, // OMG!!!!!111
      OnEachBlocksSequence & visitor){
    if(levels_visits.size()<=levelId){
      levels_visits.resize(levelId+1,0);
    }
    /*
    FOREACH(vis,levels_visits){
      cerr << *vis << '\t';
    }
    cerr << "\r";
    */
    if(levels.size() <= levelId  && levelId<2*levels.size()){
      const pair< vector<pair<int,int> >, vector<pair< pair<int,int>, pair<int,int> > > > * matching = &levels[levelId-levels.size()][chosenMatchings[levelId-levels.size()]];
      unsigned long long int variants = pow(VARIANTS,matching->second.size());
      for(unsigned long long int e=0;e<variants;++e){
        vector<vector<int> > variantDag=dag;
        unsigned long long int f=e;
        bool variantFeasible = true;
        FOREACH(choice,matching->second){
          int which = f%VARIANTS;
          f/=VARIANTS;
          int v[4]={
            choice->first.first,
            choice->first.second,
            choice->second.first,
            choice->second.second
          };

          if(VARIANTS == 3){
            //which is 00,01,10
            if((which&1)){
              swap(v[0],v[1]);
            }
            if((which&2)){
              swap(v[2],v[3]);
            }
            for(int p=0;p<4;p+=2){
              if(dagDfs(variantDag,v[p+1],v[p])){
                variantFeasible=false;
                break;
              }else if(!dagDfs(variantDag,v[p],v[p+1])){
                variantDag[v[p]].push_back(v[p+1]);
              }
            }
          }else{
            //which is 0 or 1
            const int p = which*2;
            if(dagDfs(variantDag,v[p+1],v[p])){
              variantFeasible=false;
            }else if(!dagDfs(variantDag,v[p],v[p+1])){
              variantDag[v[p]].push_back(v[p+1]);
            }
          }
          if(!variantFeasible){
            break;
          }
        }
        if(variantFeasible){
          if(rumba(chosenMatchings,progress,levelId+1,variantDag,dominantBlocks,levels,visitor)){
  //          cerr << "Level " << levelId << " e " << e << " variants " << variants << endl;
            levels_visits[levelId]+=variants-1-e;
            return true;
          }
        }else{
          levels_visits[levelId]++;
  //          cerr << "Infeasible variant so we skip recursion of " << prices[levelId+1] << endl;
  //          progress += prices[levelId+1];
        }
      }
      return false;
    }else if(levelId < levels.size()){
      for(unsigned int mid=0;mid<levels[levelId].size();++mid){
        const pair< vector<pair<int,int> >, vector<pair< pair<int,int>, pair<int,int> > > >* matching = &levels[levelId][mid];
        bool mustsFeasible = true;
        vector<vector<int> > mustsDag=dag;
        FOREACH(m,matching->first){
          if(dagDfs(mustsDag,m->second,m->first)){
            mustsFeasible=false;
            break;
          }else if(!dagDfs(mustsDag,m->first,m->second)){
            mustsDag[m->first].push_back(m->second);
          }
        }
        
        if(mustsFeasible){
          vector<int> newChosenMatchings = chosenMatchings;
          newChosenMatchings.push_back(mid);
          rumba(newChosenMatchings,progress,levelId+1,mustsDag,dominantBlocks,levels,visitor);
        }else{
          levels_visits[levelId]++;
        }
      }
      return false;//who cares...
    }else{
      progress += 1;
  //    cerr << "Leaf is worth " << prices[levelId] << endl;
      vector<int> depsCnt(dominantBlocks.size(),0);
      for(unsigned int i=dag.size();i--;){
        for(unsigned int j=dag[i].size();j--;){
          depsCnt[dag[i][j]]++;
        }
      }

      vector<unsigned int> q;
      for(unsigned int i=depsCnt.size();i--;){
        if(!depsCnt[i]){
          q.push_back(i);
        }
      }

      vector<int> permutation;
      while(!q.empty()){
        unsigned int v=q.back();
        permutation.push_back(v);
        q.pop_back();
        for(unsigned int i=dag[v].size();i--;){
          if(!--depsCnt[dag[v][i]]){
            q.push_back(dag[v][i]);
          }
        }
      }
      if(permutation.size()!=dominantBlocks.size()){
        for(unsigned int i=dag.size();i--;){
          for(unsigned int j=dag[i].size();j--;){
            cerr << "EDGE " << i << " -> " << dag[i][j] << endl;
          }
        }
        for(unsigned int i=depsCnt.size();i--;){
          if(depsCnt[i]){
            cerr << "UNREACHABLE " << i << endl;
          }
        }
      }
      assert(permutation.size()==dominantBlocks.size());
      /* not required in this algorithm
      static set<vector<int> > seen_permutations;
      if(!seen_permutations.insert(permutation).second){
        cerr << "Already saw this permutation" << endl;
        return true;
      }
      */
      vector<vector<pair<int,int> > > blocks;
      reverse(permutation.begin(),permutation.end());
  //    cerr << "Consider permutation of blocks [";
      for(unsigned i=permutation.size();i--;){
  //      cerr << permutation[i] << ",";
        if(dominantBlocks[permutation[i]].empty()){
  //        cerr << "skiping now empty block #" << permutation[i] << endl;
        }else{
          blocks.push_back(dominantBlocks[permutation[i]]);
        }
      }
  //    cerr << "]" << endl;
      visitor.onBlocksSequence(blocks);
      levels_visits[levelId]++;
      return true;
    }
  }

  void foreachDAG(const vector<vector<pair<int,int> > > & dominantBlocks,const vector<pair<int,int> > & conflictingBlocks,OnEachBlocksSequence & visitor){
    // REMARK: why conflictingBlocks?
    (void)conflictingBlocks;
    //1. zrób z dominantBlocks zwykły graf
    int maxLeft = -1;
    int maxRight = -1;
    FOREACH(block,dominantBlocks){
      FOREACH(edge,*block){
        maxLeft = max(maxLeft,edge->first);
        maxRight = max(maxRight,edge->second);
      }
    }
    maxLeft++;
    maxRight++;
    vector<vector<int> > g(maxLeft+maxRight);
    FOREACH(block,dominantBlocks){
      FOREACH(edge,*block){
        g[edge->first].push_back(maxLeft+edge->second);
        g[maxLeft+edge->second].push_back(edge->first);
      }
    }
    //2. wydobądź z tegoż grafu spójne składowe
    vector<int> component(maxLeft+maxRight,0);
    int components = 0;
    for(int i=maxLeft+maxRight;i--;){
      if(!component[i]){
        dfs(g,component,++components,i);
      }
    }
    cerr << "There are " << components << " components" << endl;
    //3. dla każdej spójnej składowej z osobna wypisz wszystkie możliwe matchingi w niej
    set< vector<pair< vector<pair<int,int> >, vector<pair< pair<int,int>, pair<int,int> > > > > > daglettsOptions;
    for(int c=1;c<=components;++c){
      int edges = 0;
      int degs_mul = 1;
      vector<int> nodes;
      for(int i=0;i<g.size();++i)if(component[i]==c){
        int deg = g[i].size();
        cerr << "Options for " << i << " are : " ;
        FOREACH(nbr,g[i]){
          cerr << *nbr << " , ";
        }
        cerr << endl;
        edges += deg;
        degs_mul *= deg;
        nodes.push_back(i);
      }
      assert(edges%2==0);
      edges/=2;
      cerr << "nodes " << nodes.size() << " edges " << edges ;
      cerr << " edges subsets " << (1ULL <<edges) << " nodes spins " << degs_mul << endl;
      set<vector<pair<int,int> > > matchings;
      for(int e=0;e<degs_mul;++e){
        int f=e;
        vector<pair<int,int> > matching;
        FOREACH(node,nodes){
          int deg=g[*node].size();
          int which = f%deg;
          f/=deg;
          int other = g[*node][which];
          {
            int p=0;
            for(int j=0;j<matching.size();++j){
              if(matching[j].second != *node || matching[j].first == other){
                matching[p++]=matching[j];
              }
            }
            matching.resize(p);
          }
          if(*node < other){
            matching.push_back(make_pair(*node,other));
          }
        }
        vector<bool> matched(g.size(),false);
        FOREACH(m,matching){
          matched[m->first]=matched[m->second]=true;
        }
        bool maximal = true;
        FOREACH(node,nodes){
          if(!matched[*node]){
            FOREACH(nbr,g[*node]){
              if(!matched[*nbr]){
                maximal = false;
              }
            }
          }
        }
        if(maximal){
          matchings.insert(matching);
        }
      }
      if(edges>1){
        vector<pair< vector<pair<int,int> >, vector<pair< pair<int,int>, pair<int,int> > > > > dagletts;
        FOREACH(matching,matchings){
          vector<int> matched_with(g.size(),-1);
          cerr << "Found maximal matching : " ;
          FOREACH(m,*matching){
            cerr << m->first << " -> " << m->second << " , ";
          }
          cerr << endl;
          FOREACH(m,*matching){
            matched_with[m->first]=m->second;
            matched_with[m->second]=m->first;
          }
  #define block_by_edge(x,y) block_by_edge(dominantBlocks,min(x,y),max(x,y)-maxLeft)
          vector<pair<int,int> > must;
          vector<pair< pair<int,int>, pair<int,int> > > options;
          FOREACH(node,nodes){
            FOREACH(nbr,g[*node]){
              if(*node < *nbr){
                if(matched_with[*node] != *nbr){
                  assert(matched_with[*node] != -1 || matched_with[*nbr] != -1);
                  int my_block = block_by_edge(*node,*nbr);
                  if(matched_with[*node] == -1){
                    must.push_back(make_pair(block_by_edge(*nbr,matched_with[*nbr]),my_block));
                  }else if(matched_with[*nbr] == -1){
                    must.push_back(make_pair(block_by_edge(*node,matched_with[*node]),my_block));
                  }else{
                    options.push_back(make_pair(
                      make_pair(block_by_edge(*nbr,matched_with[*nbr]),my_block),
                      make_pair(block_by_edge(*node,matched_with[*node]),my_block)
                    ));
                    if(options.back().first > options.back().second){
                      swap(options.back().first,options.back().second);
                    }
                  }                
                }
              }
            }
          }
          sort(must.begin(),must.end());
          sort(options.begin(),options.end());
          cerr << "Musts : " ;
          FOREACH(m,must){
            cerr  << m->first << " < " << m->second << ", ";
          }
          cerr << endl;
          cerr << "Options : " ;
          FOREACH(o,options){
            cerr  << "(" << o->first.first << " < " << o->first.second << " or " << o->second.first << " < " << o->second.second <<")";
          }
          cerr << endl;
          dagletts.push_back(make_pair(must,options));
        }
        if(!daglettsOptions.insert(dagletts).second){
          cerr << "LOL, taki już był" << endl;
        }
      }
    }
    unsigned long long int combinations=1;
    FOREACH(dagletts,daglettsOptions){
      unsigned long long int dags = 0;
      FOREACH(daglett,*dagletts){
        unsigned long long int variants = 1;
        FOREACH(choice,daglett->second){
          variants*=VARIANTS;
        }
        dags+=variants;
      }
      combinations*=dags;
    }
    cerr << "TOooOOootal " << combinations << " combinations" << endl;

    vector< vector<pair< vector<pair<int,int> >, vector<pair< pair<int,int>, pair<int,int> > > > > > levels(daglettsOptions.rbegin(),daglettsOptions.rend());

    unsigned long long int progress=0;
    rumba(vector<int> (0),progress,0,vector<vector<int> > (dominantBlocks.size()),dominantBlocks,levels,visitor);
    cerr << "Finished with progress = " << progress << endl;
  }

  void foreachBlocksSequence(const vector<vector<pair<int,int> > > & dominantBlocks,const vector<pair<int,int> > & conflictingBlocks,OnEachBlocksSequence & visitor){
    //TODO: interleave generation of DAG with verifiying/ensuring that it is a DAG, which should change complexity from O(2^k) to O(something * possible dags)

    int maxPos = -1;
    FOREACH(block,dominantBlocks){
      maxPos = max(maxPos,block->back().first);
    }

    foreachDAG(dominantBlocks,conflictingBlocks,visitor);

    ///allConflicts(0,vector<vector<int> > (dominantBlocks.size(),vector<int>()), dominantBlocks, conflictingBlocks, visitor);
  //  bla(maxPos,0,-1,vector<int>(),dominantBlocks,vector<vector<int> > (dominantBlocks.size(),vector<int> ()),vector<vector<int> > (dominantBlocks.size(),vector<int> ()),visitor);
  }

  Graph getBestMatching(const TextInfoBeta<Atom>& a,const TextInfoBeta<Atom>& b){
    vector<vector<pair<int,int> > > dominantBlocks=getDominantBlocks(a,b);
    vector<pair<int,int> > conflictingBlocks = getConflictingBlocks(dominantBlocks);
    OnEachBlocksSequence visitor(*this,a,b);
    foreachBlocksSequence(dominantBlocks,conflictingBlocks,visitor);
    cerr << "Tested " << visitor.testedSequences << " sequences of which " << visitor.validBrackets << " have valid brackets " << endl;
    return visitor.bestMatching;
  }

  template <typename Output>
  void getCheapest(string aText,string bText,DFA &dfa, Output& output){
    TextInfoBeta<Atom> a = analyzeTextBeta<Atom>(aText,dfa);
    TextInfoBeta<Atom> b = analyzeTextBeta<Atom>(bText,dfa);
    
    Graph matching = getBestMatching(a,b);
    vector<Block> blocks = getBlocks(matching);
    //debugDumpAsHtml(blocks);
    Graph sticks = getBestSticksForBlocks(blocks,a,b);
    //debugDumpAsHtml(sticks,"sticks");
    a.matchInfo.resize(a.size(),make_pair(-1,-1));
    b.matchInfo.resize(b.size(),make_pair(-1,-1));

    int freeMatchId = 0;
    for(unsigned int blockId=0;blockId<blocks.size();++blockId){
      int continuationOfLeft=-1;
      int continuationOfRight=-1;
      int matchId = blockId;
      if (0<sticks.getOutDegree(sticks.left(blockId))) {
        unsigned int prevBlockId=sticks.whichRight(sticks.getOutEdgeEnd(sticks.left(blockId),0));
        continuationOfLeft= matching.whichLeft(blocks[prevBlockId].endSrc()-1);
        continuationOfRight=matching.whichRight(blocks[prevBlockId].endDest()-1);
        assert(a.matchInfo[continuationOfLeft].first == b.matchInfo[continuationOfRight].first);
        //cerr << "changing " << matchId << " to " << a.matchInfo[continuationOfLeft].first << endl;
        matchId = a.matchInfo[continuationOfLeft].first;
      }else{
        matchId = freeMatchId++;
      }
      Block & block= blocks[blockId];

      storeMatch(matching.whichLeft(block.startSrc),block.length,continuationOfLeft,matchId,a.matchInfo);
      storeMatch(matching.whichRight(block.startDest),block.length,continuationOfRight,matchId,b.matchInfo);
    }

    inflateWhitespaces(a,b,matching);
    
    cout << "A:" << endl;
    officialOutput(a.fullMatchInfo, cout);
    cout << "B:" << endl;
    officialOutput(b.fullMatchInfo, cout);

    output << "<h1>Official</h1>";
    output << "<table><tr><td align=\"left\" style=\"padding-right: 100px;\">";
    colorfulOutput(a, output);
    output << "</td><td>";
    colorfulOutput(b, output);
    output << "</td></tr></table>";
    
  }
};
