#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

// Реализуйте следующие методы
Cell::Cell(Sheet& sheet)
    : sheet_(sheet) {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    }
    else if (text[0] == FORMULA_SIGN && text.size() > 1) {
        impl_ = std::make_unique<FormulaImpl>(std::move(text), sheet_);
        referenced_cells_ = dynamic_cast<FormulaImpl*>(impl_.get())->GetReferencedCells();
        for (auto pos : referenced_cells_) {
            if (!sheet_.GetCell(pos)) {
                sheet_.SetCell(pos, "");
            }
        }
    }
    else {
        impl_ = std::make_unique<TextImpl>(std::move(text));
    }
}

void Cell::Clear() {
    InvalidateCache();
    impl_ = std::make_unique<EmptyImpl>();
    referenced_cells_.clear();
}

Cell::Value Cell::GetValue() const {
    if (cache_.has_value()) {
        return cache_.value();
    }
    cache_ = impl_->GetValue();
    return cache_.value();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

CellInterface::Value Cell::EmptyImpl::GetValue() const {
    return 0.0;
}

std::string Cell::EmptyImpl::GetText() const {
    return "";
}

Cell::TextImpl::TextImpl(std::string text) {
    text_ = std::move(text);
}

CellInterface::Value Cell::TextImpl::GetValue() const {
    if (text_[0] == ESCAPE_SIGN) {
        return text_.substr(1, text_.size() - 1);
    }
    try {
        return std::stod(text_);
    }
    catch (...) {
        return text_;
    }
}

std::string Cell::TextImpl::GetText() const {
    return text_;
}

Cell::FormulaImpl::FormulaImpl(std::string text, Sheet& sheet)
    : formula_impl_sheet_(sheet) {
    formula_ = ParseFormula(text.substr(1, text.size() - 1));
}

CellInterface::Value Cell::FormulaImpl::GetValue() const {
    FormulaInterface::Value result = formula_->Evaluate(formula_impl_sheet_);
    if (std::holds_alternative<double>(result)) {
        return std::get<double>(result);
    }
    else if (std::holds_alternative<FormulaError>(result)) {
        return std::get<FormulaError>(result);
    }
    throw std::runtime_error("Unknown error");
}

std::string Cell::FormulaImpl::GetText() const {
    std::string result = "";
    result.push_back(FORMULA_SIGN);
    return result.append(formula_->GetExpression());
}

bool Cell::HasCyclicDependencies(const Cell* root_cell, std::vector<Position> cells) const {
    for (Position pos : cells) {
        if (root_cell == sheet_.GetCell(pos)) {
            return true;
        }
        if (sheet_.GetCell(pos) && !sheet_.GetCell(pos)->GetReferencedCells().empty()) {
            return HasCyclicDependencies(root_cell, sheet_.GetCell(pos)->GetReferencedCells());
        }
    }
    return false;
}

void Cell::InvalidateCurrentCache() {
    cache_ = {};
}

void Cell::InvalidateCache() {
    InvalidateCurrentCache();
    if (!dependent_cells_.empty()) {
        for (Position pos : dependent_cells_) {
            CellInterface* cell = sheet_.GetCell(pos);
            dynamic_cast<Cell*>(cell)->InvalidateCache();
        }
    }
}

std::vector<Position> Cell::GetReferencedCells() const {
    return referenced_cells_;
}

bool Cell::IsReferenced() const {
    return !dependent_cells_.empty();
}