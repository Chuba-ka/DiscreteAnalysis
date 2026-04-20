#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <queue>
#include <algorithm>

using namespace std;

class AhoCorasickWildcard
{
private:
    struct Node
    {
        map<string, int> next;
        int fail = 0;
        vector<int> pattern_indices;
    };

    vector<Node> trie;
    int pattern_len = 0;
    int non_wildcard_count = 0;

    string to_lower(string s)
    {
        for (char &c : s)
        {
            if (c >= 'A' && c <= 'Z')
                c += 'a' - 'A';
        }
        return s;
    }

    void insert(const string &word, int idx)
    {
        int v = 0;
        string low_word = to_lower(word);
        if (trie[v].next.find(low_word) == trie[v].next.end())
        {
            trie[v].next[low_word] = trie.size();
            trie.emplace_back();
        }
        v = trie[v].next[low_word];
        trie[v].pattern_indices.push_back(idx);
    }

    void build()
    {
        queue<int> q;
        for (auto const &[word, v] : trie[0].next)
        {
            q.push(v);
        }

        while (!q.empty())
        {
            int v = q.front();
            q.pop();

            for (auto const &[word, next_node] : trie[v].next)
            {
                int f = trie[v].fail;
                while (f > 0 && trie[f].next.find(word) == trie[f].next.end())
                {
                    f = trie[f].fail;
                }
                if (trie[f].next.find(word) != trie[f].next.end())
                {
                    f = trie[f].next[word];
                }
                trie[next_node].fail = f;
                trie[next_node].pattern_indices.insert(
                    trie[next_node].pattern_indices.end(),
                    trie[f].pattern_indices.begin(),
                    trie[f].pattern_indices.end());
                q.push(next_node);
            }
        }
    }

public:
    AhoCorasickWildcard(const string &pattern_line)
    {
        trie.emplace_back();
        stringstream ss(pattern_line);
        string word;
        while (ss >> word)
        {
            if (word != "?")
            {
                insert(word, pattern_len);
                non_wildcard_count++;
            }
            pattern_len++;
        }
        build();
    }

    void search()
    {
        string line;
        int current_line_idx = 1;
        int total_words_count = 0;
        int v = 0;

        vector<int> match_counts;
        vector<pair<int, int>> word_coords;

        while (getline(cin, line))
        {
            stringstream ss_line(line);
            string word;
            int word_in_line_idx = 1;
            while (ss_line >> word)
            {
                total_words_count++;
                word_coords.push_back({current_line_idx, word_in_line_idx});
                match_counts.push_back(0);

                string low_word = to_lower(word);
                while (v > 0 && trie[v].next.find(low_word) == trie[v].next.end())
                {
                    v = trie[v].fail;
                }
                if (trie[v].next.find(low_word) != trie[v].next.end())
                {
                    v = trie[v].next[low_word];
                }

                for (int offset : trie[v].pattern_indices)
                {
                    int start_pos = total_words_count - offset - 1;
                    if (start_pos >= 0)
                    {
                        match_counts[start_pos]++;
                    }
                }
                word_in_line_idx++;
            }
            current_line_idx++;
        }

        for (int i = 0; i <= (int)match_counts.size() - pattern_len; ++i)
        {
            if (match_counts[i] == non_wildcard_count)
            {
                cout << word_coords[i].first << ", " << word_coords[i].second << "\n";
            }
        }
    }
};

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string pattern_line;
    if (!getline(cin, pattern_line))
        return 0;

    AhoCorasickWildcard ac(pattern_line);
    ac.search();

    return 0;
}