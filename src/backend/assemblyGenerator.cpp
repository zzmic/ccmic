#include "assemblyGenerator.h"
#include <algorithm>

namespace Assembly {
AssemblyGenerator::AssemblyGenerator(
    std::shared_ptr<std::vector<std::shared_ptr<IR::StaticVariable>>>
        irStaticVariables,
    std::unordered_map<std::string,
                       std::pair<std::shared_ptr<AST::Type>,
                                 std::shared_ptr<AST::IdentifierAttribute>>>
        symbols)
    : irStaticVariables(irStaticVariables), symbols(symbols) {}

std::shared_ptr<Assembly::Program>
AssemblyGenerator::generateIR(std::shared_ptr<IR::Program> irProgram) {
    auto irTopLevels = irProgram->getTopLevels();
    auto assyTopLevels =
        std::make_shared<std::vector<std::shared_ptr<TopLevel>>>();

    // Generate assembly instructions for each IR top-level function definition.
    for (auto irTopLevel : *irTopLevels) {
        if (auto irFunctionDefinition =
                std::dynamic_pointer_cast<IR::FunctionDefinition>(irTopLevel)) {
            auto instructions =
                std::make_shared<std::vector<std::shared_ptr<Instruction>>>();
            auto assyFunctionDefinition = generateAssyFunctionDefinition(
                irFunctionDefinition, instructions);
            assyTopLevels->emplace_back(assyFunctionDefinition);
        }
        else {
            throw std::runtime_error("Unsupported top-level element");
        }
    }

    // Generate assembly instructions for each IR (either top-level or local)
    // static variable.
    for (auto irStaticVariable : *irStaticVariables) {
        auto assyStaticVariable = generateAssyStaticVariable(irStaticVariable);
        assyTopLevels->emplace_back(assyStaticVariable);
    }

    return std::make_shared<Program>(assyTopLevels);
}

std::shared_ptr<Assembly::FunctionDefinition>
AssemblyGenerator::generateAssyFunctionDefinition(
    std::shared_ptr<IR::FunctionDefinition> irFunctionDefinition,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    auto functionIdentifier = irFunctionDefinition->getFunctionIdentifier();
    auto functionGlobal = irFunctionDefinition->isGlobal();
    auto functionBody = irFunctionDefinition->getFunctionBody();

    // Generate instructions to move parameters from registers to the
    // stack.
    auto irParameters = irFunctionDefinition->getParameterIdentifiers();
    if (irParameters->size() > 0) {
        std::vector<std::string> argRegistersInStr = {"DI", "SI", "DX",
                                                      "CX", "R8", "R9"};
        int registerIndex = 0;
        for (size_t i = 0; i < irParameters->size(); ++i) {
            auto irParam = irParameters->at(i);
            auto irParamOperand =
                std::make_shared<Assembly::PseudoRegisterOperand>(irParam);
            if (i < 6) { // First six parameters from registers.
                auto registerOperand =
                    std::make_shared<Assembly::RegisterOperand>(
                        argRegistersInStr[registerIndex]);
                instructions->emplace_back(
                    std::make_shared<Assembly::MovInstruction>(registerOperand,
                                                               irParamOperand));
                registerIndex++;
            }
            else { // Remaining parameters from the stack.
                // Calculate the offset from the base pointer.
                // `(%rbp)` stores the base pointer.
                // `(%rbp + 8)` stores the return address.
                // `(%rbp + 16)` stores the first stack parameter (if any).
                // ...
                auto stackOffset = 8 * (i - 6 + 2);
                auto stackOperand = std::make_shared<Assembly::StackOperand>(
                    stackOffset, std::make_shared<Assembly::BP>());
                instructions->emplace_back(
                    std::make_shared<Assembly::MovInstruction>(stackOperand,
                                                               irParamOperand));
            }
        }
    }
    // Generate function definition that is corresponding to the IR function
    // definition (after conversion).
    auto assyFunctionDefinition = std::make_shared<FunctionDefinition>(
        functionIdentifier, functionGlobal, instructions, 0);

    // Generate assembly instructions for the function body.
    for (auto irInstruction : *functionBody) {
        generateAssyInstruction(irInstruction,
                                assyFunctionDefinition->getFunctionBody());
    }

    return assyFunctionDefinition;
}

std::shared_ptr<Assembly::StaticVariable>
AssemblyGenerator::generateAssyStaticVariable(
    std::shared_ptr<IR::StaticVariable> irStaticVariable) {
    auto identifier = irStaticVariable->getIdentifier();
    auto global = irStaticVariable->isGlobal();

    // TODO(zzmic): This is a temporary solution to convert the IR static
    // variable to the assembly static variable. `Assembly::StaticVariable`
    // needs to be updated to support more types of static initializers.
    // Currently, it only supports constant integers.
    if (auto constInt = std::dynamic_pointer_cast<AST::ConstantInt>(
            irStaticVariable->getStaticInit())) {
        return std::make_shared<StaticVariable>(identifier, global,
                                                constInt->getValue());
    }
    else if (auto constLong = std::dynamic_pointer_cast<AST::ConstantLong>(
                 irStaticVariable->getStaticInit())) {
        return std::make_shared<StaticVariable>(identifier, global,
                                                constLong->getValue());
    }
    else {
        throw std::runtime_error("Unsupported static initializer type");
    }
}

void AssemblyGenerator::generateAssyInstruction(
    std::shared_ptr<IR::Instruction> irInstruction,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    if (auto returnInstr =
            std::dynamic_pointer_cast<IR::ReturnInstruction>(irInstruction)) {
        generateAssyReturnInstruction(returnInstr, instructions);
    }
    else if (auto unaryInstr = std::dynamic_pointer_cast<IR::UnaryInstruction>(
                 irInstruction)) {
        generateAssyUnaryInstruction(unaryInstr, instructions);
    }
    else if (auto binaryInstr =
                 std::dynamic_pointer_cast<IR::BinaryInstruction>(
                     irInstruction)) {
        generateAssyBinaryInstruction(binaryInstr, instructions);
    }
    else if (auto copyInstr = std::dynamic_pointer_cast<IR::CopyInstruction>(
                 irInstruction)) {
        generateAssyCopyInstruction(copyInstr, instructions);
    }
    else if (auto jumpInstr = std::dynamic_pointer_cast<IR::JumpInstruction>(
                 irInstruction)) {
        generateAssyJumpInstruction(jumpInstr, instructions);
    }
    else if (auto jumpIfZeroInstr =
                 std::dynamic_pointer_cast<IR::JumpIfZeroInstruction>(
                     irInstruction)) {
        generateAssyJumpIfZeroInstruction(jumpIfZeroInstr, instructions);
    }
    else if (auto jumpIfNotZeroInstr =
                 std::dynamic_pointer_cast<IR::JumpIfNotZeroInstruction>(
                     irInstruction)) {
        generateAssyJumpIfNotZeroInstruction(jumpIfNotZeroInstr, instructions);
    }
    else if (auto labelInstr = std::dynamic_pointer_cast<IR::LabelInstruction>(
                 irInstruction)) {
        generateAssyLabelInstruction(labelInstr, instructions);
    }
    else if (auto functionCallInstr =
                 std::dynamic_pointer_cast<IR::FunctionCallInstruction>(
                     irInstruction)) {
        convertIRFunctionCallInstructionToAssy(functionCallInstr, instructions);
    }
}

void AssemblyGenerator::generateAssyReturnInstruction(
    std::shared_ptr<IR::ReturnInstruction> returnInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    auto returnValue = convertValue(returnInstr->getReturnValue());

    // Move the return value into the `AX` register.
    auto axReg = std::make_shared<Assembly::AX>();
    instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
        returnValue, std::make_shared<Assembly::RegisterOperand>(axReg)));

    // Generate a `Ret` instruction to return from the function.
    instructions->emplace_back(std::make_shared<Assembly::RetInstruction>());
}

