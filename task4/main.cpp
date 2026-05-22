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

    int current_line_idx = 1;
    int total_words_seen = 0;
    int current_node_v = 0;

    vector<int> match_counts;
    vector<pair<int, int>> word_coords;

    string to_lower(string s)
    {
        for (char &c : s)
        {
            if (c >= 'A' && c <= 'Z')
                c += 'a' - 'A';
        }
        return s;
    }

    void insert_pattern_part(const string &word, int position_offset)
    {
        int v = 0;
        string low_word = to_lower(word);
        if (trie[v].next.find(low_word) == trie[v].next.end())
        {
            trie[v].next[low_word] = trie.size();
            trie.emplace_back();
        }
        v = trie[v].next[low_word];
        trie[v].pattern_indices.push_back(position_offset);
    }

    void build_automaton()
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
                q.push(next_node);
            }
        }
    }

public:
    explicit AhoCorasickWildcard(const string &pattern_line)
    {
        trie.emplace_back();
        stringstream ss(pattern_line);
        string word;
        int current_pos = 0;

        while (ss >> word)
        {
            if (word != "?")
            {
                insert_pattern_part(word, current_pos);
                non_wildcard_count++;
            }
            current_pos++;
        }
        pattern_len = current_pos;
        build_automaton();
    }

    void process_line(const string &line)
    {
        stringstream ss(line);
        string word;
        int word_in_line_idx = 1;

        while (ss >> word)
        {
            total_words_seen++;
            word_coords.push_back({current_line_idx, word_in_line_idx});
            match_counts.push_back(0);

            string low_word = to_lower(word);

            while (current_node_v > 0 && trie[current_node_v].next.find(low_word) == trie[current_node_v].next.end())
            {
                current_node_v = trie[current_node_v].fail;
            }
            if (trie[current_node_v].next.find(low_word) != trie[current_node_v].next.end())
            {
                current_node_v = trie[current_node_v].next[low_word];
            }

            for (int offset : trie[current_node_v].pattern_indices)
            {
                int start_pos = total_words_seen - offset - 1;
                if (start_pos >= 0)
                {
                    match_counts[start_pos]++;
                }
            }
            word_in_line_idx++;
        }
        current_line_idx++;
    }

    vector<pair<int, int>> get_found_occurrences() const
    {
        vector<pair<int, int>> results;
        if (pattern_len == 0)
            return results;

        for (int i = 0; i <= (int)match_counts.size() - pattern_len; ++i)
        {
            if (match_counts[i] == non_wildcard_count)
            {
                results.push_back(word_coords[i]);
            }
        }
        return results;
    }
};

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string pattern;
    if (!getline(cin, pattern))
        return 0;

    AhoCorasickWildcard searcher(pattern);

    string line;
    while (getline(cin, line))
    {
        searcher.process_line(line);
    }

    auto occurrences = searcher.get_found_occurrences();
    for (const auto &pos : occurrences)
    {
        cout << pos.first << ", " << pos.second << "\n";
    }

    return 0;
}