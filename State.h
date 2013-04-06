


struct State{
  shared_ptr<Node> a;
  shared_ptr<Node> b;
  int freeId;
  State(shared_ptr<Node> a,shared_ptr<Node> b,int freeId):a(a),b(b),freeId(freeId){
  }
};

int cost(State s){
  return cost(s.a)+cost(s.b);
}

int costLB(State s){
  return costLB(s.a)+costLB(s.b);
}
bool ordered(State worse,State better){
  return cost(better) < cost(worse);
}

ostream& operator << (ostream & sout,const State & r){
  return sout << "A:" << endl << *r.a  <<"B:"<< endl << *r.b;
}