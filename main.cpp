#include <iostream>
#include <fstream>
#include <cstring>

#include "Token.h"
#include "Tokenizer.h"

using namespace std;


int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "ReqLang Interpreter" << endl
             << "  Usage: reqlang file.req <flags>" << endl
             << "  Flags:" << endl
             << "    -v : Verbose output" << endl;
        return 0;
    }

    bool verbose = argc > 2 && strcmp(argv[2], "-v") == 0;

    ifstream file;
    file.open(argv[1]);
    if (!file.is_open()) {
        cerr << "No file found (" << argv[1] << ")";
        return 1;
    }

    string program;
    string line;
    while (getline(file, line)) {
        program += line + "\n";
    }

    if (verbose) cout << "[TOK] Starting" << endl;

    Tokenizer tokenizer = Tokenizer(program);
    Token t;
    do {
        t = tokenizer.getNextToken();
        cout << t << endl;
    } while (t.type != Eof);

    return 0;
}