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
};
