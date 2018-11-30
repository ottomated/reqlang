//
// Created by ottomated on 27/11/18.
//

#include "Tokenizer.h"
#include "Token.h"
#include <cctype>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <optional>
#include <regex>

using namespace std;

Tokenizer::Tokenizer(string p) : program(std::move(p)), currentToken(Token(Empty, 0)), pos(0), line_number(1),
                                 col_number(0) {}

char doBackslash(char c);

bool isValidHeaderKeyChar(char c);

bool isValidHeaderValueChar(char c);

Token Tokenizer::getNextToken() {

    if (parsingRaw) {
        currentToken = getNextRawToken();
        return currentToken;
    }
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
    if (nextTwo == "[[") {
        next(1);
        parsingRaw = true;
        raw_name = "";
        currentToken = Token(RawOpener, string());
        return currentToken;
    }
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
            next(1); // Go one more forward
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
                {'[', LeftBracket},
                {']', RightBracket},
        };
        auto it = singleChars.find(c);

        if (it != singleChars.end()) {
            currentToken = Token(it->second);
            return currentToken;
        }
    }
    // Integers
    if (isdigit(c)) {
        currentToken = parseNumber(c);
        return currentToken;
    }
    // Strings
    if (c == '"') {
        currentToken = parseString('"');
        return currentToken;
    }
    // URLs
    if (c == '`') {
        currentToken = parseURL();
        return currentToken;
    }
    // Chars
    if (c == '\'') {
        currentToken = parseChar();
        return currentToken;
    }

    currentToken = parseName(c);
    return currentToken;
}

// Return the next character without incrementing
optional<char> Tokenizer::peek() {
    return peek(0);
}

// Return the Nth next character without incrementing
optional<char> Tokenizer::peek(int n) {
    if (pos + n >= program.size())
        return {};
    return program[pos + n];
}

// Return the current character and increment
void Tokenizer::next(int n) {
    for (int i = 0; i < n; i++)
        next();
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
            return Token(Number, res);
        }
        c = _.value();
    }
    if (c == '.') {
        double mult = 0.1;
        _ = next();
        if (!_.has_value()) {
            return Token(Number, res);
        }
        c = _.value();
        while (isdigit(c)) {
            res += (c - '0') * mult;
            mult *= 0.1;
            _ = next();
            if (!_.has_value()) {
                return Token(Number, res);
            }
            c = _.value();
        }
    }
    return Token(Number, res);
}

Token Tokenizer::parseString(char closing) {
    string res;
    char c;
    optional<char> _;
    _ = next();
    if (!_.has_value()) {
        throw TokenizerException(string("Expected closing ") + closing + ", got EOF", line_number, col_number,
                                 current_line);
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
            throw TokenizerException(string("Expected closing ") + closing + ", got EOF", line_number, col_number,
                                     current_line);
        }
        c = _.value();
    } while (c != closing || backslash);
    next();
    return Token(String, res);
}

