#include "common.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std;

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = {-1, -1};

// Реализуйте методы:
bool Position::operator==(const Position rhs) const {
    return row == rhs.row && col == rhs.col;
}

bool Position::operator<(const Position rhs) const {
    return std::tie(row, col) < std::tie(rhs.row, rhs.col);
}

bool Position::IsValid() const {
    return row >= 0 and row < MAX_ROWS and col >= 0 and col < MAX_COLS;
}

std::string Position::ToString() const {
    using namespace std::literals;

    if (!IsValid()) return ""s;

    std::string column_str = ""s;
    int tmp_column = col;

    while (tmp_column >= 0) {
        column_str = (char)('A' + (tmp_column % LETTERS)) + column_str;
        tmp_column = tmp_column / LETTERS - 1;
    }

    return column_str + std::to_string(row + 1);
}

Position Position::FromString(std::string_view str) {
    if (str.empty() || str.size() > MAX_POSITION_LENGTH)
        return Position::NONE;

    size_t i = 0;
    for (;i < str.size() && std::isalpha(str[i]); i++) {
        if (!std::isupper(str[i]) || i >= MAX_POS_LETTER_COUNT) return Position::NONE;
    }

    if (i == 0 || i == str.size()) return Position::NONE;

    std::string_view column_str = str.substr(0, i);
    std::string_view row_str = str.substr(i);

    if (row_str.empty() || row_str[0] == '0' || !std::all_of(row_str.begin(), row_str.end(), ::isdigit))
        return Position::NONE;

    int row = std::stoi(std::string(row_str)) - 1;

    if (row < 0 || row >= MAX_ROWS)
        return Position::NONE;

    int col = 0;
    for (char c : column_str) {
        col *= LETTERS;
        col += (c - 'A' + 1);
    }
    --col;

    if (col < 0 || col >= MAX_COLS) return Position::NONE;

    return Position{row, col};
}

bool Size::operator==(Size rhs) const {
    return cols == rhs.cols && rows == rhs.rows;
}


