#include <stack>
#include <unordered_map>

class Solution
{
public:
    TreeNode *sufficientSubset(TreeNode *root, int limit)
    {
        // Если дерева нет, удалять нечего
        if (!root)
            return nullptr;

        // Стек хранит пары: {текущий узел, сумма пути от корня до этого узла}
        // Это заменяет нам системный стек рекурсии
        std::stack<std::pair<TreeNode *, long long>> st;

        // Карта-справочник: для каждого узла записываем, является ли он "достаточным"
        // Достаточный = ведет хотя бы к одному листу с суммой пути >= limit
        std::unordered_map<TreeNode *, bool> isSufficient;

        TreeNode *curr = root;           // Указатель для спуска по дереву
        TreeNode *lastVisited = nullptr; // Нужен, чтобы понять, вернулись мы из правого ребенка или еще нет
        long long currentSum = 0;        // Текущая накопленная сумма пути

        // Основной цикл: работаем, пока есть узлы в дереве или в стеке
        while (curr || !st.empty())
        {

            // --- БЛОК 1: СПУСК ВНИЗ ---
            // Идем максимально влево, "нанизывая" узлы на стек и считая сумму
            while (curr)
            {
                currentSum += curr->val;
                st.push({curr, currentSum});
                curr = curr->left;
            }

            // Смотрим на узел на вершине стека (самый глубокий на данный момент)
            TreeNode *node = st.top().first;
            long long sumAtNode = st.top().second;

            // --- БЛОК 2: ПРОВЕРКА ПРАВОЙ ВЕТКИ ---
            // Если у узла есть правый ребенок и мы в нем еще не были (lastVisited)
            if (node->right && lastVisited != node->right)
            {
                curr = node->right; // Переключаемся на правую ветку
            }
            // --- БЛОК 3: ОБРАБОТКА (ПОДЪЕМ) ---
            else
            {
                // Мы зашли в тупик (лист) или уже вернулись из всех детей.
                // Время принимать решение по текущему узлу.
                bool canKeep = false;

                if (!node->left && !node->right)
                {
                    // Случай А: Это лист. Проверяем условие задачи напрямую.
                    canKeep = (sumAtNode >= (long long)limit);
                }
                else
                {
                    // Случай Б: Это внутренний узел.
                    // Он "выживает" только если выжил хотя бы один его ребенок.

                    // 1. Проверяем левого ребенка
                    if (node->left)
                    {
                        if (isSufficient[node->left])
                        {
                            canKeep = true; // Есть хороший путь через лево
                        }
                        else
                        {
                            node->left = nullptr; // Путь плохой — отрезаем ветку
                        }
                    }

                    // 2. Проверяем правого ребенка
                    if (node->right)
                    {
                        if (isSufficient[node->right])
                        {
                            canKeep = true; // Есть хороший путь через право
                        }
                        else
                        {
                            node->right = nullptr; // Путь плохой — отрезаем ветку
                        }
                    }
                }

                // Записываем вердикт для текущего узла в карту
                isSufficient[node] = canKeep;

                // Помечаем, что этот узел мы только что полностью обработали
                lastVisited = node;

                // Удаляем узел из стека (мы закончили с ним и его поддеревом)
                st.pop();

                // Обновляем текущую сумму: теперь она равна сумме родителя (нового топа стека)
                currentSum = st.empty() ? 0 : st.top().second;
            }
        }

        // В самом конце проверяем корень. Если через него нет ни одного пути >= limit,
        // всё дерево считается "недостаточным".
        return isSufficient[root] ? root : nullptr;
    }
};