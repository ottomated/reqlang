//
// Created by ottomated on 27/11/18.
//

#include "Token.h"
#include <iostream>

using namespace std;

template<typename T0, typename ... Ts>
std::ostream &operator<<(std::ostream &s,
                         std::variant<T0, Ts...> const &v) {
    std::visit([&](auto &&arg) { s << arg; }, v);
    return s;
}


std::ostream &operator<<(std::ostream &stream, const TokenType &token) {
    string tokenString = TokenNames[token];

    return stream << tokenString;
}

std::ostream &operator<<(std::ostream &stream, const Token &token) {
    if (token.type == Number || token.type == String || token.type == Boolean ||
        token.type == Char || token.type == Name || token.type == Method || token.type == HeaderKey ||
        token.type == HeaderValue)
        return stream << "Token " << token.type << "=" << token.value;
    else if (token.type == HeaderOpener)
        return stream << "Token " << token.value << token.type;
    else if (token.type == HeaderCloser)
        return stream << "Token " << token.type << token.value;
    else
        return stream << "Token " << token.type;

}
