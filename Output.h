#ifndef OUTPUT
#define OUTPUT

#include "Utilities.h"
#include "TextInfo.h"
#include "Block.h"
#include "Graph.h"


struct FileWriter
{
    std::fstream file;
    bool printDiagnostic;
    FileWriter(const std::string &name, bool diagnostic = true) :
        file(name.c_str(), std::fstream::out), printDiagnostic(diagnostic) {}
    ~FileWriter()
    {
        file.close();
    }
};
template <typename T>
FileWriter& operator<<(FileWriter& fw, const T& elem)
{
    if (fw.printDiagnostic) cerr << elem;
    fw.file << elem;
    return fw;
}


template <typename Output, class Atom>
void debugDumpAsHtml(TextInfoBeta<Atom> a, Output& output){
  output << "<style>i{background:yellow;}b{background:black;color:white}</style>";
  output << "<h1>original text:</h1>";
  output << "<pre>" << a.original_text << "</pre>";
  output << "<h1>important text:</h1>";
  output << "<pre>" << a.important_text<< "</pre>";
  output << "<h1>visualized classes:</h1>";
  output << "<pre>";
  for(unsigned int i=0;i<a.origSize();++i){
    switch (a.states[i]) {
    case DFA::IGNORABLE:
      output << "<i title=" << (int)a.atomOrigValue(i) << ">" << makeVisibleChar(a.atomOrigValue(i))<< "</i>";
      break;
   case DFA::OPEN_BRACKET:
      output << "<b class=open_bracket title=open>" << makeVisibleChar(a.atomOrigValue(i))<< "</b>";
      break;
   case DFA::END_BRACKET:
      output << "<b class=end_bracket title=close>" << makeVisibleChar(a.atomOrigValue(i)) << "</b>";
      break;
   case DFA::IMPORTANT:
     output << makeVisibleChar(a.atomOrigValue(i));
     break;
    }
  }
  output << "</pre>";
}
template <typename Output>
void debugDumpAsHtml(Graph g,string name, Output& output){
  output << "<h1>Graph "<<name << "</h1>";
  output << "<table><tr><td><pre>";
  int lastId = -1;
  for(unsigned int i=0;i<g.getLeftSize();++i){
    int id=g.left(i);
    output << id << ": ";
    for(unsigned int j=0;j<g.getOutDegree(id);++j){
      int endId = g.getOutEdgeEnd(id,j);
      if(endId!=lastId+1){
        output << "<b>" << endId << "</b>,";
      }else{
        output << endId<< ",";
      }
      lastId = endId;      
    }
    output << "\n";
  }
  output << "</pre></td><td><pre>";
  for(unsigned int i=0;i<g.getRightSize();++i){
    int id=g.right(i);
    output << id << ": ";
    for(unsigned int j=0;j<g.getOutDegree(id);++j){
      int endId = g.getOutEdgeEnd(id,j);
      if(endId!=lastId+1){
        output << "<b>" << endId << "</b>,";
      }else{
        output << endId<< ",";
      }
      lastId = endId;      
    }
    output << "\n";
  }  
  output << "</pre></td></tr></table>";
  
}

template <typename Output>
void debugDumpAsHtml(vector<Block> blocks, Output& output){
  output << "<h1>code blocks</h1>";
  output << "<pre>";
  FOREACH(block,blocks){
    output << block->startSrc << " -> " << block->startDest << " (len: " << block->length << ")<br>";
  }
  output << "</pre>";
}
template <typename Output>
void debugDumpAsHtml(vector<pair<int,int> > matchInfo, Output& output){
  output << "<pre>";
  for(unsigned int i=0;i<matchInfo.size();++i){
    output << i << ": " << matchInfo[i].first << ',' << matchInfo[i].second << "\n";
  }
  output << "</pre>";
}

template <typename Output>
void officialOutput(vector<pair<int,int> > matchInfo, Output& output){
  vector<int> open;
  for(unsigned int i=0;i<matchInfo.size();++i){
    if(matchInfo[i].first == -1){
      if(!i || matchInfo[i-1].first!=-1){
        output << i << " open 0" << "\n";
        open.push_back(i);
      }
    }else{
      if(matchInfo[i].second == -1){
        output << i << " open " << (matchInfo[i].first+1) << "\n";
        open.push_back(i);
      }else{
        while(!open.empty() && matchInfo[i].second <open.back()){
          open.pop_back();
          output << i << " close" << "\n";
        }
      }
    }
  }
  while(!open.empty()){
    open.pop_back();
    output << matchInfo.size() << " close" << "\n";
  }
}


template <typename Output, class Atom>
void colorfulOutput(const TextInfoBeta<Atom> &info, Output& output)
{
    static vector<std::string> colors = {"black", "blue", "lime", "red", "orange", "yellow", "brown"};
    vector<int> open;
    int last = 0;
    output << "<b><pre><font size=\"6\">\n";
    for(unsigned int i=0; i<info.fullMatchInfo.size(); ++i)
    {
        if(info.fullMatchInfo[i].first == -1)
        {
            if(!i || info.fullMatchInfo[i-1].first!=-1)
            {
                output << info.atomOrigSeqValue(last,i-last);
                output << "<font color=\"" << colors[0] << "\">";
                open.push_back(i);
                last = i;
            }
        }
        else
        {
            if (info.fullMatchInfo[i].second == -1)
            {
                output << info.atomOrigSeqValue(last,i-last);
                output << "<font color=\"" << colors[(info.fullMatchInfo[i].first) % (colors.size()-1) + 1] << "\">";
                open.push_back(i);
                last = i;
            }
            else
            {
                while(!open.empty() && info.fullMatchInfo[i].second <open.back())
                {
                    open.pop_back();
                    output << info.atomOrigSeqValue(last,i-last);
                    output << "</font>";
                    last = i;
                }
            }
        }
    }
    while(!open.empty())
    {
        open.pop_back();
        output << info.atomOrigSeqValue(last,info.size()-1-last);
        last = info.origSize()-1;
        output << "</font>";
    }
    output << "</font></pre></b>";
}

template <typename Output>
void printColorfulTokens(const string& text, const vector<Token>& tokens, Output& output)
{
    using namespace CharacterType;
    output << "<b><pre><font size=\"6\">\n";
    static vector<std::string> colors = {"black", "blue", "lime", "red", "orange", "brown"};
    for (int i=0, cind=0; i<tokens.size(); i++)
    {
        output << "<font color=\"" << colors[cind] << "\">";
        output << text.substr(tokens[i].start, tokens[i].length);
        if (tokens[i].type == INDENT) {
          output << "<font color=\"black\">`</font>";
        }
        output << "</font>";
        if (tokens[i].type != WHITESPACE && tokens[i].type != INDENT)
        {
            cind=(cind+1)%colors.size();
        }
    }
    output << "</font></pre></b>";
}


void debugDumpAsText(const Graph &graph, string name)
{
    cerr << "Presenting graph " << name << endl;
    for (unsigned int left=0; left<graph.getLeftSize(); left++)
    {
        int leftNode = graph.left(left);
        cerr << left << " -> ";
        for (unsigned int j=0; j<graph.getOutDegree(leftNode); j ++)
        {
            int right = graph.whichRight(graph.getOutEdgeEnd(leftNode,j));
            cerr << (j==0?"":", ") << right;
        }
        cerr << endl;
    }
}

#endif // OUTPUT
