#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

FormulaError::FormulaError(Category category) 
    : category_(category) {
}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
        case Category::Arithmetic:
        return "#ARITHM!";
        case Category::Ref:
        return "#REF!";
        case Category::Value:
        return "#VALUE!";
        default:
        return "";
    }
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression)
        : ast_(ParseFormulaAST(expression)) {}
    
    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            auto func = [&sheet](Position pos) -> CellInterface::Value {
                if (!pos.IsValid()) {
                    throw FormulaError(FormulaError::Category::Ref);
                }
                if (pos.IsValid() && !sheet.GetCell(pos)) {
                    return 0.0;
                }
                return sheet.GetCell(pos)->GetValue();
            };

            return ast_.Execute(func);
        }
        catch (const FormulaError& fe) {
            return fe;
        }
    }

    std::string GetExpression() const override {
        std::ostringstream oss;
        ast_.PrintFormula(oss);
        return oss.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        auto cells = ast_.GetCells();
        std::vector<Position> result;
        for (auto cell : cells) {
            result.push_back(cell);
        }
        result.resize(std::unique(result.begin(), result.end()) - result.begin());
        return result;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}