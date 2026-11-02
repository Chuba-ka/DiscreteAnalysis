#include <algorithm>
#include <vector>

using namespace std;

class Solution {
public:
    int minTaps(int n, vector<int>& ranges) {
        vector<int> reach(n + 1, 0);
        for (int i = 0; i <= n; ++i) {
            int left = max(0, i - ranges[i]);
            int right = min(n, i + ranges[i]);
            reach[left] = max(reach[left], right);
        }

        int taps = 0;
        int covered = 0;
        int farthest = 0;
        for (int x = 0; x < n; ++x) {
            farthest = max(farthest, reach[x]);
            if (x == covered) {
                if (farthest <= x)
                    return -1;
                covered = farthest;
                ++taps;
            }
        }
        return taps;
    }
};
