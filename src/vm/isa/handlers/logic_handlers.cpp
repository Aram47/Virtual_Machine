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

void registerLogicHandlers(
    std::unordered_map<Opcode, std::unique_ptr<IInstructionHandler>>& handlers) {
    handlers[Opcode::AND] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        const Word lhs = ctx.cpu.registers().get(instruction.rs1);
        const Word rhs = readOperand(instruction, ctx);
        const Word result = lhs & rhs;
        ctx.cpu.registers().set(instruction.rd, result);
        ctx.cpu.registers().updateFlags(result);
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });

    handlers[Opcode::OR] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        const Word lhs = ctx.cpu.registers().get(instruction.rs1);
        const Word rhs = readOperand(instruction, ctx);
        const Word result = lhs | rhs;
        ctx.cpu.registers().set(instruction.rd, result);
        ctx.cpu.registers().updateFlags(result);
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });

    handlers[Opcode::XOR] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        const Word lhs = ctx.cpu.registers().get(instruction.rs1);
        const Word rhs = readOperand(instruction, ctx);
        const Word result = lhs ^ rhs;
        ctx.cpu.registers().set(instruction.rd, result);
        ctx.cpu.registers().updateFlags(result);
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });

    handlers[Opcode::NOT] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        const Word value = ~ctx.cpu.registers().get(instruction.rd);
        ctx.cpu.registers().set(instruction.rd, value);
        ctx.cpu.registers().updateFlags(value);
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });

    handlers[Opcode::CMP] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        const Word lhs = ctx.cpu.registers().get(instruction.rs1);
        const Word rhs = readOperand(instruction, ctx);
        const Word result = lhs - rhs;
        ctx.cpu.registers().updateFlagsSub(lhs, rhs, result);
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });
}

}  // namespace vm
