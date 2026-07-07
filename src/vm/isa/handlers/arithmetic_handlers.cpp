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

void registerArithmeticHandlers(
    std::unordered_map<Opcode, std::unique_ptr<IInstructionHandler>>& handlers) {
    handlers[Opcode::ADD] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        const Word lhs = ctx.cpu.registers().get(instruction.rs1);
        const Word rhs = readOperand(instruction, ctx);
        const Word result = lhs + rhs;
        ctx.cpu.registers().set(instruction.rd, result);
        ctx.cpu.registers().updateFlags(result);
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });

    handlers[Opcode::SUB] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        const Word lhs = ctx.cpu.registers().get(instruction.rs1);
        const Word rhs = readOperand(instruction, ctx);
        const Word result = lhs - rhs;
        ctx.cpu.registers().set(instruction.rd, result);
        ctx.cpu.registers().updateFlagsSub(lhs, rhs, result);
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });

    handlers[Opcode::MUL] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        const Word lhs = ctx.cpu.registers().get(instruction.rs1);
        const Word rhs = readOperand(instruction, ctx);
        const Word result = lhs * rhs;
        ctx.cpu.registers().set(instruction.rd, result);
        ctx.cpu.registers().updateFlags(result);
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });

    handlers[Opcode::DIV] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        const Word lhs = ctx.cpu.registers().get(instruction.rs1);
        const Word rhs = readOperand(instruction, ctx);
        if (rhs == 0) {
            setExecutionError(ctx, "division by zero");
            return ExecutionResult::Error;
        }
        const Word result = lhs / rhs;
        ctx.cpu.registers().set(instruction.rd, result);
        ctx.cpu.registers().updateFlags(result);
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });

    handlers[Opcode::MOD] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        const Word lhs = ctx.cpu.registers().get(instruction.rs1);
        const Word rhs = readOperand(instruction, ctx);
        if (rhs == 0) {
            setExecutionError(ctx, "modulo by zero");
            return ExecutionResult::Error;
        }
        const Word result = lhs % rhs;
        ctx.cpu.registers().set(instruction.rd, result);
        ctx.cpu.registers().updateFlags(result);
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });

    handlers[Opcode::INC] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        const Word value = ctx.cpu.registers().get(instruction.rd) + 1;
        ctx.cpu.registers().set(instruction.rd, value);
        ctx.cpu.registers().updateFlags(value);
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });

    handlers[Opcode::DEC] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        const Word oldValue = ctx.cpu.registers().get(instruction.rd);
        const Word value = oldValue - 1;
        ctx.cpu.registers().set(instruction.rd, value);
        ctx.cpu.registers().updateFlagsSub(oldValue, 1, value);
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });
}

}  // namespace vm
