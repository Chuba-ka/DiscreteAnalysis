#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

class Solution {
    static string collapse(string s) {
        for (int i = 0; i < static_cast<int>(s.size());) {
            int j = i + 1;
            while (j < static_cast<int>(s.size()) && s[j] == s[i])
                ++j;
            if (j - i >= 3) {
                s.erase(i, j - i);
                i = 0; 
            } else {
                i = j;
            }
        }
        return s;
    }

public:
    int findMinStep(string board, string hand) {
        sort(hand.begin(), hand.end());
        int n = static_cast<int>(hand.size());
        int answer = n + 1;
        vector<unordered_set<string>> dp(1 << n);
        vector<int> used(1 << n, 0);
        dp[0].insert(board);

        for (int mask = 0; mask < (1 << n); ++mask) {
            used[mask] = used[mask >> 1] + (mask & 1);
            for (const string& current : dp[mask]) {
                if (current.empty()) {
                    answer = min(answer, used[mask]);
                    continue;
                }
                if (used[mask] >= answer)
                    continue;
                for (int i = 0; i < n; ++i) {
                    if (mask & (1 << i))
                        continue;
                    if (i > 0 && hand[i] == hand[i - 1] &&
                        !(mask & (1 << (i - 1))))
                        continue;
                    for (int pos = 0; pos <= static_cast<int>(current.size()); ++pos) {
                        if (pos > 0 && current[pos - 1] == hand[i])
                            continue;
                        bool extend = pos < static_cast<int>(current.size()) &&
                                      current[pos] == hand[i];
                        bool split = pos > 0 && pos < static_cast<int>(current.size()) &&
                                     current[pos - 1] == current[pos] &&
                                     current[pos] != hand[i];
                        if (!extend && !split)
                            continue;
                        string next = current;
                        next.insert(pos, 1, hand[i]);
                        dp[mask | (1 << i)].insert(collapse(next));
                    }
                }
            }
            dp[mask].clear();
        }
        return answer <= n ? answer : -1;
    }
};
