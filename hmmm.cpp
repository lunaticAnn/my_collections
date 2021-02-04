#include <cstdio>
#include <vector>
#include <unordered_map>
#include <set>

using namespace std;
static const vector<vector<int>> parentChildPairs1
{
    {1, 3},
    {2, 3},
    {3, 6},
    {5, 6},
    {5, 7},
    {4, 5},
    {4, 8},
    {4, 9},
    {9, 11},
    {14, 4},
    {13, 12},
    {12, 9}
};

static const vector<vector<int>> testCase1
{
    {3, 8},
    {5, 8},
    {6, 8},
    {6, 9},
    {1, 3},
    {3, 1},
    {7, 11},
    {6, 5},
    {5, 6}
};

static const vector<vector<int>> parentChildPairs2
{
    {11, 10},
    {11, 12},
    {2, 3},
    {10, 2},
    {10, 5},
    {1, 3},
    {3, 4},
    {5, 6},
    {5, 7},
    {7, 8}
};

struct edge
{
    int dir = 1;
    int next = 0;
    edge() = default;
    edge(int d, int n) : dir(d), next(n){}
};

void generateGraph (const vector<vector<int>>& edges, unordered_map<int, vector<edge>>& outputGraph, set<int>& notRoot)
{
    for (const auto& e:edges)
    {
        if (outputGraph.find(e[0]) == outputGraph.end())
        {
            outputGraph[e[0]] = vector<edge>();
        }
        
        if (outputGraph.find(e[1]) == outputGraph.end())
        {
            outputGraph[e[1]] = vector<edge>();
        }
        outputGraph[e[0]].emplace_back(-1, e[1]);
        outputGraph[e[1]].emplace_back(1, e[0]);
        notRoot.insert(e[1]);
    }
}

bool dfsUpDown(bool* result, const unordered_map<int, vector<edge>>& graph, set<int>& visited,
               int start, int end, int currentDirection, bool* mono)
{
    if (start == end) return true;
    visited.insert(start);

    for (const auto& e:graph.at(start))
    {
        if (!visited.count(e.next))
        {
            if (currentDirection > -1)
            {
                *result |= dfsUpDown(result, graph, visited, e.next, end, e.dir, mono);
            }
            else if (e.dir == -1)
            {
                *result |= dfsUpDown(result, graph, visited, e.next, end, -1, mono);
            }

            if (*result && currentDirection == 1 && currentDirection != e.dir)
            {
                *mono = false;
            }
            
            if (*result) return *result;
        }
    }
    return false;
}

int main()
{
    unordered_map<int, vector<edge>> graph;
    set<int> notRoot;
    generateGraph(parentChildPairs1, graph, notRoot);
    
    for (const auto& tc:testCase1)
    {
        bool res = false, mono = true;
        set<int> visited;
        dfsUpDown(&res, graph, visited, tc[0], tc[1], 0, &mono);
        if (mono)
            res &= notRoot.count(tc[0]) && notRoot.count(tc[1]);
        printf("has common ancestor(%d, %d): %s\n", tc[0], tc[1], res ? "True" : "False" );
    }
    return 0;
}
