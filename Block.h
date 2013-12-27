struct Block{
  unsigned int startSrc;
  unsigned int startDest;
  unsigned int length;
  Block(unsigned int startSrc,unsigned int startDest,unsigned int length):
    startSrc(startSrc),startDest(startDest),length(length){};
  unsigned int endSrc(){
    return startSrc+length;
  }
  unsigned int endDest(){
    return startDest+length;
  }
  bool containsSrc(unsigned int src){
    return startSrc <= src && src<startSrc+length;
  }
  bool containsDest(unsigned int dest){
    return startDest <= dest && dest<startDest +length;
  }
};
