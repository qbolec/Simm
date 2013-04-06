class Graph{
  unsigned int leftSize;
  unsigned int rightSize;
  vector<vector<int> > edges;
public:
  Graph(unsigned int leftSize,unsigned int rightSize):
    leftSize(leftSize),rightSize(rightSize){
    edges.resize(2+leftSize+rightSize);
  }
  unsigned int getLeftSize(){
    return leftSize;
  }
  unsigned int getRightSize(){
    return rightSize;
  }

  unsigned int source(){
    return 0;
  }
  unsigned int sink(){
    return 1+leftSize+rightSize;
  }
  unsigned int left(unsigned int pos){
    return 1+pos;
  }
  unsigned int right(unsigned int pos){
    return 1+leftSize+pos;
  }
  unsigned int getOutDegree(unsigned int id){
    return edges[id].size();
  }
  unsigned int getOutEdgeEnd(unsigned int id,unsigned int i){
    return edges[id][i];
  }
  void addEdge(unsigned int from,unsigned int to){
    edges[from].push_back(to);
  }
};