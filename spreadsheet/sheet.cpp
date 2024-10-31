#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
    std::unique_ptr<Cell> cell = std::make_unique<Cell>(*this);
    cell->Set(text);
    if (cell->HasCyclicDependencies(dynamic_cast<Cell*>(GetCell(pos)), cell->GetReferencedCells())) {
        throw CircularDependencyException("Circular dependency");
    }
    if (!GetCell(pos)) {
        cells_.insert({pos, std::move(cell)});
    }
    else {
        dynamic_cast<Cell*>(GetCell(pos))->Clear();
        dynamic_cast<Cell*>(GetCell(pos))->Set(text);
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
    if (!cells_.count(pos)) {
        return nullptr;
    }
    return cells_.at(pos).get();
}

CellInterface* Sheet::GetCell(Position pos) {
    return const_cast<CellInterface*>(static_cast<const Sheet&>(*this).GetCell(pos));
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }
    if (GetCell(pos) && dynamic_cast<Cell*>(GetCell(pos))->IsReferenced()) {
        dynamic_cast<Cell*>(GetCell(pos))->Clear();
    }
    cells_.erase(pos);
}

Size Sheet::GetPrintableSize() const {
    auto it_y = std::max_element(cells_.begin(), cells_.end(), [](const auto& a, const auto& b) {
        return a.first.row < b.first.row;
    });
    auto it_x = std::max_element(cells_.begin(), cells_.end(), [](const auto& a, const auto& b) {
        return a.first.col < b.first.col;
    });

    if (it_y == cells_.end() || it_x == cells_.end()) {
        return {0, 0};
    }

    int y_max_size = it_y->first.row + 1;
    int x_max_size = it_x->first.col + 1;

    return {y_max_size, x_max_size};
}

void Sheet::PrintCellValueToOutput(const CellInterface* cell, std::ostream& output) const {
    auto value = cell->GetValue();
    if (std::holds_alternative<std::string>(value)) {
        output << std::get<std::string>(value);
    }
    else if (std::holds_alternative<double>(value)) {
        output << std::get<double>(value);
    }
    else if (std::holds_alternative<FormulaError>(value)) {
        output << std::get<FormulaError>(value);
    }
}

void Sheet::PrintValueOrText(bool IsValue, std::ostream& output) const {
    for (int i = 0; i < GetPrintableSize().rows; ++i) {
        bool first = true;
        for (int j = 0; j < GetPrintableSize().cols; ++j) {
            if (!first) {
                output << '\t';
            }
            first = false;
            if (auto cell = GetCell(Position{i, j})) {
                if (IsValue) {
                    PrintCellValueToOutput(cell, output);
                }
                else {
                    output << cell->GetText();
                }
            }
            // else -> print nothing (except \t).
        }
        output << '\n';
    }
}

void Sheet::PrintValues(std::ostream& output) const {
    PrintValueOrText(true, output);
}
void Sheet::PrintTexts(std::ostream& output) const {
    PrintValueOrText(false, output);
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}