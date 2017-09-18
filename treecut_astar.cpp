/* A star solution to leetcode contest,
 becuse the test set is very small,
probably better using BFS than A-Star
*/
#include <cstdio>
#include <iostream>
#include <vector>
#include <vector>
#include <queue>
#include <unordered_set>
#include <cmath>
#include <limits.h>

using namespace std;

 class vec2Int{
    public:
        int x;
        int y;
        int gcost;
        int hcost;
        int height;
        vec2Int(int a=0, int b=0){
            x=a;
            y=b;
            gcost=INT_MAX;
            hcost=INT_MAX;
            height=INT_MAX;
        }
        friend bool operator ==(const vec2Int& v1,const vec2Int& v2){
            return v1.x==v2.x && v1.y==v2.y;
        }
        friend bool operator !=(const vec2Int& v1,const vec2Int& v2){
            return v1.x!=v2.x || v1.y!=v2.y;
        }
    };
    struct quick_hash{
      inline size_t operator()(const vec2Int & v) const {
             hash<int> int_hasher;
             return int_hasher(v.x + 51) ^ int_hasher(v.y);
     }
    };
    
     struct compare_cost{
         bool operator()(const vec2Int& lhs, const vec2Int& rhs) const{
          return lhs.gcost + lhs.hcost > rhs.gcost + rhs.hcost;
        }
    };
    
     struct compare_height{
         bool operator()(const vec2Int& lhs, const vec2Int& rhs) const{
          return lhs.height > rhs.height;
        }
    };
    vector<vec2Int> getNeighbor(vec2Int current,vector<vector<int>>& forest){
        vector<vec2Int> nbs;
               
        if(current.x - 1 >=0 && forest[current.x - 1][current.y]!=0){           
            nbs.push_back(vec2Int(current.x-1,current.y));
        }
        if(current.y - 1 >=0 && forest[current.x][current.y-1]!=0){
            nbs.push_back(vec2Int(current.x,current.y-1));
        }
        if(current.x + 1 < forest.size() && forest[current.x + 1][current.y]!=0){
            nbs.push_back(vec2Int(current.x+1,current.y));
        }
        if(current.y + 1 < forest[0].size() && forest[current.x][current.y+1]!=0){
            nbs.push_back(vec2Int(current.x,current.y+1));
        }
        return nbs;
    }
    
    int aStar(vec2Int s, vec2Int e, vector<vector<int>>& forest){
        unordered_set<vec2Int, quick_hash> visited;
        unordered_set<vec2Int, quick_hash> to_visit;
        priority_queue<vec2Int,vector<vec2Int>, compare_cost> frontier;
        //manhattan
        s.hcost = abs(e.x - s.x) + abs(e.y - s.y);
        s.gcost = 0;
        frontier.push(s);
        to_visit.insert(s);
        vec2Int current = s;
        while(current!=e){
            //pop to get the current node
            if(frontier.size()==0){
                return -1;
            }
            current=frontier.top();
            cout<<"current node:"<<current.x<<","<<current.y<<"---gcost:"<<current.gcost<<"hcost:"<<current.hcost<<endl;
            frontier.pop();
            if(visited.find(current)!=visited.end())
                continue;
            to_visit.erase(to_visit.find(current));
            visited.insert(current); 
            vector<vec2Int> nbs = getNeighbor(current, forest);
            for(size_t i = 0; i < nbs.size(); i++){
                if(visited.find(nbs[i])==visited.end()){
                    cout<<"neighbor "<<i<<" :"<<nbs[i].x<<","<<nbs[i].y<<endl;
                    nbs[i].gcost = current.gcost + 1;
                    nbs[i].hcost = abs(e.x - nbs[i].x) + abs(e.y - nbs[i].y);
                    auto it = to_visit.find(nbs[i]);
                    if(it!=to_visit.end()){
                       if(nbs[i].gcost + nbs[i].hcost >= it->gcost + it->hcost)
                            continue; 
                    }
                    else
                        to_visit.insert(nbs[i]);
                    frontier.push(nbs[i]);
                    
                }//update not visit nodes
            }           
            
        }
        return current.gcost;
        
    }
    
    int cutOffTree(vector<vector<int>>& forest) {
       priority_queue<vec2Int,vector<vec2Int>, compare_height> target;
       for(size_t i = 0; i < forest.size();i++){
            for(size_t j = 0; j < forest[0].size();j++){
                if(forest[i][j] > 1){
                    vec2Int c = vec2Int(i,j);
                    c.height = forest[i][j];
                    target.push(c);
                }
            }       
       }
       if(target.size()==0)
            return -1;
       vec2Int s = vec2Int(0,0);
        int sum = 0;
        while(target.size()>0){
            vec2Int e = target.top();
            cout<<"starting from.."<<s.x<<","<<s.y<<endl;
            cout<<"cutting.."<<e.x<<","<<e.y<<endl;
            target.pop();
            int tmp = aStar(s,e,forest);
            cout<<"path finding returns:"<<tmp<<endl;
            if(tmp==-1)return -1;
            sum+=tmp;
            s=e;
        }
        return sum;
        
    }

int main(){
    freopen("map.in","r",stdin);
    int m,n;
    cin>>m>>n;
    vector<vector<int>> forest(m, vector<int>(n));
    for(size_t i =0; i < m; i++)
        for(size_t j =0; j < n; j++){
            cin>>forest[i][j];
    }
    vec2Int startp = vec2Int(5, 5);
    vec2Int endp = vec2Int(1, 1);
    cout<<cutOffTree(forest);
    return 0;
}
