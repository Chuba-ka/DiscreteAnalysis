#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Solution
{
public:
    int uniqueLetterString(string s)
    {
        int n = s.length();
        // Храним индексы вхождений для каждого символа A-Z
        // Инициализируем значением -1 (левая граница)
        vector<vector<int>> index(26, vector<int>(1, -1));

        for (int i = 0; i < n; ++i)
        {
            index[s[i] - 'A'].push_back(i);
        }

        long long totalSum = 0;

        for (int i = 0; i < 26; ++i)
        {
            // Добавляем правую границу для каждого символа
            index[i].push_back(n);

            // Проходим по всем вхождениям текущего символа
            // index[i][j] — это текущий индекс символа
            // index[i][j-1] — предыдущее вхождение (L)
            // index[i][j+1] — следующее вхождение (R)
            for (int j = 1; j < index[i].size() - 1; ++j)
            {
                long long leftDist = index[i][j] - index[i][j - 1];
                long long rightDist = index[i][j + 1] - index[i][j];
                totalSum += leftDist * rightDist;
            }
        }

        return (int)totalSum;
    }
};