#ifndef ANALYZETEXT_H
#define ANALYZETEXT_H

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




#endif