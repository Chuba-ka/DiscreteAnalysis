#include <algorithm>
#include <array>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

class BurrowsWheelerTransform {
public:
    std::string encode(const std::string& text) const {
        const std::string input = text + '$';
        const int n = static_cast<int>(input.size());
        std::vector<int> order(n), classes(n), shifted(n), nextClasses(n);
        std::vector<int> count(std::max(n, 256), 0);

        for (unsigned char ch : input) {
            ++count[ch];
        }
        for (int i = 1; i < 256; ++i) {
            count[i] += count[i - 1];
        }
        for (int i = n - 1; i >= 0; --i) {
            order[--count[static_cast<unsigned char>(input[i])]] = i;
        }

        int classCount = 1;
        classes[order[0]] = 0;
        for (int i = 1; i < n; ++i) {
            if (input[order[i]] != input[order[i - 1]]) {
                ++classCount;
            }
            classes[order[i]] = classCount - 1;
        }

        for (int length = 1; length < n && classCount < n; length *= 2) {
            for (int i = 0; i < n; ++i) {
                shifted[i] = order[i] - length;
                if (shifted[i] < 0) {
                    shifted[i] += n;
                }
            }
            std::fill(count.begin(), count.begin() + classCount, 0);
            for (int index : shifted) {
                ++count[classes[index]];
            }
            for (int i = 1; i < classCount; ++i) {
                count[i] += count[i - 1];
            }
            for (int i = n - 1; i >= 0; --i) {
                const int index = shifted[i];
                order[--count[classes[index]]] = index;
            }

            int nextClassCount = 1;
            nextClasses[order[0]] = 0;
            for (int i = 1; i < n; ++i) {
                const int current = order[i];
                const int previous = order[i - 1];
                if (classes[current] != classes[previous] ||
                    classes[(current + length) % n] !=
                        classes[(previous + length) % n]) {
                    ++nextClassCount;
                }
                nextClasses[current] = nextClassCount - 1;
            }
            classes.swap(nextClasses);
            classCount = nextClassCount;
        }

        std::string result(n, '\0');
        for (int i = 0; i < n; ++i) {
            result[i] = input[(order[i] + n - 1) % n];
        }
        return result;
    }

    std::string decode(const std::string& lastColumn) const {
        const int n = static_cast<int>(lastColumn.size());
        std::array<int, 256> frequency{};
        std::array<int, 256> start{};
        int row = -1;
        for (int i = 0; i < n; ++i) {
            ++frequency[static_cast<unsigned char>(lastColumn[i])];
            if (lastColumn[i] == '$') {
                row = i;
            }
        }
        if (frequency[static_cast<unsigned char>('$')] != 1) {
            throw std::invalid_argument("Результат BWT должен содержать ровно один символ '$'");
        }
        for (int i = 1; i < 256; ++i) {
            start[i] = start[i - 1] + frequency[i - 1];
        }

        std::vector<int> lf(n);
        for (int i = 0; i < n; ++i) {
            const auto ch = static_cast<unsigned char>(lastColumn[i]);
            lf[i] = start[ch]++;
        }
        std::string result(n, '\0');
        for (int i = n - 1; i >= 0; --i) {
            result[i] = lastColumn[row];
            row = lf[row];
        }
        result.pop_back();
        return result;
    }
};

class MoveToFrontTransform {
public:
    std::vector<int> encode(const std::string& text) const {
        auto alphabet = initialAlphabet();
        std::vector<int> codes;
        codes.reserve(text.size());
        for (char ch : text) {
            const auto it = std::find(alphabet.begin(), alphabet.end(), ch);
            if (it == alphabet.end()) {
                throw std::invalid_argument("Недопустимый символ");
            }
            const int position = static_cast<int>(it - alphabet.begin());
            codes.push_back(position);
            moveToFront(alphabet, position);
        }
        return codes;
    }

    std::string decode(const std::vector<int>& codes) const {
        auto alphabet = initialAlphabet();
        std::string text;
        text.reserve(codes.size());
        for (int position : codes) {
            if (position < 0 || position >= static_cast<int>(alphabet.size())) {
                throw std::invalid_argument("Код MTF должен быть от 0 до 26");
            }
            text.push_back(alphabet[position]);
            moveToFront(alphabet, position);
        }
        return text;
    }

private:
    static std::array<char, 27> initialAlphabet() {
        std::array<char, 27> alphabet{};
        alphabet[0] = '$';
        for (int i = 1; i <= 26; ++i) {
            alphabet[i] = static_cast<char>('a' + i - 1);
        }
        return alphabet;
    }

    static void moveToFront(std::array<char, 27>& alphabet, int position) {
        const char ch = alphabet[position];
        for (int i = position; i > 0; --i) {
            alphabet[i] = alphabet[i - 1];
        }
        alphabet[0] = ch;
    }
};

struct Run {
    int count;
    int value;
};

class RunLengthEncoding {
public:
    std::vector<Run> encode(const std::vector<int>& codes) const {
        std::vector<Run> runs;
        for (int value : codes) {
            if (!runs.empty() && runs.back().value == value) {
                ++runs.back().count;
            } else {
                runs.push_back({1, value});
            }
        }
        return runs;
    }

    std::vector<int> decode(const std::vector<Run>& runs) const {
        std::size_t size = 0;
        for (const Run& run : runs) {
            if (run.count <= 0 || run.value < 0 || run.value > 26 ||
                static_cast<std::size_t>(run.count) > maxEncodedSize - size) {
                throw std::invalid_argument("Некорректная пара RLE или превышена максимальная длина текста");
            }
            size += static_cast<std::size_t>(run.count);
        }
        std::vector<int> codes;
        codes.reserve(size);
        for (const Run& run : runs) {
            codes.insert(codes.end(), run.count, run.value);
        }
        return codes;
    }

private:
    static constexpr std::size_t maxEncodedSize = 100001;
};

class TextCompressor {
public:
    std::vector<Run> compress(const std::string& text) const {
        if (text.size() > 100000 ||
            !std::all_of(text.begin(), text.end(), [](char ch) {
                return ch >= 'a' && ch <= 'z';
            })) {
            throw std::invalid_argument("Ожидается не более 100000 строчных латинских букв");
        }
        return rle_.encode(mtf_.encode(bwt_.encode(text)));
    }

    std::string decompress(const std::vector<Run>& runs) const {
        return bwt_.decode(mtf_.decode(rle_.decode(runs)));
    }

private:
    BurrowsWheelerTransform bwt_;
    MoveToFrontTransform mtf_;
    RunLengthEncoding rle_;
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    try {
        std::string command;
        if (!(std::cin >> command)) {
            return 0;
        }
        const TextCompressor compressor;
        if (command == "compress") {
            std::string text;
            std::cin >> text;
            for (const Run& run : compressor.compress(text)) {
                std::cout << run.count << ' ' << run.value << '\n';
            }
        } else if (command == "decompress") {
            std::vector<Run> runs;
            int count;
            while (std::cin >> count) {
                int value;
                if (!(std::cin >> value)) {
                    throw std::invalid_argument("Неполная пара RLE");
                }
                runs.push_back({count, value});
            }
            if (!std::cin.eof()) {
                throw std::invalid_argument("Ожидаются пары целых чисел RLE");
            }
            std::cout << compressor.decompress(runs) << '\n';
        } else {
            throw std::invalid_argument("Ожидается команда compress или decompress");
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
