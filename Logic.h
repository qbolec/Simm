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
vector<int> range(int from,int to){
  vector<int> x;
  for(int i=from;i<to;++i){
    x.push_back(i);
  }
  return x;
}
#include "Match.h"
#include "TextInfo.h"
TextInfo analyzeText(string text,DFA &dfa){
  TextInfo a;
  a.original_text = text;
  a.states = tagLetters(text,dfa);
  vector<unsigned int> stack;
  for(unsigned int i=0;i<a.states.size();++i){
    if(a.states[i] != DFA::IGNORABLE){
      a.positions.push_back(i);
      a.important_text.push_back(text[i]);
      a.next.push_back(a.positions.size());
      if(a.states[i] == DFA::OPEN_BRACKET){
        stack.push_back(a.positions.size()-1);
      }else if(a.states[i] == DFA::END_BRACKET){
        if(!stack.empty()){
          unsigned int prev = stack.back();
          stack.pop_back();
          assert(a.next.size()>=2);
          assert(a.next.size() == a.positions.size());
          if(a.next.size()-2 != prev){
            a.next[prev] = a.positions.size()-1;
            a.next[a.next.size()-2] = a.states.size();//infinity
          }
        }
      }
    }
  }
  if(!stack.empty()){
    cerr << "failed to match brackets" << endl;
    for(unsigned int i=0;i<a.next.size();++i){
      a.next[i]=i+1;
    }
  }
  return a;
}
pair<unsigned int,vector<unsigned int > > longestMatch(TextInfo a,int offset,TextInfo b){
  pair<unsigned int,vector<unsigned int > > best(0,vector<unsigned int>());
  for(unsigned int start=0;start<b.important_text.length();++start){
    unsigned int length=0;
    unsigned int at = offset;
    unsigned int bt = start;
    while(at<a.important_text.length() && bt<b.important_text.length() && a.important_text[at] == b.important_text[bt]){
      ++length;
      at=a.next[at];
      bt=b.next[bt];
    }
    if(best.first<length){
      best.first = length;
      best.second = vector<unsigned int>();
    }
    if(best.first == length){
      best.second.push_back(start);
    }
  }
  if(best.second.size()>1 && best.first>0){
    assert(best.first > 0);
    unsigned int longest_common=0;
    vector<unsigned int> best2;
    FOREACH(m,best.second){
      unsigned int length=0;
      unsigned int at = offset;
      unsigned int bt = *m;
      while(length+1<best.first){
        ++length;
        at=a.next[at];
        bt=b.next[bt];
      }
      string as = a.important_text.substr(offset,at-offset+1);
      string bs = b.important_text.substr(*m,bt-*m+1);
      unsigned int common_prefix=0;
      unsigned int common_suffix=0;
      while(common_prefix < as.length() && common_prefix < bs.length() && as[common_prefix]==bs[common_prefix]){
        common_prefix++;
        common_prefix++;
      }
      while(common_suffix < as.length() && common_suffix < bs.length() && as[as.length()-1-common_suffix]==bs[bs.length()-1-common_suffix]){
        common_suffix++;
        common_suffix++;
      }
      unsigned int common = common_prefix+common_suffix;
      if(longest_common<common){
        longest_common=common;
        best2.clear();
      }
      if(longest_common==common){
        best2.push_back(*m);
      }
    }
    swap(best2,best.second);
  }
  return best;
}
vector<Match> indexLongestFragments(TextInfo a,int positionIndex,TextInfo b){
  cerr << "indexLongestFragments " << positionIndex << endl;
  vector<Match> matches;
  unsigned int furthest = 0;
  for(unsigned int start = positionIndex;start<a.important_text.length();){
    cerr << "Indexing " << start << endl;
    pair<unsigned int,vector<unsigned int> > lenAndPos = longestMatch(a,start,b);
    unsigned int last = start;
    for(unsigned int i=0;i<lenAndPos.first;++i){
      last = a.next[last];
    }
    if(0<lenAndPos.first && furthest<last){
      furthest=last;
      cerr << "Found match " << start << " " << lenAndPos.first << " " << lenAndPos.second[0] << "(" << lenAndPos.second.size() << endl;
      matches.push_back(Match(start,lenAndPos.second,lenAndPos.first));
    }
    if(a.next[start] >= a.important_text.length()){
      break;
    }
    if(a.next[start] != start+1){
      vector<Match> innerMatches = indexLongestFragments(a,start+1,b);
      FOREACH(match,innerMatches){
        cerr << "Repushing " << match->startSrc << " " << match->length << " " << match->startDest[0] << "(" << match->startDest.size() << endl;
        matches.push_back(*match);
      }
    }
    start = a.next[start];
  }
  cerr << "exit indexLongestFragments" << positionIndex << endl;
  return matches;
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
Graph createGraph(TextInfo a,TextInfo b){
  Graph g(a.important_text.length(),b.important_text.length());
  for(unsigned int i=0;i<a.matches.size();++i){
    cerr << "i=" << i << " of " << a.matches.size() << endl;
    for(unsigned int m=0;m<a.matches[i].startDest.size();++m){
      cerr << "m=" << m << " of " << a.matches[i].startDest.size() << endl;
      unsigned int at=a.matches[i].startSrc;
      unsigned int bt=a.matches[i].startDest[m];
      for(unsigned int p=0;p<a.matches[i].length;++p){
        cerr << "p=" << p << " of " << a.matches[i].length << endl;
        assert(at < a.important_text.length());
        assert(bt < b.important_text.length());
        g.addEdge(g.left(at),g.right(bt));
        at=a.next[at];
        bt=b.next[bt];
      }
    }
  }
  for(unsigned int i=0;i<b.matches.size();++i){
    cerr << "i=" << i << " of " << a.matches.size() << endl;
    for(unsigned int m=0;m<b.matches[i].startDest.size();++m){
      cerr << "m=" << m << " of " << a.matches[i].startDest.size() << endl;
      unsigned int bt=b.matches[i].startSrc;
      unsigned int at=b.matches[i].startDest[m];
      for(unsigned int p=0;p<b.matches[i].length;++p){
        cerr << "p=" << p << " of " << a.matches[i].length << endl;
        assert(at < a.important_text.length());
        assert(bt < b.important_text.length());
        g.addEdge(g.right(bt),g.left(at));
        at=a.next[at];
        bt=b.next[bt];
      }
    }
  }
  return g;
  
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
pair<Graph,Graph> getWeakAndStrong(Graph g){
  Graph strong(g.getLeftSize(),g.getRightSize());
  Graph weak(g.getLeftSize(),g.getRightSize());
  for(unsigned int i=0;i<g.getLeftSize();++i){
    unsigned int id = g.left(i);
    for(unsigned int j=0;j<g.getOutDegree(id);++j){
      unsigned int endId = g.getOutEdgeEnd(id,j);
      if(g.edgeExists(endId,id)){
        strong.addEdge(id,endId);        
      }else{
        weak.addEdge(id,endId);
        weak.addEdge(endId,id);
      }
    }
  }
  for(unsigned int i=0;i<g.getRightSize();++i){
    unsigned int id = g.right(i);
    for(unsigned int j=0;j<g.getOutDegree(id);++j){
      unsigned int endId = g.getOutEdgeEnd(id,j);
      if(g.edgeExists(endId,id)){
        strong.addEdge(id,endId);        
      }else{
        weak.addEdge(id,endId);
        weak.addEdge(endId,id);
      }
    }
  }
  return make_pair(weak,strong);
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
    unsigned int bonus = 0;
    for(unsigned int i=0;i<a.next.size();++i){
      if(a.next[i]!=i+1 && a.next[i]<a.next.size()){
        int pb=getABlock(i);
        int nb=getABlock(a.next[i]);
        if(-1<=pb && -1<=nb && areBlocksConnected(pb,nb,sticks)){
          bonus++;
        }
      }
    }
    for(unsigned int i=0;i<b.next.size();++i){
      if(b.next[i]!=i+1 && b.next[i]<b.next.size()){
        int pb=getBBlock(i);
        int nb=getBBlock(b.next[i]);
        if(-1<=pb && -1<=nb && areBlocksConnected(pb,nb,sticks)){
          bonus++;
        }
      }
    }

    //debugDumpAsHtml(sticks,"sticks");
    unsigned int edgesCount = sticks.getEdgesCount();
    cerr << "Bonus is " << bonus <<  " edgesCount is " << edgesCount << endl;
    assert(edgesCount%2==0);
    if(bestSticksCount < edgesCount*10+bonus){
      bestSticksCount = edgesCount*10+bonus;
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
  unsigned int cost = unmatchedLetters + unmatchedGroups + matches-sticksCount;
  return cost;
}
 
struct ProcessMatching{
  Graph strongMatching;
  TextInfo a;
  TextInfo b;
  Graph bestMatching;
  unsigned int bestCost;
  ProcessMatching(Graph strongMatching,TextInfo a,TextInfo b):strongMatching(strongMatching),a(a),b(b),bestMatching(strongMatching){
    bestCost=cost(strongMatching,a,b);
  }
  void operator()(Graph matching){
    Graph merged = merge(strongMatching,matching);
    //debugDumpAsHtml(strongMatching,"strong matching");
    //debugDumpAsHtml(matching,"matching");
    const unsigned int c = cost(merged,a,b);
    if(c < bestCost){
      bestCost = c;
      bestMatching = merged;
    }    
  }
  Graph getBestMatching(){
    return bestMatching;
  }
};
struct ForEachWeak{
  Graph weak;
  TextInfo a;
  TextInfo b;
  Graph bestMatching;
  unsigned int bestCost;
  ForEachWeak(Graph weak,TextInfo a,TextInfo b):weak(weak),a(a),b(b),bestMatching(weak.getLeftSize(),weak.getRightSize()){
    bestCost = cost(bestMatching,a,b);
  }
  void operator()(Graph strongMatching){
    Graph leftOvers=weak;
    for(unsigned int i=0;i<strongMatching.getLeftSize();++i){
      unsigned int id = strongMatching.left(i);
      for(unsigned int j=0;j<strongMatching.getOutDegree(id);++j){
        unsigned int endId = strongMatching.getOutEdgeEnd(id,j);
        leftOvers.isolate(endId);        
        leftOvers.isolate(id);      
      }
    }
    debugDumpAsHtml(leftOvers,"left overs");
    ProcessMatching judge(strongMatching,a,b);
    foreachMatching(leftOvers,judge);
    Graph matching = judge.getBestMatching();
    unsigned int c=cost(matching,a,b);
    if(c<bestCost){
      bestCost = c;
      bestMatching = matching;
    }
  }
  Graph getBestMatching(){
    return bestMatching;
  }
};
void storeMatch(int from,int len,int continuationOf,int matchId,vector<pair<int,int> > &matchInfo){
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
      cout << i << " open 0" << endl;
      open.push_back(i);
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
void getCheapest(string aText,string bText,DFA &dfa){
  TextInfo a = analyzeText(aText,dfa);
  TextInfo b = analyzeText(bText,dfa);
  a.matches=indexLongestFragments(a,0,b);
  FOREACH(match,a.matches){
    cerr << "a: " << match->startSrc << " " << match->length << " " << match->startDest[0] << "(" << match->startDest.size() << endl;
  }
  b.matches=indexLongestFragments(b,0,a);
  FOREACH(match,b.matches){
    cerr << "b: " << match->startSrc << " " << match->length << " " << match->startDest[0] << "(" << match->startDest.size() << endl;
  }
  Graph g = createGraph(a,b);
  debugDumpAsHtml(g,"g");
  pair<Graph,Graph> weakAndStrong = getWeakAndStrong(g);
  Graph weak = weakAndStrong.first;
  Graph strong = weakAndStrong.second;
  ForEachWeak judge(weak,a,b);
  foreachMatching(strong, judge );
  Graph matching = judge.getBestMatching();
  vector<Block> blocks = getBlocks(matching);
  Graph sticks = getBestSticksForBlocks(blocks,a,b);
  a.matchInfo.resize(a.important_text.length(),make_pair(-1,-1));
  b.matchInfo.resize(b.important_text.length(),make_pair(-1,-1));

  for(unsigned int blockId=0;blockId<blocks.size();++blockId){
    int continuationOfLeft=-1;
    int continuationOfRight=-1;
    int matchId = blockId;
    if (0<sticks.getOutDegree(sticks.left(blockId))) {
      unsigned int prevBlockId=sticks.whichRight(sticks.getOutEdgeEnd(sticks.left(blockId),0));
      continuationOfLeft= matching.whichLeft(blocks[prevBlockId].endSrc()-1);
      continuationOfRight=matching.whichRight(blocks[prevBlockId].endDest()-1);
      matchId = a.matchInfo[continuationOfLeft].first;
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
  
  
  /*
  . add edges between letters
  . mark strong edges
  . match braces
  . foreach matching optimal in strong edges (and edges):
     . find optimal trees for this matching, honoring braces
  */
  
}