void AssemblyGenerator::generateAssyUnaryInstruction(
    std::shared_ptr<IR::UnaryInstruction> unaryInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Convert the source and destination operands to assembly operands.
    auto srcOperand = convertValue(unaryInstr->getSrc());
    auto dstOperand = convertValue(unaryInstr->getDst());

    // Get the unary operator from the IR unary instruction.
    auto unaryIROperator = unaryInstr->getUnaryOperator();

    // Generate the assembly instructions based on the unary operator.
    if (auto notOperator =
            std::dynamic_pointer_cast<IR::NotOperator>(unaryIROperator)) {
        // Generate a `Cmp` instruction to compare the source operand with
        // `0`.
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            std::make_shared<Assembly::ImmediateOperand>(0),
            std::move(srcOperand)));
        // Generate a `Mov` instruction to move `0` to the destination
        // operand.
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            std::make_shared<Assembly::ImmediateOperand>(0), dstOperand));
        // Generate a `SetCC` instruction to set the destination operand to
        // `E`.
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::E>(), std::move(dstOperand)));
    }
    else {
        // Move the source operand to the destination operand.
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            std::move(srcOperand), dstOperand));
        // Generate the assembly instructions based on the unary operator.
        if (auto negateOperator = std::dynamic_pointer_cast<IR::NegateOperator>(
                unaryIROperator)) {
            // Generate a `Neg` instruction to negate the destination
            // operand.
            instructions->emplace_back(
                std::make_shared<Assembly::UnaryInstruction>(
                    std::make_shared<Assembly::NegateOperator>(),
                    std::move(dstOperand)));
        }
        else if (auto complementOperator =
                     std::dynamic_pointer_cast<IR::ComplementOperator>(
                         unaryIROperator)) {
            // Generate a `Not` instruction to complement the destination
            // operand.
            instructions->emplace_back(
                std::make_shared<Assembly::UnaryInstruction>(
                    std::make_shared<Assembly::ComplementOperator>(),
                    std::move(dstOperand)));
        }
    }
}

