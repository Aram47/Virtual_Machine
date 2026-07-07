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

void registerStackHandlers(
    std::unordered_map<Opcode, std::unique_ptr<IInstructionHandler>>& handlers) {
    handlers[Opcode::PUSH] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        const Word value = readOperand(instruction, ctx);
        Address& sp = ctx.cpu.registers().sp();
        try {
            ctx.memory.pushWord(value, sp);
        } catch (const VmException& ex) {
            setExecutionError(ctx, ex.what());
            return ExecutionResult::Error;
        }
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });

    handlers[Opcode::POP] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        Address& sp = ctx.cpu.registers().sp();
        try {
            const Word value = ctx.memory.popWord(sp);
            ctx.cpu.registers().set(instruction.rd, value);
        } catch (const VmException& ex) {
            setExecutionError(ctx, ex.what());
            return ExecutionResult::Error;
        }
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });

    handlers[Opcode::RET] = makeHandler([](const DecodedInstruction&, ExecutionContext& ctx) {
        Address& sp = ctx.cpu.registers().sp();
        try {
            const Word returnPc = ctx.memory.popWord(sp);
            ctx.cpu.registers().setPc(static_cast<Address>(returnPc));
        } catch (const VmException& ex) {
            setExecutionError(ctx, ex.what());
            return ExecutionResult::Error;
        }
        return ExecutionResult::Jump;
    });
}

}  // namespace vm
