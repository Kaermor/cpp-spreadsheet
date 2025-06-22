#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class CellHasher {
public:
    std::size_t operator()(const Position& pos) const {
        return std::hash<int>()(pos.row) ^ std::hash<int>()(pos.col);
    }
};

class CellComparator {
public:
    bool operator()(const Position& lhs, const Position& rhs) const {
        return lhs == rhs;
    }
};

class Sheet : public SheetInterface {
public:
    using Table = std::unordered_map<Position
                                    , std::unique_ptr<Cell>
                                    , CellHasher
                                    , CellComparator>;

    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    const Cell* GetCellPtr(Position pos) const;
    Cell* GetCellPtr(Position pos);

private:
    Table cells_;
    std::unordered_map<int, int> row_sizes_;
    std::unordered_map<int, int> col_sizes_;

    void ValidatePosition(const Position& pos) const;
};