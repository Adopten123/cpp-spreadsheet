#pragma once

#include "common.h"
#include "formula.h"

#include <optional>

class Cell final : public CellInterface {
public:
    using Value = std::variant<std::string, double, FormulaError>;

    explicit Cell(SheetInterface& sheet);

    void Set(std::string text) override;
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    SheetInterface& sheet_;
    std::unique_ptr<Impl> impl_;

    std::set<Cell*> referenced_cells_;
    std::set<Cell*> dependent_cells_;

    bool IsCircular(const std::vector<Position>& positions, std::unordered_set<CellInterface*>& visited_cells);
    void UpdateReferencedCells(const std::vector<Position>& positions);
    void InvalidateCache(std::unordered_set<Cell*>& visited_cells);
    void InvalidateCache();


    class Impl {
    public:
        virtual ~Impl() = default;

        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const;
        virtual void ClearCache();
        virtual std::optional<Value> GetCachedValue() const;
    };

    class EmptyImpl : public Impl {
    public:
        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
    };

    class TextImpl : public Impl {
    public:
        explicit TextImpl(std::string text);

        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
    private:
        std::string value_;
    };

    class FormulaImpl : public Impl {
    public:
        explicit FormulaImpl(std::string text, const SheetInterface& sheet);

        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        std::optional<Value> GetCachedValue() const;
        void ClearCache() override;
    private:
        const SheetInterface& sheet_;
        std::unique_ptr<FormulaInterface> formula_;
        mutable std::optional<Value> cached_value_;
    };
};
