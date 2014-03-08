#ifndef TEXTINFO_H
#define TEXTINFO_H


#include "Match.h"
#include "CLikeDFA.h"

/*
struct TextInfo{
  string important_text;
  string original_text;
  vector<unsigned int> positions;
  vector<DFA::CHARACTER_CLASS> states;
  vector<Match> matches;
  vector<unsigned int> next;
  vector<unsigned int> prev;

  vector<pair<int,int> > matchInfo;
  vector<pair<int,int> > fullMatchInfo;
};
*/

namespace CharacterType
{
enum Type
{
    NOTYPE = 0,
    OTHER,
    ALPHA,
    NUMERIC,
    OPERATOR,
    QUOTE,
    OPEN_BRACKET,
    CLOSE_BRACKET,
    INDENT,
    WHITESPACE
};
}

struct Token
{
    int start;
    int length;
    CharacterType::Type type;
    Token(int s=-1, int l=0, CharacterType::Type t=CharacterType::OTHER) : start(s), length(l), type(t) {}
};
struct Char
{
    char value;
    int position;
};

template <typename Atom>
struct TextInfoBeta
{
private:
    string original_text;
public:
    void setText(const string &text) {
      original_text = text;
    }
    vector<Atom> original_atoms, important_atoms;
    vector<unsigned int> positions; // position of important atom in original atom

    string atomValue(int i) const;
    string atomOrigValue(int i) const;

    string atomSeqValue(int beg, int len) const {
        string ret;
        for (unsigned int i=beg; i<beg+len && i<important_atoms.size(); i++) {
            ret += atomValue(i);
        }
        return ret;
    }
    string atomOrigSeqValue(int beg, int len) const {
        string ret;
        for (unsigned int i=beg; i<beg+len && i<original_atoms.size(); i++) {
            ret += atomOrigValue(i);
        }
        return ret;
    }


    vector<unsigned int> prev, next;

    vector<pair<int,int> > matchInfo;
    vector<pair<int,int> > fullMatchInfo;

    vector<DFA::CHARACTER_CLASS> states;

    int size() const { return important_atoms.size(); }
    int origSize() const { return original_atoms.size(); }
};
// important:
template <>
string TextInfoBeta<Token>::atomValue(int i) const {
    const Token& atom = important_atoms[i];
    return original_text.substr(atom.start, atom.length); // I hope so
};

template <>
string TextInfoBeta<Char>::atomValue(int i) const {
    return string(1,important_atoms[i].value);
};
// original:
template <>
string TextInfoBeta<Token>::atomOrigValue(int i) const {
    const Token& atom = original_atoms[i];
    return original_text.substr(atom.start, atom.length); // I hope so
};

template <>
string TextInfoBeta<Char>::atomOrigValue(int i) const {
    return string(1,original_atoms[i].value);
};

DFA::CHARACTER_CLASS CharTypeToCharClass(CharacterType::Type type)
{
    switch (type) {
    case CharacterType::OPEN_BRACKET:
        return DFA::OPEN_BRACKET;
    case CharacterType::CLOSE_BRACKET:
        return DFA::END_BRACKET;
    case CharacterType::WHITESPACE:
        return DFA::IGNORABLE;
    default:
        return DFA::IMPORTANT;
    }
}

#endif // TEXTINFO_H

