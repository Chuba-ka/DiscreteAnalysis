#include <iostream>
#include <random>
#include <utility>
#include <cstdint>
#include <cstring>

using namespace std;

class MyString
{
private:
    char *data;
    size_t length;

    void copy_from(const char *s)
    {
        length = strlen(s);
        data = new char[length + 1];
        strcpy(data, s);
    }

public:
    MyString() : data(new char[1]{'\0'}), length(0) {}

    MyString(const char *s)
    {
        copy_from(s);
    }

    MyString(const MyString &other)
    {
        copy_from(other.data);
    }

    MyString(MyString &&other) noexcept : data(other.data), length(other.length)
    {
        other.data = nullptr;
        other.length = 0;
    }

    ~MyString()
    {
        delete[] data;
    }

    MyString &operator=(const MyString &other)
    {
        if (this != &other)
        {
            delete[] data;
            copy_from(other.data);
        }
        return *this;
    }

    MyString &operator=(MyString &&other) noexcept
    {
        if (this != &other)
        {
            delete[] data;
            data = other.data;
            length = other.length;
            other.data = nullptr;
            other.length = 0;
        }
        return *this;
    }

    bool operator<(const MyString &other) const
    {
        return strcmp(data, other.data) < 0;
    }

    bool operator==(const MyString &other) const
    {
        return strcmp(data, other.data) == 0;
    }

    const char *c_str() const { return data; }

    void to_lower()
    {
        for (size_t i = 0; i < length; ++i)
        {
            if (data[i] >= 'A' && data[i] <= 'Z')
            {
                data[i] = data[i] - 'A' + 'a';
            }
        }
    }

    friend istream &operator>>(istream &is, MyString &s)
    {
        char buf[1024];
        if (is >> buf)
        {
            s = MyString(buf);
        }
        return is;
    }
};

struct Node
{
    MyString key;
    uint64_t val;
    uint64_t prio;
    Node *left = nullptr, *right = nullptr, *parent = nullptr;

    Node(const MyString &k, uint64_t v) : key(k), val(v)
    {
        static mt19937_64 generator(1337);
        prio = generator();
    }
};

class Treap
{
private:
    Node *root = nullptr;

    void rotate_right(Node *x)
    {
        Node *y = x->left;
        x->left = y->right;
        if (y->right)
            y->right->parent = x;
        y->parent = x->parent;
        if (!x->parent)
            root = y;
        else if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;
        y->right = x;
        x->parent = y;
    }

    void rotate_left(Node *x)
    {
        Node *y = x->right;
        x->right = y->left;
        if (y->left)
            y->left->parent = x;
        y->parent = x->parent;
        if (!x->parent)
            root = y;
        else if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;
        y->left = x;
        x->parent = y;
    }

    Node *search(const MyString &key) const
    {
        Node *cur = root;
        while (cur)
        {
            if (key == cur->key)
                return cur;
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        return nullptr;
    }

public:
    Treap() = default;
    ~Treap()
    {
        while (root)
            erase(root->key);
    }

    bool insert(const MyString &key, uint64_t v)
    {
        if (search(key))
            return false;
        Node *newn = new Node(key, v);
        if (!root)
        {
            root = newn;
            return true;
        }

        Node *cur = root, *par = nullptr;
        while (cur)
        {
            par = cur;
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        newn->parent = par;
        if (key < par->key)
            par->left = newn;
        else
            par->right = newn;

        while (newn->parent && newn->prio > newn->parent->prio)
        {
            if (newn->parent->left == newn)
                rotate_right(newn->parent);
            else
                rotate_left(newn->parent);
        }
        return true;
    }

    bool erase(const MyString &key)
    {
        Node *node = search(key);
        if (!node)
            return false;
        while (node->left || node->right)
        {
            if (!node->right || (node->left && node->left->prio > node->right->prio))
                rotate_right(node);
            else
                rotate_left(node);
        }
        if (!node->parent)
            root = nullptr;
        else if (node->parent->left == node)
            node->parent->left = nullptr;
        else
            node->parent->right = nullptr;
        delete node;
        return true;
    }

    pair<bool, uint64_t> get(const MyString &key) const
    {
        Node *n = search(key);
        if (n)
            return {true, n->val};
        return {false, 0ULL};
    }
};

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    Treap dict;
    MyString token;

    while (cin >> token)
    {
        if (token == "+")
        {
            MyString w;
            uint64_t num;
            cin >> w >> num;
            w.to_lower();
            if (dict.insert(w, num))
                cout << "OK\n";
            else
                cout << "Exist\n";
        }
        else if (token == "-")
        {
            MyString w;
            cin >> w;
            w.to_lower();
            if (dict.erase(w))
                cout << "OK\n";
            else
                cout << "NoSuchWord\n";
        }
        else
        {
            token.to_lower();
            auto res = dict.get(token);
            if (res.first)
                cout << "OK: " << res.second << "\n";
            else
                cout << "NoSuchWord\n";
        }
    }
    return 0;
}