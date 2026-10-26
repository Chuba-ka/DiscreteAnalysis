#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

enum class Operation : char {
    None = 0,
    Subtract = 1,
    DivideBy2 = 2,
    DivideBy3 = 3
};

struct TransformationResult {
    long long cost = 0;
    vector<Operation> operations;
};

class MinCostTransformer {
public:
    TransformationResult transform(int n) const {
        vector<long long> dp(n + 1, 0);
        vector<Operation> op(n + 1, Operation::None);

        for (int i = 2; i <= n; ++i) {
            long long best = dp[i - 1];
            Operation bestOp = Operation::Subtract;

            if (i % 2 == 0 && dp[i / 2] < best) {
                best = dp[i / 2];
                bestOp = Operation::DivideBy2;
            }
            if (i % 3 == 0 && dp[i / 3] < best) {
                best = dp[i / 3];
                bestOp = Operation::DivideBy3;
            }

            dp[i] = best + i;
            op[i] = bestOp;
        }

        TransformationResult result;
        result.cost = dp[n];
        result.operations = restorePath(n, op);
        return result;
    }

private:
    static int apply(int value, Operation o) {
        switch (o) {
            case Operation::Subtract:  return value - 1;
            case Operation::DivideBy2: return value / 2;
            case Operation::DivideBy3: return value / 3;
            default:                   return value;
        }
    }

    static vector<Operation> restorePath(int n, const vector<Operation>& op) {
        vector<Operation> path;
        for (int cur = n; cur > 1; cur = apply(cur, op[cur])) {
            path.push_back(op[cur]);
        }
        return path;
    }
};

class OperationFormatter {
public:
    static string toString(Operation o) {
        switch (o) {
            case Operation::Subtract:  return "-1";
            case Operation::DivideBy2: return "/2";
            case Operation::DivideBy3: return "/3";
            default:                   return "";
        }
    }

    static string format(const vector<Operation>& ops) {
        string s;
        for (size_t i = 0; i < ops.size(); ++i) {
            if (i) s += ' ';
            s += toString(ops[i]);
        }
        return s;
    }
};

class InputReader {
public:
    explicit InputReader(istream& in) : in_(in) {}

    bool readNumber(int& n) {
        return static_cast<bool>(in_ >> n);
    }

private:
    istream& in_;
};

class ResultWriter {
public:
    explicit ResultWriter(ostream& out) : out_(out) {}

    void write(const TransformationResult& r) {
        out_ << r.cost << "\n"
             << OperationFormatter::format(r.operations) << "\n";
    }

private:
    ostream& out_;
};

class Application {
public:
    Application(InputReader reader, ResultWriter writer, MinCostTransformer transformer)
        : reader_(reader), writer_(writer), transformer_(transformer) {}

    int run() {
        int n;
        if (!reader_.readNumber(n)) return 0;

        TransformationResult result = transformer_.transform(n);
        writer_.write(result);
        return 0;
    }

private:
    InputReader reader_;
    ResultWriter writer_;
    MinCostTransformer transformer_;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    InputReader reader(cin);
    ResultWriter writer(cout);
    MinCostTransformer transformer;

    Application app(reader, writer, transformer);
    return app.run();
}

