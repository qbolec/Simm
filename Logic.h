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
  unsigned int startDest;
  unsigned int length;
  Match(unsigned int startSrc,unsigned int startDest,unsigned int length):
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
pair<unsigned int,unsigned int> longestMatch(string prefix,string body){
  pair<unsigned int,unsigned int> best(0,0);
  for(unsigned int start=0;start<body.length();++start){
    unsigned int length;
    for(length=0;start+length<body.length() && length<prefix.length() && prefix[length]==body[start+length];++length){
    }
    if(best.first<length){
      best.first = length;
      best.second = start;
    }
  }
  return best;
}
vector<Match> indexLongestFragments(string a,string b){
  vector<Match> matches;
  unsigned int furthest = 0;
  for(unsigned int start = 0;start<a.length();++start){
    pair<unsigned int,unsigned int> lenAndPos = longestMatch(a.substr(start),b);
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
    for(unsigned int p=0;p<a.matches[i].length;++p){
      g.addEdge(g.left(a.matches[i].startSrc+p),g.right(a.matches[i].startDest+p));
    }
  }
  for(unsigned int i=0;i<b.matches.size();++i){
    for(unsigned int p=0;p<b.matches[i].length;++p){
      g.addEdge(g.right(b.matches[i].startSrc+p),g.left(b.matches[i].startDest+p));
    }
  }
  return g;
  
}
void debugDumpAsHtml(Graph g){
  cout << "<h1>Graph</h1>";
  cout << "<table><tr><td><pre>";
  int lastId = -1;
  for(unsigned int i=0;i<g.getLeftSize();++i){
    int id=g.left(i);
    cout << i << ": ";
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
  debugDumpAsHtml(g);
  /*
  . add edges between letters
  . mark strong edges
  . match braces
  . foreach matching optimal in strong edges (and edges):
     . find optimal trees for this matching, honoring braces
  */
  
  
  
  return s;
}