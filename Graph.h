#ifndef GRAPH_H
#define GRAPH_H

class Graph{
  unsigned int leftSize;
  unsigned int rightSize;
  vector<vector<unsigned int> > edges;
public:
  Graph(unsigned int leftSize,unsigned int rightSize):
    leftSize(leftSize),rightSize(rightSize){
    edges.resize(2+leftSize+rightSize);
  }
  
  bool edgeExists(unsigned int from,unsigned int to) const {
    return contains(edges[from].begin(),edges[from].end(),to);
  }
  unsigned int getEdgesCount() const {
    unsigned int acc=0;
    for(unsigned int id=0;id<edges.size();++id){
      acc+=edges[id].size();
    }
    return acc;
  }
  unsigned int getLeftSize() const {
    return leftSize;
  }
  unsigned int getRightSize() const {
    return rightSize;
  }
  unsigned int getNodesCount() const {
    return 1+leftSize+rightSize+1;
  }
  unsigned int source() const {
    return 0;
  }
  unsigned int sink() const {
    return 1+leftSize+rightSize;
  }
  unsigned int left(unsigned int pos) const {
    return 1+pos;
  }
  unsigned int whichLeft(unsigned int id) const {
    assert(1<=id && id < 1+ leftSize);
    return id-1;
  }
  unsigned int whichRight(unsigned int id) const {
    assert(1+leftSize<=id);
    return id-1-leftSize;
  }
  unsigned int right(unsigned int pos) const {
    return 1+leftSize+pos;
  }
  unsigned int getOutDegree(unsigned int id) const {
    return edges[id].size();
  }
  unsigned int getOutEdgeEnd(unsigned int id,unsigned int i) const {
    return edges[id][i];
  }
  void isolate(unsigned int id){
    FOREACH(e,edges[id]){
      assert(edgeExists(*e,id));
      edges[*e]=erase(edges[*e],id);
    }
    edges[id].clear();
  }
  void addEdge(unsigned int from,unsigned int to){
    assert(!edgeExists(from,to));
    edges[from].push_back(to);
    assert(edgeExists(from,to));
  }
  void addEdges(unsigned int a,unsigned int b){
    addEdge(a,b);
    addEdge(b,a);
  }
};


void copyEdges(Graph a,Graph &b){
  for(unsigned int id=0;id<a.getNodesCount();++id){
    for(unsigned int j=0;j<a.getOutDegree(id);++j){
      unsigned int endId = a.getOutEdgeEnd(id,j);
      b.addEdge(id,endId);
    }
  }  
}
Graph merge(Graph a,Graph b){
  assert(a.getLeftSize()==b.getLeftSize());
  assert(a.getRightSize()==b.getRightSize());
  Graph merged(a.getLeftSize(),a.getRightSize());
  copyEdges(a,merged);
  copyEdges(b,merged);
  return merged;

}

#endif