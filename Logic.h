vector<DFA::CHARACTER_CLASS> tagLetters(string text,DFA &dfa){
  vector<DFA::CHARACTER_CLASS> tags;
  dfa.reset();
  for(unsigned int i=0;i<text.length();++i){
    auto emitted = dfa.react(text[i]);
    tags.insert(tags.end(),emitted.begin(),emitted.end());
  }
  {
    auto emitted = dfa.react(0);
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
struct Match{
  unsigned int startSrc;
  vector<unsigned int> startDest;
  unsigned int length;
  Match(unsigned int startSrc,vector<unsigned int> startDest,unsigned int length):
    startSrc(startSrc),startDest(startDest),length(length){
  };
};
struct TextInfo{
  string important_text;
  string original_text;
  vector<unsigned int> positions;
  vector<DFA::CHARACTER_CLASS> states;
  vector<Match> matches;
  vector<pair<unsigned int,pair<bool,unsigned int> > >  points;
};
TextInfo analyzeText(string text,DFA &dfa){
  TextInfo a;
  a.original_text = text;
  a.states = tagLetters(text,dfa);
  for(unsigned int i=0;i<a.states.size();++i){
    if(a.states[i] != DFA::IGNORABLE){
      a.positions.push_back(i);
      a.important_text.push_back(text[i]);
    }
  }
  return a;
}
pair<unsigned int,vector<unsigned int > > longestMatch(string prefix,string body){
  pair<unsigned int,vector<unsigned int > > best(0,vector<unsigned int>());
  for(unsigned int start=0;start<body.length();++start){
    unsigned int length;
    for(length=0;start+length<body.length() && length<prefix.length() && prefix[length]==body[start+length];++length){
    }
    if(best.first<length){
      best.first = length;
      best.second = vector<unsigned int>();
    }
    if(best.first == length){
      best.second.push_back(start);
    }
  }
  return best;
}
vector<Match> indexLongestFragments(string a,string b){
  vector<Match> matches;
  unsigned int furthest = 0;
  for(unsigned int start = 0;start<a.length();++start){
    pair<unsigned int,vector<unsigned int> > lenAndPos = longestMatch(a.substr(start),b);
    if(0<lenAndPos.first && furthest<start+lenAndPos.first){
      furthest=start+lenAndPos.first;
      matches.push_back(Match(start,lenAndPos.second,lenAndPos.first));
    }
  }
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
  cout << "<style>i{background:yellow;}b{background:black;color:white}</style>";
  cout << "<h1>original text:</h1>";
  cout << "<pre>" << a.original_text << "</pre>";
  cout << "<h1>important text:</h1>";
  cout << "<pre>" << a.important_text<< "</pre>";
  cout << "<h1>visualized classes:</h1>";
  cout << "<pre>";
  for(unsigned int i=0;i<a.original_text.length();++i){
    switch (a.states[i]) {
    case DFA::IGNORABLE:
      cout << "<i title=" << (int)a.original_text[i] << ">" << makeVisibleChar(a.original_text[i])<< "</i>";
      break;
   case DFA::OPEN_BRACKET:
      cout << "<b class=open_bracket title=open>" << makeVisibleChar(a.original_text[i])<< "</b>";
      break;
   case DFA::END_BRACKET:
      cout << "<b class=end_bracket title=close>" << makeVisibleChar(a.original_text[i]) << "</b>";
      break;
   case DFA::IMPORTANT:
     cout << makeVisibleChar(a.original_text[i]);
     break;
    }
  }
  cout << "</pre>";
  cout << "<h1>visualized fragments:</h1>";
  cout << "<pre>";

  unsigned int waiting = 0;
  set<unsigned int> opened;
  for(unsigned int i=0;i<a.important_text.length();++i){
    bool needsChange = (waiting< a.points.size() && a.points[waiting].first == i);
    while(waiting< a.points.size() && a.points[waiting].first == i){
      if(!a.points[waiting].second.first){
        opened.erase(a.points[waiting].second.second);
        cout << "<b>}" << a.points[waiting].second.second << "</b>";
      }else{
        opened.insert(a.points[waiting].second.second);
        cout << "<b>" << a.points[waiting].second.second << "{</b>";
      }
      waiting++;
    }
    cout << makeVisibleChar(a.important_text[i]);
  }
  while(waiting< a.points.size() && a.points[waiting].first == a.important_text.length()){
    assert(!a.points[waiting].second.first);
    opened.erase(a.points[waiting].second.second);
    cout << "<b>}" << a.points[waiting].second.second << "</b>";
    waiting++;
  }
  assert(opened.empty());
  cout << "</pre>";
  
}
vector<pair<unsigned int,pair<bool,unsigned int> > > computeSplitPoints(vector<Match> &matches){
  vector<pair<unsigned int,pair<bool,unsigned int> > > points;
  for(unsigned int i=0;i<matches.size();++i){
    points.push_back(make_pair(matches[i].startSrc,make_pair(true,i)));
    points.push_back(make_pair(matches[i].startSrc+matches[i].length,make_pair(false,i)));
  }
  sort(points.begin(),points.end());
  return points;
}
Graph createGraph(TextInfo a,TextInfo b){
  Graph g(a.important_text.length(),b.important_text.length());
  for(unsigned int i=0;i<a.matches.size();++i){
    for(unsigned int m=0;m<a.matches[i].startDest.size();++m){
      for(unsigned int p=0;p<a.matches[i].length;++p){
        g.addEdge(g.left(a.matches[i].startSrc+p),g.right(a.matches[i].startDest[m]+p));
      }
    }
  }
  for(unsigned int i=0;i<b.matches.size();++i){
    for(unsigned int m=0;m<b.matches[i].startDest.size();++m){
      for(unsigned int p=0;p<b.matches[i].length;++p){
        g.addEdge(g.right(b.matches[i].startSrc+p),g.left(b.matches[i].startDest[m]+p));
      }
    }
  }
  return g;
  
}
void debugDumpAsHtml(Graph g,string name){
  cout << "<h1>Graph "<<name << "</h1>";
  cout << "<table><tr><td><pre>";
  int lastId = -1;
  for(unsigned int i=0;i<g.getLeftSize();++i){
    int id=g.left(i);
    cout << id << ": ";
    for(unsigned int j=0;j<g.getOutDegree(id);++j){
      int endId = g.getOutEdgeEnd(id,j);
      if(endId!=lastId+1){
        cout << "<b>" << endId << "</b>,";
      }else{
        cout << endId<< ",";
      }
      lastId = endId;      
    }
    cout << endl;
  }
  cout << "</pre></td><td><pre>";
  for(unsigned int i=0;i<g.getRightSize();++i){
    int id=g.right(i);
    cout << id << ": ";
    for(unsigned int j=0;j<g.getOutDegree(id);++j){
      int endId = g.getOutEdgeEnd(id,j);
      if(endId!=lastId+1){
        cout << "<b>" << endId << "</b>,";
      }else{
        cout << endId<< ",";
      }
      lastId = endId;      
    }
    cout << endl;
  }  
  cout << "</pre></td></tr></table>";
  
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
    unsigned int id=g.left(leftPos);
    bool canSkip = true;
    for(unsigned int i=0;i<g.getOutDegree(id);++i){
      unsigned int endId = g.getOutEdgeEnd(id,i);
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
struct ProcessMatching{
  Graph strongMatching;
  ProcessMatching(Graph strongMatching):strongMatching(strongMatching){}
  void operator()(Graph matching){
    Graph merged = merge(strongMatching,matching);
    //debugDumpAsHtml(strongMatching,"strong matching");
    //debugDumpAsHtml(matching,"matching");
    debugDumpAsHtml(merged,"merged");
  }
};
struct ForEachWeak{
  Graph weak;
  ForEachWeak(Graph weak):weak(weak){}
  void operator()(Graph matching){
    Graph leftOvers=weak;
    for(unsigned int i=0;i<matching.getLeftSize();++i){
      unsigned int id = matching.left(i);
      for(unsigned int j=0;j<matching.getOutDegree(id);++j){
        unsigned int endId = matching.getOutEdgeEnd(id,j);
        leftOvers.isolate(endId);        
        leftOvers.isolate(id);      
      }
    }
    //debugDumpAsHtml(leftOvers,"left overs");

    foreachMatching(leftOvers,ProcessMatching(matching));
  }
};
State getCheapest(State s,DFA &dfa){
  TextInfo a = analyzeText(s.a->innerText,dfa);
  TextInfo b = analyzeText(s.b->innerText,dfa);
  a.matches=indexLongestFragments(a.important_text,b.important_text);
  b.matches=indexLongestFragments(b.important_text,a.important_text);
  a.points = computeSplitPoints(a.matches);
  b.points = computeSplitPoints(b.matches);
  cout << "<table><tr><td>";
  debugDumpAsHtml(a);
  cout << "</td><td>";
  debugDumpAsHtml(b);
  cout << "</td></tr></table>";
  Graph g = createGraph(a,b);
  debugDumpAsHtml(g,"g");
  auto weakAndStrong = getWeakAndStrong(g);
  Graph weak = weakAndStrong.first;
  debugDumpAsHtml(weak,"weak");
  Graph strong = weakAndStrong.second;
  debugDumpAsHtml(strong,"strong");

  foreachMatching(strong, ForEachWeak(weak));
  
  /*
  . add edges between letters
  . mark strong edges
  . match braces
  . foreach matching optimal in strong edges (and edges):
     . find optimal trees for this matching, honoring braces
  */
  
  
  
  return s;
}