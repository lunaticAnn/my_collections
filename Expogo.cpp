/*
 Problem
 You have just received the best gift ever: an Expogo stick. You can stand on it and use it to make increasingly large jumps.

 You are currently standing on point (0, 0) in your infinite two-dimensional backyard, and you are trying to reach a goal point (X, Y), with integer coordinates, in as few jumps as possible. You must land exactly on the goal point; it is not sufficient to pass over it on a jump.

 Each time you use your Expogo stick to jump, you pick a cardinal direction: north, south, east, or west. The i-th jump with your Expogo stick moves you 2(i-1) units in the chosen direction, so your first jump takes you 1 unit, your second jump takes you 2 units, your third jump takes you 4 units, and so on.

 Given a goal point (X, Y), determine whether it is possible to get there, and if so, demonstrate how to do it using as few jumps as possible.

 Input
 The first line of the input gives the number of test cases, T. T test cases follow. Each consists of a single line with two integers X and Y: the coordinates of the goal point.

 Output
 For each test case, output one line containing Case #x: y, where x is the test case number (starting from 1) and y is IMPOSSIBLE if the goal point cannot be reached. Otherwise, y must be a string of one or more characters, each of which is either N (north), S (south), E (east), or W (west), representing the directions of the jumps that you will make, in order. This sequence of jumps must reach the goal point at the end of the final jump, and it must be as short as possible.

 Limits
 Time limit: 20 seconds per test set.
 Memory limit: 1GB.
 (X, Y) ≠ (0, 0).
 Test set 1 (Visible Verdict)
 1 ≤ T ≤ 80.
 -4 ≤ X ≤ 4.
 -4 ≤ Y ≤ 4.
 Test set 2 (Visible Verdict)
 1 ≤ T ≤ 100.
 -100 ≤ X ≤ 100.
 -100 ≤ Y ≤ 100.
 Test set 3 (Visible Verdict)
 1 ≤ T ≤ 100.
 -109 ≤ X ≤ 109.
 -109 ≤ Y ≤ 109.
 */

#include <iostream>
#include <vector>
using namespace std;

int main()
{
    int numCase;
    cin >> numCase;
    for (int caseIndex = 1; caseIndex < numCase + 1; ++caseIndex)
    {
        long x, y;
        cin >> x >> y;
        long md = abs(x) + abs(y);
        if (md % 2 == 0)
        {
            // impossible
            cout << "Case #"<<caseIndex<<": IMPOSSIBLE"<<endl;
        }
        else
        {
            int exp2 = 0;
            vector<long> steps(31, -1);
            
            // initialize possible bits
            while (md >= 1 << exp2)
            {
               steps[exp2] = 1 << exp2;
               ++exp2;
               if (exp2 == 31) break;
            }
            steps.resize(exp2);
            
            //find diff
            long diff = ((1 << exp2) - 1 - md) / 2;
            
            // mark negative ones
            for (int i = 0; i < exp2; ++i)
            {
                if (diff & (1 << i))
                    steps[i] = - steps[i];
            }
            
            long s1 = abs(x);
            long s2 = abs(y);
            vector<bool> xSteps(exp2, false);
            for (int i = exp2 - 1; i >= 0; --i)
            {
                if (s1 == 0) break;
                if (s2 == 0)
                {
                    // all left overs are xSteps
                    for (int j = i; j >= 0; --j)
                    {
                        xSteps[j] = true;
                    }
                    break;
                }

                if (steps[i] > 0)
                {
                    if (s1 > s2)
                    {
                        s1 -= steps[i];
                        xSteps[i] = true;
                    }
                    else
                    {
                        s2 -= steps[i];
                    }
                }
                else
                {
                    if (s1 < s2)
                    {
                        s1 -= steps[i];
                        xSteps[i] = true;
                    }
                    else
                    {
                        s2 -= steps[i];
                    }
                }
            }
            
            cout << "Case #"<<caseIndex<<": ";
            long signX = x > 0 ? 1 : -1;
            long signY = y > 0 ? 1 : -1;
            for (int i = 0; i < exp2; ++i)
            {
                if (xSteps[i])
                {
                    cout << (signX * steps[i] > 0 ? "E":"W");
                }
                else
                {
                    cout << (signY * steps[i] > 0 ? "N":"S");
                }
            }
            cout<<endl;
        }
    }
    return 0;
}
