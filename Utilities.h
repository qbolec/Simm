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
#include<unordered_map>
using namespace std;
#define FOREACH(it,con) for(auto it=con.begin();it!=con.end();++it)
//#define FOREACH(it,con) for(__typeof(con.begin()) it=con.begin();it!=con.end();++it)
string readfile(char * name){
  ifstream a(name);
  stringstream buffer;
  buffer << a.rdbuf();
  return buffer.str();
}

string erase(string a,int start,int len){
  return a.substr(0,start) + a.substr(start+len);
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
template<typename T,typename I>
bool contains(I begin,I end,T x){
  for(auto i=begin;i!=end;++i){
    if(*i == x){
      return true;
    }
  }
}
  