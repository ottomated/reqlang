//
// Created by ottomated on 27/11/18.
//

#include "Tokenizer.h"
#include <cctype>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <optional>

using namespace std;

Tokenizer::Tokenizer(string p) : program(std::move(p)), currentToken(Token(Empty, 0)), pos(0), line_number(1),
                                 col_number(0) {}

Token Tokenizer::getNextToken() {
    char c;
    optional<char> _;
    // Get next char of program
    _ = next();
    if (!_.has_value()) {
        currentToken = Token(Eof);
        return currentToken;
    }
    c = _.value();

    // Ignore whitespace
    while (isspace(c)) {
        _ = next();
        if (!_.has_value()) {
            currentToken = Token(Eof);
            return currentToken;
        }
        c = _.value();
    }
    if (!c) {
        currentToken = Token(Eof);
        return currentToken;
    }

    {
        // Find single chars that can't be confused
        unordered_map<char, TokenType> singleChars = {
                {'(', LeftParen},
                {')', RightParen},
                {'[', LeftBracket},
                {']', RightBracket},
                {'{', LeftBrace},
                {'}', RightBrace},
                {';', Semicolon},
                {':', Colon},
                {',', Comma},
                {'.', Dot},
        };
        auto it = singleChars.find(c);

        if (it != singleChars.end()) {
            currentToken = Token(it->second);
            return currentToken;
        }
    }


    _ = peek();
    if (!_.has_value()) {
        currentToken = Token(Eof);
        return currentToken;
    }
    // Find 2-char tokens that could be confused
    string nextTwo = string() + c + _.value();
    {
        unordered_map<string, TokenType> doubleChars = {
                {"++", Increment},
                {"--", Decrement},
                {"+=", PlusEquals},
                {"-=", MinusEquals},
                {"*=", MultiplyEquals},
                {"/=", DivideEquals},
                {"%=", ModuloEquals},
                {"<=", LessThanEquals},
                {">=", GreaterThanEquals},
                {"==", DoubleEquals},
                {"&&", And},
                {"||", Or},
                {"!=", NotEquals}
        };
        auto it = doubleChars.find(nextTwo);
        if (it != doubleChars.end()) {
            next(); // Go one more forward
            currentToken = Token(it->second);
            return currentToken;
        }
    }
    // Find confusable single chars
    {
        unordered_map<char, TokenType> singleChars = {
                {'+', Plus},
                {'-', Minus},
                {'*', Multiply},
                {'/', Divide},
                {'%', Modulo},
                {'=', SingleEquals},
                {'<', LessThan},
                {'>', GreaterThan},
                {'&', BinaryAnd},
                {'|', BinaryOr},
                {'^', BinaryXor},
                {'!', Not},
        };
        auto it = singleChars.find(c);

        if (it != singleChars.end()) {
            currentToken = Token(it->second);
            return currentToken;
        }
    }
    // Integers
    if (isdigit(c)) {
        return parseNumber(c);
    }
    // Strings
    if (c == '"') {
        return parseString();
    }
    // Chars
    if (c == '\'') {
        return parseChar();
    }

    return parseName(c);
}

// Return the next character without incrementing
optional<char> Tokenizer::peek() {
    return peek(1);
}

// Return the Nth next character without incrementing
optional<char> Tokenizer::peek(int n) {
    if (pos + n >= program.size())
        return {};
    return program[pos + n];
}

// Return the current character and increment
optional<char> Tokenizer::next() {
    if (pos >= program.size())
        return {};
    col_number++;
    char c = program[pos++];
    current_line += c;

    if (c == '\n') {
        col_number = 0;
        line_number++;
        current_line = "";
    }
    return c;
}

// Return the current character and increment
void Tokenizer::unnext() {
    col_number--;
    pos--;
}

