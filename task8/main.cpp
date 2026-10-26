#include <algorithm>
#include <cmath>
#include <iostream>
#include <utility>
#include <vector>

struct Supplement {
    std::vector<long double> substances;
    int price;
    int index;
};

class LinearBasis {
public:
    explicit LinearBasis(int dimension) : dimension_(dimension) {}

    bool Add(std::vector<long double> row) {
        for (std::size_t i = 0; i < rows_.size(); ++i) {
            const int pivot = pivots_[i];
            const long double factor = row[pivot];
            for (int j = 0; j < dimension_; ++j) {
                row[j] -= factor * rows_[i][j];
            }
            row[pivot] = 0;
        }

        int pivot = -1;
        for (int j = 0; j < dimension_; ++j) {
            if (pivot == -1 || std::abs(row[j]) > std::abs(row[pivot])) {
                pivot = j;
            }
        }
        if (pivot == -1 || std::abs(row[pivot]) < kEpsilon) {
            return false;
        }

        const long double divisor = row[pivot];
        for (long double& value : row) {
            value /= divisor;
        }
        row[pivot] = 1;
        pivots_.push_back(pivot);
        rows_.push_back(std::move(row));
        return true;
    }

private:
    static constexpr long double kEpsilon = 1e-12L;
    int dimension_;
    std::vector<int> pivots_;
    std::vector<std::vector<long double>> rows_;
};

class SupplementSelector {
public:
    explicit SupplementSelector(int substance_count)
        : substance_count_(substance_count) {}

    std::vector<int> Select(std::vector<Supplement> supplements) const {
        if (supplements.size() < static_cast<std::size_t>(substance_count_)) {
            return {};
        }
        std::sort(supplements.begin(), supplements.end(),
                  [](const Supplement& lhs, const Supplement& rhs) {
                      return lhs.price < rhs.price;
                  });

        LinearBasis basis(substance_count_);
        std::vector<int> selected;
        selected.reserve(substance_count_);
        for (Supplement& supplement : supplements) {
            if (basis.Add(std::move(supplement.substances))) {
                selected.push_back(supplement.index);
                if (selected.size() == static_cast<std::size_t>(substance_count_)) {
                    std::sort(selected.begin(), selected.end());
                    return selected;
                }
            }
        }
        return {};
    }

private:
    int substance_count_;
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int supplement_count;
    int substance_count;
    if (!(std::cin >> supplement_count >> substance_count)) {
        return 0;
    }

    std::vector<Supplement> supplements(supplement_count);
    for (int i = 0; i < supplement_count; ++i) {
        Supplement& supplement = supplements[i];
        supplement.substances.resize(substance_count);
        for (long double& amount : supplement.substances) {
            std::cin >> amount;
        }
        std::cin >> supplement.price;
        supplement.index = i + 1;
    }

    SupplementSelector selector(substance_count);
    const std::vector<int> selected = selector.Select(std::move(supplements));
    if (selected.empty()) {
        std::cout << "-1\n";
    } else {
        for (std::size_t i = 0; i < selected.size(); ++i) {
            if (i != 0) {
                std::cout << ' ';
            }
            std::cout << selected[i];
        }
        std::cout << '\n';
    }
    return 0;
}
