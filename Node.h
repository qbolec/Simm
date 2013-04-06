struct Node{
  int start;
  int length;
  int match;
  string innerText;
  vector<int> charPos;
  vector<shared_ptr<Node> > children;
  static shared_ptr<Node> fromString(string s){
    shared_ptr<Node> n(new Node());
    n->start=0;
    n->length=s.length();
    n->innerText=s;
    for(int i=0;i<n->length;++i){
      n->charPos.push_back(i);
    }
    n->match=0;
    return n;
  }
};
bool compareStarts(shared_ptr<Node> a,shared_ptr<Node> b){
  return a->start < b->start;
}
bool contains(shared_ptr<Node> big,shared_ptr<Node> small){
  return big->start <= small->start && small->start+small->length <= big->start+big->length;
}
/*
typedef pair<shared_ptr<Node>,shared_ptr<Node> > FatherAndSon;
FatherAndSon elevate(shared_ptr<Node> node,vector<int> path){
  assert(!node->match);
  assert(node->innerText.length());
  assert(node->charPos.size() == node->innerText.length());
  assert(path.size());
  shared_ptr<Node> son(new Node());
  assert(path.front()<=path.back());
  assert(path.back() < node->innerText.length());
  son->start = node->charPos[path.front()];
  son->length = node->charPos[path.back()] - son->start+1;
  assert(son->length);
  son->match = 0;
  {
    stringstream buffer;
    FOREACH(it,path){
      assert(*it<node->innerText.length());
      buffer << node->innerText[*it];
      son->charPos.push_back(node->charPos[*it]);
    }
    son->innerText = buffer.str();   
  }
  assert(son->innerText.length());
  assert(son->innerText.length()==son->charPos.size());
  vector<shared_ptr<Node> > filteredChildren;
  FOREACH(it,node->children){
    if(contains(son,*it)){
      son->children.push_back(*it);
    }else{
      filteredChildren.push_back(*it);
    }
  }
  for(int i=1;i<path.size();++i){
    if(path[i-1]!=path[i]-1){
      shared_ptr<Node> grandSon(new Node());
      assert(path[i-1]+1 < node->charPos.size());
      grandSon->start = node->charPos[path[i-1]+1];
      grandSon->length = node->charPos[path[i]-1]+1 - grandSon->start;
      grandSon->match = 0;
      grandSon->innerText = node->innerText.substr(path[i-1]+1,path[i]-(path[i-1]+1));
      grandSon->charPos = vector<int>(node->charPos.begin()+path[i-1]+1,node->charPos.begin()+path[i]);
      vector<shared_ptr<Node> > filteredGrandChildren;
      FOREACH(it,son->children){
        if(contains(grandSon,*it)){
          grandSon->children.push_back(*it);
        }else{
          filteredGrandChildren.push_back(*it);
        }
      }
      filteredGrandChildren.push_back(grandSon);
      son->children = filteredGrandChildren;
    }
  }
  sort(son->children.begin(),son->children.end(),compareStarts);
  
  filteredChildren.push_back(son);
  sort(filteredChildren.begin(),filteredChildren.end(),compareStarts);
  shared_ptr<Node> newNode(new Node());
  *newNode = *node;
  {
    vector<int> charPos;
    stringstream buffer;
    for(int i=0;i<node->charPos.size();++i){
      if(i<path.front() || path.back()<i){
        charPos.push_back(node->charPos[i]);
        buffer << node->innerText[i];
      }
    }
    newNode->innerText = buffer.str();
    newNode->charPos = charPos;
  }
  newNode->children = filteredChildren;
  return make_pair(newNode,son);
}
shared_ptr<Node>  substitute(shared_ptr<Node> root, shared_ptr<Node> oldNode,shared_ptr<Node> newNode){
  if(root==oldNode){
    return newNode;
  }else{
    for(int i=0;i<root->children.size();++i){
      shared_ptr<Node> newChild = substitute(root->children[i],oldNode,newNode);
      if(newChild != root->children[i]){
        shared_ptr<Node>  newRoot(new Node());
        *newRoot = *root;
        newRoot->children[i] = newChild;
        return newRoot;
      }
    }
    return root;
  }
}
*/
int cost(shared_ptr<Node> node){
  int c = node->match ? 1 : 2*node->innerText.length();
  FOREACH(it,node->children){
    c+= cost(*it);
  }
  return c;
}
int costLB(shared_ptr<Node> node){
  int c = node->match ? 1 : min(1U,node->innerText.length());
  FOREACH(it,node->children){
    c+= costLB(*it);
  }
  return c;
}


ostream& operator << (ostream & sout,const Node & r){
  sout << r.start << " open " << r.match << endl;
  //sout << "<span>";
  int p=0;
  for(unsigned int i=0;i<r.children.size();++i){
    //while(p< r.charPos.size() && r.charPos[p]<r.children[i]->start){
    //  sout << r.innerText[p];
    //  ++p;
    //}
    sout << *r.children[i];
  }
  //while(p< r.charPos.size()){
  //  sout << r.innerText[p];
  //  ++p;
  //}
  //sout << "</span>";
  sout << r.start+r.length << " close" << endl;
  return sout;
}
void getAllUnmatchedNodes(shared_ptr<Node> r,vector<shared_ptr<Node> > &nodes){
  if(!r->match){
    nodes.push_back(r);
  }
  for(unsigned int i=0;i<r->children.size();++i){
    getAllUnmatchedNodes(r->children[i],nodes);
  }
}