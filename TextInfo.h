struct TextInfo{
  string important_text;
  string original_text;
  vector<unsigned int> positions;
  vector<DFA::CHARACTER_CLASS> states;
  vector<Match> matches;
  vector<pair<unsigned int,pair<bool,unsigned int> > >  points;

  vector<pair<int,int> > matchInfo;
  vector<pair<int,int> > fullMatchInfo;
};
