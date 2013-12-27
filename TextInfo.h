struct TextInfo{
  string important_text;
  string original_text;
  vector<unsigned int> positions;
  vector<DFA::CHARACTER_CLASS> states;
  vector<Match> matches;
  vector<unsigned int> next;

  vector<pair<int,int> > matchInfo;
  vector<pair<int,int> > fullMatchInfo;
};
