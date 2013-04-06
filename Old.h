
int cap;
State getCheapest(State s);


State getCheapest(State s,shared_ptr<Node> a,shared_ptr<Node> b,vector<pair<int,int> > path){
  assert(path.size());
  //cerr << "common path of length:" << path.size() <<endl;
  //cerr << "begin" <<endl;
  assert(a->innerText.length() == a->charPos.size());
  assert(b->innerText.length() == b->charPos.size());
  for(int i=0;i<path.size();++i){
    assert(path[i].first < a->innerText.length());
    assert(path[i].second < b->innerText.length());
    assert(a->innerText[path[i].first]==b->innerText[path[i].second]);
    //cerr << path[i].first << ' ' << path[i].second << ' '  <<a->innerText[path[i].first] << endl;
  }
  //cerr << "end" <<endl;
  vector<int> aPath;
  vector<int> bPath;
  FOREACH(it,path){
    aPath.push_back(it->first);
    bPath.push_back(it->second);
  }
  //1. elevatedA = unieœ zakres a
  FatherAndSon newAandSon = elevate(a,aPath);
  //2. elevatedB = unieœ zakres b
  FatherAndSon newBandSon = elevate(b,bPath);
  //3. zmaczuj ze sob¹ elevatedA i elevatedB
  assert(s.freeId);
  assert(!newAandSon.second->match);
  assert(!newBandSon.second->match);
  assert(newAandSon.second->innerText == newBandSon.second->innerText);
  newAandSon.second->match = newBandSon.second->match = s.freeId;
  
  State new_s(substitute(s.a,a,newAandSon.first),substitute(s.b,b,newBandSon.first),s.freeId+1);
  if(!(cost(new_s)<cost(s))){
    cout << new_s << endl << " lost with " << s << endl << " wtf?" << endl;
  }
  assert(cost(new_s)<cost(s));
  assert(costLB(s) <= cost(new_s));
  cap = min(cap, cost(new_s));
  //return new_s;
  return getCheapest(new_s);
}
State getCheapest(State s,shared_ptr<Node> a,shared_ptr<Node> b,LCSDescription &d,int i,int j,vector<pair<int,int> > & path){
  if(!d[i][j]){
    vector<pair<int,int> > rev_path(path.rbegin(),path.rend());
    assert(!d[i][j]);
    return getCheapest(s,a,b,rev_path);
  }
  State best = s;
  assert(a->innerText.length()==a->charPos.size());
  assert(b->innerText.length()==b->charPos.size());
  assert(i-1<a->innerText.length());
  assert(j-1<b->innerText.length());
	
  vector<pair<int,int> > now,then;
  set<pair<int,int> > seen;
  now.push_back(make_pair(i,j));
  seen.insert(now[0]);
  while(!now.empty()||!then.empty()){
    if(now.empty()){
      swap(now,then);
    }
    int i=now.back().first;
    int j=now.back().second;
    now.pop_back();
    assert(i&&j);
    if(a->innerText[i-1]==b->innerText[j-1]){
      assert(d[i][j]==d[i-1][j-1]+1);
      path.push_back(make_pair(i-1,j-1));
      State new_s = getCheapest(s,a,b,d,i-1,j-1,path);
      if(ordered(best,new_s)){
        best = new_s;
      }
      path.pop_back();
    }
    for(int z=0;z<=1;++z){
      if(d[i-z][j-1+z]==d[i][j]){
        then.push_back(make_pair(i-z,j-1+z));
        if(!seen.insert(then.back()).second){
          then.pop_back();
        }
      }
    }
  }
  assert(costLB(s)<=cost(best));
  return best;
  
}
State getCheapest(State s,shared_ptr<Node> a,shared_ptr<Node> b,LCSDescription &d){
  //  for all possible lcs paths p
  //    s' = update(s,n1,n2,p)
  //    res = getCheapest(s')
  //    if res < best
  //      best = res
  //return best
  vector<pair<int,int> > path;
  assert(d.size());
  assert(d.back().size());
  
  return getCheapest(s,a,b,d,d.size()-1,d.back().size()-1,path);
}
State getCheapest(State s){
  if(cap<=costLB(s)){
    cerr << "Capping at " << cap << " for " << costLB(s) << endl;
    return s;
  }
  vector<shared_ptr<Node> > Anodes;
  getAllUnmatchedNodes(s.a,Anodes);
  vector<shared_ptr<Node> > Bnodes;
  getAllUnmatchedNodes(s.b,Bnodes);
  
  int langsteStrasse=0;
  FOREACH(aNodeIt,Anodes){
    FOREACH(bNodeIt,Bnodes){
      int length = lcs((*aNodeIt)->innerText,(*bNodeIt)->innerText).back().back();
      if(langsteStrasse<length){
        langsteStrasse = length;
      }
    }
  }
  //cerr << "langsteStrasse:" << langsteStrasse << endl;
  if(langsteStrasse==0){
    return s;
  }
  State best = s;
  FOREACH(aNodeIt,Anodes){
    FOREACH(bNodeIt,Bnodes){
      LCSDescription desc = lcs((*aNodeIt)->innerText,(*bNodeIt)->innerText);
      assert(desc.size());
      assert(desc.back().size());
      if(desc.back().back()==langsteStrasse){
        State new_s = getCheapest(s,*aNodeIt,*bNodeIt,desc);
        assert(costLB(s)<=cost(new_s));
        if(ordered(best,new_s)){
          best=new_s;
        }
      }
    }
  }
  return best;
}