/**
 * Definition for singly-linked list.
 * struct ListNode {
 * int val;
 * ListNode *next;
 * ListNode() : val(0), next(nullptr) {}
 * ListNode(int x) : val(x), next(nullptr) {}
 * ListNode(int x, ListNode *next) : val(x), next(next) {}
 * };
 */
class Solution
{
public:
    ListNode *oddEvenList(ListNode *head)
    {
        // Если в списке 0, 1 или 2 узла, ничего менять не нужно
        if (!head || !head->next || !head->next->next)
        {
            return head;
        }

        // 'odd' будет указывать на текущий последний узел нечетного списка
        ListNode *odd = head;
        // 'evenHead' сохраняем, чтобы в конце приклеить его к нечетному списку
        ListNode *evenHead = head->next;
        // 'even' будет указывать на текущий последний узел четного списка
        ListNode *even = evenHead;

        // Проходим по списку, пока есть пары или хотя бы один следующий узел
        while (even && even->next)
        {
            // Соединяем текущий нечетный со следующим нечетным
            odd->next = even->next;
            odd = odd->next;

            // Соединяем текущий четный со следующим четным
            even->next = odd->next;
            even = even->next;
        }

        // Соединяем конец нечетного списка с началом четного
        odd->next = evenHead;

        return head;
    }
};