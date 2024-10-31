#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <memory>
#include <unordered_map>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

	// Можете дополнить ваш класс нужными полями и методами

private:
	// Можете дополнить ваш класс нужными полями и методами
    void PrintCellValueToOutput(const CellInterface* cell, std::ostream& output) const;
    void PrintValueOrText(bool IsValue, std::ostream& output) const;

    class PositionHasher {
    public:
        size_t operator()(const Position& pos) const {
            return std::hash<int>()(pos.row) ^ std::hash<int>()(pos.col);
        }
    };

    std::unordered_map<Position, std::unique_ptr<CellInterface>, PositionHasher> cells_;
};