void AssemblyGenerator::generateAssyBinaryInstruction(
    std::shared_ptr<IR::BinaryInstruction> binaryInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Convert the source operands to assembly operands.
    auto src1Operand = convertValue(binaryInstr->getSrc1());
    auto src2Operand = convertValue(binaryInstr->getSrc2());
    auto dstOperand = convertValue(binaryInstr->getDst());

    // Get the binary operator from the IR binary instruction.
    auto binaryIROperator = binaryInstr->getBinaryOperator();
    // Generate the assembly instructions based on the IR binary operator.
    if (std::dynamic_pointer_cast<IR::AddOperator>(binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            std::move(src1Operand), dstOperand));
        instructions->emplace_back(
            std::make_shared<Assembly::BinaryInstruction>(
                std::make_shared<Assembly::AddOperator>(),
                std::move(src2Operand), std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::SubtractOperator>(
                 binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            std::move(src1Operand), dstOperand));
        instructions->emplace_back(
            std::make_shared<Assembly::BinaryInstruction>(
                std::make_shared<Assembly::SubtractOperator>(),
                std::move(src2Operand), std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::MultiplyOperator>(
                 binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            std::move(src1Operand), dstOperand));
        instructions->emplace_back(
            std::make_shared<Assembly::BinaryInstruction>(
                std::make_shared<Assembly::MultiplyOperator>(),
                std::move(src2Operand), std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::DivideOperator>(binaryIROperator)) {
        auto axReg = std::make_shared<Assembly::AX>();
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            std::move(src1Operand),
            std::make_shared<Assembly::RegisterOperand>(axReg)));
        instructions->emplace_back(
            std::make_shared<Assembly::CdqInstruction>());
        instructions->emplace_back(std::make_shared<Assembly::IdivInstruction>(
            std::move(src2Operand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            std::make_shared<Assembly::RegisterOperand>(std::move(axReg)),
            std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::RemainderOperator>(
                 binaryIROperator)) {
        auto axReg = std::make_shared<Assembly::AX>();
        auto dxReg = std::make_shared<Assembly::DX>();
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            std::move(src1Operand),
            std::make_shared<Assembly::RegisterOperand>(std::move(axReg))));
        instructions->emplace_back(
            std::make_shared<Assembly::CdqInstruction>());
        instructions->emplace_back(std::make_shared<Assembly::IdivInstruction>(
            std::move(src2Operand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            std::make_shared<Assembly::RegisterOperand>(std::move(dxReg)),
            std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::EqualOperator>(binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            std::move(src2Operand), std::move(src1Operand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            std::make_shared<Assembly::ImmediateOperand>(0), dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::E>(), std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::NotEqualOperator>(
                 binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            std::move(src2Operand), std::move(src1Operand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            std::make_shared<Assembly::ImmediateOperand>(0), dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::NE>(), std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::LessThanOperator>(
                 binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            std::move(src2Operand), std::move(src1Operand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            std::make_shared<Assembly::ImmediateOperand>(0), dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::L>(), std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::LessThanOrEqualOperator>(
                 binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            std::move(src2Operand), std::move(src1Operand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            std::make_shared<Assembly::ImmediateOperand>(0), dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::LE>(), std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::GreaterThanOperator>(
                 binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            std::move(src2Operand), std::move(src1Operand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            std::make_shared<Assembly::ImmediateOperand>(0), dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::G>(), std::move(dstOperand)));
    }
    else if (std::dynamic_pointer_cast<IR::GreaterThanOrEqualOperator>(
                 binaryIROperator)) {
        instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
            std::move(src2Operand), std::move(src1Operand)));
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            std::make_shared<Assembly::ImmediateOperand>(0), dstOperand));
        instructions->emplace_back(std::make_shared<Assembly::SetCCInstruction>(
            std::make_shared<Assembly::GE>(), std::move(dstOperand)));
    }
}

void AssemblyGenerator::generateAssyJumpInstruction(
    std::shared_ptr<IR::JumpInstruction> jumpInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Generate a `Jmp` instruction to jump to the target (label).
    instructions->emplace_back(
        std::make_shared<Assembly::JmpInstruction>(jumpInstr->getTarget()));
}

void AssemblyGenerator::generateAssyJumpIfZeroInstruction(
    std::shared_ptr<IR::JumpIfZeroInstruction> jumpIfZeroInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Generate a `Cmp` instruction to compare the condition with `0`.
    instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
        std::make_shared<Assembly::ImmediateOperand>(0),
        convertValue(jumpIfZeroInstr->getCondition())));

    // Generate a `JmpCC` instruction to conditionally jump to the target
    // (label).
    instructions->emplace_back(std::make_shared<Assembly::JmpCCInstruction>(
        std::make_shared<Assembly::E>(), jumpIfZeroInstr->getTarget()));
}

void AssemblyGenerator::generateAssyJumpIfNotZeroInstruction(
    std::shared_ptr<IR::JumpIfNotZeroInstruction> jumpIfNotZeroInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Generate a `Cmp` instruction to compare the condition with `0`.
    instructions->emplace_back(std::make_shared<Assembly::CmpInstruction>(
        std::make_shared<Assembly::ImmediateOperand>(0),
        convertValue(jumpIfNotZeroInstr->getCondition())));

    // Generate a `JmpCC` instruction to conditionally jump to the target
    // (label).
    instructions->emplace_back(std::make_shared<Assembly::JmpCCInstruction>(
        std::make_shared<Assembly::NE>(), jumpIfNotZeroInstr->getTarget()));
}

void AssemblyGenerator::generateAssyCopyInstruction(
    std::shared_ptr<IR::CopyInstruction> copyInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Convert the source and destination operands to assembly operands.
    auto srcOperand = convertValue(copyInstr->getSrc());
    auto dstOperand = convertValue(copyInstr->getDst());

    // Generate a `Mov` instruction to copy the source operand to the
    // destination operand.
    instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
        std::move(srcOperand), std::move(dstOperand)));
}

void AssemblyGenerator::generateAssyLabelInstruction(
    std::shared_ptr<IR::LabelInstruction> labelInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    // Generate a `Label` instruction with the label name.
    instructions->emplace_back(
        std::make_shared<Assembly::LabelInstruction>(labelInstr->getLabel()));
}

void AssemblyGenerator::convertIRFunctionCallInstructionToAssy(
    std::shared_ptr<IR::FunctionCallInstruction> functionCallInstr,
    std::shared_ptr<std::vector<std::shared_ptr<Assembly::Instruction>>>
        instructions) {
    std::vector<std::string> argRegistersInStr = {"DI", "SI", "DX",
                                                  "CX", "R8", "R9"};
    auto axReg = std::make_shared<Assembly::AX>();

    auto irArgs = functionCallInstr->getArgs();
    auto irRegisterArgs =
        std::make_shared<std::vector<std::shared_ptr<IR::Value>>>();
    auto irStackArgs =
        std::make_shared<std::vector<std::shared_ptr<IR::Value>>>();
    if (irArgs->size() < 6) {
        for (size_t i = 0; i < irArgs->size(); ++i) {
            irRegisterArgs->emplace_back(irArgs->at(i));
        }
    }
    else {
        for (size_t i = 0; i < irArgs->size(); ++i) {
            if (i < 6) {
                irRegisterArgs->emplace_back(irArgs->at(i));
            }
            else {
                irStackArgs->emplace_back(irArgs->at(i));
            }
        }
    }
    // Adjust the stack alignment (if the number of arguments on the stack is
    // odd).
    auto stackPadding = irStackArgs->size() % 2 != 0 ? 8 : 0;
    if (stackPadding != 0) {
        instructions->emplace_back(
            std::make_shared<Assembly::AllocateStackInstruction>(stackPadding));
    }

    // Pass the arguments in registers.
    int registerIndex = 0;
    for (auto irRegisterArg : *irRegisterArgs) {
        auto registerOperand = std::make_shared<Assembly::RegisterOperand>(
            argRegistersInStr[registerIndex]);
        auto assyRegisterArg = convertValue(irRegisterArg);
        instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
            assyRegisterArg, registerOperand));
        registerIndex++;
    }

    // Pass the arguments on the stack.
    std::reverse(
        irStackArgs->begin(),
        irStackArgs->end()); // Reverse the order of the stack arguments since
                             // they should be pushed in the reverse order.
    for (auto irStackArg : *irStackArgs) {
        auto assyStackArg = convertValue(irStackArg);
        if (std::dynamic_pointer_cast<Assembly::RegisterOperand>(
                assyStackArg) ||
            std::dynamic_pointer_cast<Assembly::ImmediateOperand>(
                assyStackArg)) {
            instructions->emplace_back(
                std::make_shared<Assembly::PushInstruction>(
                    std::move(assyStackArg)));
        }
        else {
            instructions->emplace_back(
                std::make_shared<Assembly::MovInstruction>(
                    std::move(assyStackArg),
                    std::make_shared<Assembly::RegisterOperand>(axReg)));
            instructions->emplace_back(
                std::make_shared<Assembly::PushInstruction>(
                    std::make_shared<Assembly::RegisterOperand>(axReg)));
        }
    }

    // Emit a `Call` instruction (to call the function).
    instructions->emplace_back(std::make_shared<Assembly::CallInstruction>(
        functionCallInstr->getFunctionIdentifier()));

    // Adjust the stack pointer (after the function call).
    auto bytesToPop = 8 * irStackArgs->size() + stackPadding;
    if (bytesToPop != 0) {
        instructions->emplace_back(
            std::make_shared<Assembly::DeallocateStackInstruction>(bytesToPop));
    }

    // Retrieve the return value.
    auto assyDst = convertValue(functionCallInstr->getDst());
    instructions->emplace_back(std::make_shared<Assembly::MovInstruction>(
        std::make_shared<Assembly::RegisterOperand>(std::move(axReg)),
        std::move(assyDst)));
}

std::shared_ptr<Assembly::Operand>
AssemblyGenerator::convertValue(std::shared_ptr<IR::Value> irValue) {
    if (auto constantVal =
            std::dynamic_pointer_cast<IR::ConstantValue>(irValue)) {
        if (auto constInt = std::dynamic_pointer_cast<AST::ConstantInt>(
                constantVal->getASTConstant())) {
            return std::make_shared<Assembly::ImmediateOperand>(
                constInt->getValue());
        }
        else if (auto constLong = std::dynamic_pointer_cast<AST::ConstantLong>(
                     constantVal->getASTConstant())) {
            return std::make_shared<Assembly::ImmediateOperand>(
                constLong->getValue());
        }
        else {
            throw std::runtime_error("Unsupported constant type");
        }
    }
    else if (auto varVal =
                 std::dynamic_pointer_cast<IR::VariableValue>(irValue)) {
        return std::make_shared<Assembly::PseudoRegisterOperand>(
            varVal->getIdentifier());
    }
    else {
        throw std::runtime_error("Unsupported IR value type");
    }
}
} // namespace Assembly
