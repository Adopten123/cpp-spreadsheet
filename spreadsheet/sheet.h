#pragma once

#include "cell.h"
#include "common.h"
#include <unordered_map>

#include <functional>

class Sheet final : public SheetInterface {
public:
    ~Sheet() override = default;

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:

    struct DataHasher {
        size_t operator()(Position pos) const{
            return pos.row + pos.col*59;
        }
    };

    using SheetData = std::unordered_map<Position, std::unique_ptr<Cell>, DataHasher>;

    SheetData data_;
    Size data_size_ = {0, 0};

    static void IsCorrectPosition(Position pos);
    bool IsOutTablePosition(Position pos) const;
    bool IsExtremePosition(Position pos) const;

    void AddPosition(Position pos);
    void RemovePosition();
};