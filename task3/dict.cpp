#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <utility>
#include <cstdint>
#include <functional>

using namespace std;

struct Node
{
    string key;
    uint64_t val;
    uint64_t prio;
    Node *left = nullptr;
    Node *right = nullptr;
    Node *parent = nullptr;

    Node(const string &k, uint64_t v) : key(k), val(v), left(nullptr), right(nullptr), parent(nullptr)
    {
        static mt19937_64 generator(random_device{}());
        prio = generator();
    }
};

class Treap
{
private:
    Node *root = nullptr;

    using Compare = less<string>;
    Compare comp;

    void rotate_right(Node *x)
    {
        if (!x || !x->left)
            return;
        Node *y = x->left;
        Node *p = x->parent;

        x->left = y->right;
        if (y->right)
            y->right->parent = x;

        y->right = x;
        x->parent = y;
        y->parent = p;

        if (p)
        {
            if (p->left == x)
                p->left = y;
            else
                p->right = y;
        }
        else
        {
            root = y;
        }
    }

    void rotate_left(Node *x)
    {
        if (!x || !x->right)
            return;
        Node *y = x->right;
        Node *p = x->parent;

        x->right = y->left;
        if (y->left)
            y->left->parent = x;

        y->left = x;
        x->parent = y;
        y->parent = p;

        if (p)
        {
            if (p->left == x)
                p->left = y;
            else
                p->right = y;
        }
        else
        {
            root = y;
        }
    }

    Node *search(const string &key) const
    {
        Node *cur = root;
        while (cur)
        {
            if (!comp(key, cur->key) && !comp(cur->key, key))
            {
                return cur;
            }
            if (comp(key, cur->key))
            {
                cur = cur->left;
            }
            else
            {
                cur = cur->right;
            }
        }
        return nullptr;
    }

public:
    Treap() = default;

    // Запрещаем копирование, чтобы не было проблем с двойным удалением памяти
    Treap(const Treap &) = delete;
    Treap &operator=(const Treap &) = delete;

    // Итеративный деструктор без рекурсии
    ~Treap()
    {
        Node *cur = root;
        while (cur)
        {
            if (cur->left)
            {
                cur = cur->left;
            }
            else if (cur->right)
            {
                cur = cur->right;
            }
            else
            {
                Node *p = cur->parent;
                if (p)
                {
                    if (p->left == cur)
                        p->left = nullptr;
                    else
                        p->right = nullptr;
                }
                delete cur;
                cur = p;
            }
        }
        root = nullptr;
    }

    bool insert(const string &key, uint64_t v)
    {
        if (search(key))
            return false;

        if (!root)
        {
            root = new Node(key, v);
            return true;
        }

        Node *cur = root;
        Node *par = nullptr;
        bool go_left = false;
        while (cur)
        {
            par = cur;
            if (comp(key, cur->key))
            {
                go_left = true;
                cur = cur->left;
            }
            else
            {
                go_left = false;
                cur = cur->right;
            }
        }

        Node *newn = new Node(key, v);
        newn->parent = par;
        if (go_left)
            par->left = newn;
        else
            par->right = newn;

        Node *node = newn;
        while (node->parent)
        {
            Node *p = node->parent;
            if (node->prio <= p->prio)
                break;
            if (p->left == node)
            {
                rotate_right(p);
            }
            else
            {
                rotate_left(p);
            }
        }
        return true;
    }

    bool erase(const string &key)
    {
        Node *node = search(key);
        if (!node)
            return false;

        while (node->left || node->right)
        {
            uint64_t left_p = node->left ? node->left->prio : 0ULL;
            uint64_t right_p = node->right ? node->right->prio : 0ULL;
            if (left_p > right_p)
            {
                rotate_right(node);
            }
            else
            {
                rotate_left(node);
            }
        }

        Node *p = node->parent;
        if (p)
        {
            if (p->left == node)
                p->left = nullptr;
            else
                p->right = nullptr;
        }
        else
        {
            root = nullptr;
        }
        delete node;
        return true;
    }

    pair<bool, uint64_t> get(const string &key) const
    {
        Node *n = search(key);
        if (n)
            return {true, n->val};
        return {false, 0ULL};
    }
};

string normalize(string s)
{
    for (char &c : s)
    {
        c = tolower(static_cast<unsigned char>(c));
    }
    return s;
}

int main()
{
    // Оптимизация ввода-вывода
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    Treap dict;
    string line;
    while (getline(cin, line))
    {
        if (line.empty())
            continue;
        istringstream ss(line);
        string token;
        if (!(ss >> token))
            continue;

        if (token == "+")
        {
            string w;
            uint64_t num;
            if (ss >> w >> num)
            {
                string key = normalize(w);
                if (dict.insert(key, num))
                    cout << "OK" << "\n";
                else
                    cout << "Exist" << "\n";
            }
        }
        else if (token == "-")
        {
            string w;
            if (ss >> w)
            {
                string key = normalize(w);
                if (dict.erase(key))
                    cout << "OK" << "\n";
                else
                    cout << "NoSuchWord" << "\n";
            }
        }
        else
        {
            string key = normalize(token);
            auto [found, num] = dict.get(key);
            if (found)
                cout << "OK: " << num << "\n";
            else
                cout << "NoSuchWord" << "\n";
        }
    }
    return 0;
}