#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <iostream>

using namespace std::literals;

void Sheet::IsCorrectPosition(Position pos) {
    if (!pos.IsValid())
        throw InvalidPositionException("Invalid position"s);
}

bool Sheet::IsOutTablePosition(Position pos) const {
    return pos.row > data_size_.rows || pos.col > data_size_.cols;
}

bool Sheet::IsExtremePosition(Position pos) const {
    return data_size_.rows == pos.row + 1 || data_size_.cols == pos.col + 1;
}

void Sheet::AddPosition(Position pos) {
    if (data_size_.rows <= pos.row)
        data_size_.rows = pos.row + 1;

    if (data_size_.cols <= pos.col)
        data_size_.cols = pos.col + 1;
}

void Sheet::RemovePosition() {
    Size temp_size = Size{-1,-1};
    for (const auto& [pos, cell_ptr] : data_) {
        if (temp_size.rows < pos.row)
            temp_size.rows = pos.row;
        if (temp_size.cols < pos.col)
            temp_size.cols = pos.col;
    }

    data_size_ = {temp_size.rows + 1, temp_size.cols + 1};
}

void Sheet::SetCell(Position pos, std::string text) {
    IsCorrectPosition(pos);

    AddPosition(pos);

    if (IsOutTablePosition(pos) or data_.count(pos) == 0)
        data_[pos] = std::make_unique<Cell>(*this);

    data_[pos]->Set(text);
    //std::cerr << data_size_.rows << ", " << data_size_.cols << std::endl;
}

const CellInterface* Sheet::GetCell(Position pos) const {
    IsCorrectPosition(pos);
    if (IsOutTablePosition(pos) or data_.count(pos) == 0)
        return nullptr;
    return data_.at(pos).get();
}

CellInterface* Sheet::GetCell(Position pos) {
    return const_cast<CellInterface*>(static_cast<const Sheet*>(this)->GetCell(pos));
}

void Sheet::ClearCell(Position pos) {
    IsCorrectPosition(pos);

    if (!GetCell(pos)) return;

    data_.erase(pos);

    if (IsExtremePosition(pos))
        RemovePosition();
}

Size Sheet::GetPrintableSize() const {
    return data_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int row = 0; row < data_size_.rows; row++) {
        for (int col = 0; col < data_size_.cols; col++) {
            auto cell = GetCell(Position{row, col});
            Cell::Value value = cell ? cell->GetValue() : Cell::Value();

            switch (value.index()) {
                case 0: {
                    output << std::get<std::string>(value);
                    break;
                }
                case 1: {
                    output << std::get<double>(value);
                    break;
                }
                case 2: {
                    output << std::get<FormulaError>(value);
                    break;
                }
                default:
                    assert(false);
            }
            output << (col + 1 == data_size_.cols ? ""s : "\t"s);
        }
        output << "\n"s;
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for (int row = 0; row < data_size_.rows; ++row) {
        for (int col = 0; col < data_size_.cols; ++col) {
            const CellInterface* cell = GetCell({row, col});
            if (cell) {
                output << cell->GetText();
            }
            output << (col + 1 == data_size_.cols ? ""s : "\t"s);
        }
        output << "\n"s;
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}