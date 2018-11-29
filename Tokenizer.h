#include <utility>

//
// Created by ottomated on 27/11/18.
//

#ifndef REQLANG_TOKENIZER_H
#define REQLANG_TOKENIZER_H

#include "Token.h"
#include <optional>
#include <sstream>

using namespace std;

class Tokenizer {
private:
    int pos{};
    int line_number;
    int col_number;
    string current_line;

    Token parseNumber(char start);

    Token parseString();

    Token parseName(char start);

    Token parseChar();

public:
    explicit Tokenizer(string p);

    string program;
    Token currentToken;

    Token getNextToken();

    optional<char> peek();

    optional<char> next();

    void unnext();
};

class TokenizerException : public runtime_error {
private:
public:
    TokenizerException(const string &message, int line_number, int col_number, const string &line) :
            runtime_error("") {
        stringstream ss;
        ss << message << endl
           << line << endl
           << string(col_number, ' ') << "^~~~" << endl
           << "  at line " << line_number << " col " << col_number;
        static_cast<std::runtime_error &>(*this) = std::runtime_error(ss.str());
    }
};

#endif //REQLANG_TOKENIZER_H
