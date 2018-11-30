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
    bool parsingHeaderBlock;
    string header_block_name;

    Token parseNumber(char start);
    Token parseString(char closing);
    Token parseName(char start);
    Token parseChar();
    Token parseHeaderKey(char start);
    Token parseHeaderValue(char start);

    Token parseURL();

public:
    explicit Tokenizer(string p);

    string program;
    Token currentToken;

    Token getNextToken();
    Token getNextHeaderToken();

    optional<char> peek();
    optional<char> peek(int n);

    optional<char> next();
    void next(int n);

    void unnext();
};

class TokenizerException : public runtime_error {
private:
public:
    TokenizerException(const string &message, int line_number, int col_number, const string &line) :
            runtime_error("") {
        stringstream ss;
        cout << line << endl;
        ss << message << '\n'
           << line << '\n'
           << string(col_number, ' ') << "^~~~" << '\n'
           << "  at line " << line_number << " col " << col_number;
        static_cast<std::runtime_error &>(*this) = std::runtime_error(ss.str());
    }
};

#endif //REQLANG_TOKENIZER_H
