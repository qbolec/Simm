#ifndef DFA_H
#define DFA_H

class DFA{
public:
  enum CHARACTER_CLASS{
    IMPORTANT = 0,
    IGNORABLE = 1,
    OPEN_BRACKET = 2,
    END_BRACKET = 3,
  };
  enum SPECIAL_CHARACTER{
    END_OF_FILE = 0,
    ANY_OTHER_CHARACTER = -1,
  };  
private:

  typedef map<char,pair<vector<CHARACTER_CLASS>,int> > MapType;
  vector<MapType> stepFunction;
  unsigned int state;
public:
  void addDefault(int from,int to,vector<CHARACTER_CLASS> emit){
    addEdge(from,ANY_OTHER_CHARACTER,to,emit);
  }
  void addEdge(unsigned int from,char trough,unsigned int to,vector<CHARACTER_CLASS> emit){
    if(stepFunction.size()<=from){
      stepFunction.resize(from+1);
    }
    stepFunction[from][trough] = make_pair(emit,to);
  }
  void reset(){
    state = 0;
  }
  vector<CHARACTER_CLASS> react(char input){
    assert(state < stepFunction.size());
    MapType::iterator ptr = stepFunction[state].find(input);
    if(ptr != stepFunction[state].end()){
      state = ptr->second.second;
      return ptr->second.first;
    }else if(input!=END_OF_FILE){
      ptr = stepFunction[state].find(ANY_OTHER_CHARACTER);
      assert(ptr!=stepFunction[state].end());
      state = ptr->second.second;
      return ptr->second.first;      
    }else{
      return vector<CHARACTER_CLASS>();
    }
  }
};

#endif
