//
// Created by ottomated on 27/11/18.
//

#ifndef REQLANG_TOKENIZER_H
#define REQLANG_TOKENIZER_H

#include "Token.h"
#include <optional>

using namespace std;

class Tokenizer {
private:
    int pos{};
    Token parseNumber(char start);
    Token parseString();
public:
    string program;
    Token currentToken;
    explicit Tokenizer(string p);
    Token getNextToken();
    char peek();
    char next();
};


#endif //REQLANG_TOKENIZER_H
