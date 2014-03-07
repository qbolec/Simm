#ifndef BLOCK_H
#define BLOCK_H

struct Block
{
    unsigned int startSrc;
    unsigned int startDest;
    unsigned int length;
    Block(unsigned int _startSrc,unsigned int _startDest,unsigned int _length):
        startSrc(_startSrc),startDest(_startDest),length(_length) {};
    unsigned int endSrc() const
    {
        return startSrc+length;
    }
    unsigned int endDest() const
    {
        return startDest+length;
    }
    bool containsSrc(unsigned int src) const
    {
        return startSrc <= src && src<startSrc+length;
    }
    bool containsDest(unsigned int dest) const
    {
        return startDest <= dest && dest<startDest +length;
    }
};

bool areAligned(Block a,Block b){
  assert(a.startSrc < b.startSrc);
  return a.startSrc < b.startSrc && a.startDest < b.startDest;
}
bool conflict(unsigned int aLo,unsigned int aHi,unsigned int bLo,unsigned int bHi){
  assert(aHi<aLo);
  assert(bHi<bLo);
  return (aHi < bHi && bHi < aLo &&  aLo < bLo) ||
    (bHi < aHi && aHi < bLo && bLo < aLo);
}
bool conflict(Block aLo,Block aHi,Block bLo,Block bHi){
  return conflict(aLo.startSrc,aHi.startSrc,bLo.startSrc,bHi.startSrc) ||
    conflict(aLo.startDest,aHi.startDest,bLo.startDest,bHi.startDest);
}

#endif // BLOCK_H
