#ifndef MATCH_H
#define MATCH_H

struct Match{
  unsigned int startSrc;
  vector<unsigned int> startDest;
  unsigned int length;
  Match(unsigned int startSrc,vector<unsigned int> startDest,unsigned int length):
    startSrc(startSrc),startDest(startDest),length(length){
  };
};

#endif