#include "vm/isa/handlers/register_handlers.hpp"

#include "vm/cpu/cpu_context.hpp"

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

void registerControlHandlers(
    std::unordered_map<Opcode, std::unique_ptr<IInstructionHandler>>& handlers) {
    handlers[Opcode::NOP] = makeHandler([](const DecodedInstruction&, ExecutionContext& ctx) {
        ctx.cpu.registers().advancePc();
        return ExecutionResult::Continue;
    });

    handlers[Opcode::HALT] = makeHandler([](const DecodedInstruction&, ExecutionContext& ctx) {
        ctx.cpu.setHalted(true);
        return ExecutionResult::Halt;
    });
}

}  // namespace vm
