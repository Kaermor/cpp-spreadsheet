#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <stack>

class Cell::Impl {
public:
    virtual ~Impl() = default;
    virtual Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
    virtual std::vector<Position> GetReferencedCells() const {
        return {};
    }
    virtual bool IsCacheValid() const {
        return true;
    }
    virtual void InvalidateCache() {}
};

class Cell::EmptyImpl : public Impl {
public:
    Value GetValue() const override {
        return "";
    }

    std::string GetText() const override {
        return "";
    }
};

class Cell::TextImpl : public Impl {
public:
    TextImpl(std::string text)
        : text_(std::move(text)) {
        if (text_.empty()) {
            throw std::logic_error("");
        }
    }

    Value GetValue() const override {

        if (text_[0] == ESCAPE_SIGN) {
            return text_.substr(1);
        }

        return text_;
    }

    std::string GetText() const override {
        return text_;
    }

private:
    std::string text_;
};

class Cell::FormulaImpl : public Impl {
    public:
    explicit FormulaImpl(std::string expression
                        , const SheetInterface& sheet)
        : sheet_(sheet) {

        if (expression.empty() || expression[0] != FORMULA_SIGN) {
            throw std::logic_error("");
        }

        formula_ptr_ = ParseFormula(expression.substr(1));
    }

    Value GetValue() const override {
        if (!cache_) {
            cache_ = formula_ptr_->Evaluate(sheet_);
        }
        
        auto& value = *cache_;
        if (std::holds_alternative<double>(value)) {
            return std::get<double>(value);
        }

        return std::get<FormulaError>(value);
    }

    std::string GetText() const override {
        return FORMULA_SIGN + formula_ptr_->GetExpression();
    }

    bool IsCacheValid() const override {
        return cache_.has_value();
    }

    void InvalidateCache() override {
        cache_.reset();
    }

    std::vector<Position> GetReferencedCells() const override {
        return formula_ptr_->GetReferencedCells();
    }

private:
    std::unique_ptr<FormulaInterface> formula_ptr_;
    const SheetInterface& sheet_;
    mutable std::optional<FormulaInterface::Value> cache_;
};

bool Cell::IsCircularDependency(const Impl& tmp_impl) const {
    std::vector<Position> referenced_cells_vec = tmp_impl.GetReferencedCells();

    if (referenced_cells_vec.empty()) {
        return false;
    }

    std::unordered_set<const Cell*> referenced_cells;

    for (const Position& pos : referenced_cells_vec) {
        referenced_cells.insert(sheet_.GetCellPtr(pos));
    }

    std::unordered_set<const Cell*> visited;
    std::stack<const Cell*> stack;
    stack.push(this);

    while (!stack.empty()) {
        const Cell* current = stack.top();
        stack.pop();
        visited.insert(current);

        if (referenced_cells.find(current) != referenced_cells.end()) {
            return true;
        }

        for (const Cell* dep_cell : current->dependent_cells_) {
            if (visited.find(dep_cell) == visited.end()) {
                stack.push(dep_cell);
            }
        }
    }

    return false;
}

void Cell::InvalidateDependentCellsCache(bool force) {
    if (!impl_->IsCacheValid() || force) {
        impl_->InvalidateCache();
        for (Cell* dep_cell : dependent_cells_) {
            dep_cell->InvalidateDependentCellsCache(true);
        }
    }
}

Cell::Cell(Sheet& sheet)
: impl_(std::make_unique<EmptyImpl>())
, sheet_(sheet) {}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> tmp_impl;

    if (text.empty()) {
        tmp_impl = std::make_unique<EmptyImpl>();
    } else if (text.size() > 1 && text[0] == FORMULA_SIGN) {
        tmp_impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
    } else {
        tmp_impl = std::make_unique<TextImpl>(std::move(text));
    }

    if (IsCircularDependency(*tmp_impl)) {
        throw CircularDependencyException("");
    }

    impl_ = std::move(tmp_impl);

    for (Cell* ref_cell : referenced_cells_) {
        ref_cell->dependent_cells_.erase(this);
    }

    referenced_cells_.clear();

    for (const auto& pos : impl_->GetReferencedCells()) {
        Cell* ref_cell = sheet_.GetCellPtr(pos);

        if (!ref_cell) {
            sheet_.SetCell(pos, "");
            ref_cell = sheet_.GetCellPtr(pos);
        }

        referenced_cells_.insert(ref_cell);
        ref_cell->dependent_cells_.insert(this);
    }

    InvalidateDependentCellsCache(true);
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    assert(impl_ != nullptr);
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    assert(impl_ != nullptr);
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    assert(impl_ != nullptr);
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
    return !referenced_cells_.empty();
}