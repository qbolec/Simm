vector<DFA::CHARACTER_CLASS> tagLetters(string text,DFA &dfa){
  vector<DFA::CHARACTER_CLASS> tags;
  dfa.reset();
  for(unsigned int i=0;i<text.length();++i){
    vector<DFA::CHARACTER_CLASS> emitted = dfa.react(text[i]);
    tags.insert(tags.end(),emitted.begin(),emitted.end());
  }
  {
    vector<DFA::CHARACTER_CLASS> emitted = dfa.react(0);
    tags.insert(tags.end(),emitted.begin(),emitted.end());
  }
  //TODO : if there is / at the end of file, the length of tags will not match text
  assert(text.length() == tags.size());
  return tags;
}
#include "Match.h"
#include "TextInfo.h"
TextInfo analyzeText(string text,DFA &dfa){
  TextInfo a;
  a.original_text = text;
  a.states = tagLetters(text,dfa);
  vector<unsigned int> stack;
  for(unsigned int i=0;i<a.states.size();++i){
    cerr << " " << i << ": STATE_" << a.states[i] << " letter " << a.original_text[i] << endl;
    if(a.states[i] != DFA::IGNORABLE){
      a.positions.push_back(i);
      a.important_text.push_back(text[i]);
      a.next.push_back(a.original_text.length());
      if(a.states[i] == DFA::OPEN_BRACKET){
        stack.push_back(a.positions.size()-1);
      }else if(a.states[i] == DFA::END_BRACKET){
        if(!stack.empty()){
          unsigned int prev = stack.back();
          stack.pop_back();
          assert(a.next.size()>=2);
          assert(a.next.size() == a.positions.size());
          a.next[prev] = a.positions.size()-1;
        }
      }
    }
  }
  if(!stack.empty()){
    cerr << "failed to match brackets" << endl;
    for(unsigned int i=0;i<a.next.size();++i){
      a.next[i]=a.important_text.length();
    }
  }
  a.prev.resize(a.important_text.length(),a.important_text.length());
  for(unsigned int i=0;i<a.next.size();++i){
    if(a.next[i] < a.prev.size()){
      a.prev[a.next[i]]=i;
    }
  }
  return a;
}
string makeVisibleChar(char x){
  switch(x){
  case '\n':
    return "&#8617;\n";    
  default:
    string s;
    s+=x;
    return s;
  }
}
void debugDumpAsHtml(TextInfo a){
  cerr << "<style>i{background:yellow;}b{background:black;color:white}</style>";
  cerr << "<h1>original text:</h1>";
  cerr << "<pre>" << a.original_text << "</pre>";
  cerr << "<h1>important text:</h1>";
  cerr << "<pre>" << a.important_text<< "</pre>";
  cerr << "<h1>visualized classes:</h1>";
  cerr << "<pre>";
  for(unsigned int i=0;i<a.original_text.length();++i){
    switch (a.states[i]) {
    case DFA::IGNORABLE:
      cerr << "<i title=" << (int)a.original_text[i] << ">" << makeVisibleChar(a.original_text[i])<< "</i>";
      break;
   case DFA::OPEN_BRACKET:
      cerr << "<b class=open_bracket title=open>" << makeVisibleChar(a.original_text[i])<< "</b>";
      break;
   case DFA::END_BRACKET:
      cerr << "<b class=end_bracket title=close>" << makeVisibleChar(a.original_text[i]) << "</b>";
      break;
   case DFA::IMPORTANT:
     cerr << makeVisibleChar(a.original_text[i]);
     break;
    }
  }
  cerr << "</pre>";
}
void debugDumpAsHtml(Graph g,string name){
  cerr << "<h1>Graph "<<name << "</h1>";
  cerr << "<table><tr><td><pre>";
  int lastId = -1;
  for(unsigned int i=0;i<g.getLeftSize();++i){
    int id=g.left(i);
    cerr << id << ": ";
    for(unsigned int j=0;j<g.getOutDegree(id);++j){
      int endId = g.getOutEdgeEnd(id,j);
      if(endId!=lastId+1){
        cerr << "<b>" << endId << "</b>,";
      }else{
        cerr << endId<< ",";
      }
      lastId = endId;      
    }
    cerr << endl;
  }
  cerr << "</pre></td><td><pre>";
  for(unsigned int i=0;i<g.getRightSize();++i){
    int id=g.right(i);
    cerr << id << ": ";
    for(unsigned int j=0;j<g.getOutDegree(id);++j){
      int endId = g.getOutEdgeEnd(id,j);
      if(endId!=lastId+1){
        cerr << "<b>" << endId << "</b>,";
      }else{
        cerr << endId<< ",";
      }
      lastId = endId;      
    }
    cerr << endl;
  }  
  cerr << "</pre></td></tr></table>";
  
}
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
void copyEdges(Graph a,Graph &b){
  for(unsigned int id=0;id<a.getNodesCount();++id){
    for(unsigned int j=0;j<a.getOutDegree(id);++j){
      unsigned int endId = a.getOutEdgeEnd(id,j);
      b.addEdge(id,endId);
    }
  }  
}
Graph merge(Graph a,Graph b){
  assert(a.getLeftSize()==b.getLeftSize());
  assert(a.getRightSize()==b.getRightSize());
  Graph merged(a.getLeftSize(),a.getRightSize());
  copyEdges(a,merged);
  copyEdges(b,merged);
  return merged;

}
#include "Block.h"
void debugDumpAsHtml(vector<Block> blocks){
  cerr << "<h1>code blocks</h1>";
  cerr << "<pre>";
  FOREACH(block,blocks){
    cerr << block->startSrc << " -> " << block->startDest << " (len: " << block->length << ")<br>";
  }
  cerr << "</pre>";
}
bool areAligned(Block a,Block b){
  return a.startSrc < b.startSrc && a.startDest < b.startDest;
}
bool conflict(unsigned int aLo,unsigned int aHi,unsigned int bLo,unsigned int bHi){
  assert(aHi<aLo);
  assert(bHi<bLo);
  return aHi < bHi && bHi < aLo &&  aLo < bLo ||
    bHi < aHi && aHi < bLo && bLo < aLo;
}
bool conflict(Block aLo,Block aHi,Block bLo,Block bHi){
  return conflict(aLo.startSrc,aHi.startSrc,bLo.startSrc,bHi.startSrc) ||
    conflict(aLo.startDest,aHi.startDest,bLo.startDest,bHi.startDest);
}
template<typename Callback>
void foreachBlocksSticking(vector<Block> blocks,Callback &foo,unsigned int blockId,Graph sticks){
  if(blockId == blocks.size()){
    foo(sticks);
  }else{
    for(unsigned int prevBlockId=0;prevBlockId<blockId;++prevBlockId){
      unsigned int from = sticks.left(blockId);
            
      if(areAligned(blocks[prevBlockId],blocks[blockId])){
        unsigned int to =sticks.right(prevBlockId);             
        if(!sticks.getOutDegree(to)){
          bool stop = false;
          for(unsigned int otherBlockId=0;otherBlockId<blockId;++otherBlockId){
            unsigned int id=sticks.left(otherBlockId);
            for(unsigned int j=0;j<sticks.getOutDegree(id);++j){
              unsigned int endId = sticks.getOutEdgeEnd(id,j);
              unsigned int otherBlockPrevId = sticks.whichRight(endId);
              if(conflict(blocks[blockId],blocks[prevBlockId],blocks[otherBlockId],blocks[otherBlockPrevId])){
                stop=true;
              }
            }
          }
          if(!stop){
            sticks.addEdges(from,to);
            foreachBlocksSticking(blocks,foo,blockId+1,sticks);
            sticks.isolate(to);
          }
        }
      }
    }
    foreachBlocksSticking(blocks,foo,blockId+1,sticks);
  }
}

