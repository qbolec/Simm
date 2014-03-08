#ifndef ANALYZETEXT_H
#define ANALYZETEXT_H

#ifndef ANALYZE_TEXT_H
#define ANALYZE_TEXT_H

#include "CLikeDFA.h"
#include "TextInfo.h"
#include "Tokenize.h"

vector<DFA::CHARACTER_CLASS> tagLetters(string text,DFA &dfa)
{
    vector<DFA::CHARACTER_CLASS> tags;
    dfa.reset();
    for(unsigned int i=0; i<text.size(); ++i)
    {
        vector<DFA::CHARACTER_CLASS> emitted = dfa.react(text[i]);
        tags.insert(tags.end(),emitted.begin(),emitted.end());
    }
    {
        vector<DFA::CHARACTER_CLASS> emitted = dfa.react(0);
        tags.insert(tags.end(),emitted.begin(),emitted.end());
    }
    //TODO : if there is / at the end of file, the length of tags will not match text
    assert(text.size() == tags.size());
    return tags;
}

template <class Atom>
void matchBrackets(TextInfoBeta<Atom> &textInfo)
{
    vector<unsigned int> stack;
    for(unsigned int i=0; i<textInfo.states.size(); ++i)
    {
        // cerr << " " << i << ": STATE_" << textInfo.states[i] << " letter " << textInfo.original_text[i] << endl;
        if(textInfo.states[i] != DFA::IGNORABLE)
        {
            textInfo.positions.push_back(i);
            textInfo.important_atoms.push_back(textInfo.original_atoms[i]);
            textInfo.next.push_back(textInfo.original_atoms.size());
            if(textInfo.states[i] == DFA::OPEN_BRACKET)
            {
                stack.push_back(textInfo.positions.size()-1);
            }
            else if(textInfo.states[i] == DFA::END_BRACKET)
            {
                if(!stack.empty())
                {
                    unsigned int prev = stack.back();
                    stack.pop_back();
                    assert(textInfo.next.size()>=2);
                    assert(textInfo.next.size() == textInfo.positions.size());
                    textInfo.next[prev] = textInfo.positions.size()-1;
                }
            }
        }
    }
    if(!stack.empty())
    {
        cerr << "failed to match brackets" << endl;
        for(unsigned int i=0; i<textInfo.next.size(); ++i)
        {
            textInfo.next[i]=textInfo.important_atoms.size();
        }
    }
    textInfo.prev.resize(textInfo.important_atoms.size(),textInfo.important_atoms.size());
    for(unsigned int i=0; i<textInfo.next.size(); ++i)
    {
        if(textInfo.next[i] < textInfo.prev.size())
        {
            textInfo.prev[textInfo.next[i]]=i;
        }
    }
}

template <class Atom>
TextInfoBeta<Atom> analyzeTextBeta(string text, DFA& dfa);

template <>
TextInfoBeta<Char> analyzeTextBeta(string text, DFA& dfa)
{
    TextInfoBeta<Char> a;
    a.setText(text);
    for (unsigned int i=0; i<text.size(); i++)
    {
        a.original_atoms.push_back({text[i], (int)i});
    }
    a.states = tagLetters(text,dfa);
    matchBrackets(a);
    return a;
}

template <>
TextInfoBeta<Token> analyzeTextBeta(string text, DFA&)
{
    TextInfoBeta<Token> textInfo;
    textInfo.setText(text);

    vector<Token> tokenized = tokenize(text);
    textInfo.original_atoms = tokenized;

    for (Token token: tokenized)
    {
        textInfo.states.push_back(CharTypeToCharClass(token.type));
    }

    matchBrackets(textInfo);

    return textInfo;
}

#endif

/*
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
*/



#endif