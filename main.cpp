#include "vm/virtual_machine.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void printUsage(const char* program) {
    std::cerr << "Usage: " << program << " [options] <program.asm>...\n"
              << "Options:\n"
              << "  --cores <n>     Number of virtual CPU cores (default: 4)\n"
              << "  --quantum <n>   Instruction time slice (default: 100)\n"
              << "  --trace         Print instruction trace to stderr\n";
}

std::size_t parsePositiveOption(const char* name, const char* value) {
    try {
        const unsigned long parsed = std::stoul(value);
        if (parsed == 0) {
            throw std::invalid_argument("value must be positive");
        }
        return static_cast<std::size_t>(parsed);
    } catch (const std::exception&) {
        throw std::invalid_argument(std::string("invalid value for ") + name + ": " + value);
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    std::size_t cores = vm::kDefaultCoreCount;
    std::size_t quantum = vm::kDefaultTimeSlice;
    bool trace = false;
    std::vector<std::string> programs;

    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--cores" && i + 1 < argc) {
                cores = parsePositiveOption("--cores", argv[++i]);
            } else if (arg == "--quantum" && i + 1 < argc) {
                quantum = parsePositiveOption("--quantum", argv[++i]);
            } else if (arg == "--trace") {
                trace = true;
            } else if (arg == "--help") {
                printUsage(argv[0]);
                return 0;
            } else {
                programs.push_back(arg);
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        printUsage(argv[0]);
        return 1;
    }

    if (programs.empty()) {
        printUsage(argv[0]);
        return 1;
    }

    try {
        vm::VirtualMachine machine(cores, quantum);
        if (trace) {
            machine.enableTrace(true);
        }
        for (const auto& program : programs) {
            machine.loadProgram(program);
        }
        machine.run();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
