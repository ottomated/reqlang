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

Tokenizer::Tokenizer(string p) : program(std::move(p)), currentToken(Token(Empty, 0)), pos(0) {}

Token Tokenizer::getNextToken() {
    char c = next();
    while (isspace(c)) {
        c = next();
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


    // Find 2-char tokens that could be confused
    string nextTwo = string() + c + peek();
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

    return parseName(c);
}

// Return the next character without incrementing
char Tokenizer::peek() {
    //cout << "Peek: " << pos + 1 << " " << program[pos + 1] << endl;
    return program[pos + 1];
}

// Return the current character and increment
char Tokenizer::next() {
    //cout << "Read: " << pos+1 << " " << program[pos] << endl;
    return program[pos++];
}

Token Tokenizer::parseNumber(char c) {
    double res = 0;
    while (isdigit(c)) {
        res *= 10;
        res += c - '0';
        c = next();
    }
    if (c == '.') {
        double mult = 0.1;
        c = next();
        while (isdigit(c)) {
            res += (c - '0') * mult;
            mult *= 0.1;
            c = next();
        }
    }
    currentToken = Token(Number, res);
    return currentToken;
}

char doBackslash(char c);

Token Tokenizer::parseString() {
    string res;
    char c = next();
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
        c = next();
    } while (c != '"' || backslash);
    next();
    currentToken = Token(String, res);
    return currentToken;
}

Token Tokenizer::parseName(char c) {
    const unordered_set<char> validNameChars =
            {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'z', 'x',
             'c', 'v', 'b', 'n', 'm', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 'A', 'S', 'D', 'F', 'G', 'H',
             'J', 'K', 'L', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '_'};
    string res;
    // while it's a valid name char
    while (validNameChars.find(c) != validNameChars.end()) {
        res += c;
        c = peek();
    }
    if (res.length() == 0)
        return Token();

    if (res == "true")
        return Token(Boolean, true);
    else if (res == "false")
        return Token(Boolean, false);
    return Token(Name, res);
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
        default:
            return c;
    }
}