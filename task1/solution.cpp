#include <iostream>
#include <cstddef>
#include <algorithm>

template <typename T>
class Vector
{
private:
    T *data;
    size_t sz;
    size_t capacity;

    void reallocate(size_t new_capacity)
    {
        T *new_data = new T[new_capacity];
        for (size_t i = 0; i < sz; ++i)
        {
            new_data[i] = data[i];
        }
        delete[] data;
        data = new_data;
        capacity = new_capacity;
    }

public:
    Vector() : data(nullptr), sz(0), capacity(0) {}

    explicit Vector(size_t n) : sz(n), capacity(n)
    {
        data = new T[n];
    }

    ~Vector() { delete[] data; }

    Vector(const Vector &other) : sz(other.sz), capacity(other.capacity)
    {
        data = new T[capacity];
        for (size_t i = 0; i < sz; ++i)
            data[i] = other.data[i];
    }

    Vector &operator=(const Vector &other)
    {
        if (this != &other)
        {
            delete[] data;
            sz = other.sz;
            capacity = other.capacity;
            data = new T[capacity];
            for (size_t i = 0; i < sz; ++i)
                data[i] = other.data[i];
        }
        return *this;
    }

    void push_back(const T &value)
    {
        if (sz == capacity)
        {
            reallocate(capacity == 0 ? 8 : capacity * 2);
        }
        data[sz++] = value;
    }

    size_t size() const { return sz; }

    T &operator[](size_t index) { return data[index]; }

    const T &operator[](size_t index) const { return data[index]; }

    T *begin() { return data; }
    T *end() { return data + sz; }
};

struct Pair
{
    unsigned long long key;
    unsigned long long value;
};

class RadixSorter
{
public:
    static void sort(Vector<Pair> &arr)
    {
        size_t n = arr.size();
        if (n <= 1)
            return;

        Vector<Pair> buffer(n);

        // Указатели на источник и приёмник.
        // Начинаем: читаем из arr, пишем в buffer.
        Vector<Pair> *src = &arr;
        Vector<Pair> *dst = &buffer;

        // Ключ 64-битный (unsigned long long). Делим его на 8 частей по 8 бит (по 1 байту).
        // Проходим от младшего байта (shift=0) к старшему (shift=56).
        // Это LSD (Least Significant Digit) — сначала сортируем по младшим разрядам.
        for (int shift = 0; shift < 64; shift += 8)
        {
            // Массив для подсчёта частот каждого возможного байта (0..255)
            size_t count[256] = {0};

            // --- ШАГ 1: Подсчёт частот (гистограмма) ---
            // Смотрим на текущий байт каждого элемента в массиве-источнике
            for (size_t i = 0; i < n; ++i)
            {
                // Сдвигаем ключ вправо на shift бит и маской 0xFF оставляем только младший байт
                unsigned char byte = ((*src)[i].key >> shift) & 0xFF;
                count[byte]++; // Увеличиваем счётчик для этого байта
            }

            // --- ШАГ 2: Преобразование частот в стартовые позиции (префиксные суммы) ---
            // Теперь count[i] будет хранить НЕ количество, а ИНДЕКС в массиве-приёмнике,
            // с которого нужно начинать вставку элементов с байтом i.
            size_t start_pos = 0;
            for (int i = 0; i < 256; ++i)
            {
                size_t c = count[i];  // Запоминаем, сколько было элементов с байтом i
                count[i] = start_pos; // Теперь это стартовая позиция для байта i
                start_pos += c;       // Следующий байт начнётся после этой группы
            }

            // --- ШАГ 3: Распределение элементов в массив-приёмник ---
            // Проходим по исходному массиву СТРОГО ПОДРЯД (это гарантирует стабильность!).
            // Стабильность: если у двух элементов одинаковый текущий байт,
            // они попадут в dst в том же порядке, в котором шли в src,
            // сохраняя результат сортировки по предыдущим (более младшим) байтам.
            for (size_t i = 0; i < n; ++i)
            {
                unsigned char byte = ((*src)[i].key >> shift) & 0xFF;
                // Вставляем элемент на предназначенное ему место и сдвигаем указатель
                (*dst)[count[byte]++] = (*src)[i];
            }

            // --- ШАГ 4: Смена ролей массивов ---
            // Теперь отсортированный (по текущему байту) массив находится в dst.
            // На следующей итерации он станет ИСТОЧНИКОМ для сортировки по следующему байту.
            // Меняем указатели, чтобы не копировать данные обратно (экономия O(n) операций).
            std::swap(src, dst);
        }

        // --- ФИНАЛЬНЫЙ ШАГ: Проверка, где оказался результат ---
        // После 8 итераций (чётное число) данные окажутся в исходном массиве arr.
        // Если итераций было нечётное количество (в другом коде) — результат в buffer.
        // Копируем buffer в arr, если нужно.
        if (src != &arr)
        {
            for (size_t i = 0; i < n; ++i)
            {
                arr[i] = buffer[i];
            }
        }
    }
};

int main()
{
    // Ускорение ввода-вывода за счёт отключения синхронизации с C-потоками
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    Vector<Pair> data; // Основной массив для хранения пар
    unsigned long long k, v;

    // Читаем пары ключ-значение из стандартного ввода, пока есть данные
    while (std::cin >> k >> v)
    {
        data.push_back({k, v});
    }

    // Запускаем LSD поразрядную сортировку
    RadixSorter::sort(data);

    // Выводим результат: ключ и значение через табуляцию
    for (size_t i = 0; i < data.size(); ++i)
    {
        std::cout << data[i].key << "\t" << data[i].value << "\n";
    }

    return 0;
}