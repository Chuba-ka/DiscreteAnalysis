#include <bits/stdc++.h>
using namespace std;

class Solution
{
public:
    int minNumberOfFrogs(string croakOfFrogs)
    {
        int n = croakOfFrogs.size();

        vector<int> count(26, 0);
        for (char c : croakOfFrogs)
        {
            count[c - 'a']++;
        }

        if (count['c' - 'a'] != count['r' - 'a'] ||
            count['c' - 'a'] != count['o' - 'a'] ||
            count['c' - 'a'] != count['a' - 'a'] ||
            count['c' - 'a'] != count['k' - 'a'])
        {
            return -1;
        }

        if (count['c' - 'a'] == 0)
            return 0;

        int c = 0, r = 0, o = 0, a = 0, k = 0;
        int maxFrogs = 0;
        int active = 0;

        for (char ch : croakOfFrogs)
        {
            switch (ch)
            {
            case 'c':
                c++;
                active++;
                maxFrogs = max(maxFrogs, active);
                break;

            case 'r':
                if (r + 1 > c)
                    return -1;
                r++;
                break;

            case 'o':
                if (o + 1 > r)
                    return -1;
                o++;
                break;

            case 'a':
                if (a + 1 > o)
                    return -1;
                a++;
                break;

            case 'k':
                if (k + 1 > a)
                    return -1;
                k++;
                active--;
                break;
            }
        }

        if (c == r && r == o && o == a && a == k)
        {
            return maxFrogs;
        }

        return -1;
    }
};