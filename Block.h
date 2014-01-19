struct Block{
  unsigned int startSrc;
  unsigned int startDest;
  unsigned int length;
  Block(unsigned int startSrc,unsigned int startDest,unsigned int length):
    startSrc(startSrc),startDest(startDest),length(length){};
  unsigned int endSrc()const{
    return startSrc+length;
  }
  unsigned int endDest()const{
    return startDest+length;
  }
  bool containsSrc(unsigned int src)const{
    return startSrc <= src && src<startSrc+length;
  }
  bool containsDest(unsigned int dest)const{
    return startDest <= dest && dest<startDest +length;
  }
};
