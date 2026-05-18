#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"
#include "disasm.h"
#include <iostream>
#include <fstream>
#include <sstream>

// ============================================================
// CVM++ Entry Point
// Supports three modes:
//   1. REPL   — Launch without arguments for interactive session
//   2. Script — Pass a .cvm file path to compile and execute
//   3. Dump   — Use --dump flag to disassemble bytecode without running
// ============================================================

static constexpr const char* CVM_VERSION = "1.1.0";

struct RunOptions {
    bool dumpBytecode = false;  // REPL: Show disassembly alongside execution.
    bool dumpOnly = false;      // CLI: disassemble without executing.
};

void run(const std::string& source, Compiler& compiler, VM& vm, const RunOptions& opts) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        auto statements = parser.parse();

        Chunk chunk = compiler.compile(statements);

        if (opts.dumpBytecode || opts.dumpOnly) {
            disassembleChunk(chunk, "Compiled Bytecode");
        }

        if (!opts.dumpOnly) {
            vm.execute(chunk);
        }
    } catch (const LexError& e) {
        std::cerr << "Error: " << e.what() << '\n';
    } catch (const ParseError& e) {
        std::cerr << "Error: " << e.what() << '\n';
    } catch (const CompileError& e) {
        std::cerr << "Error: " << e.what() << '\n';
    } catch (const RuntimeError& e) {
        std::cerr << "Error: " << e.what() << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        compiler.reset();
        vm.reset();
    }
}

void repl() {
    std::cout << "CVM++ REPL v" << CVM_VERSION << "\nType 'exit' or 'quit' to close.\n";
    VM vm;
    Compiler compiler;
    RunOptions opts;
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line == "disasm on") {
            opts.dumpBytecode = true;
            std::cout << "Bytecode disassembly enabled.\n";
            continue;
        }
        if (line == "disasm off") {
            opts.dumpBytecode = false;
            std::cout << "Bytecode disassembly disabled.\n";
            continue;
        }
        if (line.empty()) continue;
        
        run(line, compiler, vm, opts);
    }
}

void runFile(const std::string& path, const RunOptions& opts) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << path << '\n';
        return;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    VM vm;
    Compiler compiler;
    run(buffer.str(), compiler, vm, opts);
}

void printUsage() {
    std::cout << "Usage: cvm [options] [script.cvm]\n"
              << "\nOptions:\n"
              << "  --dump       Disassemble bytecode without executing\n"
              << "  --help       Show this help message\n"
              << "  --version    Show version information\n"
              << "\nExamples:\n"
              << "  cvm                    Start the REPL\n"
              << "  cvm script.cvm         Run a script file\n"
              << "  cvm --dump script.cvm  Disassemble a script\n";
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        repl();
    } else {
        RunOptions opts;
        std::string path;
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--dump") {
                opts.dumpOnly = true;
            } else if (arg == "--help" || arg == "-h") {
                printUsage();
                return 0;
            } else if (arg == "--version" || arg == "-v") {
                std::cout << "CVM++ v" << CVM_VERSION << '\n';
                return 0;
            } else {
                path = arg;
            }
        }
        if (path.empty()) {
            std::cerr << "Error: No script file specified.\n";
            printUsage();
            return 1;
        }
        runFile(path, opts);
    }
    return 0;
}
