#include "vm/isa/handlers/register_handlers.hpp"

#include "vm/cpu/cpu_context.hpp"
#include "vm/isa/instruction_helpers.hpp"
#include "vm/memory/process_memory.hpp"

namespace vm {

namespace {

class ConditionalJumpHandler : public IInstructionHandler {
public:
    ExecutionResult execute(const DecodedInstruction& instruction,
                            ExecutionContext& ctx) override {
        if (takeJump(ctx.cpu.registers().flags(), instruction.opcode)) {
            ctx.cpu.registers().setPc(static_cast<Address>(instruction.imm));
            return ExecutionResult::Jump;
        }
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    }
};

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

void registerJumpHandlers(
    std::unordered_map<Opcode, std::unique_ptr<IInstructionHandler>>& handlers) {
    handlers[Opcode::JMP] = makeHandler([](const DecodedInstruction& instruction, ExecutionContext& ctx) {
        ctx.cpu.registers().setPc(static_cast<Address>(instruction.imm));
        return ExecutionResult::Jump;
    });

    handlers[Opcode::JE] = std::make_unique<ConditionalJumpHandler>();
    handlers[Opcode::JNE] = std::make_unique<ConditionalJumpHandler>();
    handlers[Opcode::JG] = std::make_unique<ConditionalJumpHandler>();
    handlers[Opcode::JGE] = std::make_unique<ConditionalJumpHandler>();
    handlers[Opcode::JL] = std::make_unique<ConditionalJumpHandler>();
    handlers[Opcode::JLE] = std::make_unique<ConditionalJumpHandler>();
}

}  // namespace vm