template<typename Callback>
void foreachBlocksSticking(vector<Block> blocks,Callback &foo){
  Graph sticks(blocks.size(),blocks.size());
  foreachBlocksSticking(blocks,foo,0,sticks);
}
struct JudgeSticking{
  vector<Block> blocks;
  TextInfo a;
  TextInfo b;
  Graph bestSticks;
  unsigned int bestSticksCount;
  JudgeSticking(vector<Block> blocks,TextInfo a,TextInfo b):blocks(blocks),a(a),b(b),bestSticks(blocks.size(),blocks.size()),bestSticksCount(0){
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
    for(unsigned int i=0;i<a.next.size();++i){
      if(a.next[i]<a.next.size()){
        int pb=getABlock(i);
        int nb=getABlock(a.next[i]);
        
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
  foreachBlocksSticking(blocks,judge);
  return judge.getBestSticks();
}
Graph getBestSticksForMatching(Graph matching,TextInfo a,TextInfo b){
  return getBestSticksForBlocks(getBlocks(matching),a,b);
}
unsigned int cost(Graph matching,TextInfo a,TextInfo b){
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
void debugDumpAsHtml(vector<pair<int,int> > matchInfo){
  cerr << "<pre>";
  for(unsigned int i=0;i<matchInfo.size();++i){
    cerr << i << ": " << matchInfo[i].first << ',' << matchInfo[i].second << endl;
  }
  cerr << "</pre>";
}
void naiveInflate(TextInfo &info){
  info.fullMatchInfo = vector<pair<int,int> >(info.original_text.length(),make_pair(-1,-1));
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
void inflateWhitespaces(TextInfo &a,TextInfo &b,Graph matching){
  naiveInflate(a);
  naiveInflate(b);
  
  int expected =0;
  for(unsigned int i=0;i<a.important_text.length();++i){
    if(a.positions[i]!=expected){
      unsigned int start =expected;
      unsigned int end=a.positions[i];
      if(0<matching.getOutDegree(matching.left(i))){
        unsigned int rightEnd= b.positions[matching.whichRight(matching.getOutEdgeEnd(matching.left(i),0))];
        unsigned int rightStart=rightEnd;
        while(0<rightStart && b.states[rightStart-1]==DFA::IGNORABLE && b.fullMatchInfo[rightStart-1].first==-1){
          --rightStart;
        }
        string leftString = a.original_text.substr(start,end-start);
        string rightString = b.original_text.substr(rightStart,rightEnd-rightStart);
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
void officialOutput(vector<pair<int,int> > matchInfo){
  vector<int> open;
  for(unsigned int i=0;i<matchInfo.size();++i){
    if(matchInfo[i].first == -1){
      if(!i || matchInfo[i-1].first!=-1){
        cout << i << " open 0" << endl;
        open.push_back(i);
      }
    }else{
      if(matchInfo[i].second == -1){
        cout << i << " open " << (matchInfo[i].first+1) << endl;
        open.push_back(i);
      }else{
        while(!open.empty() && matchInfo[i].second <open.back()){
          open.pop_back();
          cout << i << " close" << endl;
        }
      }
    }
  }
  while(!open.empty()){
    open.pop_back();
    cout << matchInfo.size() << " close" << endl;
  }
}
void dfs(const TextInfo &a,const TextInfo &b,unsigned int ai,unsigned int bi,int color,vector<vector<int> > &blockId){
  assert(ai < a.important_text.length() && bi < b.important_text.length());
  assert(a.important_text[ai]==b.important_text[bi]);
  if(blockId[ai][bi]==color){
    return;
  }
  assert(blockId[ai][bi]==-1);
  blockId[ai][bi]=color;
  if(0<ai && 0<bi && a.important_text[ai-1]==b.important_text[bi-1]){
    dfs(a,b,ai-1,bi-1,color,blockId);
  }
  if(ai+1<a.important_text.length() && bi+1<b.important_text.length() && a.important_text[ai+1]==b.important_text[bi+1]){
    dfs(a,b,ai+1,bi+1,color,blockId);
  }
  if(a.next[ai]<a.important_text.length() && b.next[bi]<b.important_text.length()){
    assert(a.important_text[a.next[ai]] == b.important_text[b.next[bi]]);
    dfs(a,b,a.next[ai],b.next[bi],color,blockId);
  }
  if(a.prev[ai]<a.important_text.length() && b.prev[bi]<b.important_text.length()){
    assert(a.important_text[a.prev[ai]] == b.important_text[b.prev[bi]]);
    dfs(a,b,a.prev[ai],b.prev[bi],color,blockId);
  }
}
vector<vector<pair<int,int> > > getDominantBlocks(TextInfo a,TextInfo b){
  vector<vector<int> > blockId(a.important_text.length(),vector<int>(b.important_text.length(),-1));
  vector<vector<pair<int,int> > > blocks;
  for(unsigned int ai=0;ai<a.important_text.length();++ai){
    for(unsigned int bi=0;bi<b.important_text.length();++bi){
      if(blockId[ai][bi]<0 && a.important_text[ai]==b.important_text[bi]){
        dfs(a,b,ai,bi,blocks.size(),blockId);
        blocks.push_back(vector<pair<int,int> >());
      }
    }
  }
  for(unsigned int ai=0;ai<a.important_text.length();++ai){
    for(unsigned int bi=0;bi<b.important_text.length();++bi){
      if(0<=blockId[ai][bi]){
        blocks[blockId[ai][bi]].push_back(make_pair(ai,bi));
      }
    }
  }
  vector<bool> isBlockDominant(blocks.size(),false);
  for(unsigned int ai=0;ai<a.important_text.length();++ai){
    size_t longest_length = 0;
    for(unsigned int bi=0;bi<b.important_text.length();++bi){
      if(0<=blockId[ai][bi]){
        longest_length = max(longest_length,blocks[blockId[ai][bi]].size());
      }
    }
    for(unsigned int bi=0;bi<b.important_text.length();++bi){
      if(0<=blockId[ai][bi] && longest_length == blocks[blockId[ai][bi]].size()){
        isBlockDominant[blockId[ai][bi]]=true;
      }
    }
  }
  for(unsigned int bi=0;bi<b.important_text.length();++bi){
    size_t longest_length = 0;
    for(unsigned int ai=0;ai<a.important_text.length();++ai){
      if(0<=blockId[ai][bi]){
        longest_length = max(longest_length,blocks[blockId[ai][bi]].size());
      }
    }
    for(unsigned int ai=0;ai<a.important_text.length();++ai){
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
      cerr << " " << dominantBlocks[i][j].first << ": " << a.important_text[dominantBlocks[i][j].first] << "  -- " << dominantBlocks[i][j].second << ": " << b.important_text[dominantBlocks[i][j].second] << endl;
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
      if(a.important_text.length() <= shortBlocks[i][j].first || b.important_text.length() <= shortBlocks[i][j].second){
        cerr << "WTF is this: " << shortBlocks[i][j].first << " ---> " << shortBlocks[i][j].second << endl;
      }
      cerr << " " << shortBlocks[i][j].first << ": " << a.important_text[shortBlocks[i][j].first] << "  -- " << shortBlocks[i][j].second << ": " << b.important_text[shortBlocks[i][j].second] << endl;
    }
  }
  
  for(bool changed=true;changed;){
    changed=false;
    vector<int> aCoverage(a.important_text.length(),0);
    vector<int> bCoverage(b.important_text.length(),0);
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
                    shortBlocks[i][first_j].first <= shortBlocks[k].back().first && shortBlocks[k].back().first <= shortBlocks[i][last_j].first ||
                    shortBlocks[i][first_j].second <= shortBlocks[k].back().second && shortBlocks[k].back().second <= shortBlocks[i][last_j].second
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

  cerr << "There are " << shortBlocks.size() << " final short blocks" << endl;
  for(unsigned int i=0;i<shortBlocks.size();++i){
    cerr << "Short block #" << i << " is:" << endl;
    for(unsigned int j=0;j<shortBlocks[i].size();++j){
      if(a.important_text.length() <= shortBlocks[i][j].first || b.important_text.length() <= shortBlocks[i][j].second){
        cerr << "WTF is this: " << shortBlocks[i][j].first << " ---> " << shortBlocks[i][j].second << endl;
      }
      cerr << " " << shortBlocks[i][j].first << ": " << a.important_text[shortBlocks[i][j].first] << "  -- " << shortBlocks[i][j].second << ": " << b.important_text[shortBlocks[i][j].second] << endl;
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
  TextInfo &a;
  TextInfo &b;
  Graph bestMatching;
  unsigned int bestCost;
  unsigned long long testedSequences;
  OnEachBlocksSequence(TextInfo &a,TextInfo &b):a(a),b(b),bestMatching(a.important_text.length(),b.important_text.length()),bestCost(1e9),testedSequences(0){
  }
  void onBlocksSequence(const vector<vector<pair<int,int> > > &blocks){
    testedSequences++;
    Graph matching(a.important_text.length(),b.important_text.length());
    for(unsigned int i=0;i<blocks.size();++i){
      for(unsigned int j=0;j<blocks[i].size();++j){
        unsigned int left=matching.left(blocks[i][j].first);
        unsigned int right=matching.right(blocks[i][j].second);
        if(0==matching.getOutDegree(left) && 0==matching.getOutDegree(right)){
          matching.addEdges(left,right);
        }
      }
    }
    for(unsigned int i=0;i<a.important_text.length();++i){
      if(a.next[i] < a.important_text.length()){
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
    //debugDumpAsHtml(matching,"Candidate matching");
    unsigned int currentCost = cost(matching,a,b);
    //cerr << "cost of this matching is " << currentCost << endl;
    if(currentCost < bestCost){
      bestCost = currentCost;
      bestMatching = matching;
    }
  }
};

bool include(vector<int> &v,int x){
  if(find(v.begin(),v.end(),x)==v.end()){
    v.push_back(x);
    return true;
  }else{
    return false;
  }
}
bool exclude(vector<int> &v,int x){
  if(find(v.begin(),v.end(),x)==v.end()){
    return false;
  }else{
    v.erase(find(v.begin(),v.end(),x));
    return true;
  }
}
void eliminate(vector<vector<int> > & dag,int i){
  for(unsigned int j=dag.size();j--;){
    if(find(dag[j].begin(),dag[j].end(),i)!=dag[j].end()){
      assert(j!=i);
      for(unsigned int k=dag[i].size();k--;){
        include(dag[j],dag[i][k]);
      }
      exclude(dag[j],i);
    }
  }
  dag[i].clear();
}
void bla(const int maxPos,unsigned int leftPos,int winner,vector<int> activeIds, const vector<vector<pair<int,int> > > & dominantBlocks,vector<vector<int> > activeDag,vector<vector<int> > dag,OnEachBlocksSequence & visitor){
 // cerr << "bla " << maxPos << " leftPos " << leftPos << " winner " << winner << " activeIds " << activeIds.size() << endl;
  if (maxPos < leftPos) {
    //cerr << "bottom" << endl;

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
    //cerr << "Consider permutation of blocks [";
    for(unsigned i=permutation.size();i--;){
     // cerr << permutation[i] << ",";
      blocks.push_back(dominantBlocks[permutation[i]]);
    }
    //cerr << "]" << endl;
    visitor.onBlocksSequence(blocks);
  } else {
    //1. pewne bloki się kończą na literze leftPos-1
    //   zostawiamy je w DAGu żeby zachować wiedzę o przechodniości/porządku (?)
    //   oznaczamy je jako inactive
    for(unsigned int i=dominantBlocks.size();i--;){
      if(dominantBlocks[i].back().first+1 == leftPos){
        exclude(activeIds,i);
        eliminate(activeDag,i);
      }
    }
    //3. mamy pewien zbiór bloków które są aktywne i nic aktywnego ich nie zasłania (czyli chyba potrzebujemy wyindukować sobie DAG aktywnych bloków)
    //   w zasadzie każdy z nich może wygrać

    vector<unsigned int> possibleWinners;
    FOREACH(i,activeIds){
      if(activeDag[*i].empty()){
        possibleWinners.push_back(*i);
      }
    }

    //2. pewne bloki się zaczynają na literze leftPos
    //   dodajemy je do DAGu jako izolowane i aktywne(?)
    for(unsigned int i=dominantBlocks.size();i--;){
      if(dominantBlocks[i].front().first == leftPos){
        activeIds.push_back(i);
        possibleWinners.push_back(i);
      }
    }

    //cerr << "Possible Winners:" << possibleWinners.size() << endl;
    //   każdego możliwego zwycięzce rozpatrujemy rekurencyjnie poprzez dodanie krawędzi potwierdzających jego zwycięztwo nad pozostałymi
    FOREACH(i,possibleWinners){
      //4. wydaje mi się, że powinniśmy też jakby śledzić kto jest zwycięzcą głównie po to, by zabronić mu powrotu do władzy, gdy już raz przegra 
      //cerr << "coping" << endl;
      vector<int>  new_activeIds=activeIds;
      vector<vector<int> > new_activeDag=activeDag;
      vector<vector<int> > new_dag=dag;
      //cerr << "adding deps" << endl;
      FOREACH(j,possibleWinners)if(*i!=*j){
        //cerr << "NEW EDGE FROM " << *j << " TO " << *i << endl;
        include(new_dag[*j],*i);
        include(new_activeDag[*j],*i);
      }
      if(*i!=winner && winner!=-1){
        //cerr << "removing winner" << endl;
        exclude(new_activeIds,winner);
        //cerr << "eliminating" << endl;
        eliminate(new_activeDag,winner);
      }
      //cerr << "recursing" << endl;

      bla(maxPos,leftPos+1,*i,new_activeIds,dominantBlocks,new_activeDag,new_dag,visitor);
    }
    if(possibleWinners.empty()){
      //cerr << "empty case" << endl;
      bla(maxPos,leftPos+1,-1,activeIds,dominantBlocks,activeDag,dag,visitor);
    }
    
  }



}
void foreachBlocksSequence(const vector<vector<pair<int,int> > > & dominantBlocks,const vector<pair<int,int> > & conflictingBlocks,OnEachBlocksSequence & visitor){
  //TODO: interleave generation of DAG with verifiying/ensuring that it is a DAG, which should change complexity from O(2^k) to O(something * possible dags)

  int maxPos = -1;
  FOREACH(block,dominantBlocks){
    maxPos = max(maxPos,block->back().first);
  }
  bla(maxPos,0,-1,vector<int>(),dominantBlocks,vector<vector<int> > (dominantBlocks.size(),vector<int> ()),vector<vector<int> > (dominantBlocks.size(),vector<int> ()),visitor);
}
Graph getBestMatching(TextInfo a,TextInfo b){
  vector<vector<pair<int,int> > > dominantBlocks=getDominantBlocks(a,b);
  vector<pair<int,int> > conflictingBlocks = getConflictingBlocks(dominantBlocks);
  OnEachBlocksSequence visitor(a,b);
  foreachBlocksSequence(dominantBlocks,conflictingBlocks,visitor);
  cerr << "Tested " << visitor.testedSequences << " sequences!" << endl;
  return visitor.bestMatching;
}
void getCheapest(string aText,string bText,DFA &dfa){
  TextInfo a = analyzeText(aText,dfa);
  TextInfo b = analyzeText(bText,dfa);
  
  Graph matching = getBestMatching(a,b);
  vector<Block> blocks = getBlocks(matching);
  //debugDumpAsHtml(blocks);
  Graph sticks = getBestSticksForBlocks(blocks,a,b);
  //debugDumpAsHtml(sticks,"sticks");
  a.matchInfo.resize(a.important_text.length(),make_pair(-1,-1));
  b.matchInfo.resize(b.important_text.length(),make_pair(-1,-1));

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
  


  cerr << "<h1>Official</h1>";
  cerr << "<pre>";
  cout << "A:" << endl;
  officialOutput(a.fullMatchInfo);
  cout << "B:" << endl;
  officialOutput(b.fullMatchInfo);
  cerr << "</pre>";
  
  
}
