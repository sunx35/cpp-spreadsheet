#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <memory>
#include <optional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;
    bool HasCyclicDependencies(const Cell* root_cell, std::vector<Position> cells) const;

private:
    void InvalidateCache();
    void InvalidateCurrentCache();

    class Impl {
    public:
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
    };

    class EmptyImpl : public Impl {
    public:
        Value GetValue() const override;
        std::string GetText() const override;
    };

    class TextImpl : public Impl {
    public:
        TextImpl(std::string text);

        Value GetValue() const override;
        std::string GetText() const override;
    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl(std::string text, Sheet& sheet);

        Value GetValue() const override;
        std::string GetText() const override;

        std::vector<Position> GetReferencedCells() const {
            return formula_->GetReferencedCells();
        }
    private:
        std::unique_ptr<FormulaInterface> formula_;
        Sheet& formula_impl_sheet_;
    };

    std::unique_ptr<Impl> impl_;
    mutable std::optional<Value> cache_;
    std::vector<Position> referenced_cells_ = {};
    std::vector<Position> dependent_cells_ = {};
    Sheet& sheet_;
};