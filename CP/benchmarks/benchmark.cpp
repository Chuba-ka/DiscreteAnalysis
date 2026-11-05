#define main compression_program_main
#include "../main.cpp"
#undef main

#include <chrono>
#include <iomanip>

volatile std::size_t benchmarkResult = 0;

template <typename Operation>
void measure(const char* name, Operation operation) {
    using Clock = std::chrono::steady_clock;
    auto batch = [&](int repetitions) {
        const auto begin = Clock::now();
        for (int i = 0; i < repetitions; ++i) {
            benchmarkResult = operation().size();
        }
        return std::chrono::duration<double, std::milli>(Clock::now() - begin).count();
    };
    int repetitions = 1;
    while (batch(repetitions) < 20.0) {
        repetitions *= 2;
    }
    for (int sample = 0; sample < 5; ++sample) {
        const double elapsed = batch(repetitions);
        std::cout << name << ',' << sample << ',' << repetitions << ','
                  << std::setprecision(10) << elapsed / repetitions << '\n';
    }
}

int main() {
    std::string text;
    std::cin >> text;
    const BurrowsWheelerTransform bwt;
    const MoveToFrontTransform mtf;
    const RunLengthEncoding rle;
    const TextCompressor compressor;
    const auto transformed = bwt.encode(text);
    const auto codes = mtf.encode(transformed);
    const auto runs = rle.encode(codes);
    if (compressor.decompress(runs) != text) {
        std::cerr << "Восстановленный текст не совпадает с исходным\n";
        return 1;
    }
    measure("bwt_encode", [&] { return bwt.encode(text); });
    measure("mtf_encode", [&] { return mtf.encode(transformed); });
    measure("rle_encode", [&] { return rle.encode(codes); });
    measure("rle_decode", [&] { return rle.decode(runs); });
    measure("mtf_decode", [&] { return mtf.decode(codes); });
    measure("bwt_decode", [&] { return bwt.decode(transformed); });
    measure("compress", [&] { return compressor.compress(text); });
    measure("decompress", [&] { return compressor.decompress(runs); });
}
