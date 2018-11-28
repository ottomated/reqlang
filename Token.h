#include <utility>


//
// Created by ottomated on 27/11/18.
//

#ifndef REQLANG_TOKEN_H
#define REQLANG_TOKEN_H

#include <variant>
#include <string>
#include <iostream>

using namespace std;
using tokenValue = variant<int, double, string, bool, char>;

enum TokenType {
    LeftParen, RightParen, LeftBracket, RightBracket, LeftBrace, RightBrace, Semicolon, Colon, Comma, Dot, Eof,
    Plus, Minus, Multiply, Divide, Modulo,
    SingleEquals, Increment, Decrement, PlusEquals, MinusEquals, MultiplyEquals, DivideEquals, ModuloEquals,
    LessThan, GreaterThan, LessThanEquals, GreaterThanEquals, DoubleEquals, NotEquals,
    And, Or, BinaryAnd, BinaryOr, Not, BinaryXor,
    Number, Boolean, String, Char,
    Name, Empty
};
static const char *TokenNames[] = {"(", ")", "[", "]", "{", "}", ";", ":", ",", ".", "EOF", "+", "-", "*", "/", "%",
                                   "=", "++", "--", "+=", "-=", "*=", "/=", "%=", "<", ">", "<=", ">=", "==", "!=",
                                   "&&",
                                   "||", "&", "|", "!", "^", "Number", "Boolean", "String", "Char", "Name",
                                   ""};

class Token {
public:

    TokenType type;
    tokenValue value;

    Token(TokenType t, tokenValue v) : type(t), value(std::move(v)) {};

    explicit Token(TokenType t) : type(t), value(0) {};

    Token() : type(Empty), value(0) {}

};

std::ostream &operator<<(std::ostream &stream, const Token &token);

std::ostream &operator<<(std::ostream &stream, const TokenType &token);

#endif //REQLANG_TOKEN_H
