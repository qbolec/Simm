#ifndef UTILITIES_H
#define UTILITIES_H

#include<set>
#include<memory>
#include<cassert>
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<utility>
#include<vector>
#include<algorithm>
#include<map>
using namespace std;
//#define FOREACH(it,con) for(auto it=con.begin();it!=con.end();++it)
#define FOREACH(it,con) for(__typeof((con).begin()) it=(con).begin();it!=(con).end();++it)
string readfile(char * name){
  ifstream a;
  a.open(name);
  if (!a) {
    a.open((string("samples/") + name).c_str());
  }
  if (!a) {
    cerr << "Failed to open file " << name << "\n";
    return "";
  }
  stringstream buffer;
  buffer << a.rdbuf();
  return buffer.str();
}

string erase(string a,int start,int len){
  return a.substr(0,start) + a.substr(start+len);
}

template<typename T,typename A>
vector<T,A> erase(const vector<T,A> &v,T x){
  vector<T,A> newV;
  for(unsigned int i=0;i<v.size();++i){
    if(v[i]!=x){
      newV.push_back(v[i]);
    }
  }
  return newV;
}

typedef vector<vector<int> > LCSDescription;
LCSDescription lcs(string a,string b){
  LCSDescription d(a.length()+1,vector<int>(b.length()+1,0));
  for(unsigned int i=0;i<a.length();++i){
    for(unsigned int j=0;j<b.length();++j){
      if(a[i]==b[j]){
        d[i+1][j+1] = 1+d[i][j];
      }else{
        d[i+1][j+1] = max(d[i][j+1],d[i+1][j]);
      }
    }
  }
  return d;
}
int editDistance(string a,string b){
  cerr << '[' << a << " vs. " << b << ']' << endl;
  LCSDescription desc = lcs(a,b);
  return a.length()+b.length()-2*desc.back().back();
}

vector<pair<int,int> > getAnyLCS(LCSDescription lcs){
  vector<pair<int,int> > path;
  unsigned int a=lcs.size()-1;
  unsigned int b=lcs.back().size()-1;
  while(a&&b){
    if(lcs[a][b] == lcs[a-1][b-1]+1){
      a--;
      b--;
      path.push_back(make_pair(a,b));      
    }else{
      if(lcs[a][b] == lcs[a-1][b]){
        a--;
      }else{
        b--;
      }
    }
  }
  reverse(path.begin(),path.end());
  return path;
}

template<typename T,typename I>
bool contains(I begin,I end,T x){
  for(I i=begin;i!=end;++i){
    if(*i == x){
      return true;
    }
  }
  return false;
}

bool include(vector<int> &v,int x){
  if(find(v.begin(),v.end(),x)==v.end()){
    v.push_back(x);
    return true;
  }else{
    return false;
  }
}
bool exclude(vector<int> &v,int x){
  if(find(v.begin(),v.end(),x)==v.end()){
    return false;
  }else{
    v.erase(find(v.begin(),v.end(),x));
    return true;
  }
}
void eliminate(vector<vector<int> > & dag,int i){
  for(unsigned int j=dag.size();j--;){
    if(find(dag[j].begin(),dag[j].end(),i)!=dag[j].end()){
      assert(j!=i);
      for(unsigned int k=dag[i].size();k--;){
        include(dag[j],dag[i][k]);
      }
      exclude(dag[j],i);
    }
  }
  dag[i].clear();
}
  
string makeVisibleChar(char x){
  switch(x){
  case '\n':
    return "&#8617;\n";    
  default:
    string s;
    s+=x;
    return s;
  }
}


bool isSubsetOf(vector<int> &A,vector<int> &B,int a,int b){
  (void)a;
  if(A.size()<=B.size()){
    int bp=0;
    for(int i=0;i<A.size();++i)if(A[i]!=b){
      while(bp<B.size() && B[bp]<A[i]){
        bp++;
      }
      if(bp>=B.size() || B[bp]!=A[i]){
        return false;
      }
    }
    return true;
  }
  return false;
}
bool isOk(int v,const vector<int> &take,const vector<vector<int> > &originalGraph){
  if(take[v]==-1){
    FOREACH(n,originalGraph[v]){
      if(take[*n]==1){
        return true;
      }
    }
    return false;
  }
  return true;
}

#endif
