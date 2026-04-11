#include <bits/stdc++.h>
using namespace std;

using ull = unsigned long long;

struct Node
{
    string key;
    ull value;
    unsigned int priority;
    Node *left;
    Node *right;

    Node(const string &k, ull v)
        : key(k), value(v), priority(rand() ^ (rand() << 15)), left(nullptr), right(nullptr) {}
};

int compareIgnoreCase(const string &a, const string &b)
{
    size_t len = min(a.size(), b.size());
    for (size_t i = 0; i < len; ++i)
    {
        char ca = tolower(static_cast<unsigned char>(a[i]));
        char cb = tolower(static_cast<unsigned char>(b[i]));
        if (ca != cb)
            return ca < cb ? -1 : 1;
    }
    if (a.size() != b.size())
        return a.size() < b.size() ? -1 : 1;
    return 0;
}

Node *rotateLeft(Node *node)
{
    if (!node || !node->right)
        return node;
    Node *r = node->right;
    node->right = r->left;
    r->left = node;
    return r;
}

Node *rotateRight(Node *node)
{
    if (!node || !node->left)
        return node;
    Node *l = node->left;
    node->left = l->right;
    l->right = node;
    return l;
}

Node *insert(Node *root, const string &k, ull v)
{
    if (!root)
        return new Node(k, v);

    Node *newNode = new Node(k, v);
    Node *cur = root;
    Node *parent = nullptr;
    vector<Node *> path;

    while (cur)
    {
        path.push_back(cur);
        int cmp = compareIgnoreCase(k, cur->key);
        if (cmp == 0)
        {
            delete newNode;
            return root;
        }
        parent = cur;
        if (cmp < 0)
            cur = cur->left;
        else
            cur = cur->right;
    }

    if (compareIgnoreCase(k, parent->key) < 0)
        parent->left = newNode;
    else
        parent->right = newNode;

    Node *child = newNode;
    while (!path.empty())
    {
        Node *p = path.back();
        path.pop_back();

        if (child->priority <= p->priority)
            break;

        if (p->left == child)
        {
            if (path.empty())
            {
                root = rotateRight(p);
            }
            else
            {
                Node *grand = path.back();
                if (grand->left == p)
                    grand->left = rotateRight(p);
                else
                    grand->right = rotateRight(p);
            }
            child = p;
        }
        else
        {
            if (path.empty())
            {
                root = rotateLeft(p);
            }
            else
            {
                Node *grand = path.back();
                if (grand->left == p)
                    grand->left = rotateLeft(p);
                else
                    grand->right = rotateLeft(p);
            }
            child = p;
        }
    }

    return root;
}

pair<bool, ull> findWord(Node *root, const string &k)
{
    Node *cur = root;
    while (cur)
    {
        int cmp = compareIgnoreCase(k, cur->key);
        if (cmp == 0)
        {
            return {true, cur->value};
        }
        cur = (cmp < 0) ? cur->left : cur->right;
    }
    return {false, 0};
}

Node *remove(Node *root, const string &k)
{
    if (!root)
        return nullptr;

    Node *cur = root;
    Node *parent = nullptr;
    bool isLeftChild = false;

    while (cur)
    {
        int cmp = compareIgnoreCase(k, cur->key);
        if (cmp == 0)
            break;

        parent = cur;
        if (cmp < 0)
        {
            cur = cur->left;
            isLeftChild = true;
        }
        else
        {
            cur = cur->right;
            isLeftChild = false;
        }
    }

    if (!cur)
        return root;

    while (cur->left || cur->right)
    {
        if (!cur->left)
        {
            if (parent == nullptr)
            {
                root = rotateLeft(cur);
                parent = root;
            }
            else if (isLeftChild)
            {
                parent->left = rotateLeft(cur);
                parent = parent->left;
            }
            else
            {
                parent->right = rotateLeft(cur);
                parent = parent->right;
            }
            cur = parent->right;
            isLeftChild = false;
        }
        else if (!cur->right)
        {
            if (parent == nullptr)
            {
                root = rotateRight(cur);
                parent = root;
            }
            else if (isLeftChild)
            {
                parent->left = rotateRight(cur);
                parent = parent->left;
            }
            else
            {
                parent->right = rotateRight(cur);
                parent = parent->right;
            }
            cur = parent->left;
            isLeftChild = true;
        }
        else
        {
            if (cur->left->priority > cur->right->priority)
            {
                if (parent == nullptr)
                {
                    root = rotateRight(cur);
                    parent = root;
                }
                else if (isLeftChild)
                {
                    parent->left = rotateRight(cur);
                    parent = parent->left;
                }
                else
                {
                    parent->right = rotateRight(cur);
                    parent = parent->right;
                }
                cur = parent->left;
                isLeftChild = true;
            }
            else
            {
                if (parent == nullptr)
                {
                    root = rotateLeft(cur);
                    parent = root;
                }
                else if (isLeftChild)
                {
                    parent->left = rotateLeft(cur);
                    parent = parent->left;
                }
                else
                {
                    parent->right = rotateLeft(cur);
                    parent = parent->right;
                }
                cur = parent->right;
                isLeftChild = false;
            }
        }
    }

    if (parent == nullptr)
    {
        delete root;
        return nullptr;
    }
    else if (isLeftChild)
    {
        delete parent->left;
        parent->left = nullptr;
    }
    else
    {
        delete parent->right;
        parent->right = nullptr;
    }

    return root;
}

void clearTree(Node *node)
{
    if (!node)
        return;
    clearTree(node->left);
    clearTree(node->right);
    delete node;
}

int main()
{
    srand(time(nullptr));

    Node *root = nullptr;
    string line;

    while (getline(cin, line))
    {
        if (line.empty())
            continue;

        stringstream ss(line);
        string cmd;
        ss >> cmd;

        if (cmd == "+")
        {
            string word;
            ull num;
            ss >> word >> num;

            Node *newRoot = insert(root, word, num);
            if (newRoot == root)
            {
                cout << "Exist" << endl;
            }
            else
            {
                root = newRoot;
                cout << "OK" << endl;
            }
        }
        else if (cmd == "-")
        {
            string word;
            ss >> word;

            Node *newRoot = remove(root, word);
            if (newRoot == root && findWord(root, word).first)
            {
                cout << "NoSuchWord" << endl;
            }
            else
            {
                root = newRoot;
                cout << "OK" << endl;
            }
        }
        else
        {
            string word = cmd;
            auto [found, val] = findWord(root, word);
            if (found)
            {
                cout << "OK: " << val << endl;
            }
            else
            {
                cout << "NoSuchWord" << endl;
            }
        }
    }

    clearTree(root);
    return 0;
}