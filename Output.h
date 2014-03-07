#ifndef OUTPUT
#define OUTPUT

#include "Utilities.h"
#include "TextInfo.h"
#include "Block.h"
#include "Graph.h"

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

void debugDumpAsHtml(vector<Block> blocks){
  cerr << "<h1>code blocks</h1>";
  cerr << "<pre>";
  FOREACH(block,blocks){
    cerr << block->startSrc << " -> " << block->startDest << " (len: " << block->length << ")<br>";
  }
  cerr << "</pre>";
}
void debugDumpAsHtml(vector<pair<int,int> > matchInfo){
  cerr << "<pre>";
  for(unsigned int i=0;i<matchInfo.size();++i){
    cerr << i << ": " << matchInfo[i].first << ',' << matchInfo[i].second << endl;
  }
  cerr << "</pre>";
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

#endif // OUTPUT
