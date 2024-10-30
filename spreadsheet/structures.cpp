#include "common.h"

#include <cassert>
#include <cctype>
#include <cmath>
#include <deque>
#include <sstream>
#include <string>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = {-1, -1};

// Реализуйте методы:
bool Position::operator==(Position rhs) const {
    return row == rhs.row && col == rhs.col;
}

bool Position::operator<(Position rhs) const {
    return std::tie(row, col) < std::tie(rhs.row, rhs.col);
}

bool Size::operator==(Size rhs) const {
    return rows == rhs.rows && cols == rhs.cols;
}

bool Position::IsValid() const {
    if (row >= 0 && col >= 0 && row < 16384 && col < 16384) {
        return true;
    }
    return false;
}

char ConvertIntToLetter(int index) {
    assert(index > 0 && index < 27);
    return 'A' + index - 1;
}

void ConvertIntIndexToLetterRecursion(int index, std::deque<char>& result) {
    int new_order = index / 26;
    int rest = index % 26 + 1;
    
    result.push_front(ConvertIntToLetter(rest));

    if (new_order == 0) {
        return;
    }
    if (new_order < 27) {
        result.push_front(ConvertIntToLetter(new_order));
        return;
    }

    ConvertIntIndexToLetterRecursion(new_order - 1, result);
}

std::string ConvertDequeToString(std::deque<char> input) {
    std::string result;
    for (char c : input) {
        result.append(1, c);
    }
    return result;
}

// index from 1 (A == 1)
std::string ConvertIntIndexToLetter(int index) {
    // 28 -> AB
    // 680 -> YE

    std::deque<char> result;

    ConvertIntIndexToLetterRecursion(index, result);

    return ConvertDequeToString(result);
}

std::string Position::ToString() const {
    // if not valid, return empty string
    if (!IsValid()) {
        return "";
    }

    std::string result;

    int horizontal_index = col; // единица прибавится внутри функции Convert...
    int vertical_index = row + 1;

    std::string_view horizontal_index_letter = ConvertIntIndexToLetter(horizontal_index);

    result.append(horizontal_index_letter);
    result.append(std::to_string(vertical_index));

    return result;
}

// working with numbers from 1 (A == 1)
int ConvertLetterToNumber(char letter) {
    return letter - 'A' + 1;
}

// working with numbers from 1 (A == 1)
int ConvertLetterIndexToInt(std::string_view horizontal_index) {
    int result = 0;

    for (size_t i = 1, j = 0; i <= horizontal_index.size(); ++i, ++j) {
        int num = ConvertLetterToNumber(horizontal_index[horizontal_index.size() - i]);
        if (j == 0) {
            result += num;
        }
        else if (j > 0) {
            result += pow(LETTERS, j) * num; // 26^1 * A == 26^1 * 1 || 26^2 * A == 676 * A
        }
    }

    return result;
}

Position Position::FromString(std::string_view str) {
    // if not valid or non-format, return Position::NONE
    std::string alfabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string numbers = "0123456789";

    if (str.empty()) {
        return Position::NONE;
    }

    size_t first_symbol = 0;
    
    if (alfabet.find(str[first_symbol]) == std::string::npos) {
        return Position::NONE;
    }

    size_t next_non_letter = str.find_first_not_of(alfabet, first_symbol + 1);

    if (next_non_letter == std::string::npos) {
        return Position::NONE;
    }
    if (numbers.find(str[next_non_letter]) == std::string::npos) {
        return Position::NONE;
    }

    size_t next_after_number = str.find_first_not_of(numbers, next_non_letter + 1);

    if (next_after_number != std::string::npos) {
        return Position::NONE;
    }
    
    std::string_view horizontal_index = str.substr(0, next_non_letter);
    std::string_view vertical_index = str.substr(next_non_letter, str.size() - next_non_letter);

    if (horizontal_index.size() > 3 || vertical_index.size() > 5) {
        return Position::NONE;
    }

    int horizontal_index_int = ConvertLetterIndexToInt(horizontal_index);
    int vertical_index_int = std::stoi(std::string(vertical_index));

    Position pos;
    pos.col = horizontal_index_int - 1;
    pos.row = vertical_index_int - 1;

    if (!pos.IsValid()) {
        return Position::NONE;
    }

    return pos;
}