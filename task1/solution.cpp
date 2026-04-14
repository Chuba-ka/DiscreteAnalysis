#include <iostream>
#include <cstddef>
#include <algorithm>
#include <stdexcept>
#include <cstring>

template <typename T>
class Vector
{
private:
    T *data;
    size_t sz;
    size_t capacity;

    void reallocate(size_t new_capacity)
    {
        if (new_capacity < sz)
        {
            throw std::logic_error("err: bad reallocation");
        }

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
    static void sort(Vector<Pair> &arr, int group_bits = 8)
    {

        size_t n = arr.size();
        if (n <= 1)
            return;

        Vector<Pair> buffer(n);
        Vector<Pair> *src = &arr;
        Vector<Pair> *dst = &buffer;

        const int total_bits = 64;

        for (int shift = 0; shift < total_bits; shift += group_bits)
        {
            int current_bits = std::min(group_bits, total_bits - shift);

            size_t bucket_count = 1ULL << current_bits;

            unsigned long long mask = bucket_count - 1;

            Vector<size_t> count(bucket_count);

            std::memset(&count[0], 0, bucket_count * sizeof(size_t));

            for (size_t i = 0; i < n; ++i)
            {
                size_t bucket = ((*src)[i].key >> shift) & mask;
                count[bucket]++;
            }

            size_t start_pos = 0;
            for (size_t i = 0; i < bucket_count; ++i)
            {
                size_t c = count[i];
                count[i] = start_pos;
                start_pos += c;
            }

            for (size_t i = 0; i < n; ++i)
            {
                size_t bucket = ((*src)[i].key >> shift) & mask;
                (*dst)[count[bucket]++] = (*src)[i];
            }

            std::swap(src, dst);
        }

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
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    Vector<Pair> data;
    unsigned long long k, v;

    while (std::cin >> k >> v)
    {
        data.push_back({k, v});
    }

    RadixSorter::sort(data, 16);

    for (size_t i = 0; i < data.size(); ++i)
    {
        std::cout << data[i].key << "\t" << data[i].value << "\n";
    }

    return 0;
}