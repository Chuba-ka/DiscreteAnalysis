#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

class SuffixTree
{
private:
    struct Node
    {
        std::map<char, Node *> children;
        int start;
        int *end;
        std::vector<int> suffix_indices;

        Node(int s, int *e) : start(s), end(e) {}
        ~Node()
        {
            for (auto &pair : children)
            {
                delete pair.second;
            }
        }
    };

    Node *root;
    std::string text;
    int *root_end;
    int leaf_end_val;

    void insert(int suffix_start)
    {
        Node *curr = root;
        int i = suffix_start;
        while (i < (int)text.length())
        {
            char c = text[i];
            if (curr->children.find(c) == curr->children.end())
            {
                curr->children[c] = new Node(i, &leaf_end_val);
                curr->children[c]->suffix_indices.push_back(suffix_start + 1);
                return;
            }

            Node *child = curr->children[c];
            int j = child->start;
            int edge_len = *(child->end) - child->start;
            int k = 0;

            while (k < edge_len && i + k < (int)text.length() && text[j + k] == text[i + k])
            {
                k++;
            }

            if (k < edge_len)
            {
                int *split_end = new int(child->start + k);
                Node *split_node = new Node(child->start, split_end);
                curr->children[c] = split_node;
                child->start += k;
                split_node->children[text[child->start]] = child;
                split_node->suffix_indices = child->suffix_indices;
                curr = split_node;
                i += k;
            }
            else
            {
                curr = child;
                i += edge_len;
            }
            curr->suffix_indices.push_back(suffix_start + 1);
        }
    }

public:
    explicit SuffixTree(const std::string &t) : text(t), leaf_end_val((int)t.length())
    {
        root_end = new int(-1);
        root = new Node(-1, root_end);
        for (int i = 0; i < (int)text.length(); ++i)
        {
            insert(i);
        }
    }

    ~SuffixTree()
    {
        delete root;
        delete root_end;
    }

    std::vector<int> find_occurrences(const std::string &pattern) const
    {
        Node *curr = root;
        int i = 0;
        while (i < (int)pattern.length())
        {
            char c = pattern[i];
            auto it = curr->children.find(c);
            if (it == curr->children.end())
                return {};

            Node *child = it->second;
            int edge_len = *(child->end) - child->start;
            int match_len = std::min((int)pattern.length() - i, edge_len);

            if (pattern.compare(i, match_len, text, child->start, match_len) != 0)
            {
                return {};
            }

            i += match_len;
            curr = child;
        }

        std::vector<int> result = curr->suffix_indices;
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }
};

class SearchApplication
{
private:
    std::istream &input;
    std::ostream &output;

    void print_results(int id, const std::vector<int> &positions) const
    {
        output << id << ": ";
        for (size_t i = 0; i < positions.size(); ++i)
        {
            output << positions[i] << (i == positions.size() - 1 ? "" : ", ");
        }
        output << "\n";
    }

public:
    SearchApplication(std::istream &in, std::ostream &out) : input(in), output(out) {}

    void run()
    {
        std::string text;
        if (!(input >> text))
            return;

        SuffixTree tree(text);
        std::string pattern;
        int pattern_id = 1;

        while (input >> pattern)
        {
            std::vector<int> positions = tree.find_occurrences(pattern);
            if (!positions.empty())
            {
                print_results(pattern_id, positions);
            }
            pattern_id++;
        }
    }
};

int main()
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    SearchApplication app(std::cin, std::cout);
    app.run();

    return 0;
}