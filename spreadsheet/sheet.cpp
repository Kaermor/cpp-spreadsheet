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

    const auto cell_it = cells_.find(pos);

    if (cell_it == cells_.end()) {
        cells_.emplace(pos, std::make_unique<Cell>(*this));
    }

    cells_.at(pos)->Set(std::move(text));

    ++row_sizes_[pos.row];
    ++col_sizes_[pos.col]; 
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return GetCellPtr(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    return GetCellPtr(pos);
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    const auto cell_it = cells_.find(pos);
    if (cell_it != cells_.end() && cell_it->second) {
        cell_it->second->Clear();
        if (!cell_it->second->IsReferenced()) {
            cell_it->second.reset();
        }
    }

    --row_sizes_[pos.row];
    --col_sizes_[pos.col];

    if (row_sizes_[pos.row] <= 0) {
        row_sizes_.erase(pos.row);
    }

    if (col_sizes_[pos.col] <= 0) {
        col_sizes_.erase(pos.col);
    }
    
    if (cells_.empty()) {
        row_sizes_.clear();
        col_sizes_.clear();
    }
}

Size Sheet::GetPrintableSize() const {
    Size size;
    for (const auto& [row, count] : row_sizes_) {
        size.rows = std::max(size.rows, row + 1);
    }
    for (const auto& [col, count] : col_sizes_) {
        size.cols = std::max(size.cols, col + 1);
    }
    return size;
}

void Sheet::PrintValues(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            Position pos{row, col};
            const auto cell_it = cells_.find(pos);
            if (cell_it != cells_.end() && cell_it->second) {
                auto value = cell_it->second->GetValue();
                std::visit(
                    [&](const auto& x) {
                        output << x;
                    },
                    value);
            }
            if (col < size.cols - 1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            Position pos{row, col};
            const auto cell_it = cells_.find(pos);
            if (cell_it != cells_.end() && cell_it->second) {
                output << cell_it->second->GetText();
            }
            if (col < size.cols - 1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

const Cell* Sheet::GetCellPtr(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    const auto cell_it = cells_.find(pos);
    if (cell_it != cells_.end()) {
        return cell_it->second.get();
    }

    return nullptr;
}

Cell* Sheet::GetCellPtr(Position pos) {
    return const_cast<Cell*>(static_cast<const Sheet*>(this)->GetCellPtr(pos));
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}