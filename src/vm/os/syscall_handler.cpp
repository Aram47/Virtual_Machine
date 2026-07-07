#include "vm/os/syscall_handler.hpp"

#include "vm/process/process_control_block.hpp"

#include <iostream>
#include <sstream>

namespace vm {

SyscallHandler::SyscallHandler(std::ostream& out, std::istream& in) : out_(out), in_(in) {}

void SyscallHandler::handle(SyscallId id, ProcessControlBlock& process, Word argAddress) {
    switch (id) {
        case SyscallId::Print: {
            const Word value = process.memory().readDataWord(argAddress);
            std::lock_guard<std::mutex> lock(outMutex_);
            out_ << value << std::endl;
            break;
        }
        case SyscallId::Scan: {
            Word value = 0;
            in_ >> value;
            if (in_.fail()) {
                throw VmException("syscall scan: failed to read integer from input");
            }
            process.memory().writeDataWord(argAddress, value);
            break;
        }
        case SyscallId::Exit:
            process.setState(ProcessState::Halted);
            process.context().setHalted(true);
            break;
        case SyscallId::Yield:
            process.context().setShouldYield(true);
            break;
        case SyscallId::Load: {
            std::ostringstream pathBuilder;
            for (Address offset = 0; offset < 256; ++offset) {
                const std::byte byte = process.memory().readDataByte(argAddress + offset);
                if (byte == std::byte{0}) {
                    break;
                }
                pathBuilder << static_cast<char>(byte);
            }
            if (!loadCallback_) {
                throw VmException("syscall load: load callback is not configured");
            }
            loadCallback_(pathBuilder.str());
            break;
        }
    }
}

}  // namespace vm
