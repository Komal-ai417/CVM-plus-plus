#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"
#include <iostream>
#include <fstream>
#include <sstream>

void run(const std::string& source, Compiler& compiler, VM& vm) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        auto statements = parser.parse();

        Chunk chunk = compiler.compile(statements);

        vm.execute(chunk);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        compiler = Compiler(); // Clean up corrupted REPL depths implicitly
    }
}

void repl() {
    std::cout << "CVM++ REPL v1.0\nType 'exit' or 'quit' to close.\n";
    VM vm;
    Compiler compiler;
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line.empty()) continue;
        
        run(line, compiler, vm);
    }
}

void runFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << path << std::endl;
        return;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    VM vm;
    Compiler compiler;
    run(buffer.str(), compiler, vm);
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        std::cerr << "Usage: cvm [path to script]\n";
        return 1;
    }
    return 0;
}