Token Tokenizer::parseNumber(char c) {
    optional<char> _;
    double res = 0;
    while (isdigit(c)) {
        res *= 10;
        res += c - '0';

        _ = next();
        if (!_.has_value()) {
            currentToken = Token(Number, res);
            return currentToken;
        }
        c = _.value();
    }
    if (c == '.') {
        double mult = 0.1;
        _ = next();
        if (!_.has_value()) {
            currentToken = Token(Number, res);
            return currentToken;
        }
        c = _.value();
        while (isdigit(c)) {
            res += (c - '0') * mult;
            mult *= 0.1;
            _ = next();
            if (!_.has_value()) {
                currentToken = Token(Number, res);
                return currentToken;
            }
            c = _.value();
        }
    }
    currentToken = Token(Number, res);
    return currentToken;
}

char doBackslash(char c);

Token Tokenizer::parseString() {
    string res;
    char c;
    optional<char> _;
    _ = next();
    if (!_.has_value()) {
        throw TokenizerException("Expected closing `\"`, got EOF", line_number, col_number, current_line);
    }
    c = _.value();
    bool backslash = false;
    do {
        if (backslash) {
            backslash = false;
            res += doBackslash(c);
        } else {
            if (c == '\\')
                backslash = true;
            else {
                backslash = false;
                res += c;
            }
        }
        _ = next();
        if (!_.has_value()) {
            throw TokenizerException("Expected closing `\"`, got EOF", line_number, col_number, current_line);
        }
        c = _.value();
    } while (c != '"' || backslash);
    next();
    currentToken = Token(String, res);
    return currentToken;
}

Token Tokenizer::parseName(char c) {
    optional<char> _;
    const unordered_set<char> validNameChars =
            {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'z', 'x',
             'c', 'v', 'b', 'n', 'm', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 'A', 'S', 'D', 'F', 'G', 'H',
             'J', 'K', 'L', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '_'};
    string res;
    // while it's a valid name char
    while (validNameChars.find(c) != validNameChars.end()) {
        res += c;
        _ = next();
        if (!_.has_value()) {
            currentToken = Token(Name, res);
            return currentToken;
        }
        c = _.value();
    }
    unnext();

    if (res.length() == 0) {
        throw TokenizerException(string("Unexpected character `") + c + "`", line_number, col_number, current_line);
    }

    if (res == "true")
        return Token(Boolean, true);
    else if (res == "false")
        return Token(Boolean, false);
    return Token(Name, res);
}

Token Tokenizer::parseChar() {
    optional<char> _ = next();
    if (!_.has_value()) {
        throw TokenizerException("Expected char constant, found EOF", line_number, col_number, current_line);
    }
    char c = _.value();
    if (c == '\\') {
        // Get char after backslash
        _ = next();
        if (!_.has_value()) {
            throw TokenizerException("Expected char constant, found EOF", line_number, col_number, current_line);
        }
        char nextC = _.value();
        // Ensure it ends with a '
        _ = next();
        if (!_.has_value()) {
            throw TokenizerException("Expected `'`, found EOF", line_number, col_number, current_line);
        }
        if (_.value() != '\'') {
            throw TokenizerException(string("Expected `'`, found ") + _.value(), line_number, col_number, current_line);
        }
        return Token(Char, doBackslash(nextC));
    } else{
        _ = next();
        if (!_.has_value()) {
            throw TokenizerException("Expected `'`, found EOF", line_number, col_number, current_line);
        }
        if (_.value() != '\'') {
            throw TokenizerException(string("Expected `'`, found ") + _.value(), line_number, col_number, current_line);
        }
        return Token(Char, c);
    }
}

char doBackslash(char c) {
    switch (c) {
        case 't':
            return '\t';
        case 'n':
            return '\n';
        case 'a':
            return '\a';
        case 'b':
            return '\b';
        case 'r':
            return '\r';
        case 'v':
            return '\v';
        case 'f':
            return '\f';
        case '"':
            return '"';
        case '\\':
            return '\\';
        case '\'':
            return '\'';
        default:
            return c;
    }
}