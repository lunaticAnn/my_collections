#include <vector>

using namespace std;
int numIslands(vector<vector<char> >& grid) {
    if(grid.size() == 0 || grid[0].size() == 0) return 0;
    struct helper
    {
        bool mark(vector<vector<char> >& grid, int x, int y)
        {
            if (y < 0 || y >= grid.size() || x < 0 || x >= grid[0].size() || grid[x][y] == '0' ) return false;
            grid[x][y] = '0';
            return mark(grid, x , y + 1) || mark(grid, x, y - 1) || mark(grid, x - 1, y) || mark(grid, x + 1, y);
        }
    };
    helper h;
    int count = 0;
    for (size_t i = 0; i < grid.size(); ++i)
        for (size_t j = 0; j < grid[0].size(); ++j)
        {
            if (grid[i][j] == '1')
            {
                h.mark(grid, i, j);
                ++count;
            }
        }
    return count;
}
int main()
{
    vector<vector<char> > map{
        {'1','0','1','1'},
        {'0','0','1','1'},
        {'1','0','0','0'},
        {'1','0','1','1'}
    };
    printf("count :%d\n", numIslands(map));
}