Token Tokenizer::parseName(char c) {
    optional<char> _;
    string res;
    // while it's a valid name char
    while ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        res += c;
        _ = next();
        if (!_.has_value()) {
            return Token(Name, res);
        }
        c = _.value();
    }
    unnext();

    if (res.length() == 0) {
        throw TokenizerException(string("Unexpected character '") + c + "'", line_number, col_number, current_line);
    }

    if (res == "true")
        return Token(Boolean, true);
    else if (res == "false")
        return Token(Boolean, false);
    else {
        // Http Method parsing
        if (res.compare(0, 2, "M_") == 0) {
            return Token(Method, res.substr(2));
        }
        const unordered_set<string> defaultMethods =
                {"GET", "POST", "PUT", "HEAD", "OPTIONS", "DELETE", "TRACE", "CONNECT"};
        auto it = defaultMethods.find(res);
        if (it != defaultMethods.end()) {
            return Token(Method, res);
        }
    }

    _ = peek();
    if (!_.has_value()) {
        return Token(Name, res);
    }
    optional<char> _1 = peek(1);
    if (!_1.has_value()) {
        return Token(Name, res);
    }

    if (_.value() == '[' && _1.value() == '[') {
        next(2);
        parsingRaw = true;
        raw_name = res;
        return Token(RawOpener, res);
    }
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
    } else {
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

Token Tokenizer::parseURL() {
    string url = get<string>(parseString('`').value);
    regex url_regex = std::regex(R"(^(https?:\/\/)?([\da-z\.-]+)\.([a-z\.]{2,6})([\/\w \.-]*)*\/?$)");
    if (!regex_match(url, url_regex)) {
        throw TokenizerException("Malformed URL: " + url, line_number, col_number, current_line);
    } else {
        // TODO: Possibly add a Url token?
        return Token(String, url);
    }
}

Token Tokenizer::getNextRawToken() {
    char c;
    optional<char> _;
    // Get next char of program
    _ = next();
    if (!_.has_value()) {
        throw TokenizerException("Unexpected EOF while parsing Raw", line_number, col_number, current_line);
    }
    c = _.value();

    // Ignore whitespace
    while (isspace(c)) {
        _ = next();
        if (!_.has_value()) {
            throw TokenizerException("Unexpected EOF while parsing Raw", line_number, col_number, current_line);
        }
        c = _.value();
    }

    if (c == ']') {
        _ = peek();
        if (!_.has_value()) {
            throw TokenizerException("Unexpected EOF while parsing Raw", line_number, col_number, current_line);
        } else if (_.value() == ']') {
            int i;
            bool matches = true;
            for (i = 0; i < raw_name.length(); i++) {
                _ = peek(1 + i);
                if (!_.has_value()) {
                    throw TokenizerException("Unexpected EOF while parsing Raw", line_number, col_number,
                                             current_line);
                }
                if (_.value() != raw_name[i]) {
                    matches = false;
                    break;
                }
            }
            if (matches) {
                next(1 + i);
                parsingRaw = false;
                return Token(RawCloser, raw_name);
            }
        }
    }
    if (c == ':') {
        _ = peek();
        if (!_.has_value())
            throw TokenizerException("Unexpected EOF while parsing Raw", line_number, col_number, current_line);
        if (_.value() != ':') {
            return Token(Colon);
        } else {
            next();
        }
    }
    if (currentToken.type == Colon) {
        return parseHeaderValue(c);
    } else {
        return parseHeaderKey(c);
    }
}

Token Tokenizer::parseHeaderKey(char c) {
    string res = string() + c;
    optional<char> _;
    _ = next();
    if (!_.has_value()) {
        throw TokenizerException("Unexpected EOF when parsing HeaderKey in Raw", line_number, col_number, current_line);
    }
    c = _.value();
    if (c == ':') {
        unnext();
        return Token(HeaderKey, res);
    }
    bool backslash = false;
    do {
        if (backslash) {
            backslash = false;
            char slashed = doBackslash(c);
            if (!isValidHeaderKeyChar(slashed)) {
                throw TokenizerException(string("Invalid char \"\\") + c + "\" in HeaderKey", line_number, col_number,
                                         current_line);
            }
            res += slashed;
        } else {
            if (!isValidHeaderKeyChar(c)) {
                throw TokenizerException(string("Invalid char \"") + c + "\" in HeaderKey", line_number, col_number,
                                         current_line);
            }
            if (c == '\\')
                backslash = true;
            else {
                backslash = false;
                res += c;
            }
        }
        _ = next();
        if (!_.has_value()) {
            throw TokenizerException("Unexpected EOF when parsing HeaderKey in Raw", line_number, col_number,
                                     current_line);
        }
        c = _.value();
    } while (c != ':' || backslash);
    unnext();
    return Token(HeaderKey, res);
}

Token Tokenizer::parseHeaderValue(char c) {
    string res = string() + c;
    optional<char> _;
    _ = next();
    if (!_.has_value()) {
        throw TokenizerException("Unexpected EOF when parsing HeaderValue in Raw", line_number, col_number,
                                 current_line);
    }
    c = _.value();
    if (c == '\n') {
        unnext();
        return Token(HeaderValue, res);
    }
    bool backslash = false;
    do {
        if (backslash) {
            backslash = false;
            char slashed = doBackslash(c);
            if (!isValidHeaderValueChar(slashed)) {
                throw TokenizerException(string("Invalid char \"\\") + c + "\" in HeaderValue", line_number, col_number,
                                         current_line);
            }
            res += slashed;
        } else {
            if (!isValidHeaderValueChar(c)) {
                throw TokenizerException(string("Invalid char \"") + c + "\" in HeaderValue", line_number, col_number,
                                         current_line);
            }
            if (c == '\\')
                backslash = true;
            else {
                backslash = false;
                res += c;
            }
        }
        _ = next();
        if (!_.has_value()) {
            throw TokenizerException("Unexpected EOF when parsing HeaderValue in Raw", line_number, col_number,
                                     current_line);
        }
        c = _.value();
    } while (c != '\n' || backslash);
    unnext();
    return Token(HeaderValue, res);
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

bool isValidHeaderKeyChar(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '!' || c == '#' ||
           c == '$' || c == '%' || c == '&' || c == '\'' || c == '*' || c == '+' || c == '-' || c == '.' || c == '^' ||
           c == '_' || c == '`' || c == '|' || c == '~';
}

bool isValidHeaderValueChar(char c) {
    return (c >= 0x20 && c <= 0x7E);
}