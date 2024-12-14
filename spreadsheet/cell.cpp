#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

using namespace std::literals;


//-------------------------------------Cell--------------------------------------------
Cell::Cell(SheetInterface& sheet)
    : sheet_(sheet)
    , impl_(std::make_unique<EmptyImpl>()) {
}

void Cell::Set(std::string text) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    } else if (text[0] == FORMULA_SIGN and text.size() > 1) {
        std::unique_ptr<Impl> impl = std::make_unique<FormulaImpl>(text.substr(1), sheet_);
        const std::vector<Position>& referenced_cells = impl->GetReferencedCells();

        std::unordered_set<CellInterface*> visited_cells;
        if (IsCircular(referenced_cells, visited_cells))
            throw CircularDependencyException("Circular dependency was found!"s);

        if (!referenced_cells.empty())
            UpdateReferencedCells(referenced_cells);
        impl_ = std::move(impl);
    } else {
        impl_ = std::make_unique<TextImpl>(std::move(text));
    }
    InvalidateCache();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
    dependent_cells_.clear();
    referenced_cells_.clear();
    InvalidateCache();
}

CellInterface::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

void Cell::UpdateReferencedCells(const std::vector<Position>& positions) {
    for (Cell* cell : dependent_cells_) {
        if (cell)
            cell->dependent_cells_.erase(this);
    }

    for (const Position& position : positions) {
        if (!sheet_.GetCell(position))
            sheet_.SetCell(position, ""s);

        if (Cell* referenced_cell = dynamic_cast<Cell*>(sheet_.GetCell(position))) {
            referenced_cells_.insert(referenced_cell);
            referenced_cell->dependent_cells_.insert(this);
        }
    }
}

bool Cell::IsCircular(const std::vector<Position>& positions,
    std::unordered_set<CellInterface*>& visited_cells) {

    for (const Position& pos : positions) {
        CellInterface* cell = sheet_.GetCell(pos);
        if (this == cell)
            return true;

        if (!cell or visited_cells.count(cell)) continue;

        if (IsCircular(cell->GetReferencedCells(), visited_cells))
            return true;

        visited_cells.insert(cell);
    }
    return false;
}


void Cell::InvalidateCache(std::unordered_set<Cell*>& visited_cells) {
    for (const auto& cell : dependent_cells_) {
        cell->impl_->ClearCache();

        if (visited_cells.count(cell)) continue;

        if (!cell->dependent_cells_.empty())
            cell->InvalidateCache(visited_cells);
        visited_cells.insert(cell);
    }
}

void Cell::InvalidateCache() {
    impl_->ClearCache();
    std::unordered_set<Cell*> visited_cells;
    InvalidateCache(visited_cells);
}

//-------------------------------------Cell--------------------------------------------

//----------------------------Impl---------------------------------------------

std::vector<Position> Cell::Impl::GetReferencedCells() const {
    return {};
}

void Cell::Impl::ClearCache() {
    return;
}

std::optional<FormulaInterface::Value> Cell::Impl::GetCachedValue() const{
    return nullptr;
}

//----------------------------Impl---------------------------------------------

//----------------------------EmptyImpl---------------------------------------------
CellInterface::Value Cell::EmptyImpl::GetValue() const {
    return 0.0;
}

std::string Cell::EmptyImpl::GetText() const {
    return ""s;
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
    return  {};
}
//----------------------------EmptyImpl---------------------------------------------

//----------------------------TextImpl---------------------------------------------
Cell::TextImpl::TextImpl(std::string text) : value_(std::move(text)) {}

CellInterface::Value Cell::TextImpl::GetValue() const {
    if(!value_.empty() and value_[0] == ESCAPE_SIGN)
        return value_.substr(1);
    return value_;
}

std::string Cell::TextImpl::GetText() const {
    return value_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
    return  {};
}
//----------------------------TextImpl---------------------------------------------

//----------------------------FormulaImpl------------------------------------------
Cell::FormulaImpl::FormulaImpl(std::string text, const SheetInterface& sheet)
    : sheet_(sheet)
    , formula_(ParseFormula(std::move(text))) {
}

Cell::Value Cell::FormulaImpl::GetValue() const {
    if (!cached_value_.has_value()) {
        cached_value_ = formula_->Evaluate(sheet_);
    }

    switch (cached_value_.value().index()) {
        case 0:
            return std::get<std::string>(cached_value_.value());
        case 1:
            return std::get<double>(cached_value_.value());
        case 2:
            return std::get<FormulaError>(cached_value_.value());
        default:
            cached_value_ = 0.0;
            return 0.0;
    }
}

std::string Cell::FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

std::optional<FormulaInterface::Value> Cell::FormulaImpl::GetCachedValue() const {
    return cached_value_;
}

void Cell::FormulaImpl::ClearCache() {
    cached_value_.reset();
}

//----------------------------FormulaImpl------------------------------------------

