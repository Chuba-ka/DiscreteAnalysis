#include <stack>
#include <unordered_map>

/**
 * Definition for a binary tree node.
 * struct TreeNode {
 * int val;
 * TreeNode *left;
 * TreeNode *right;
 * TreeNode() : val(0), left(nullptr), right(nullptr) {}
 * TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
 * TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
 * };
 */

class Solution
{
public:
    TreeNode *sufficientSubset(TreeNode *root, int limit)
    {
        if (!root)
            return nullptr;

        // Стек для обхода: хранит пару {узел, текущая сумма от корня до этого узла}
        std::stack<std::pair<TreeNode *, long long>> st;
        // Карта для хранения "максимальной суммы пути через данный узел до листа"
        // Нам нужно знать, смог ли узел найти хотя бы один путь >= limit
        std::unordered_map<TreeNode *, bool> isSufficient;

        TreeNode *curr = root;
        TreeNode *lastVisited = nullptr;
        long long currentSum = 0;

        while (curr || !st.empty())
        {
            // Идем максимально влево
            while (curr)
            {
                currentSum += curr->val;
                st.push({curr, currentSum});
                curr = curr->left;
            }

            TreeNode *node = st.top().first;
            long long sumAtNode = st.top().second;

            // Если правый ребенок существует и мы пришли не от него — идем в него
            if (node->right && lastVisited != node->right)
            {
                curr = node->right;
            }
            else
            {
                // Обработка узла (Post-order step)
                bool canKeep = false;

                if (!node->left && !node->right)
                {
                    // Это лист: проверяем условие напрямую
                    canKeep = (sumAtNode >= limit);
                }
                else
                {
                    // Внутренний узел: проверяем, остался ли хоть один "достаточный" ребенок
                    if (node->left && isSufficient[node->left])
                    {
                        canKeep = true;
                    }
                    else
                    {
                        node->left = nullptr; // Удаляем "недостаточного" ребенка
                    }

                    if (node->right && isSufficient[node->right])
                    {
                        canKeep = true;
                    }
                    else
                    {
                        node->right = nullptr; // Удаляем "недостаточного" ребенка
                    }
                }

                isSufficient[node] = canKeep;
                lastVisited = node;
                st.pop();
                currentSum = st.empty() ? 0 : st.top().second;
            }
        }

        // Если корень сам оказался недостаточным — возвращаем nullptr
        return isSufficient[root] ? root : nullptr;
    }
};