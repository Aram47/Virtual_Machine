#include "vm/isa/handlers/register_handlers.hpp"

#include "vm/cpu/cpu_context.hpp"
#include "vm/isa/instruction_helpers.hpp"
#include "vm/memory/process_memory.hpp"

namespace vm {

namespace {

template <typename Fn>
class FunctorHandler : public IInstructionHandler {
public:
    explicit FunctorHandler(Fn fn) : fn_(std::move(fn)) {}

    ExecutionResult execute(const DecodedInstruction& instruction,
                            ExecutionContext& ctx) override {
        return fn_(instruction, ctx);
    }

private:
    Fn fn_;
};

template <typename Fn>
std::unique_ptr<IInstructionHandler> makeHandler(Fn fn) {
    return std::make_unique<FunctorHandler<Fn>>(std::move(fn));
}

}  // namespace

void registerMemoryHandlers(
    std::unordered_map<Opcode, std::unique_ptr<IInstructionHandler>>& handlers) {
    handlers[Opcode::MOV] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        const Word value = readOperand(instruction, ctx);
        ctx.cpu.registers().set(instruction.rd, value);
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });

    handlers[Opcode::LOAD] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        const Address offset = static_cast<Address>(instruction.imm);
        const Word value = ctx.memory.readDataWord(offset);
        ctx.cpu.registers().set(instruction.rd, value);
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });

    handlers[Opcode::STORE] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        const Address offset = static_cast<Address>(instruction.imm);
        const Word value = ctx.cpu.registers().get(instruction.rd);
        ctx.memory.writeDataWord(offset, value);
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });
}

}  // namespace vm
